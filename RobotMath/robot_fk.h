#ifndef __ROBOT_FK_H
#define __ROBOT_FK_H
#include "robot_math_types.h"
typedef struct
{
    RobotPose joints[AXIS_COUNT];
    Vec3f world_axes[AXIS_COUNT];
    RobotPose tcp;
} RobotFrames;
ArmResult RobotFK_ComputeFrames(const JointVec6f *q, RobotFrames *frames);
ArmResult RobotFK_Solve(const JointVec6f *q, RobotPose *pose);
#endif
