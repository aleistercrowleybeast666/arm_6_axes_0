#include "robot_trajectory.h"
#include <math.h>
/* Exact extrema of s(u)=10u^3-15u^4+6u^5 on [0,1]. */
#define QUINTIC_PEAK_VELOCITY 1.875f
#define QUINTIC_PEAK_ACCELERATION 5.7735026919f
bool Trajectory_IsValidMotionLimits(const RobotMotionLimits *m)
{
    if (m == NULL || !RobotMath_IsFiniteJoint(&m->velocity_rad_s) ||
        !RobotMath_IsFiniteJoint(&m->acceleration_rad_s2))
        return false;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        if (m->velocity_rad_s.q[i] <= 0 || m->acceleration_rad_s2.q[i] <= 0)
            return false;
    return true;
}
ArmResult Trajectory_Start(RobotTrajectory *t, const JointVec6f *a, const JointVec6f *b, const RobotLimits *l,
                           const RobotMotionLimits *m, float duration)
{
    if (t == NULL || !Trajectory_IsValidMotionLimits(m) || !isfinite(duration) || duration < 0)
        return ARM_INVALID_ARGUMENT;
    ArmResult r = RobotLimits_Check(l, a);
    if (r != ARM_OK)
        return r;
    r = RobotLimits_Check(l, b);
    if (r != ARM_OK)
        return r;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        float d = fabsf(b->q[i] - a->q[i]);
        duration = fmaxf(duration, QUINTIC_PEAK_VELOCITY * d / m->velocity_rad_s.q[i]);
        duration = fmaxf(duration, sqrtf(QUINTIC_PEAK_ACCELERATION * d / m->acceleration_rad_s2.q[i]));
    }
    if (!isfinite(duration))
        return ARM_INVALID_ARGUMENT;
    *t = (RobotTrajectory){
        .start = *a, .target = *b, .limits = *l, .duration_s = fmaxf(duration, 0.001f), .active = true};
    return ARM_OK;
}
ArmResult Trajectory_Step(RobotTrajectory *t, float dt, RobotTrajectorySample *out)
{
    if (t == NULL || out == NULL || !isfinite(dt) || dt < 0)
        return ARM_INVALID_ARGUMENT;
    if (!t->active && !t->finished)
        return ARM_NOT_READY;
    t->elapsed_s = fminf(t->duration_s, t->elapsed_s + dt);
    float u = t->elapsed_s / t->duration_s, u2 = u * u, u3 = u2 * u;
    float s = u3 * (10.0f + u * (-15.0f + 6.0f * u));
    float ds = 30.0f * u2 * (1.0f - u) * (1.0f - u) / t->duration_s;
    float dds = 60.0f * u * (1.0f - u) * (1.0f - 2.0f * u) / (t->duration_s * t->duration_s);
    s = RobotMath_Clamp(s, 0, 1); /* Guard single precision endpoint rounding. */
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        float d = t->target.q[i] - t->start.q[i];
        out->q.q[i] = (u >= 1) ? t->target.q[i] : t->start.q[i] + d * s;
        out->dq_rad_s.q[i] = d * ds;
        out->ddq_rad_s2.q[i] = d * dds;
    }
    ArmResult r = RobotLimits_Check(&t->limits, &out->q);
    if (r != ARM_OK)
    {
        Trajectory_Stop(t);
        return r;
    }
    if (u >= 1)
    {
        t->active = false;
        t->finished = true;
    }
    return ARM_OK;
}
void Trajectory_Stop(RobotTrajectory *t)
{
    if (t != NULL)
    {
        t->active = false;
        t->finished = false;
    }
}
bool Trajectory_IsFinished(const RobotTrajectory *t)
{
    return t != NULL && t->finished;
}
