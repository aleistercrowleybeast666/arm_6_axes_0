#ifndef __ROBOT_CONTROL_H
#define __ROBOT_CONTROL_H
#include "robot_command.h"
#include "robot_state_machine.h"
#include "motor_manager.h"
typedef struct
{
    RobotStateMachine machine;
    RobotTrajectory trajectory;
    RobotTrajectorySample hold;
    RobotLimits hard, soft;
    bool enable_pending;
    uint32_t enable_tick;
} RobotControl;
ArmResult RobotControl_Init(RobotControl *control);
ArmResult RobotControl_Execute(RobotControl *control, const RobotCommand *command, uint32_t now_ms);
ArmResult RobotControl_Step(RobotControl *control, uint32_t now_ms);
void RobotControl_Fault(RobotControl *control, ArmResult reason);
#endif
