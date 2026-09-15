#include "cybergear_motor.h"
#include "can_transport.h"
#include <math.h>
static ArmResult Motor_SendSimple(const CyberGearMotor *m, CyberGearFrameType type)
{
    if (m == NULL)
        return ARM_INVALID_ARGUMENT;
    CanFrame f;
    ArmResult r = CyberGear_EncodeSimple(m->motor_id, m->host_id, type, &f);
    return r == ARM_OK ? CanTransport_Submit(&f, 1) : r;
}
ArmResult CyberGear_Enable(const CyberGearMotor *m)
{
    return Motor_SendSimple(m, CG_ENABLE);
}
ArmResult CyberGear_Disable(const CyberGearMotor *m)
{
    return Motor_SendSimple(m, CG_DISABLE);
}
ArmResult CyberGear_SetMechanicalZero(const CyberGearMotor *m)
{
    if (m == NULL || m->state.enabled)
        return ARM_NOT_READY;
    return Motor_SendSimple(m, CG_SET_ZERO);
}
ArmResult CyberGear_SendMotionCommand(const CyberGearMotor *m, const CyberGearCommand *c)
{
    if (m == NULL)
        return ARM_INVALID_ARGUMENT;
    CanFrame f;
    ArmResult r = CyberGear_EncodeMotion(m->motor_id, m->profile, c, &f);
    return r == ARM_OK ? CanTransport_Submit(&f, 1) : r;
}
ArmResult CyberGear_SetPositionMode(const CyberGearMotor *m)
{
    if (m == NULL || m->state.enabled)
        return ARM_NOT_READY;
    CanFrame f;
    ArmResult r = CyberGear_EncodeParameter(m->motor_id, m->host_id, CG_PARAM_RUN_MODE, CG_MODE_POSITION, &f);
    return r == ARM_OK ? CanTransport_Submit(&f, 1) : r;
}
ArmResult CyberGear_SetLimits(const CyberGearMotor *m, float v, float t, float a)
{
    if (m == NULL || m->state.enabled || !isfinite(v) || !isfinite(t) || !isfinite(a) || v <= 0 || v > 30 ||
        t <= 0 || t > 12 || a <= 0 || a > 23)
        return ARM_INVALID_ARGUMENT;
    CanFrame f[3];
    ArmResult r = CyberGear_EncodeParameter(m->motor_id, m->host_id, CG_PARAM_VELOCITY_LIMIT, v, &f[0]);
    if (r != ARM_OK)
        return r;
    r = CyberGear_EncodeParameter(m->motor_id, m->host_id, CG_PARAM_TORQUE_LIMIT, t, &f[1]);
    if (r != ARM_OK)
        return r;
    r = CyberGear_EncodeParameter(m->motor_id, m->host_id, CG_PARAM_CURRENT_LIMIT, a, &f[2]);
    return r == ARM_OK ? CanTransport_Submit(f, 3) : r;
}
ArmResult CyberGear_ProcessFeedback(CyberGearMotor *m, const CanFrame *f)
{
    return m == NULL ? ARM_INVALID_ARGUMENT
                     : CyberGear_DecodeFeedback(f, m->motor_id, m->host_id, m->profile, &m->state);
}
bool CyberGear_IsOnline(const CyberGearMotor *m, uint32_t now, uint32_t timeout)
{
    return m != NULL && m->state.online && (uint32_t)(now - m->state.last_rx_tick) < timeout;
}
ArmResult CyberGear_GetState(const CyberGearMotor *m, CyberGearState *s)
{
    if (m == NULL || s == NULL)
        return ARM_INVALID_ARGUMENT;
    *s = m->state;
    return ARM_OK;
}
