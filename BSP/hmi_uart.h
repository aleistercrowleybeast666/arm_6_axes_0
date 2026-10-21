#ifndef ARM_HMI_UART_H
#define ARM_HMI_UART_H
#include "robot_math_types.h"
#include "hmi_config.h"
#include "stm32f4xx_hal.h"
ArmResult HmiUart_Init(void);
bool HmiUart_Poll(void); /* true means a receive gap/error: reset the protocol session. */
bool HmiUart_Read(uint8_t *byte);
bool HmiUart_Write(void *context, const uint8_t *bytes, size_t count);
void HmiUart_RxComplete(UART_HandleTypeDef *uart);
void HmiUart_TxComplete(UART_HandleTypeDef *uart);
void HmiUart_Error(UART_HandleTypeDef *uart);
#endif
