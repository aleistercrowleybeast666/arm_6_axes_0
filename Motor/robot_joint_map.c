#include "robot_joint_map.h"
#include <math.h>
static bool JointMap_IsValid(const RobotJointMap *m)
{
    return m != NULL && (m->joint_sign == 1 || m->joint_sign == -1) && isfinite(m->motor_zero_offset_rad);
}
ArmResult RobotJointMap_ToRobot(const RobotJointMap *m, float v, float *out)
{
    if (!JointMap_IsValid(m) || !isfinite(v) || out == NULL)
        return ARM_INVALID_ARGUMENT;
    *out = (float)m->joint_sign * (v - m->motor_zero_offset_rad);
    return isfinite(*out) ? ARM_OK : ARM_INVALID_ARGUMENT;
}
ArmResult RobotJointMap_ToMotor(const RobotJointMap *m, float v, float *out)
{
    if (!JointMap_IsValid(m) || !isfinite(v) || out == NULL)
        return ARM_INVALID_ARGUMENT;
    *out = (float)m->joint_sign * v + m->motor_zero_offset_rad;
    return isfinite(*out) ? ARM_OK : ARM_INVALID_ARGUMENT;
}
ArmResult RobotJointMap_MapRate(const RobotJointMap *m, float v, float *out)
{
    if (!JointMap_IsValid(m) || !isfinite(v) || out == NULL)
        return ARM_INVALID_ARGUMENT;
    *out = (float)m->joint_sign * v;
    return ARM_OK;
}
