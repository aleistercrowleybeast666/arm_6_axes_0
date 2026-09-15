#ifndef __MOTOR_TYPES_H
#define __MOTOR_TYPES_H
#include "cybergear_motor.h"
#include "robot_joint_map.h"
typedef struct
{
    uint8_t motor_id;
    RobotJointMap map;
    CyberGearProfile profile;
    float kp, kd, velocity_limit_rad_s, acceleration_limit_rad_s2, torque_limit_nm, current_limit_a;
    bool protocol_confirmed;
} MotorAxisConfig;
typedef struct
{
    uint8_t motor_id;
    bool online, enabled;
    float position_rad, velocity_rad_s, torque_nm, temperature_c;
    uint32_t fault, last_rx_tick;
    float command_position, command_velocity, command_torque;
    float motor_zero_offset;
    int joint_sign;
} MotorAxisState;
typedef struct
{
    MotorAxisState axes[AXIS_COUNT];
    bool configured;
} MotorSnapshot;
#endif
