#include "startup_manager.h"
#include "robot_geometry.h"
#include <math.h>
#define STARTUP_WAIT_TIMEOUT_MS 15000U
void StartupManager_Init(StartupManager *s, StartupPositionPolicy p, uint32_t now)
{
    if (s)
        *s = (StartupManager){.state = STARTUP_BOOT, .policy = p, .began_ms = now};
}
static ArmResult Startup_Fail(StartupManager *s, RobotControl *c, ArmResult r)
{
    s->state = STARTUP_FAILED;
    s->result = r;
    if (MotorManager_IsArmed())
        RobotControl_Fault(c, r);
    return r;
}
ArmResult StartupManager_Step(StartupManager *s, RobotControl *c, uint32_t now)
{
    if (!s || !c)
        return ARM_INVALID_ARGUMENT;
    if (s->state == STARTUP_READY)
        return ARM_OK;
    if (s->state == STARTUP_FAILED)
        return s->result;
    if (c->machine.state == ROBOT_FAULT || c->machine.state == ROBOT_SOFT_STOPPED || c->stopping.active)
        return Startup_Fail(s, c, ARM_NOT_READY);
    if (s->policy != STARTUP_POSITION_ASSUME_STOW)
        return Startup_Fail(s, c, ARM_NOT_CONFIGURED);
    if (s->state == STARTUP_BOOT)
    {
        s->state = STARTUP_WAIT_MOTORS;
        s->began_ms = now;
    }
    if (s->state == STARTUP_WAIT_MOTORS)
    {
        MotorSnapshot snap;
        MotorManager_GetSnapshot(now, &snap);
        if (!MotorManager_IsHealthy(&snap, false))
        {
            if ((uint32_t)(now - s->began_ms) > STARTUP_WAIT_TIMEOUT_MS)
                return Startup_Fail(s, c, ARM_TIMEOUT);
            return ARM_NOT_READY;
        }
        s->state = STARTUP_CAPTURE_STOW;
    }
    if (s->state == STARTUP_CAPTURE_STOW)
    {
        JointVec6f stow, home;
        RobotGeometry_GetFrozenPose(ROBOT_STOW, &stow);
        RobotGeometry_GetFrozenPose(ROBOT_HOME, &home);
        if (RobotLimits_Check(&c->hard, &stow) != ARM_OK || RobotLimits_Check(&c->soft, &home) != ARM_OK ||
            home.q[AXIS_J3] <= stow.q[AXIS_J3])
            return Startup_Fail(s, c, ARM_OUT_OF_LIMIT);
        /* The only hard-limit exception: fixed STOW to fixed HOME. No caller-supplied endpoints. */
        ArmResult r = MotorManager_CaptureStartupPose(&stow, now);
        RobotMotionLimits m;
        if (r == ARM_OK)
            r = MotorManager_GetMotionLimits(&m);
        if (r == ARM_OK)
            r = Trajectory_Start(&c->trajectory, &stow, &home, &c->hard, &m, 0);
        if (r == ARM_OK)
            r = MotorManager_ArmOutput();
        if (r == ARM_OK)
            r = MotorManager_EnableAll(now);
        if (r != ARM_OK)
            return Startup_Fail(s, c, r);
        c->startup_escape = true;
        c->enable_pending = true;
        c->enable_tick = now;
        c->hold = (RobotTrajectorySample){.q = stow};
        c->machine.state = ROBOT_STARTUP;
        s->state = STARTUP_TO_HOME;
    }
    if (s->state == STARTUP_TO_HOME && c->machine.state == ROBOT_READY)
    {
        s->state = STARTUP_READY;
        s->result = ARM_OK;
    }
    return s->state == STARTUP_READY ? ARM_OK : ARM_NOT_READY;
}
