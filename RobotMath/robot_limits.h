#ifndef __ROBOT_LIMITS_H
#define __ROBOT_LIMITS_H
#include "robot_math_types.h"
#define CAD_PROVISIONAL_LIMIT 1
#define ROBOT_DEFAULT_SOFT_MARGIN_DEG 2.0f /* Development value, NOT a released hardware margin. */
typedef struct
{
    JointVec6f lower_rad, upper_rad;
} RobotLimits;
ArmResult RobotLimits_MakeCad(RobotLimits *hard_limit, RobotLimits *soft_limit, float margin_rad);
ArmResult RobotLimits_Check(const RobotLimits *limits, const JointVec6f *q);
ArmResult RobotLimits_Clamp(const RobotLimits *limits, JointVec6f *q);
ArmResult RobotLimits_DistanceToBoundary(const RobotLimits *limits, const JointVec6f *q,
                                         JointVec6f *distance_rad);
bool RobotLimits_IsValid(const RobotLimits *limits);
#endif
