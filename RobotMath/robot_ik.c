#include "robot_ik.h"
#include "robot_jacobian.h"
#include <math.h>
#define IK_ITERATION_CAP 2000U
#define IK_MIN_PIVOT 1.0e-12f
#define IK_POOR_RATIO 1.0e-6f
RobotIKOptions RobotIK_DefaultOptions(void)
{
    RobotIKOptions o = {0};
    RobotLimits hard;
    o.max_iterations = 200;
    o.position_tolerance_mm = 0.05f;
    o.orientation_tolerance_rad = 0.0005f;
    o.max_joint_step_rad = 0.15f;
    o.damping_lambda = 0.02f;
    o.position_weight = 0.01f;
    o.orientation_weight = 1.0f;
    (void)RobotLimits_MakeCad(&hard, &o.limits, RobotMath_DegToRad(ROBOT_DEFAULT_SOFT_MARGIN_DEG));
    return o;
}
/* Pivoted elimination; fixed-size workspace, no inverse or allocation. */
static bool IK_SolveLinear(float a[6][6], float b[6], float x[6], float *ratio)
{
    float lo = INFINITY, hi = 0;
    for (unsigned k = 0; k < 6; ++k)
    {
        unsigned pivot = k;
        for (unsigned i = k + 1; i < 6; ++i)
            if (fabsf(a[i][k]) > fabsf(a[pivot][k]))
                pivot = i;
        float v = fabsf(a[pivot][k]);
        if (!isfinite(v) || v < IK_MIN_PIVOT)
            return false;
        lo = fminf(lo, v);
        hi = fmaxf(hi, v);
        if (pivot != k)
        {
            for (unsigned j = k; j < 6; ++j)
            {
                float t = a[k][j];
                a[k][j] = a[pivot][j];
                a[pivot][j] = t;
            }
            float t = b[k];
            b[k] = b[pivot];
            b[pivot] = t;
        }
        for (unsigned i = k + 1; i < 6; ++i)
        {
            float f = a[i][k] / a[k][k];
            for (unsigned j = k; j < 6; ++j)
                a[i][j] -= f * a[k][j];
            b[i] -= f * b[k];
        }
    }
    for (int i = 5; i >= 0; --i)
    {
        float sum = b[i];
        for (unsigned j = (unsigned)i + 1; j < 6; ++j)
            sum -= a[i][j] * x[j];
        x[i] = sum / a[i][i];
        if (!isfinite(x[i]))
            return false;
    }
    *ratio = lo / hi;
    return true;
}
static float IK_GetError(RobotPose target, const JointVec6f *q, const RobotIKOptions *o, float e[6],
                         RobotIKSolution *s)
{
    RobotPose p;
    Vec3f w;
    if (RobotFK_Solve(q, &p) != ARM_OK ||
        Quat_OrientationError(target.orientation, p.orientation, &w) != ARM_OK)
        return INFINITY;
    Vec3f v = Vec3_Subtract(target.position_mm, p.position_mm);
    s->position_error_mm = Vec3_Norm(v);
    s->orientation_error_rad = Vec3_Norm(w);
    e[0] = v.x * o->position_weight;
    e[1] = v.y * o->position_weight;
    e[2] = v.z * o->position_weight;
    e[3] = w.x * o->orientation_weight;
    e[4] = w.y * o->orientation_weight;
    e[5] = w.z * o->orientation_weight;
    float cost = 0;
    for (unsigned i = 0; i < 6; ++i)
        cost += e[i] * e[i];
    return cost;
}
RobotIKResult RobotIK_Solve(const RobotPose *target, const JointVec6f *seed, const RobotIKOptions *o,
                            RobotIKSolution *s)
{
    if (s == NULL)
        return IK_INVALID_TARGET;
    *s = (RobotIKSolution){.status = IK_INVALID_TARGET};
    if (target == NULL || o == NULL || !RobotMath_IsFiniteJoint(seed) || !RobotLimits_IsValid(&o->limits))
        return s->status;
    s->q = *seed;
    const float values[] = {o->position_tolerance_mm, o->orientation_tolerance_rad, o->max_joint_step_rad,
                            o->damping_lambda,        o->position_weight,           o->orientation_weight};
    for (unsigned i = 0; i < 6; ++i)
        if (!isfinite(values[i]) || values[i] <= 0)
            return s->status;
    if (o->max_iterations == 0 || o->max_iterations > IK_ITERATION_CAP)
        return s->status;
    RobotPose t = *target;
    if (!isfinite(t.position_mm.x) || !isfinite(t.position_mm.y) || !isfinite(t.position_mm.z) ||
        Quat_Normalize(&t.orientation) != ARM_OK)
        return s->status;
    if (RobotLimits_Check(&o->limits, seed) != ARM_OK)
        return s->status = IK_OUT_OF_LIMIT;
    bool clipped = false, poor = false;
    float lambda = o->damping_lambda;
    for (unsigned it = 0; it <= o->max_iterations; ++it)
    {
        float e[6];
        s->iterations = it;
        float cost = IK_GetError(t, &s->q, o, e, s);
        if (!isfinite(cost))
            return s->status = IK_NUMERIC_ERROR;
        if (s->position_error_mm <= o->position_tolerance_mm &&
            s->orientation_error_rad <= o->orientation_tolerance_rad)
            return s->status = IK_OK;
        if (it == o->max_iterations)
            break;
        Jacobian6f j;
        if (RobotJacobian_Compute(&s->q, &j) != ARM_OK)
            return s->status = IK_NUMERIC_ERROR;
        for (unsigned r = 0; r < 6; ++r)
            for (unsigned c = 0; c < 6; ++c)
                j.m[r][c] *= r < 3 ? o->position_weight : o->orientation_weight;
        float a[6][6] = {{0}}, b[6] = {0}, dq[6] = {0};
        for (unsigned r = 0; r < 6; ++r)
            for (unsigned k = 0; k < 6; ++k)
            {
                b[r] += j.m[k][r] * e[k];
                for (unsigned c = 0; c < 6; ++c)
                    a[r][c] += j.m[k][r] * j.m[k][c];
            }
        for (unsigned r = 0; r < 6; ++r)
            a[r][r] += lambda * lambda;
        if (!IK_SolveLinear(a, b, dq, &s->pivot_ratio))
        {
            poor = true;
            lambda = fminf(lambda * 4.0f, 100.0f);
            continue;
        }
        s->damping_used = lambda;
        if (s->pivot_ratio < IK_POOR_RATIO)
        {
            poor = true;
            lambda = fminf(lambda * 2.0f, 100.0f);
        }
        float peak = 0;
        for (unsigned i = 0; i < 6; ++i)
            peak = fmaxf(peak, fabsf(dq[i]));
        float scale = peak > o->max_joint_step_rad ? o->max_joint_step_rad / peak : 1.0f;
        bool accepted = false;
        for (unsigned trial = 0; trial < 8; ++trial)
        {
            JointVec6f candidate = s->q;
            for (unsigned i = 0; i < 6; ++i)
                candidate.q[i] += dq[i] * scale;
            if (RobotLimits_Clamp(&o->limits, &candidate) == ARM_OUT_OF_LIMIT)
                clipped = true;
            RobotIKSolution temp = {0};
            float te[6];
            float next = IK_GetError(t, &candidate, o, te, &temp);
            if (isfinite(next) && next < cost)
            {
                s->q = candidate;
                accepted = true;
                break;
            }
            scale *= 0.5f;
        }
        lambda = accepted ? fmaxf(o->damping_lambda, lambda * 0.8f) : fminf(100.0f, lambda * 2.0f);
        if (!accepted && lambda >= 100.0f)
        {
            poor = true;
            break;
        }
    }
    return s->status = clipped ? IK_OUT_OF_LIMIT : (poor ? IK_SINGULAR_OR_POOR_CONDITION : IK_MAX_ITER);
}
