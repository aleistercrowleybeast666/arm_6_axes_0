#ifndef __CYBERGEAR_MOTOR_H
#define __CYBERGEAR_MOTOR_H
#include "cybergear_protocol.h"
ArmResult CyberGear_Enable(const CyberGearMotor *motor);
ArmResult CyberGear_Disable(const CyberGearMotor *motor);
ArmResult CyberGear_SetMechanicalZero(const CyberGearMotor *motor);
ArmResult CyberGear_SendMotionCommand(const CyberGearMotor *motor, const CyberGearCommand *command);
ArmResult CyberGear_SetPositionMode(const CyberGearMotor *motor);
ArmResult CyberGear_SetLimits(const CyberGearMotor *motor, float velocity_rad_s, float torque_nm,
                              float current_a);
ArmResult CyberGear_ProcessFeedback(CyberGearMotor *motor, const CanFrame *frame);
bool CyberGear_IsOnline(const CyberGearMotor *motor, uint32_t now_ms, uint32_t timeout_ms);
ArmResult CyberGear_GetState(const CyberGearMotor *motor, CyberGearState *state);
#endif
