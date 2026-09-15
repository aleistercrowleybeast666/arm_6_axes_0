#ifndef __ROBOT_TRAJECTORY_H
#define __ROBOT_TRAJECTORY_H
#include "robot_limits.h"
typedef struct
{
    JointVec6f velocity_rad_s, acceleration_rad_s2;
} RobotMotionLimits;
typedef struct
{
    JointVec6f q, dq_rad_s, ddq_rad_s2;
} RobotTrajectorySample;
typedef struct
{
    JointVec6f start, target;
    RobotLimits limits;
    float duration_s, elapsed_s;
    bool active, finished;
} RobotTrajectory;
bool Trajectory_IsValidMotionLimits(const RobotMotionLimits *limits);
ArmResult Trajectory_Start(RobotTrajectory *trajectory, const JointVec6f *start, const JointVec6f *target,
                           const RobotLimits *limits, const RobotMotionLimits *motion,
                           float minimum_duration_s);
ArmResult Trajectory_Step(RobotTrajectory *trajectory, float dt_s, RobotTrajectorySample *sample);
void Trajectory_Stop(RobotTrajectory *trajectory);
bool Trajectory_IsFinished(const RobotTrajectory *trajectory);
#endif
