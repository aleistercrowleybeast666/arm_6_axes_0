#include "teach_player.h"
#include <math.h>
/* Stop-to-stop quintic segments: continuous velocity/acceleration at samples.
 * Reject recordings whose recorded timing cannot satisfy the supplied limits.
 * This small RAM implementation owns its copy; future storage supplies a reader. */
ArmResult TeachPlayer_Load(TeachPlayer *p, const TeachSample *s, size_t n, const RobotLimits *l,
                           const RobotMotionLimits *m)
{
    if (p == NULL)
        return ARM_INVALID_ARGUMENT;
    p->active = false;
    p->count = 0;
    if (s == NULL || n < 2 || n > TEACH_CAPACITY || s[0].time_ms != 0 || !RobotLimits_IsValid(l) ||
        !Trajectory_IsValidMotionLimits(m))
        return ARM_INVALID_ARGUMENT;
    for (size_t i = 0; i < n; ++i)
    {
        ArmResult r = RobotLimits_Check(l, &s[i].q_rad);
        if (r != ARM_OK)
            return r;
        if (i > 0)
        {
            if (s[i].time_ms <= s[i - 1].time_ms)
                return ARM_INVALID_ARGUMENT;
            float duration = (float)(s[i].time_ms - s[i - 1].time_ms) * 0.001f;
            RobotTrajectory t;
            r = Trajectory_Start(&t, &s[i - 1].q_rad, &s[i].q_rad, l, m, 0);
            if (r != ARM_OK)
                return r;
            if (t.duration_s > duration + 1.0e-5f)
                return ARM_OUT_OF_LIMIT;
        }
    }
    for (size_t i = 0; i < n; ++i)
        p->samples[i] = s[i];
    p->count = n;
    p->limits = *l;
    p->motion = *m;
    p->elapsed_s = 0;
    p->segment = 0;
    return ARM_OK;
}
ArmResult TeachPlayer_Start(TeachPlayer *p)
{
    if (p == NULL || p->count < 2)
        return ARM_NOT_READY;
    p->elapsed_s = 0;
    p->segment = 0;
    p->active = true;
    return ARM_OK;
}
ArmResult TeachPlayer_Step(TeachPlayer *p, float dt, RobotTrajectorySample *out)
{
    if (p == NULL || out == NULL || !isfinite(dt) || dt < 0)
        return ARM_INVALID_ARGUMENT;
    if (!p->active)
        return ARM_NOT_READY;
    p->elapsed_s = fminf(p->elapsed_s + dt, (float)p->samples[p->count - 1].time_ms * 0.001f);
    for (size_t k = p->segment; k + 2 < p->count; ++k)
    {
        if (p->elapsed_s < (float)p->samples[k + 1].time_ms * 0.001f)
            break;
        p->segment = k + 1;
    }
    const TeachSample *a = &p->samples[p->segment], *b = a + 1;
    float duration = (float)(b->time_ms - a->time_ms) * 0.001f;
    RobotTrajectory t;
    ArmResult r = Trajectory_Start(&t, &a->q_rad, &b->q_rad, &p->limits, &p->motion, duration);
    if (r != ARM_OK)
    {
        p->active = false;
        return r;
    }
    r = Trajectory_Step(&t, p->elapsed_s - (float)a->time_ms * 0.001f, out);
    if (r != ARM_OK || p->elapsed_s >= (float)p->samples[p->count - 1].time_ms * 0.001f)
        p->active = false;
    return r;
}
void TeachPlayer_Stop(TeachPlayer *p)
{
    if (p != NULL)
        p->active = false;
}
