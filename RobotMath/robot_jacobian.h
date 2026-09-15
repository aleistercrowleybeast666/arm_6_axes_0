#ifndef __ROBOT_JACOBIAN_H
#define __ROBOT_JACOBIAN_H
#include "robot_fk.h"
ArmResult RobotJacobian_Compute(const JointVec6f *q, Jacobian6f *jacobian);
#endif
