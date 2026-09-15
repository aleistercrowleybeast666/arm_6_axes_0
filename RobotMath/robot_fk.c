#include "robot_fk.h"
#include "robot_geometry.h"
ArmResult RobotFK_ComputeFrames(const JointVec6f *q, RobotFrames *out)
{
    if (!RobotMath_IsFiniteJoint(q) || out == NULL)
        return ARM_INVALID_ARGUMENT;
    RobotPose p = {{0, 0, 0}, {1, 0, 0, 0}};
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        const RobotJointGeometry *g = RobotGeometry_GetJoint((RobotAxis)i);
        p.position_mm = Vec3_Add(p.position_mm, Quat_Rotate(p.orientation, g->parent_translation_mm));
        out->world_axes[i] = Quat_Rotate(p.orientation, g->local_axis);
        p.orientation = Quat_Multiply(p.orientation, Quat_FromAxisAngle(g->local_axis, q->q[i]));
        if (Quat_Normalize(&p.orientation) != ARM_OK)
            return ARM_INVALID_ARGUMENT;
        out->joints[i] = p;
    }
    p.position_mm =
        Vec3_Add(p.position_mm, Quat_Rotate(p.orientation, (Vec3f){RobotGeometry_GetToolLengthMm(), 0, 0}));
    out->tcp = p;
    return ARM_OK;
}
ArmResult RobotFK_Solve(const JointVec6f *q, RobotPose *pose)
{
    RobotFrames f;
    if (pose == NULL)
        return ARM_INVALID_ARGUMENT;
    ArmResult r = RobotFK_ComputeFrames(q, &f);
    if (r == ARM_OK)
        *pose = f.tcp;
    return r;
}
