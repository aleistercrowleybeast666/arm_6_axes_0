#ifndef __CYBERGEAR_TYPES_H
#define __CYBERGEAR_TYPES_H
#include "robot_math_types.h"
#include "can_frame.h"
#define CYBERGEAR_UNASSIGNED_ID 0xFFU
#define CYBERGEAR_MAX_VELOCITY_RAD_S 30.0f
#define CYBERGEAR_MAX_TORQUE_NM 12.0f
#define CYBERGEAR_MAX_CURRENT_A 23.0f
#define CYBERGEAR_MAX_KP 500.0f
#define CYBERGEAR_MAX_KD 5.0f
typedef enum
{
    CG_DISCOVER = 0,
    CG_MOTION = 1,
    CG_FEEDBACK = 2,
    CG_ENABLE = 3,
    CG_DISABLE = 4,
    CG_SET_ZERO = 6,
    CG_SET_ID = 7,
    CG_READ_PARAM = 17,
    CG_WRITE_PARAM = 18,
    CG_FAULT_FEEDBACK = 21
} CyberGearFrameType;
typedef enum
{
    CG_MODE_MOTION,
    CG_MODE_POSITION,
    CG_MODE_VELOCITY,
    CG_MODE_CURRENT
} CyberGearMode;
typedef enum
{
    CG_PARAM_RUN_MODE = 0x7005,
    CG_PARAM_CURRENT = 0x7006,
    CG_PARAM_VELOCITY = 0x700A,
    CG_PARAM_TORQUE_LIMIT = 0x700B,
    CG_PARAM_POSITION = 0x7016,
    CG_PARAM_VELOCITY_LIMIT = 0x7017,
    CG_PARAM_CURRENT_LIMIT = 0x7018
} CyberGearParameter;
typedef enum
{
    CG_PROFILE_MANUAL_4PI,
    CG_PROFILE_LEGACY_12_5
} CyberGearProfile;
typedef struct
{
    float position_rad, velocity_rad_s, torque_nm, current_a, kp, kd;
} CyberGearCommand;
typedef struct
{
    float position_rad, velocity_rad_s, torque_nm, temperature_c;
    uint8_t fault, mode;
    uint32_t last_rx_tick;
    bool online, enabled;
} CyberGearState;
typedef struct
{
    uint8_t motor_id, host_id;
    CyberGearProfile profile;
    CyberGearState state;
} CyberGearMotor;
#endif
