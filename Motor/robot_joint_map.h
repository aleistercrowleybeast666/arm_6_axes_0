#ifndef __ROBOT_JOINT_MAP_H
#define __ROBOT_JOINT_MAP_H
#include "robot_math_types.h"
typedef struct
{
    int joint_sign;
    float motor_zero_offset_rad;
    bool calibrated;
} RobotJointMap;
ArmResult RobotJointMap_ToRobot(const RobotJointMap *map, float motor_rad, float *joint_rad);
ArmResult RobotJointMap_ToMotor(const RobotJointMap *map, float joint_rad, float *motor_rad);
ArmResult RobotJointMap_MapRate(const RobotJointMap *map, float input, float *output);
#endif
