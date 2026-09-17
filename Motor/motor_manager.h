#ifndef __MOTOR_MANAGER_H
#define __MOTOR_MANAGER_H
#include "motor_types.h"
#include "robot_trajectory.h"
ArmResult MotorManager_Init(const MotorAxisConfig config[AXIS_COUNT]);
ArmResult MotorManager_ProcessFeedback(const CanFrame *frame);
void MotorManager_GetSnapshot(uint32_t now_ms, MotorSnapshot *snapshot);
ArmResult MotorManager_GetMotionLimits(RobotMotionLimits *limits);
ArmResult MotorManager_EnableAll(uint32_t now_ms);
void MotorManager_DisableAll(void);
ArmResult MotorManager_SendJointCommand(const RobotTrajectorySample *sample, uint32_t now_ms);
bool MotorManager_IsHealthy(const MotorSnapshot *snapshot, bool require_enabled);
ArmResult MotorManager_CaptureStartupPose(const JointVec6f *known_pose, uint32_t now_ms);
ArmResult MotorManager_ArmOutput(void);
bool MotorManager_IsArmed(void);
bool MotorManager_HasBusFault(void);
ArmResult MotorManager_EnterTeachMode(void);
ArmResult MotorManager_ExitTeachMode(void);
#endif
