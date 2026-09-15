#include "robot_state_machine.h"
void RobotState_Init(RobotStateMachine *m)
{
    if (m != NULL)
        *m = (RobotStateMachine){.state = ROBOT_DISABLED, .fault_reason = ARM_OK};
}
void RobotState_SetFault(RobotStateMachine *m, ArmResult r)
{
    if (m != NULL)
    {
        m->state = ROBOT_FAULT;
        m->fault_reason = r;
    }
}
ArmResult RobotState_SetReady(RobotStateMachine *m, bool healthy, bool armed)
{
    if (m == NULL)
        return ARM_INVALID_ARGUMENT;
    if (m->state != ROBOT_DISABLED || !healthy || !armed)
        return ARM_NOT_READY;
    m->state = ROBOT_READY;
    return ARM_OK;
}
ArmResult RobotState_StartMotion(RobotStateMachine *m)
{
    if (m == NULL)
        return ARM_INVALID_ARGUMENT;
    if (m->state != ROBOT_READY)
        return ARM_NOT_READY;
    m->state = ROBOT_RUNNING;
    return ARM_OK;
}
void RobotState_FinishMotion(RobotStateMachine *m)
{
    if (m != NULL && m->state == ROBOT_RUNNING)
        m->state = ROBOT_READY;
}
void RobotState_Disable(RobotStateMachine *m)
{
    if (m != NULL && m->state != ROBOT_FAULT && m->state != ROBOT_ESTOP_RESERVED)
        m->state = ROBOT_DISABLED;
}
ArmResult RobotState_ClearFault(RobotStateMachine *m, bool healthy)
{
    if (m == NULL)
        return ARM_INVALID_ARGUMENT;
    if (m->state != ROBOT_FAULT || !healthy)
        return ARM_NOT_READY;
    RobotState_Init(m);
    return ARM_OK;
}
