#ifndef __DEBUG_UART_H
#define __DEBUG_UART_H
#include "robot_math_types.h"
ArmResult DebugUart_Init(void);
void DebugUart_Poll(uint32_t now_ms);
#endif
