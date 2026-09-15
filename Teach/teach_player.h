#ifndef __TEACH_PLAYER_H
#define __TEACH_PLAYER_H
#include "teach_types.h"
ArmResult TeachPlayer_Load(TeachPlayer *player, const TeachSample *samples, size_t count,
                           const RobotLimits *limits, const RobotMotionLimits *motion);
ArmResult TeachPlayer_Start(TeachPlayer *player);
ArmResult TeachPlayer_Step(TeachPlayer *player, float dt_s, RobotTrajectorySample *sample);
void TeachPlayer_Stop(TeachPlayer *player);
#endif
