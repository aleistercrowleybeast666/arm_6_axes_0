#include "robot_math_types.h"
#include <math.h>
float RobotMath_DegToRad(float deg)
{
    return deg * ROBOT_DEG_TO_RAD;
}
float RobotMath_RadToDeg(float rad)
{
    return rad / ROBOT_DEG_TO_RAD;
}
float RobotMath_Clamp(float v, float lo, float hi)
{
    return fminf(hi, fmaxf(lo, v));
}
bool RobotMath_IsFiniteJoint(const JointVec6f *q)
{
    if (q == NULL)
        return false;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        if (!isfinite(q->q[i]))
            return false;
    return true;
}
Vec3f Vec3_Add(Vec3f a, Vec3f b)
{
    return (Vec3f){a.x + b.x, a.y + b.y, a.z + b.z};
}
Vec3f Vec3_Subtract(Vec3f a, Vec3f b)
{
    return (Vec3f){a.x - b.x, a.y - b.y, a.z - b.z};
}
Vec3f Vec3_Scale(Vec3f a, float s)
{
    return (Vec3f){a.x * s, a.y * s, a.z * s};
}
Vec3f Vec3_Cross(Vec3f a, Vec3f b)
{
    return (Vec3f){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
float Vec3_Norm(Vec3f a)
{
    return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}
ArmResult Quat_Normalize(Quatf *q)
{
    if (q == NULL)
        return ARM_INVALID_ARGUMENT;
    float n = sqrtf(q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z);
    if (!isfinite(n) || n < 1.0e-8f)
        return ARM_INVALID_ARGUMENT;
    q->w /= n;
    q->x /= n;
    q->y /= n;
    q->z /= n;
    return ARM_OK;
}
Quatf Quat_Multiply(Quatf a, Quatf b)
{
    return (Quatf){
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z, a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x, a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w};
}
Quatf Quat_Conjugate(Quatf q)
{
    return (Quatf){q.w, -q.x, -q.y, -q.z};
}
Quatf Quat_FromAxisAngle(Vec3f axis, float rad)
{
    float s = sinf(rad * 0.5f);
    return (Quatf){cosf(rad * 0.5f), axis.x * s, axis.y * s, axis.z * s};
}
Vec3f Quat_Rotate(Quatf q, Vec3f v)
{
    Vec3f u = {q.x, q.y, q.z};
    Vec3f t = Vec3_Scale(Vec3_Cross(u, v), 2.0f);
    return Vec3_Add(v, Vec3_Add(Vec3_Scale(t, q.w), Vec3_Cross(u, t)));
}
ArmResult Quat_OrientationError(Quatf target, Quatf actual, Vec3f *e)
{
    if (e == NULL || Quat_Normalize(&target) != ARM_OK || Quat_Normalize(&actual) != ARM_OK)
        return ARM_INVALID_ARGUMENT;
    Quatf d = Quat_Multiply(target, Quat_Conjugate(actual));
    if (d.w < 0.0f)
        d = (Quatf){-d.w, -d.x, -d.y, -d.z};
    Vec3f v = {d.x, d.y, d.z};
    float n = Vec3_Norm(v);
    *e = Vec3_Scale(v, n < 1.0e-6f ? 2.0f : 2.0f * atan2f(n, d.w) / n);
    return ARM_OK;
}
