#include "debug_uart.h"
#include "debug_cli.h"
#include "usart.h"
#include "board_config.h"
#include "platform_lock.h"
#include <string.h>
#define DEBUG_RX_BYTES 256U
#define DEBUG_TX_BYTES 512U
static uint8_t rx[DEBUG_RX_BYTES], tx[DEBUG_TX_BYTES], rx_byte, tx_chunk[32];
static volatile unsigned rx_head, rx_tail;
static unsigned tx_head, tx_tail;
static volatile bool rx_overflow, tx_active, rx_error;
static char line[DEBUG_LINE_BYTES];
static unsigned line_length;
static bool discard_line;
ArmResult DebugUart_Init(void)
{
    return HAL_UART_Receive_IT(Uart_GetHandle(), &rx_byte, 1) == HAL_OK ? ARM_OK : ARM_BUS_ERROR;
}
static void Debug_Enqueue(const char *s)
{
    size_t n = strlen(s);
    unsigned used = (tx_head + DEBUG_TX_BYTES - tx_tail) % DEBUG_TX_BYTES;
    if (n >= DEBUG_TX_BYTES - used)
        return;
    for (size_t i = 0; i < n; ++i)
    {
        tx[tx_head] = (uint8_t)s[i];
        tx_head = (tx_head + 1) % DEBUG_TX_BYTES;
    }
}
void DebugUart_Poll(uint32_t now)
{
    char response[256];
    if (DebugCli_Poll(response, sizeof(response)))
        Debug_Enqueue(response);
    Platform_EnterCritical();
    if (rx_overflow)
    {
        rx_tail = rx_head;
        rx_overflow = false;
        discard_line = true;
    }
    bool error = rx_error;
    rx_error = false;
    Platform_ExitCritical();
    if (error)
    {
        (void)HAL_UART_AbortReceive(Uart_GetHandle());
        (void)DebugUart_Init();
        discard_line = true;
    }
    for (unsigned budget = 0; budget < 64; ++budget)
    {
        Platform_EnterCritical();
        if (rx_tail == rx_head)
        {
            Platform_ExitCritical();
            break;
        }
        char ch = (char)rx[rx_tail];
        rx_tail = (rx_tail + 1) % DEBUG_RX_BYTES;
        Platform_ExitCritical();
        if (ch == '\n' || ch == '\r')
        {
            if (!discard_line && line_length)
            {
                line[line_length] = 0;
                DebugCli_Execute(line, now, response, sizeof(response));
                Debug_Enqueue(response);
            }
            line_length = 0;
            discard_line = false;
        }
        else if ((unsigned char)ch < 32 || (unsigned char)ch > 126)
            discard_line = true;
        else if (!discard_line)
        {
            if (line_length + 1 < sizeof(line))
                line[line_length++] = ch;
            else
                discard_line = true;
        }
    }
    if (!tx_active && tx_tail != tx_head)
    {
        unsigned n = 0, next = tx_tail;
        while (next != tx_head && n < sizeof(tx_chunk))
        {
            tx_chunk[n++] = tx[next];
            next = (next + 1) % DEBUG_TX_BYTES;
        }
        tx_active = true;
        if (HAL_UART_Transmit_IT(Uart_GetHandle(), tx_chunk, (uint16_t)n) == HAL_OK)
            tx_tail = next;
        else
            tx_active = false;
    }
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *u)
{
    if (u != Uart_GetHandle() || u->Instance != BOARD_UART_INSTANCE)
        return;
    unsigned next = (rx_head + 1) % DEBUG_RX_BYTES;
    if (next == rx_tail)
        rx_overflow = true;
    else
    {
        rx[rx_head] = rx_byte;
        rx_head = next;
    }
    (void)HAL_UART_Receive_IT(u, &rx_byte, 1);
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *u)
{
    if (u == Uart_GetHandle() && u->Instance == BOARD_UART_INSTANCE)
        tx_active = false;
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *u)
{
    if (u == Uart_GetHandle() && u->Instance == BOARD_UART_INSTANCE)
        rx_error = true;
}
