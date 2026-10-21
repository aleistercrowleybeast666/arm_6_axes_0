#ifndef HMI_FAKE_HAL_H
#define HMI_FAKE_HAL_H
#include <stddef.h>
#include <stdint.h>
typedef enum { HAL_OK, HAL_ERROR } HAL_StatusTypeDef;
typedef struct
{
    uint32_t BaudRate, WordLength, StopBits, Parity, Mode, HwFlowCtl, OverSampling;
} UART_InitTypeDef;
typedef struct { void *Instance; UART_InitTypeDef Init; } UART_HandleTypeDef;
typedef struct { uint32_t Pin, Mode, Pull, Speed, Alternate; } GPIO_InitTypeDef;
#define USART1 ((void *)1)
#define GPIOA ((void *)2)
#define GPIO_PIN_9 (1U << 9)
#define GPIO_PIN_10 (1U << 10)
#define GPIO_MODE_AF_PP 2U
#define GPIO_PULLUP 1U
#define GPIO_SPEED_FREQ_VERY_HIGH 3U
#define GPIO_AF7_USART1 7U
#define UART_WORDLENGTH_8B 8U
#define UART_STOPBITS_1 1U
#define UART_PARITY_NONE 0U
#define UART_MODE_TX_RX 3U
#define UART_HWCONTROL_NONE 0U
#define UART_OVERSAMPLING_16 16U
#define USART1_IRQn 37U
#define __HAL_RCC_GPIOA_CLK_ENABLE() Fake_Clock()
#define __HAL_RCC_USART1_CLK_ENABLE() Fake_Clock()
void Fake_Clock(void);
void HAL_GPIO_Init(void *port, const GPIO_InitTypeDef *pins);
void HAL_NVIC_SetPriority(unsigned irq, unsigned priority, unsigned subpriority);
void HAL_NVIC_EnableIRQ(unsigned irq);
HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *uart);
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *uart, uint8_t *byte, uint16_t count);
HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef *uart, uint8_t *bytes, uint16_t count);
HAL_StatusTypeDef HAL_UART_Abort(UART_HandleTypeDef *uart);
void HAL_UART_IRQHandler(UART_HandleTypeDef *uart);
uint32_t HAL_GetTick(void);
#endif
