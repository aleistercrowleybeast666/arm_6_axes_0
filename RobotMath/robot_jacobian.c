#include "robot_jacobian.h"
ArmResult RobotJacobian_Compute(const JointVec6f *q, Jacobian6f *j)
{
    RobotFrames f;
    if (j == NULL)
        return ARM_INVALID_ARGUMENT;
    ArmResult r = RobotFK_ComputeFrames(q, &f);
    if (r != ARM_OK)
        return r;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        Vec3f w = f.world_axes[i];
        Vec3f v = Vec3_Cross(w, Vec3_Subtract(f.tcp.position_mm, f.joints[i].position_mm));
        j->m[0][i] = v.x;
        j->m[1][i] = v.y;
        j->m[2][i] = v.z;
        j->m[3][i] = w.x;
        j->m[4][i] = w.y;
        j->m[5][i] = w.z;
    }
    return ARM_OK;
}
