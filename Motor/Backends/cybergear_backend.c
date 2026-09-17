#include "motor_backend.h"
#include "cybergear_motor.h"
#include "robot_hw_config.h"
#include "can_transport.h"
#include "platform_lock.h"
#include <math.h>
static MotorAxisConfig axis_config[AXIS_COUNT];
static CyberGearMotor motors[AXIS_COUNT];
static MotorAxisState axis_state[AXIS_COUNT];
static bool configured;
static bool Backend_IsHealthy(const MotorSnapshot *s, bool enabled);
static void Backend_GetSnapshot(uint32_t now, MotorSnapshot *s);
static bool Manager_IsValidConfig(const MotorAxisConfig *c)
{
    return CyberGear_IsValidId(c->motor_id) && c->motor_id != ROBOT_HOST_CAN_ID &&
           (c->map.joint_sign == 1 || c->map.joint_sign == -1) && isfinite(c->map.motor_zero_offset_rad) &&
           c->protocol_confirmed && isfinite(CyberGear_GetPositionRange(c->profile)) && isfinite(c->kp) &&
           c->kp > 0 && c->kp <= CYBERGEAR_MAX_KP && isfinite(c->kd) && c->kd >= 0 &&
           c->kd <= CYBERGEAR_MAX_KD && isfinite(c->velocity_limit_rad_s) && c->velocity_limit_rad_s > 0 &&
           c->velocity_limit_rad_s <= 30 && isfinite(c->acceleration_limit_rad_s2) &&
           c->acceleration_limit_rad_s2 > 0 && isfinite(c->torque_limit_nm) && c->torque_limit_nm > 0 &&
           c->torque_limit_nm <= 12 && isfinite(c->current_limit_a) && c->current_limit_a > 0 &&
           c->current_limit_a <= 23;
}
static ArmResult Backend_Init(const MotorAxisConfig c[AXIS_COUNT])
{
    if (c == NULL)
        return ARM_INVALID_ARGUMENT;
    if (CanTransport_IsArmed())
        return ARM_NOT_READY;
    Platform_EnterCritical();
    configured = true;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        axis_config[i] = c[i];
        motors[i] = (CyberGearMotor){
            .motor_id = c[i].motor_id, .host_id = ROBOT_HOST_CAN_ID, .profile = c[i].profile};
        axis_state[i] = (MotorAxisState){.motor_id = c[i].motor_id,
                                         .joint_sign = c[i].map.joint_sign,
                                         .motor_zero_offset = c[i].map.motor_zero_offset_rad};
        if (!Manager_IsValidConfig(&c[i]))
            configured = false;
        for (unsigned j = 0; j < i; ++j)
            if (c[i].motor_id == c[j].motor_id)
                configured = false;
    }
    bool valid = configured;
    Platform_ExitCritical();
    return valid ? ARM_OK : ARM_NOT_CONFIGURED;
}
static ArmResult Backend_ProcessFeedback(const CanFrame *f)
{
    if (f == NULL)
        return ARM_INVALID_ARGUMENT;
    ArmResult r = ARM_INVALID_ARGUMENT;
    Platform_EnterCritical();
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        if (!CyberGear_IsValidId(motors[i].motor_id) || ((f->ext_id >> 8) & 255U) != motors[i].motor_id)
            continue;
        /* Unknown detailed fault layout is conservatively latched without interpreting bytes. */
        if (f->extended && !f->remote && f->dlc == 8 && f->ext_id <= 0x1FFFFFFFU &&
            (f->ext_id >> 24) == CG_FAULT_FEEDBACK && (uint8_t)f->ext_id == ROBOT_HOST_CAN_ID)
        {
            axis_state[i].fault |= 0x100U;
            r = ARM_FAULT;
            break;
        }
        r = CyberGear_ProcessFeedback(&motors[i], f);
        if (r == ARM_OK)
        {
            CyberGearState *s = &motors[i].state;
            MotorAxisState *d = &axis_state[i];
            d->online = true;
            d->enabled = s->enabled;
            d->last_rx_tick = s->last_rx_tick;
            d->temperature_c = s->temperature_c;
            d->fault |= s->fault; /* faults require re-initialization after physical diagnosis */
            if (RobotJointMap_ToRobot(&axis_config[i].map, s->position_rad, &d->position_rad) != ARM_OK ||
                RobotJointMap_MapRate(&axis_config[i].map, s->velocity_rad_s, &d->velocity_rad_s) != ARM_OK ||
                RobotJointMap_MapRate(&axis_config[i].map, s->torque_nm, &d->torque_nm) != ARM_OK)
                d->fault |= 0x200U;
        }
        break;
    }
    Platform_ExitCritical();
    return r;
}
static void Backend_GetSnapshot(uint32_t now, MotorSnapshot *s)
{
    if (s == NULL)
        return;
    Platform_EnterCritical();
    s->configured = configured;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        axis_state[i].online = CyberGear_IsOnline(&motors[i], now, MOTOR_FEEDBACK_TIMEOUT_MS);
        s->axes[i] = axis_state[i];
    }
    Platform_ExitCritical();
}
static bool Backend_IsHealthy(const MotorSnapshot *s, bool enabled)
{
    if (s == NULL || !s->configured)
        return false;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        if (!s->axes[i].online || s->axes[i].fault || s->axes[i].temperature_c >= MOTOR_MAX_TEMPERATURE_C ||
            (enabled && !s->axes[i].enabled))
            return false;
    return true;
}
static ArmResult Backend_EnableAll(uint32_t now)
{
    MotorSnapshot s;
    Backend_GetSnapshot(now, &s);
    if (!Backend_IsHealthy(&s, false))
        return ARM_NOT_READY;
    CanFrame batch[AXIS_COUNT * 6];
    size_t n = 0;
    Platform_EnterCritical();
    ArmResult r = ARM_OK;
    for (unsigned i = 0; i < AXIS_COUNT && r == ARM_OK; ++i)
    {
        const MotorAxisConfig *c = &axis_config[i];
        if (s.axes[i].enabled)
        {
            r = ARM_NOT_READY;
            break;
        }
        r = CyberGear_EncodeParameter(c->motor_id, ROBOT_HOST_CAN_ID, CG_PARAM_RUN_MODE, CG_MODE_MOTION,
                                      &batch[n++]);
        if (r == ARM_OK)
            r = CyberGear_EncodeParameter(c->motor_id, ROBOT_HOST_CAN_ID, CG_PARAM_VELOCITY_LIMIT,
                                          c->velocity_limit_rad_s, &batch[n++]);
        if (r == ARM_OK)
            r = CyberGear_EncodeParameter(c->motor_id, ROBOT_HOST_CAN_ID, CG_PARAM_TORQUE_LIMIT,
                                          c->torque_limit_nm, &batch[n++]);
        if (r == ARM_OK)
            r = CyberGear_EncodeParameter(c->motor_id, ROBOT_HOST_CAN_ID, CG_PARAM_CURRENT_LIMIT,
                                          c->current_limit_a, &batch[n++]);
        CyberGearCommand neutral = {0};
        if (r == ARM_OK)
            r = CyberGear_EncodeMotion(c->motor_id, c->profile, &neutral, &batch[n++]);
        if (r == ARM_OK)
            r = CyberGear_EncodeSimple(c->motor_id, ROBOT_HOST_CAN_ID, CG_ENABLE, &batch[n++]);
    }
    if (r == ARM_OK)
        r = CanTransport_Submit(batch, n);
    Platform_ExitCritical();
    return r;
}
static void Backend_DisableAll(void)
{
    /* Invalidate queued motion and abort mailboxes before best-effort stop frames. */
    CanTransport_RevokeOutput();
    Platform_EnterCritical();
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        if (CyberGear_IsValidId(motors[i].motor_id))
            (void)CyberGear_Disable(&motors[i]);
    Platform_ExitCritical();
}
static ArmResult Backend_SendJointCommand(const RobotTrajectorySample *s, uint32_t now)
{
    if (s == NULL || !RobotMath_IsFiniteJoint(&s->q) || !RobotMath_IsFiniteJoint(&s->dq_rad_s) ||
        !RobotMath_IsFiniteJoint(&s->ddq_rad_s2))
        return ARM_INVALID_ARGUMENT;
    MotorSnapshot snap;
    Backend_GetSnapshot(now, &snap);
    if (!Backend_IsHealthy(&snap, true))
        return ARM_FAULT;
    CanFrame frames[AXIS_COUNT];
    ArmResult r = ARM_OK;
    Platform_EnterCritical();
    for (unsigned i = 0; i < AXIS_COUNT && r == ARM_OK; ++i)
    {
        const MotorAxisConfig *cfg = &axis_config[i];
        CyberGearCommand c = {.kp = cfg->kp, .kd = cfg->kd};
        r = RobotJointMap_ToMotor(&cfg->map, s->q.q[i], &c.position_rad);
        if (r == ARM_OK)
            r = RobotJointMap_MapRate(&cfg->map, s->dq_rad_s.q[i], &c.velocity_rad_s);
        if (r == ARM_OK && (fabsf(c.position_rad) > CyberGear_GetPositionRange(cfg->profile) ||
                            fabsf(c.velocity_rad_s) > cfg->velocity_limit_rad_s ||
                            fabsf(s->ddq_rad_s2.q[i]) > cfg->acceleration_limit_rad_s2 * 1.001f))
            r = ARM_OUT_OF_LIMIT;
        if (r == ARM_OK)
            r = CyberGear_EncodeMotion(cfg->motor_id, cfg->profile, &c, &frames[i]);
    }
    if (r == ARM_OK)
        r = CanTransport_Submit(frames, AXIS_COUNT);
    if (r == ARM_OK)
        for (unsigned i = 0; i < AXIS_COUNT; ++i)
        {
            axis_state[i].command_position = s->q.q[i];
            axis_state[i].command_velocity = s->dq_rad_s.q[i];
            axis_state[i].command_torque = 0;
        }
    Platform_ExitCritical();
    return r;
}

static ArmResult Backend_EnterTeach(void)
{
    return ARM_NOT_CONFIGURED;
}
static ArmResult Backend_ExitTeach(void)
{
    return ARM_NOT_CONFIGURED;
}
static bool Backend_IsOnline(uint32_t now)
{
    MotorSnapshot s;
    Backend_GetSnapshot(now, &s);
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        if (!s.axes[i].online)
            return false;
    return true;
}
static uint32_t Backend_GetFault(uint32_t now)
{
    MotorSnapshot s;
    Backend_GetSnapshot(now, &s);
    uint32_t f = 0;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        f |= s.axes[i].fault;
    return f;
}
const MotorBackend *MotorBackend_Get(void)
{
    static const MotorBackend backend = {
        Backend_Init,         Backend_ProcessFeedback,  Backend_GetSnapshot,    Backend_EnableAll,
        Backend_DisableAll,   Backend_SendJointCommand, Backend_EnterTeach,     Backend_ExitTeach,
        Backend_IsOnline,     Backend_GetFault,         CanTransport_ArmOutput, CanTransport_IsArmed,
        CanTransport_HasFault};
    return &backend;
}
