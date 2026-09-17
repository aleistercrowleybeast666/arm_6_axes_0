#ifndef __SOFT_STOP_H
#define __SOFT_STOP_H
#include "robot_trajectory.h"
typedef struct
{
    RobotTrajectorySample start;
    RobotLimits limits;
    float elapsed_s, duration_s;
    bool active;
} SoftStop;
ArmResult SoftStop_Start(SoftStop *stop, const RobotTrajectorySample *current, const RobotLimits *limits,
                         const RobotMotionLimits *motion);
ArmResult SoftStop_Step(SoftStop *stop, float dt_s, RobotTrajectorySample *out);
#endif
