#include "debug_cli.h"
#include "app_snapshot.h"
#include "force_sensor.h"
#include "storage_service.h"
#include "motor_manager.h"
#include "bsp_can.h"
#include "text_format.h"

#include <string.h>
#include <math.h>
static uint32_t pending_ticket;
static bool Debug_ParsePositive(const char *s, float *v)
{
    float n = 0, scale = 1;
    bool digit = false, dot = false;
    unsigned decimals = 0;
    for (; *s; ++s)
    {
        if (*s == '.' && !dot)
        {
            dot = true;
            continue;
        }
        if (*s < '0' || *s > '9')
            return false;
        digit = true;
        if (dot)
        {
            if (++decimals > 6)
                return false;
            scale *= 0.1f;
            n += (*s - '0') * scale;
        }
        else
            n = n * 10 + (*s - '0');
        if (n > 1000000)
            return false;
    }
    *v = n;
    return digit && n > 0;
}

static void Debug_Submit(StorageRequest *r, char *out, size_t n)
{
    if (pending_ticket)
    {
        (void)Text_Format(out, n, "BUSY\r\n");
        return;
    }
    TrackResult t = StorageService_Request(r, &pending_ticket);
    (void)Text_Format(out, n, "storage queued result=%u\r\n", (unsigned)t);
}
void DebugCli_Execute(const char *line, uint32_t now, char *out, size_t n)
{
    if (!line || !out || !n)
        return;
    AppSnapshot app;
    App_GetSnapshot(&app);
    if (!strcmp(line, "help"))
    {
        (void)Text_Format(out, n,
                          "help status motors can fault tf tracks track info force [raw|tare|cal "
                          "<N>|save]\r\nMotion commands disabled.\r\n");
        return;
    }
    if (!strcmp(line, "status") || !strcmp(line, "fault"))
    {
        (void)Text_Format(out, n, "startup=%u robot=%u operation=%u result=%u track=%u\r\n", app.startup,
                          app.robot, app.operation.state, app.operation.result, app.operation.track_result);
        return;
    }
    if (!strcmp(line, "motors"))
    {
        MotorSnapshot s;
        MotorManager_GetSnapshot(now, &s);
        unsigned online = 0, enabled = 0;
        uint32_t faults = 0;
        for (unsigned i = 0; i < AXIS_COUNT; ++i)
        {
            online |= (unsigned)s.axes[i].online << i;
            enabled |= (unsigned)s.axes[i].enabled << i;
            faults |= s.axes[i].fault;
        }
        (void)Text_Format(out, n, "configured=%u coordinates=%u online=0x%02x enabled=0x%02x fault=0x%lx\r\n",
                          s.configured, s.coordinates_ready, online, enabled, (unsigned long)faults);
        return;
    }
    if (!strcmp(line, "can"))
    {
        BspCanDiagnostics d;
        BspCan_GetDiagnostics(&d);
        (void)Text_Format(out, n, "CAN errors=0x%lx RX overflow=%lu armed=%u\r\n",
                          (unsigned long)d.hal_errors, (unsigned long)d.rx_overflow, MotorManager_IsArmed());
        return;
    }
    if (!strcmp(line, "tf") || !strcmp(line, "tracks"))
    {
        StorageSnapshot s;
        StorageService_GetSnapshot(&s);
        (void)Text_Format(out, n, "TF mounted=%u customs=%u factories=%u%u%u last=%u %s\r\n", s.mounted,
                          s.count, s.factory_available[0], s.factory_available[1], s.factory_available[2],
                          s.last_error, s.list_full ? "CUSTOM_TRACK_LIST_FULL" : "");
        return;
    }
    if (!strcmp(line, "track info"))
    {
        TrackHeader h = TrackBuffer_GetHeader();
        (void)Text_Format(out, n, "owner=%u samples=%lu period_us=%lu model=%08lx\r\n",
                          TrackBuffer_GetOwner(), (unsigned long)h.count, (unsigned long)h.period_us,
                          (unsigned long)h.model_id);
        return;
    }
    if (!strcmp(line, "force") || !strcmp(line, "force raw"))
    {
        ForceSnapshot f;
        ForceSensor_GetSnapshot(now, &f);
        (void)Text_Format(
            out, n, "raw=%ld force_mN=%ld calibrated=%u valid=%u stale=%u overload=%u\r\n", (long)f.raw,
            (long)((isfinite(f.force_n) && fabsf(f.force_n) < 2147483.0f) ? f.force_n * 1000 : 0),
            f.calibrated, f.valid, f.stale, f.overload);
        return;
    }
    bool idle =
        app.startup == STARTUP_READY && app.robot == ROBOT_READY &&
        (app.operation.state == OP_IDLE || app.operation.state == OP_DONE || app.operation.state == OP_ERROR);
    if (!strcmp(line, "force tare") || !strncmp(line, "force cal ", 10) || !strcmp(line, "force save"))
    {
        if (!idle)
        {
            (void)Text_Format(out, n, "NOT READY: calibration requires idle hold\r\n");
            return;
        }
        if (!strcmp(line, "force save"))
        {
            StorageRequest r = {.type = STORAGE_SAVE_FORCE_CAL};
            ForceSensor_GetCalibration(&r.calibration);
            Debug_Submit(&r, out, n);
            return;
        }
        ArmResult r = ARM_INVALID_ARGUMENT;
        if (!strcmp(line, "force tare"))
            r = ForceSensor_Tare(now);
        else
        {
            float known;
            if (Debug_ParsePositive(line + 10, &known))
                r = ForceSensor_Calibrate(known, now);
        }
        ForceCalibration c;
        ForceSensor_GetCalibration(&c);
        (void)Text_Format(out, n, "cal result=%u valid=%u (range/overload/rate must be commissioned)\r\n", r,
                          c.valid);
        return;
    }
    (void)Text_Format(out, n, "UNKNOWN OR DISABLED; use help\r\n");
}
bool DebugCli_Poll(char *out, size_t n)
{
    if (!pending_ticket)
        return false;
    StorageResponse r;
    if (StorageService_TakeResult(pending_ticket, &r) != TRACK_OK)
        return false;
    pending_ticket = 0;
    (void)Text_Format(out, n, "storage complete=%u\r\n", r.result);
    return true;
}
