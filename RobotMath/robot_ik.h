#ifndef __ROBOT_IK_H
#define __ROBOT_IK_H
#include "robot_limits.h"
typedef enum
{
    IK_OK,
    IK_MAX_ITER,
    IK_OUT_OF_LIMIT,
    IK_SINGULAR_OR_POOR_CONDITION,
    IK_INVALID_TARGET,
    IK_NUMERIC_ERROR
} RobotIKResult;
typedef struct
{
    unsigned max_iterations;
    float position_tolerance_mm, orientation_tolerance_rad, max_joint_step_rad;
    float damping_lambda, position_weight, orientation_weight;
    RobotLimits limits;
} RobotIKOptions;
typedef struct
{
    JointVec6f q;
    unsigned iterations;
    float position_error_mm, orientation_error_rad, pivot_ratio, damping_used;
    RobotIKResult status;
} RobotIKSolution;
RobotIKOptions RobotIK_DefaultOptions(void);
RobotIKResult RobotIK_Solve(const RobotPose *target, const JointVec6f *seed, const RobotIKOptions *options,
                            RobotIKSolution *result);
#endif
