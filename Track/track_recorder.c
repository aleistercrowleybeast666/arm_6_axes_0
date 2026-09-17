#include "track_recorder.h"
#include "robot_geometry.h"
#include <math.h>
TrackResult TrackRecorder_Start(TrackRecorder *r, const JointVec6f *q, uint32_t now)
{
    if (!r || !RobotMath_IsFiniteJoint(q))
        return TRACK_BAD_FORMAT;
    JointVec6f home;
    RobotGeometry_GetFrozenPose(ROBOT_HOME, &home);
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        if (fabsf(q->q[i] - home.q[i]) > RobotMath_DegToRad(1))
            return TRACK_LIMIT_ERROR;
    TrackResult t = TrackBuffer_Transfer(TRACK_OWNER_FREE, TRACK_OWNER_RECORDER);
    if (t != TRACK_OK)
        return t;
    *r = (TrackRecorder){.active = true, .last_tick = now - 20U};
    return TRACK_OK;
}
TrackResult TrackRecorder_Push(TrackRecorder *r, const JointVec6f *q, uint32_t now, const RobotLimits *limits)
{
    if (!r || !r->active)
        return TRACK_NOT_READY;
    if ((uint32_t)(now - r->last_tick) < 20)
        return TRACK_OK;
    if (r->count >= TRACK_MAX_SAMPLES)
    {
        r->active = false;
        return r->result = TRACK_BUFFER_FULL;
    }
    /* Fixed-period data must never silently compress a missed sampling interval. */
    if ((uint32_t)(now - r->last_tick) > 25 || RobotLimits_Check(limits, q) != ARM_OK)
    {
        TrackRecorder_Abort(r);
        return r->result = TRACK_LIMIT_ERROR;
    }
    TrackSample s;
    TrackResult t = TrackFormat_Quantize(q, &s);
    if (t != TRACK_OK)
    {
        TrackRecorder_Abort(r);
        return r->result = t;
    }
    t = TrackBuffer_Write(TRACK_OWNER_RECORDER, r->count, &s);
    if (t == TRACK_OK)
    {
        ++r->count;
        r->last_tick = now;
    }
    return t;
}
TrackResult TrackRecorder_Finish(TrackRecorder *r)
{
    if (!r || r->aborted || r->count < 2)
        return TRACK_NOT_READY;
    r->active = false;
    TrackHeader h = {TRACK_RECORD_PERIOD_US, r->count, TrackFormat_GetModelId(), 0};
    return TrackBuffer_SetHeader(TRACK_OWNER_RECORDER, &h);
}
void TrackRecorder_Abort(TrackRecorder *r)
{
    if (r)
    {
        r->active = false;
        r->aborted = true;
        r->result = TRACK_ABORTED;
        (void)TrackBuffer_Transfer(TRACK_OWNER_RECORDER, TRACK_OWNER_FREE);
    }
}
