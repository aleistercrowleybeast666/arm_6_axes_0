#ifndef __ROBOT_STATE_MACHINE_H
#define __ROBOT_STATE_MACHINE_H
#include "robot_math_types.h"
typedef enum
{
    ROBOT_BOOT,
    ROBOT_DISABLED,
    ROBOT_READY,
    ROBOT_RUNNING,
    ROBOT_FAULT,
    ROBOT_ESTOP_RESERVED,
    ROBOT_STARTUP,
    ROBOT_SOFT_STOPPED
} RobotState;
typedef struct
{
    RobotState state;
    ArmResult fault_reason;
} RobotStateMachine;
void RobotState_Init(RobotStateMachine *machine);
void RobotState_SetFault(RobotStateMachine *machine, ArmResult reason);
ArmResult RobotState_SetReady(RobotStateMachine *machine, bool all_enabled_and_healthy, bool output_armed);
ArmResult RobotState_StartMotion(RobotStateMachine *machine);
void RobotState_FinishMotion(RobotStateMachine *machine);
void RobotState_Disable(RobotStateMachine *machine);
ArmResult RobotState_ClearFault(RobotStateMachine *machine, bool healthy_and_disabled);
#endif
