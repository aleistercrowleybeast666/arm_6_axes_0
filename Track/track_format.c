#include "track_format.h"
#include "robot_geometry.h"
#include <math.h>
#include <string.h>
static void Format_Put32(uint8_t *b, uint32_t v)
{
    for (unsigned i = 0; i < 4; ++i)
        b[i] = (uint8_t)(v >> (8 * i));
}
static uint32_t Format_Get32(const uint8_t *b)
{
    return (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
}
uint32_t TrackCrc_Update(uint32_t s, const uint8_t *b, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        s ^= b[i];
        for (unsigned j = 0; j < 8; ++j)
            s = (s >> 1) ^ ((s & 1) ? 0xEDB88320U : 0);
    }
    return s;
}
uint32_t TrackCrc_Compute(const uint8_t *b, size_t n)
{
    return TrackCrc_Update(UINT32_MAX, b, n) ^ UINT32_MAX;
}
uint32_t TrackFormat_GetModelId(void)
{
    /* Canonical LE int32: joint count, each parent translation in um and axis in 1e6, tool um. */
    uint32_t crc = UINT32_MAX;
    uint8_t b[4];
    Format_Put32(b, AXIS_COUNT);
    crc = TrackCrc_Update(crc, b, 4);
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        const RobotJointGeometry *g = RobotGeometry_GetJoint((RobotAxis)i);
        float v[6] = {g->parent_translation_mm.x * 1000, g->parent_translation_mm.y * 1000,
                      g->parent_translation_mm.z * 1000, g->local_axis.x * 1000000,
                      g->local_axis.y * 1000000,         g->local_axis.z * 1000000};
        for (unsigned j = 0; j < 6; ++j)
        {
            Format_Put32(b, (uint32_t)(int32_t)lroundf(v[j]));
            crc = TrackCrc_Update(crc, b, 4);
        }
    }
    Format_Put32(b, (uint32_t)lroundf(RobotGeometry_GetToolLengthMm() * 1000));
    return TrackCrc_Update(crc, b, 4) ^ UINT32_MAX;
}
void TrackFormat_EncodeHeader(const TrackHeader *h, uint8_t b[TRACK_HEADER_BYTES])
{
    memset(b, 0, TRACK_HEADER_BYTES);
    memcpy(b, "A6TR", 4);
    b[4] = 1;
    b[6] = TRACK_HEADER_BYTES;
    b[8] = AXIS_COUNT;
    b[9] = 1;
    Format_Put32(b + 12, h->period_us);
    Format_Put32(b + 16, h->count);
    Format_Put32(b + 20, h->model_id);
    Format_Put32(b + 24, h->count * TRACK_SAMPLE_BYTES);
    Format_Put32(b + 28, h->payload_crc);
    Format_Put32(b + 36, TrackCrc_Compute(b, 36));
}
TrackResult TrackFormat_DecodeHeader(const uint8_t b[TRACK_HEADER_BYTES], TrackHeader *h)
{
    if (!b || !h || memcmp(b, "A6TR", 4))
        return TRACK_BAD_FORMAT;
    if (b[4] != 1 || b[5])
        return TRACK_UNSUPPORTED_VERSION;
    if (Format_Get32(b + 36) != TrackCrc_Compute(b, 36))
        return TRACK_BAD_CRC;
    if (b[6] != TRACK_HEADER_BYTES || b[7] || b[8] != AXIS_COUNT || b[9] != 1 || b[10] || b[11] ||
        Format_Get32(b + 32))
        return TRACK_BAD_FORMAT;
    *h =
        (TrackHeader){Format_Get32(b + 12), Format_Get32(b + 16), Format_Get32(b + 20), Format_Get32(b + 28)};
    if (h->count < 2 || h->count > TRACK_MAX_SAMPLES)
        return TRACK_BUFFER_FULL;
    if ((h->period_us != 10000 && h->period_us != 20000 && h->period_us != 40000) ||
        Format_Get32(b + 24) != h->count * TRACK_SAMPLE_BYTES)
        return TRACK_BAD_FORMAT;
    return h->model_id == TrackFormat_GetModelId() ? TRACK_OK : TRACK_WRONG_MODEL;
}
void TrackFormat_EncodeSample(const TrackSample *s, uint8_t b[TRACK_SAMPLE_BYTES])
{
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        uint16_t v = (uint16_t)s->q_cdeg[i];
        b[2 * i] = (uint8_t)v;
        b[2 * i + 1] = (uint8_t)(v >> 8);
    }
}
void TrackFormat_DecodeSample(const uint8_t b[TRACK_SAMPLE_BYTES], TrackSample *s)
{
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        uint32_t v = (uint32_t)b[2 * i] | (uint32_t)b[2 * i + 1] << 8;
        s->q_cdeg[i] = (int16_t)(v < 32768 ? (int32_t)v : (int32_t)v - 65536);
    }
}
TrackResult TrackFormat_Quantize(const JointVec6f *q, TrackSample *s)
{
    if (!s || !RobotMath_IsFiniteJoint(q))
        return TRACK_BAD_FORMAT;
    TrackSample v;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        float c = RobotMath_RadToDeg(q->q[i]) * 100;
        if (c < -32768 || c > 32767)
            return TRACK_LIMIT_ERROR;
        v.q_cdeg[i] = (int16_t)lroundf(c);
    }
    *s = v;
    return TRACK_OK;
}
void TrackFormat_ToJoint(const TrackSample *s, JointVec6f *q)
{
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        q->q[i] = RobotMath_DegToRad((float)s->q_cdeg[i] * 0.01f);
}
