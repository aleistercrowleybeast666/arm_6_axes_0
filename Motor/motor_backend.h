#ifndef __MOTOR_BACKEND_H
#define __MOTOR_BACKEND_H
#include "motor_types.h"
#include "robot_trajectory.h"
/* One statically selected six-axis driver. Commands and feedback here are MOTOR coordinates. */
typedef struct
{
    ArmResult (*init)(const MotorAxisConfig config[AXIS_COUNT]);
    ArmResult (*process_feedback)(const CanFrame *frame);
    void (*get_feedback)(uint32_t now_ms, MotorSnapshot *snapshot);
    ArmResult (*enable)(uint32_t now_ms);
    void (*disable)(void);
    ArmResult (*set_command)(const RobotTrajectorySample *command, uint32_t now_ms);
    ArmResult (*enter_teach_mode)(void);
    ArmResult (*exit_teach_mode)(void);
    bool (*is_online)(uint32_t now_ms);
    uint32_t (*get_fault)(uint32_t now_ms);
    ArmResult (*arm_output)(void);
    bool (*is_armed)(void);
    bool (*has_bus_fault)(void);
} MotorBackend;
const MotorBackend *MotorBackend_Get(void);
#endif
