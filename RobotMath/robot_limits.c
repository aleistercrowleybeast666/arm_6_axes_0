#include "robot_limits.h"
#include <math.h>
static const float cad_deg[AXIS_COUNT][2] = {{-170, 170}, {-15, 100},  {-150, 100},
                                             {-180, 180}, {-120, 120}, {-180, 180}};
bool RobotLimits_IsValid(const RobotLimits *l)
{
    if (l == NULL || !RobotMath_IsFiniteJoint(&l->lower_rad) || !RobotMath_IsFiniteJoint(&l->upper_rad))
        return false;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        if (l->lower_rad.q[i] >= l->upper_rad.q[i])
            return false;
    return true;
}
ArmResult RobotLimits_MakeCad(RobotLimits *hard, RobotLimits *soft, float margin)
{
    if (hard == NULL || soft == NULL || hard == soft || !isfinite(margin) || margin < 0)
        return ARM_INVALID_ARGUMENT;
    RobotLimits h, s;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        h.lower_rad.q[i] = RobotMath_DegToRad(cad_deg[i][0]);
        h.upper_rad.q[i] = RobotMath_DegToRad(cad_deg[i][1]);
        s.lower_rad.q[i] = h.lower_rad.q[i] + margin;
        s.upper_rad.q[i] = h.upper_rad.q[i] - margin;
    }
    if (!RobotLimits_IsValid(&s))
        return ARM_INVALID_ARGUMENT;
    *hard = h;
    *soft = s;
    return ARM_OK;
}
ArmResult RobotLimits_Check(const RobotLimits *l, const JointVec6f *q)
{
    if (!RobotLimits_IsValid(l) || !RobotMath_IsFiniteJoint(q))
        return ARM_INVALID_ARGUMENT;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        if (q->q[i] < l->lower_rad.q[i] || q->q[i] > l->upper_rad.q[i])
            return ARM_OUT_OF_LIMIT;
    return ARM_OK;
}
ArmResult RobotLimits_Clamp(const RobotLimits *l, JointVec6f *q)
{
    ArmResult r = RobotLimits_Check(l, q);
    if (r == ARM_INVALID_ARGUMENT)
        return r;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        q->q[i] = RobotMath_Clamp(q->q[i], l->lower_rad.q[i], l->upper_rad.q[i]);
    return r;
}
ArmResult RobotLimits_DistanceToBoundary(const RobotLimits *l, const JointVec6f *q, JointVec6f *d)
{
    ArmResult r = RobotLimits_Check(l, q);
    if (d == NULL || r == ARM_INVALID_ARGUMENT)
        return ARM_INVALID_ARGUMENT;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        d->q[i] = fminf(q->q[i] - l->lower_rad.q[i], l->upper_rad.q[i] - q->q[i]);
    return r;
}
