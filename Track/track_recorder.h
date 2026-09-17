#ifndef __TRACK_RECORDER_H
#define __TRACK_RECORDER_H
#include "track_buffer.h"
typedef struct
{
    uint32_t count, last_tick;
    bool active, aborted;
    TrackResult result;
} TrackRecorder;
TrackResult TrackRecorder_Start(TrackRecorder *recorder, const JointVec6f *actual, uint32_t now_ms);
TrackResult TrackRecorder_Push(TrackRecorder *recorder, const JointVec6f *actual, uint32_t now_ms,
                               const RobotLimits *limits);
TrackResult TrackRecorder_Finish(TrackRecorder *recorder);
void TrackRecorder_Abort(TrackRecorder *recorder);
#endif
