#include "hmi_uart.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned clocks, aborts, nesting, tx_count;
static uint32_t tick;
static UART_HandleTypeDef *handle;
static uint8_t *rx_destination;
static uint8_t transmitted[1024];
static bool receive_error, transmit_error;
void Fake_Clock(void) { ++clocks; }
uint32_t HAL_GetTick(void) { return tick; }
void Platform_EnterCritical(void) { ++nesting; }
void Platform_ExitCritical(void) { assert(nesting); --nesting; }
void HAL_GPIO_Init(void *port, const GPIO_InitTypeDef *pins)
{
    assert(port == GPIOA && pins->Pin == (GPIO_PIN_9 | GPIO_PIN_10));
    assert(pins->Alternate == GPIO_AF7_USART1);
}
void HAL_NVIC_SetPriority(unsigned irq, unsigned priority, unsigned subpriority)
{ assert(irq == USART1_IRQn && priority == 5 && subpriority == 0); }
void HAL_NVIC_EnableIRQ(unsigned irq) { assert(irq == USART1_IRQn); }
HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *u)
{
    handle = u;
    assert(u->Instance == USART1 && u->Init.BaudRate == 115200);
    assert(u->Init.WordLength == UART_WORDLENGTH_8B && u->Init.StopBits == UART_STOPBITS_1);
    assert(u->Init.Parity == UART_PARITY_NONE && u->Init.HwFlowCtl == UART_HWCONTROL_NONE);
    return HAL_OK;
}
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *u, uint8_t *byte, uint16_t count)
{
    assert(u == handle && count == 1);
    rx_destination = byte;
    return receive_error ? HAL_ERROR : HAL_OK;
}
HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef *u, uint8_t *bytes, uint16_t count)
{
    assert(u == handle && count <= 64);
    if (transmit_error)
        return HAL_ERROR;
    assert(tx_count + count <= sizeof(transmitted));
    memcpy(transmitted + tx_count, bytes, count);
    tx_count += count;
    return HAL_OK;
}
HAL_StatusTypeDef HAL_UART_Abort(UART_HandleTypeDef *u)
{ assert(u == handle); ++aborts; return HAL_OK; }
void HAL_UART_IRQHandler(UART_HandleTypeDef *u) { assert(u == handle); }
#if ARM_ENABLE_DGUS_HMI
static void Inject(uint8_t byte)
{
    *rx_destination = byte;
    HmiUart_RxComplete(handle);
}
#endif
int main(void)
{
    uint8_t byte = 0, payload[400];
    memset(payload, 0x5A, sizeof(payload));
    UART_HandleTypeDef unrelated = {0};
#if ARM_ENABLE_DGUS_HMI
    assert(HmiUart_Init() == ARM_OK && clocks == 2);
    assert(!HmiUart_Read(NULL));
    Inject(42);
    HmiUart_RxComplete(&unrelated);
    assert(HmiUart_Read(&byte) && byte == 42 && !HmiUart_Read(&byte));
    assert(HmiUart_Write(NULL, payload, sizeof(payload)));
    assert(!HmiUart_Write(NULL, payload, sizeof(payload))); /* Whole-frame backpressure. */
    for (unsigned i = 0; i < 7; ++i)
    {
        assert(!HmiUart_Poll());
        HmiUart_TxComplete(handle);
    }
    assert(tx_count == sizeof(payload) && !memcmp(transmitted, payload, sizeof(payload)));
    for (unsigned i = 0; i < 256; ++i)
        Inject((uint8_t)i);
    assert(!HmiUart_Read(&byte) && HmiUart_Poll() && aborts == 1);
    assert(!HmiUart_Read(&byte));
    Inject(33);
    assert(HmiUart_Read(&byte) && byte == 33);
    HmiUart_Error(&unrelated);
    assert(!HmiUart_Poll());
    HmiUart_Error(handle);
    receive_error = true;
    assert(HmiUart_Poll());
    receive_error = false;
    assert(HmiUart_Poll() && !HmiUart_Poll());
    transmit_error = true;
    assert(HmiUart_Write(NULL, payload, 10));
    assert(!HmiUart_Poll() && HmiUart_Poll());
    transmit_error = false;
    assert(HmiUart_Write(NULL, payload, 10));
    assert(!HmiUart_Poll());
    tick = 100;
    assert(HmiUart_Poll()); /* Lost completion interrupt has a bounded recovery. */
    assert(!nesting);
#else
    assert(HmiUart_Init() == ARM_NOT_CONFIGURED);
    assert(!HmiUart_Read(&byte) && !HmiUart_Write(NULL, payload, sizeof(payload)));
    HmiUart_RxComplete(&unrelated);
    HmiUart_TxComplete(&unrelated);
    HmiUart_Error(&unrelated);
    assert(!HmiUart_Poll() && !clocks && !handle && !aborts && !tx_count);
#endif
    puts("DGUS UART defaults/8N1/pin candidate/isolation/bounded queues/overflow/error/timeout PASS");
    return 0;
}
