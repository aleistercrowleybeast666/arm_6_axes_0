#ifndef __ROBOT_CONTROL_H
#define __ROBOT_CONTROL_H
#include "robot_command.h"
#include "robot_state_machine.h"
#include "motor_manager.h"
#include "soft_stop.h"
typedef struct
{
    RobotStateMachine machine;
    RobotTrajectory trajectory;
    RobotTrajectorySample hold;
    RobotLimits hard, soft;
    bool enable_pending;
    uint32_t enable_tick;
    bool startup_escape, external_command, teach_active;
    SoftStop stopping;
} RobotControl;
ArmResult RobotControl_Init(RobotControl *control);
ArmResult RobotControl_Execute(RobotControl *control, const RobotCommand *command, uint32_t now_ms);
ArmResult RobotControl_Step(RobotControl *control, uint32_t now_ms);
void RobotControl_Fault(RobotControl *control, ArmResult reason);
ArmResult RobotControl_SetExternal(RobotControl *control, const RobotTrajectorySample *sample);
ArmResult RobotControl_RequestSoftStop(RobotControl *control);
ArmResult RobotControl_AcknowledgeStop(RobotControl *control);
#endif
