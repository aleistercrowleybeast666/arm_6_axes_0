#include "teach_recorder.h"
ArmResult TeachRecorder_Init(TeachRecorder *r, const RobotLimits *l)
{
    if (r == NULL || !RobotLimits_IsValid(l))
        return ARM_INVALID_ARGUMENT;
    *r = (TeachRecorder){.limits = *l};
    return ARM_OK;
}
ArmResult TeachRecorder_Start(TeachRecorder *r)
{
    if (r == NULL || !RobotLimits_IsValid(&r->limits))
        return ARM_INVALID_ARGUMENT;
    r->count = 0;
    r->recording = true;
    return ARM_OK;
}
ArmResult TeachRecorder_PushSample(TeachRecorder *r, const TeachSample *s)
{
    if (r == NULL || s == NULL)
        return ARM_INVALID_ARGUMENT;
    if (!r->recording)
        return ARM_NOT_READY;
    ArmResult status = RobotLimits_Check(&r->limits, &s->q_rad);
    if (status != ARM_OK)
        return status;
    if (r->count >= TEACH_CAPACITY)
    {
        r->recording = false;
        return ARM_QUEUE_FULL;
    }
    if ((r->count == 0 && s->time_ms != 0) ||
        (r->count > 0 && s->time_ms <= r->samples[r->count - 1].time_ms))
        return ARM_INVALID_ARGUMENT;
    r->samples[r->count++] = *s;
    return ARM_OK;
}
void TeachRecorder_Stop(TeachRecorder *r)
{
    if (r != NULL)
        r->recording = false;
}
