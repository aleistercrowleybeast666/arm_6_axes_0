#include "robot_control.h"
#include "robot_hw_config.h"
#include "can_transport.h"
ArmResult RobotControl_Init(RobotControl *c)
{
    if (c == NULL)
        return ARM_INVALID_ARGUMENT;
    *c = (RobotControl){0};
    RobotState_Init(&c->machine);
    return RobotLimits_MakeCad(&c->hard, &c->soft, RobotMath_DegToRad(ROBOT_DEFAULT_SOFT_MARGIN_DEG));
}
void RobotControl_Fault(RobotControl *c, ArmResult reason)
{
    if (c != NULL)
    {
        Trajectory_Stop(&c->trajectory);
        c->enable_pending = false;
        RobotState_SetFault(&c->machine, reason);
        MotorManager_DisableAll();
    }
}
static JointVec6f Control_GetJoint(const MotorSnapshot *s)
{
    JointVec6f q;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        q.q[i] = s->axes[i].position_rad;
    return q;
}
ArmResult RobotControl_Execute(RobotControl *c, const RobotCommand *cmd, uint32_t now)
{
    if (c == NULL || cmd == NULL)
        return ARM_INVALID_ARGUMENT;
    MotorSnapshot s;
    MotorManager_GetSnapshot(now, &s);
    JointVec6f q = Control_GetJoint(&s);
    switch (cmd->type)
    {
    case ROBOT_CMD_DISABLE:
        Trajectory_Stop(&c->trajectory);
        c->enable_pending = false;
        MotorManager_DisableAll();
        RobotState_Disable(&c->machine);
        return ARM_OK;
    case ROBOT_CMD_ENABLE:
    {
        if (c->machine.state != ROBOT_DISABLED || c->enable_pending || !MotorManager_IsHealthy(&s, false))
            return ARM_NOT_READY;
        ArmResult r = RobotLimits_Check(&c->soft, &q);
        if (r != ARM_OK)
            return r;
        r = CanTransport_ArmOutput();
        if (r != ARM_OK)
            return r;
        r = MotorManager_EnableAll(now);
        if (r != ARM_OK)
        {
            MotorManager_DisableAll();
            return r;
        }
        c->enable_pending = true;
        c->enable_tick = now;
        return ARM_OK;
    }
    case ROBOT_CMD_JOINT_MOVE:
    {
        if (c->machine.state != ROBOT_READY || !MotorManager_IsHealthy(&s, true))
            return ARM_NOT_READY;
        RobotMotionLimits motion;
        ArmResult r = MotorManager_GetMotionLimits(&motion);
        if (r != ARM_OK)
            return r;
        r = Trajectory_Start(&c->trajectory, &q, &cmd->target_rad, &c->soft, &motion, 0);
        if (r != ARM_OK)
            return r;
        return RobotState_StartMotion(&c->machine);
    }
    case ROBOT_CMD_CLEAR_FAULT:
    {
        bool healthy = MotorManager_IsHealthy(&s, false) && !CanTransport_HasFault();
        for (unsigned i = 0; i < AXIS_COUNT; ++i)
            if (s.axes[i].enabled)
                healthy = false;
        return RobotState_ClearFault(&c->machine, healthy);
    }
    default:
        return ARM_INVALID_ARGUMENT;
    }
}
ArmResult RobotControl_Step(RobotControl *c, uint32_t now)
{
    if (c == NULL)
        return ARM_INVALID_ARGUMENT;
    MotorSnapshot s;
    MotorManager_GetSnapshot(now, &s);
    if (c->enable_pending)
    {
        if (!MotorManager_IsHealthy(&s, false) || (uint32_t)(now - c->enable_tick) >= MOTOR_ENABLE_TIMEOUT_MS)
        {
            RobotControl_Fault(c, ARM_TIMEOUT);
            return ARM_TIMEOUT;
        }
        bool fresh = true;
        for (unsigned i = 0; i < AXIS_COUNT; ++i)
            if ((int32_t)(s.axes[i].last_rx_tick - c->enable_tick) <= 0)
                fresh = false;
        if (fresh && MotorManager_IsHealthy(&s, true))
        {
            c->hold = (RobotTrajectorySample){.q = Control_GetJoint(&s)};
            if (RobotState_SetReady(&c->machine, true, CanTransport_IsArmed()) != ARM_OK)
            {
                RobotControl_Fault(c, ARM_NOT_READY);
                return ARM_NOT_READY;
            }
            c->enable_pending = false;
        }
    }
    if (c->machine.state != ROBOT_READY && c->machine.state != ROBOT_RUNNING)
        return ARM_OK;
    JointVec6f actual = Control_GetJoint(&s);
    if (!MotorManager_IsHealthy(&s, true) || !CanTransport_IsArmed() || CanTransport_HasFault() ||
        RobotLimits_Check(&c->hard, &actual) != ARM_OK)
    {
        RobotControl_Fault(c, ARM_FAULT);
        return ARM_FAULT;
    }
    if (c->machine.state == ROBOT_RUNNING)
    {
        ArmResult r = Trajectory_Step(&c->trajectory, (float)ROBOT_CONTROL_PERIOD_MS * 0.001f, &c->hold);
        if (r != ARM_OK)
        {
            RobotControl_Fault(c, r);
            return r;
        }
        if (Trajectory_IsFinished(&c->trajectory))
            RobotState_FinishMotion(&c->machine);
    }
    ArmResult r = RobotLimits_Check(&c->soft, &c->hold.q);
    if (r == ARM_OK)
        r = MotorManager_SendJointCommand(&c->hold, now);
    if (r != ARM_OK)
        RobotControl_Fault(c, r);
    return r;
}
