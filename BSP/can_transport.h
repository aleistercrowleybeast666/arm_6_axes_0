#ifndef __CAN_TRANSPORT_H
#define __CAN_TRANSPORT_H
#include "can_frame.h"
#include "robot_math_types.h"
ArmResult CanTransport_Init(void);
ArmResult CanTransport_Submit(const CanFrame *frames, size_t count);
void CanTransport_RevokeOutput(void);
ArmResult CanTransport_ArmOutput(void);
void CanTransport_Pump(uint32_t now_ms);
bool CanTransport_HasFault(void);
bool CanTransport_IsArmed(void);
#endif
