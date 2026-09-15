#include "robot_geometry.h"
/* Extracted from the pinned arm_0 mechanical baseline; no mesh dimensions. */
static const RobotJointGeometry joints[AXIS_COUNT] = {{{0, 0, 0}, {0, 0, 1}},    {{0, 0, 180}, {0, -1, 0}},
                                                      {{320, 0, 0}, {0, -1, 0}}, {{290, 0, 0}, {1, 0, 0}},
                                                      {{60, 0, 0}, {0, -1, 0}},  {{86, 0, 0}, {1, 0, 0}}};
static const float poses_deg[ROBOT_POSE_COUNT][AXIS_COUNT] = {
    {0, 55, -150, 0, 95, 0},
    {0, 52, -142, 0, 90, 0},
    {0, 50, -110, 0, 60, 0},
    {0, 60.4965754675011f, -72.4522811069283f, 0, -78.0442943605728f, 0}};
const RobotJointGeometry *RobotGeometry_GetJoint(RobotAxis axis)
{
    return (unsigned)axis < AXIS_COUNT ? &joints[axis] : NULL;
}
float RobotGeometry_GetToolLengthMm(void)
{
    return 90.0f;
}
ArmResult RobotGeometry_GetFrozenPose(RobotFrozenPose pose, JointVec6f *q)
{
    if ((unsigned)pose >= ROBOT_POSE_COUNT || q == NULL)
        return ARM_INVALID_ARGUMENT;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        q->q[i] = RobotMath_DegToRad(poses_deg[pose][i]);
    return ARM_OK;
}
