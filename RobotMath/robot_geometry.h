#ifndef __ROBOT_GEOMETRY_H
#define __ROBOT_GEOMETRY_H
#include "robot_math_types.h"
#define ROBOT_MECHANICAL_SHA "1b1cc4982e09ccf0d09f9921e68e24940a76d877"
typedef enum
{
    ROBOT_STOW,
    ROBOT_HOME,
    ROBOT_SAFE_UNFOLD,
    ROBOT_CAKE_APPROACH,
    ROBOT_POSE_COUNT
} RobotFrozenPose;
typedef struct
{
    Vec3f parent_translation_mm;
    Vec3f local_axis;
} RobotJointGeometry;
const RobotJointGeometry *RobotGeometry_GetJoint(RobotAxis axis);
float RobotGeometry_GetToolLengthMm(void);
ArmResult RobotGeometry_GetFrozenPose(RobotFrozenPose pose, JointVec6f *q);
#endif
