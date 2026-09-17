#include "force_sensor.h"
#include "track_format.h"
#include "platform_lock.h"
#include <math.h>
#include <string.h>
#define FORCE_IIR_ALPHA 0.2f
static ForceCalibration calibration;
static ForceSnapshot snapshot;
static ContactDetector detector;
static int32_t history[3];
static unsigned history_count, history_index;
static float filtered;
static bool have_sample;
ArmResult ContactDetector_Configure(ContactDetector *d, const ContactConfig *c)
{
    if (!d || !c || !c->configured || !isfinite(c->on_n) || !isfinite(c->off_n) || c->on_n <= c->off_n ||
        !c->stable_samples)
        return ARM_NOT_CONFIGURED;
    *d = (ContactDetector){.config = *c};
    return ARM_OK;
}
bool ContactDetector_Update(ContactDetector *d, float n)
{
    if (!d || !d->config.configured || !isfinite(n))
        return false;
    bool change = d->contact ? n <= d->config.off_n : n >= d->config.on_n;
    if (change)
    {
        if (++d->count >= d->config.stable_samples)
        {
            d->contact = !d->contact;
            d->count = 0;
        }
    }
    else
        d->count = 0;
    return d->contact;
}
ArmResult ForceCondition_Evaluate(const ForceCondition *c, ForceConditionState *s, const ForceSnapshot *f,
                                  uint32_t now)
{
    if (!c || !s || !f)
        return ARM_INVALID_ARGUMENT;
    if (c->type == FORCE_CONDITION_NONE)
        return ARM_FINISHED;
    if (!c->configured || c->type == FORCE_CONDITION_RIPPLE_BELOW || c->type == FORCE_CONDITION_RESERVED ||
        !isfinite(c->threshold_n) || !isfinite(c->hysteresis_n) || c->hysteresis_n < 0 || !c->timeout_ms)
        return ARM_NOT_CONFIGURED;
    if (!f->valid || !f->calibrated || f->stale || f->overload)
        return ARM_NOT_READY;
    if ((uint32_t)(now - s->started_ms) >= c->timeout_ms)
        return ARM_TIMEOUT;
    bool met = false;
    if (c->type == FORCE_CONDITION_CONTACT)
        met = f->force_n >= c->threshold_n - (s->candidate ? c->hysteresis_n : 0);
    else if (c->type == FORCE_CONDITION_RELEASE)
        met = f->force_n <= c->threshold_n + (s->candidate ? c->hysteresis_n : 0);
    else if (c->type == FORCE_CONDITION_STABLE)
        met = fabsf(f->force_n - c->threshold_n) <= c->hysteresis_n;
    else
        return ARM_NOT_CONFIGURED;
    if (!met)
    {
        s->candidate = false;
        return ARM_NOT_READY;
    }
    if (!s->candidate)
    {
        s->candidate = true;
        s->since_ms = now;
    }
    return (uint32_t)(now - s->since_ms) >= c->stable_ms ? ARM_FINISHED : ARM_NOT_READY;
}
void ForceSensor_Init(void)
{
    Platform_EnterCritical();
    calibration = (ForceCalibration){0};
    snapshot = (ForceSnapshot){0};
    detector = (ContactDetector){0};
    history_count = history_index = 0;
    have_sample = false;
    filtered = 0;
    Platform_ExitCritical();
}
static bool Force_IsCalibrationValid(const ForceCalibration *c)
{
    return c && c->valid && isfinite(c->newton_per_count) && c->newton_per_count != 0 &&
           isfinite(c->full_scale_n) && c->full_scale_n > 0 && isfinite(c->overload_n) && c->overload_n > 0 &&
           c->overload_n <= c->full_scale_n && (c->sample_rate_hz == 10 || c->sample_rate_hz == 80) &&
           c->zero_raw >= -8388608 && c->zero_raw <= 8388607;
}
ArmResult ForceSensor_SetCalibration(const ForceCalibration *c)
{
    if (!Force_IsCalibrationValid(c))
        return ARM_NOT_CONFIGURED;
    Platform_EnterCritical();
    calibration = *c;
    history_count = history_index = 0;
    have_sample = false;
    snapshot.valid = false;
    detector.count = 0;
    detector.contact = false;
    Platform_ExitCritical();
    return ARM_OK;
}
void ForceSensor_GetCalibration(ForceCalibration *c)
{
    if (c)
    {
        Platform_EnterCritical();
        *c = calibration;
        Platform_ExitCritical();
    }
}
ArmResult ForceSensor_ConfigureContact(const ContactConfig *c)
{
    Platform_EnterCritical();
    ArmResult r = ContactDetector_Configure(&detector, c);
    Platform_ExitCritical();
    return r;
}
void ForceSensor_PushRaw(int32_t raw, uint32_t now)
{
    Platform_EnterCritical();
    if (raw < -8388608 || raw > 8388607)
    {
        snapshot.valid = false;
        have_sample = false;
        Platform_ExitCritical();
        return;
    }
    history[history_index] = raw;
    history_index = (history_index + 1) % 3;
    if (history_count < 3)
        ++history_count;
    int32_t v[3] = {history[0], history[1], history[2]};
    for (unsigned i = 0; i < history_count; ++i)
        for (unsigned j = i + 1; j < history_count; ++j)
            if (v[j] < v[i])
            {
                int32_t t = v[i];
                v[i] = v[j];
                v[j] = t;
            }
    float median = (float)v[history_count / 2];
    filtered = have_sample ? filtered + FORCE_IIR_ALPHA * (median - filtered) : median;
    have_sample = true;
    snapshot = (ForceSnapshot){
        .raw = raw, .timestamp_ms = now, .calibrated = Force_IsCalibrationValid(&calibration)};
    snapshot.force_n =
        snapshot.calibrated ? (filtered - (float)calibration.zero_raw) * calibration.newton_per_count : 0;
    /* Protection also observes the unfiltered sample: filtering must not hide an overload. */
    float instantaneous_n = ((float)raw - (float)calibration.zero_raw) * calibration.newton_per_count;
    snapshot.overload = raw == -8388608 || raw == 8388607 ||
                        (snapshot.calibrated &&
                         (fabsf(snapshot.force_n) >= calibration.overload_n || !isfinite(instantaneous_n) ||
                          fabsf(instantaneous_n) >= calibration.overload_n));
    snapshot.valid = snapshot.calibrated && isfinite(snapshot.force_n) && !snapshot.overload;
    snapshot.contact = snapshot.valid ? ContactDetector_Update(&detector, snapshot.force_n) : false;
    if (!snapshot.valid)
    {
        detector.count = 0;
        detector.contact = false;
    }
    Platform_ExitCritical();
}
void ForceSensor_GetSnapshot(uint32_t now, ForceSnapshot *s)
{
    if (!s)
        return;
    Platform_EnterCritical();
    *s = snapshot;
    s->stale = !have_sample || (uint32_t)(now - s->timestamp_ms) > FORCE_STALE_TIMEOUT_MS;
    if (s->stale)
    {
        s->valid = false;
        s->contact = false;
        detector.count = 0;
        detector.contact = false;
    }
    Platform_ExitCritical();
}
ArmResult ForceSensor_Tare(uint32_t now)
{
    ForceSnapshot s;
    ForceSensor_GetSnapshot(now, &s);
    if (s.stale)
        return ARM_NOT_READY;
    Platform_EnterCritical();
    calibration.zero_raw = s.raw;
    calibration.valid = false;
    snapshot.valid = false;
    snapshot.calibrated = false;
    Platform_ExitCritical();
    return ARM_OK;
}
ArmResult ForceSensor_Calibrate(float n, uint32_t now)
{
    ForceSnapshot s;
    ForceSensor_GetSnapshot(now, &s);
    if (s.stale || !isfinite(n) || n <= 0)
        return ARM_INVALID_ARGUMENT;
    Platform_EnterCritical();
    int32_t delta = s.raw - calibration.zero_raw;
    if (!delta)
    {
        Platform_ExitCritical();
        return ARM_INVALID_ARGUMENT;
    }
    calibration.newton_per_count = n / (float)delta;
    calibration.valid = true;
    bool valid = Force_IsCalibrationValid(&calibration);
    calibration.valid = valid;
    snapshot.valid = false;
    Platform_ExitCritical();
    return valid ? ARM_OK : ARM_NOT_CONFIGURED;
}
static void Force_Put32(uint8_t *b, uint32_t v)
{
    for (unsigned i = 0; i < 4; ++i)
        b[i] = (uint8_t)(v >> (8 * i));
}
static uint32_t Force_Get32(const uint8_t *b)
{
    return (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
}
void ForceCalibration_Encode(const ForceCalibration *c, uint8_t b[FORCE_CAL_BYTES])
{
    memset(b, 0, FORCE_CAL_BYTES);
    memcpy(b, "A6FC", 4);
    b[4] = 1;
    b[6] = FORCE_CAL_BYTES;
    Force_Put32(b + 8, (uint32_t)c->zero_raw);
    float f[3] = {c->newton_per_count, c->full_scale_n, c->overload_n};
    for (unsigned i = 0; i < 3; ++i)
    {
        uint32_t v;
        memcpy(&v, &f[i], 4);
        Force_Put32(b + 12 + i * 4, v);
    }
    Force_Put32(b + 24, c->sample_rate_hz);
    b[28] = c->valid;
    Force_Put32(b + 36, TrackCrc_Compute(b, 36));
}
ArmResult ForceCalibration_Decode(const uint8_t b[FORCE_CAL_BYTES], ForceCalibration *c)
{
    if (!b || !c || memcmp(b, "A6FC", 4) || b[4] != 1 || b[5] || b[6] != FORCE_CAL_BYTES || b[7] ||
        Force_Get32(b + 36) != TrackCrc_Compute(b, 36) || b[28] != 1 || b[29] || b[30] || b[31] ||
        Force_Get32(b + 32))
        return ARM_INVALID_ARGUMENT;
    *c = (ForceCalibration){
        .zero_raw = (int32_t)Force_Get32(b + 8), .sample_rate_hz = Force_Get32(b + 24), .valid = true};
    float *v[3] = {&c->newton_per_count, &c->full_scale_n, &c->overload_n};
    for (unsigned i = 0; i < 3; ++i)
    {
        uint32_t x = Force_Get32(b + 12 + i * 4);
        memcpy(v[i], &x, 4);
    }
    return Force_IsCalibrationValid(c) ? ARM_OK : ARM_NOT_CONFIGURED;
}
