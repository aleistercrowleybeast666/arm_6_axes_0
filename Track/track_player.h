#ifndef __TRACK_PLAYER_H
#define __TRACK_PLAYER_H
#include "track_buffer.h"
typedef struct
{
    const TrackSample *samples;
    TrackHeader header;
    float elapsed_s;
    bool active, finished;
} TrackPlayer;
TrackResult TrackPlayer_Validate(const TrackSample *samples, const TrackHeader *header,
                                 const RobotLimits *limits, const RobotMotionLimits *motion);
TrackResult TrackPlayer_Sample(const TrackSample *samples, const TrackHeader *header, float time_s,
                               RobotTrajectorySample *out);
TrackResult TrackPlayer_Start(TrackPlayer *player, const JointVec6f *actual, float tolerance_rad);
TrackResult TrackPlayer_Step(TrackPlayer *player, float dt_s, RobotTrajectorySample *out);
void TrackPlayer_Stop(TrackPlayer *player);
#endif
