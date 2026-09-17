#include "track_player.h"
#include <math.h>
static float Player_Q(const TrackSample *s, unsigned i, unsigned j)
{
    return RobotMath_DegToRad(s[i].q_cdeg[j] * 0.01f);
}
static float Player_Tangent(const TrackSample *s, unsigned n, unsigned i, unsigned j)
{
    if (i == 0 || i + 1 == n)
        return 0;
    float a = Player_Q(s, i, j) - Player_Q(s, i - 1, j), b = Player_Q(s, i + 1, j) - Player_Q(s, i, j);
    return a * b <= 0 ? 0 : 2 * a * b / (a + b);
}
static void Player_Coefficients(const TrackSample *s, unsigned n, unsigned i, unsigned j, float c[4])
{
    float a = Player_Q(s, i, j), b = Player_Q(s, i + 1, j), m = Player_Tangent(s, n, i, j),
          k = Player_Tangent(s, n, i + 1, j);
    c[0] = a;
    c[1] = m;
    c[2] = 3 * (b - a) - 2 * m - k;
    c[3] = 2 * (a - b) + m + k;
}
static void Player_Eval(const float c[4], float u, float dt, float *q, float *v, float *a)
{
    *q = c[0] + u * (c[1] + u * (c[2] + u * c[3]));
    *v = (c[1] + u * (2 * c[2] + 3 * u * c[3])) / dt;
    *a = (2 * c[2] + 6 * u * c[3]) / (dt * dt);
}
TrackResult TrackPlayer_Validate(const TrackSample *s, const TrackHeader *h, const RobotLimits *l,
                                 const RobotMotionLimits *m)
{
    if (!s || !h || !RobotLimits_IsValid(l) || !Trajectory_IsValidMotionLimits(m) || h->count < 2 ||
        h->count > TRACK_MAX_SAMPLES ||
        (h->period_us != 10000 && h->period_us != 20000 && h->period_us != 40000))
        return TRACK_BAD_FORMAT;
    float dt = h->period_us * 0.000001f;
    for (unsigned i = 0; i < h->count; ++i)
    {
        JointVec6f q;
        TrackFormat_ToJoint(&s[i], &q);
        if (RobotLimits_Check(l, &q) != ARM_OK)
            return TRACK_LIMIT_ERROR;
    }
    for (unsigned i = 0; i + 1 < h->count; ++i)
        for (unsigned j = 0; j < AXIS_COUNT; ++j)
        {
            float c[4];
            Player_Coefficients(s, h->count, i, j, c);
            /* Cubic extrema: velocity quadratic roots for position, acceleration root for velocity. */
            float u[5] = {0, 1, -1, -1, -1};
            if (fabsf(c[3]) > 1e-12f)
            {
                u[2] = -c[2] / (3 * c[3]);
                float d = 4 * c[2] * c[2] - 12 * c[3] * c[1];
                if (d >= 0)
                {
                    u[3] = (-2 * c[2] + sqrtf(d)) / (6 * c[3]);
                    u[4] = (-2 * c[2] - sqrtf(d)) / (6 * c[3]);
                }
            }
            else if (fabsf(c[2]) > 1e-12f)
                u[3] = -c[1] / (2 * c[2]);
            for (unsigned k = 0; k < 5; ++k)
                if (u[k] >= 0 && u[k] <= 1)
                {
                    float q, v, a;
                    Player_Eval(c, u[k], dt, &q, &v, &a);
                    if (!isfinite(q) || q < l->lower_rad.q[j] - 1e-6f || q > l->upper_rad.q[j] + 1e-6f ||
                        fabsf(v) > m->velocity_rad_s.q[j] || fabsf(a) > m->acceleration_rad_s2.q[j])
                        return TRACK_LIMIT_ERROR;
                }
        }
    return TRACK_OK;
}
TrackResult TrackPlayer_Sample(const TrackSample *s, const TrackHeader *h, float t, RobotTrajectorySample *o)
{
    if (!s || !h || !o || h->count < 2 || h->count > TRACK_MAX_SAMPLES || !h->period_us || !isfinite(t) ||
        t < 0)
        return TRACK_BAD_FORMAT;
    float dt = h->period_us * 0.000001f, duration = (h->count - 1) * dt;
    *o = (RobotTrajectorySample){0};
    if (t >= duration)
    {
        TrackFormat_ToJoint(&s[h->count - 1], &o->q);
        return TRACK_OK;
    }
    unsigned i = (unsigned)(t / dt);
    float u = (t - i * dt) / dt;
    for (unsigned j = 0; j < AXIS_COUNT; ++j)
    {
        float c[4];
        Player_Coefficients(s, h->count, i, j, c);
        Player_Eval(c, u, dt, &o->q.q[j], &o->dq_rad_s.q[j], &o->ddq_rad_s2.q[j]);
    }
    return TRACK_OK;
}
TrackResult TrackPlayer_Start(TrackPlayer *p, const JointVec6f *actual, float tolerance)
{
    if (!p || !RobotMath_IsFiniteJoint(actual) || !isfinite(tolerance) || tolerance < 0)
        return TRACK_BAD_FORMAT;
    const TrackSample *s = TrackBuffer_GetSamples(TRACK_OWNER_READY);
    TrackHeader h = TrackBuffer_GetHeader();
    if (!s || h.count < 2)
        return TRACK_NOT_READY;
    JointVec6f first;
    TrackFormat_ToJoint(s, &first);
    for (unsigned j = 0; j < AXIS_COUNT; ++j)
        if (fabsf(first.q[j] - actual->q[j]) > tolerance)
            return TRACK_LIMIT_ERROR;
    if (TrackBuffer_Transfer(TRACK_OWNER_READY, TRACK_OWNER_PLAYER) != TRACK_OK)
        return TRACK_BUSY;
    *p = (TrackPlayer){.samples = s, .header = h, .active = true};
    return TRACK_OK;
}
TrackResult TrackPlayer_Step(TrackPlayer *p, float dt, RobotTrajectorySample *o)
{
    if (!p || !o || !p->active || TrackBuffer_GetOwner() != TRACK_OWNER_PLAYER || !isfinite(dt) || dt < 0)
        return TRACK_NOT_READY;
    p->elapsed_s += dt;
    TrackResult r = TrackPlayer_Sample(p->samples, &p->header, p->elapsed_s, o);
    if (p->elapsed_s >= (p->header.count - 1) * p->header.period_us * 0.000001f)
    {
        p->active = false;
        p->finished = true;
        (void)TrackBuffer_Transfer(TRACK_OWNER_PLAYER, TRACK_OWNER_FREE);
    }
    return r;
}
void TrackPlayer_Stop(TrackPlayer *p)
{
    if (p)
    {
        p->active = false;
        p->finished = false;
        (void)TrackBuffer_Transfer(TRACK_OWNER_PLAYER, TRACK_OWNER_FREE);
    }
}
