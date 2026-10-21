#include "hmi_uart.h"
#if ARM_ENABLE_DGUS_HMI
#include "platform_lock.h"
#define HMI_RX_BYTES 256U
#define HMI_TX_BYTES 512U
static UART_HandleTypeDef hmi_uart;
static uint8_t rx[HMI_RX_BYTES], rx_byte, tx[HMI_TX_BYTES], tx_chunk[64];
static volatile unsigned rx_head, rx_tail;
static unsigned tx_head, tx_tail;
static volatile bool rx_gap, tx_active;
static uint32_t tx_started_ms;
static bool initialized;
ArmResult HmiUart_Init(void)
{
    /* USART2 PA2/PA3 remains the engineering console. No CH340 or connector assumptions. */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    GPIO_InitTypeDef pins = {0};
    pins.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    pins.Mode = GPIO_MODE_AF_PP;
    pins.Pull = GPIO_PULLUP;
    pins.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    pins.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &pins);
    hmi_uart.Instance = USART1;
    hmi_uart.Init.BaudRate = HMI_UART_BAUD;
    hmi_uart.Init.WordLength = UART_WORDLENGTH_8B;
    hmi_uart.Init.StopBits = UART_STOPBITS_1;
    hmi_uart.Init.Parity = UART_PARITY_NONE;
    hmi_uart.Init.Mode = UART_MODE_TX_RX;
    hmi_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    hmi_uart.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&hmi_uart) != HAL_OK)
        return ARM_BUS_ERROR;
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    initialized = true;
    if (HAL_UART_Receive_IT(&hmi_uart, &rx_byte, 1) != HAL_OK)
    {
        rx_gap = true;
        return ARM_BUS_ERROR;
    }
    return ARM_OK;
}
bool HmiUart_Read(uint8_t *byte)
{
    if (!byte)
        return false;
    Platform_EnterCritical();
    bool available = !rx_gap && rx_head != rx_tail;
    if (available)
    {
        *byte = rx[rx_tail];
        rx_tail = (rx_tail + 1U) % HMI_RX_BYTES;
    }
    Platform_ExitCritical();
    return available;
}
bool HmiUart_Write(void *context, const uint8_t *bytes, size_t count)
{
    (void)context;
    if (!initialized || !bytes || !count || rx_gap)
        return false;
    unsigned used = (tx_head + HMI_TX_BYTES - tx_tail) % HMI_TX_BYTES;
    if (count >= HMI_TX_BYTES - used)
        return false; /* Whole frame or nothing; no blocking transmit. */
    for (size_t i = 0; i < count; ++i)
    {
        tx[tx_head] = bytes[i];
        tx_head = (tx_head + 1U) % HMI_TX_BYTES;
    }
    return true;
}
bool HmiUart_Poll(void)
{
    if (!initialized)
        return true;
    if (tx_active && (uint32_t)(HAL_GetTick() - tx_started_ms) >= 100U)
        rx_gap = true;
    Platform_EnterCritical();
    bool gap = rx_gap;
    rx_gap = false;
    if (gap)
        rx_tail = rx_head;
    Platform_ExitCritical();
    if (gap)
    {
        /* Abort both directions before resetting staging buffers. HAL disables the UART IRQ sources. */
        (void)HAL_UART_Abort(&hmi_uart);
        Platform_EnterCritical();
        rx_tail = rx_head;
        tx_head = tx_tail = 0;
        tx_active = false;
        Platform_ExitCritical();
        if (HAL_UART_Receive_IT(&hmi_uart, &rx_byte, 1) != HAL_OK)
            rx_gap = true;
        return true;
    }
    if (!tx_active && tx_head != tx_tail)
    {
        unsigned count = 0, next = tx_tail;
        while (next != tx_head && count < sizeof(tx_chunk))
        {
            tx_chunk[count++] = tx[next];
            next = (next + 1U) % HMI_TX_BYTES;
        }
        tx_active = true;
        tx_started_ms = HAL_GetTick();
        if (HAL_UART_Transmit_IT(&hmi_uart, tx_chunk, (uint16_t)count) == HAL_OK)
            tx_tail = next;
        else
        {
            tx_active = false;
            rx_gap = true; /* Resynchronize rather than silently retrying obsolete status forever. */
        }
    }
    return false;
}
void HmiUart_RxComplete(UART_HandleTypeDef *u)
{
    if (u != &hmi_uart)
        return;
    unsigned next = (rx_head + 1U) % HMI_RX_BYTES;
    if (next == rx_tail)
        rx_gap = true;
    else if (!rx_gap)
    {
        rx[rx_head] = rx_byte;
        rx_head = next;
    }
    if (HAL_UART_Receive_IT(u, &rx_byte, 1) != HAL_OK)
        rx_gap = true;
}
void HmiUart_TxComplete(UART_HandleTypeDef *u)
{
    if (u == &hmi_uart)
        tx_active = false;
}
void HmiUart_Error(UART_HandleTypeDef *u)
{
    if (u == &hmi_uart)
        rx_gap = true;
}
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&hmi_uart);
}
#else
ArmResult HmiUart_Init(void) { return ARM_NOT_CONFIGURED; }
bool HmiUart_Poll(void) { return false; }
bool HmiUart_Read(uint8_t *byte) { (void)byte; return false; }
bool HmiUart_Write(void *context, const uint8_t *bytes, size_t count)
{ (void)context; (void)bytes; (void)count; return false; }
void HmiUart_RxComplete(UART_HandleTypeDef *uart) { (void)uart; }
void HmiUart_TxComplete(UART_HandleTypeDef *uart) { (void)uart; }
void HmiUart_Error(UART_HandleTypeDef *uart) { (void)uart; }
#endif
