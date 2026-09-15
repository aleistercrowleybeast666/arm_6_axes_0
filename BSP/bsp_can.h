#ifndef __BSP_CAN_H
#define __BSP_CAN_H
#include "can_frame.h"
#include "robot_math_types.h"
typedef struct
{
    uint32_t rx_overflow, rx_invalid, rx_received, hal_errors, last_hal_error;
} BspCanDiagnostics;
ArmResult BspCan_Init(void);
ArmResult BspCan_Receive(CanFrame *frame, uint32_t wait_ms);
ArmResult BspCan_TrySend(const CanFrame *frame);
void BspCan_AbortTx(void);
void BspCan_GetDiagnostics(BspCanDiagnostics *diagnostics);
#endif
