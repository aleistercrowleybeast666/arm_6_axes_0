#include "soft_stop.h"
#include <math.h>
ArmResult SoftStop_Start(SoftStop *s, const RobotTrajectorySample *q, const RobotLimits *l,
                         const RobotMotionLimits *m)
{
    if (!s || !q || !Trajectory_IsValidMotionLimits(m) || RobotLimits_Check(l, &q->q) != ARM_OK ||
        !RobotMath_IsFiniteJoint(&q->dq_rad_s))
        return ARM_INVALID_ARGUMENT;
    float t = 0.005f;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        t = fmaxf(t, 1.5f * fabsf(q->dq_rad_s.q[i]) / m->acceleration_rad_s2.q[i]);
    JointVec6f end = q->q;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        end.q[i] += q->dq_rad_s.q[i] * t * 0.5f;
    if (RobotLimits_Check(l, &end) != ARM_OK)
        return ARM_OUT_OF_LIMIT;
    *s = (SoftStop){.start = *q, .limits = *l, .duration_s = t, .active = true};
    return ARM_OK;
}
ArmResult SoftStop_Step(SoftStop *s, float dt, RobotTrajectorySample *o)
{
    if (!s || !o || !s->active || !isfinite(dt) || dt < 0)
        return ARM_NOT_READY;
    s->elapsed_s = fminf(s->duration_s, s->elapsed_s + dt);
    float u = s->elapsed_s / s->duration_s;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        float v = s->start.dq_rad_s.q[i];
        o->q.q[i] = s->start.q.q[i] + v * s->duration_s * (u - u * u * u + 0.5f * u * u * u * u);
        o->dq_rad_s.q[i] = v * (1 - 3 * u * u + 2 * u * u * u);
        o->ddq_rad_s2.q[i] = v * (-6 * u + 6 * u * u) / s->duration_s;
    }
    if (u >= 1)
        s->active = false;
    return RobotLimits_Check(&s->limits, &o->q);
}
