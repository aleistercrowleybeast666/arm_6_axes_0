#include "motor_manager.h"
#include "motor_backend.h"
#include "robot_hw_config.h"
#include "platform_lock.h"
#include <math.h>
static MotorAxisConfig configs[AXIS_COUNT];
static bool coordinates_ready;
ArmResult MotorManager_Init(const MotorAxisConfig c[AXIS_COUNT])
{
    if (!c)
        return ARM_INVALID_ARGUMENT;
    if (MotorManager_IsArmed())
        return ARM_NOT_READY;
    MotorAxisConfig raw[AXIS_COUNT];
    bool mapped = true;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        raw[i] = c[i];
        raw[i].map = (RobotJointMap){.joint_sign = 1, .calibrated = true};
        if ((c[i].map.joint_sign != 1 && c[i].map.joint_sign != -1) ||
            !isfinite(c[i].map.motor_zero_offset_rad))
            return ARM_NOT_CONFIGURED;
        mapped &= c[i].map.calibrated;
    }
    Platform_EnterCritical();
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        configs[i] = c[i];
    coordinates_ready = mapped;
    ArmResult r = MotorBackend_Get()->init(raw);
    Platform_ExitCritical();
    return r;
}
ArmResult MotorManager_ProcessFeedback(const CanFrame *f)
{
    return MotorBackend_Get()->process_feedback(f);
}
void MotorManager_GetSnapshot(uint32_t now, MotorSnapshot *s)
{
    if (!s)
        return;
    Platform_EnterCritical();
    MotorBackend_Get()->get_feedback(now, s);
    s->coordinates_ready = coordinates_ready;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        MotorAxisState *a = &s->axes[i];
        const RobotJointMap *m = &configs[i].map;
        a->motor_position_rad = a->position_rad;
        a->motor_zero_offset = m->motor_zero_offset_rad;
        a->joint_sign = m->joint_sign;
        if (RobotJointMap_ToRobot(m, a->position_rad, &a->position_rad) != ARM_OK ||
            RobotJointMap_MapRate(m, a->velocity_rad_s, &a->velocity_rad_s) != ARM_OK)
            a->fault |= 0x200U;
        (void)RobotJointMap_MapRate(m, a->torque_nm, &a->torque_nm);
        (void)RobotJointMap_ToRobot(m, a->command_position, &a->command_position);
        (void)RobotJointMap_MapRate(m, a->command_velocity, &a->command_velocity);
    }
    Platform_ExitCritical();
}
bool MotorManager_IsHealthy(const MotorSnapshot *s, bool enabled)
{
    if (!s || !s->configured || (enabled && !s->coordinates_ready))
        return false;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        if (!s->axes[i].online || s->axes[i].fault || s->axes[i].temperature_c >= MOTOR_MAX_TEMPERATURE_C ||
            (enabled && !s->axes[i].enabled))
            return false;
    return true;
}
ArmResult MotorManager_CaptureStartupPose(const JointVec6f *q, uint32_t now)
{
    if (!RobotMath_IsFiniteJoint(q) || MotorManager_IsArmed())
        return ARM_NOT_READY;
    Platform_EnterCritical();
    MotorSnapshot s;
    MotorBackend_Get()->get_feedback(now, &s);
    if (!MotorManager_IsHealthy(&s, false))
    {
        Platform_ExitCritical();
        return ARM_NOT_READY;
    }
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        if (s.axes[i].enabled)
        {
            Platform_ExitCritical();
            return ARM_NOT_READY;
        }
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        configs[i].map.motor_zero_offset_rad =
            s.axes[i].position_rad - (float)configs[i].map.joint_sign * q->q[i];
        configs[i].map.calibrated = true;
    }
    coordinates_ready = true;
    Platform_ExitCritical();
    return ARM_OK;
}
ArmResult MotorManager_GetMotionLimits(RobotMotionLimits *m)
{
    if (!m)
        return ARM_INVALID_ARGUMENT;
    Platform_EnterCritical();
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        m->velocity_rad_s.q[i] = configs[i].velocity_limit_rad_s;
        m->acceleration_rad_s2.q[i] = configs[i].acceleration_limit_rad_s2;
    }
    Platform_ExitCritical();
    return Trajectory_IsValidMotionLimits(m) ? ARM_OK : ARM_NOT_CONFIGURED;
}
ArmResult MotorManager_EnableAll(uint32_t now)
{
    if (!coordinates_ready)
        return ARM_NOT_CONFIGURED;
    return MotorBackend_Get()->enable(now);
}
void MotorManager_DisableAll(void)
{
    MotorBackend_Get()->disable();
}
ArmResult MotorManager_SendJointCommand(const RobotTrajectorySample *s, uint32_t now)
{
    if (!s || !coordinates_ready)
        return ARM_NOT_READY;
    RobotTrajectorySample raw = *s;
    Platform_EnterCritical();
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        const RobotJointMap *m = &configs[i].map;
        if (RobotJointMap_ToMotor(m, s->q.q[i], &raw.q.q[i]) != ARM_OK ||
            RobotJointMap_MapRate(m, s->dq_rad_s.q[i], &raw.dq_rad_s.q[i]) != ARM_OK)
        {
            Platform_ExitCritical();
            return ARM_INVALID_ARGUMENT;
        }
        raw.ddq_rad_s2.q[i] = (float)m->joint_sign * s->ddq_rad_s2.q[i];
    }
    ArmResult r = MotorBackend_Get()->set_command(&raw, now);
    Platform_ExitCritical();
    return r;
}
ArmResult MotorManager_ArmOutput(void)
{
    return MotorBackend_Get()->arm_output();
}
bool MotorManager_IsArmed(void)
{
    return MotorBackend_Get()->is_armed();
}
bool MotorManager_HasBusFault(void)
{
    return MotorBackend_Get()->has_bus_fault();
}
ArmResult MotorManager_EnterTeachMode(void)
{
    return MotorBackend_Get()->enter_teach_mode();
}
ArmResult MotorManager_ExitTeachMode(void)
{
    return MotorBackend_Get()->exit_teach_mode();
}
