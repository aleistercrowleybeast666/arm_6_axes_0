#include "bsp_can.h"
#include "board_config.h"
#include "can.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
static StaticQueue_t rx_control;
static uint8_t rx_storage[CAN_RX_QUEUE_DEPTH * sizeof(CanFrame)];
static QueueHandle_t rx_queue;
static volatile BspCanDiagnostics diagnostics;
ArmResult BspCan_Init(void)
{
    CAN_FilterTypeDef filter = {0};
    filter.FilterBank = 0;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    /* Accept extended data frames; protocol/motor/host validation occurs in task. */
    filter.FilterIdLow = 4;
    filter.FilterMaskIdLow = 6;
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterActivation = ENABLE;
    filter.SlaveStartFilterBank = 14;
    CAN_HandleTypeDef *can = Can_GetHandle();
    /* HAL timeout needs TIM6 interrupts: do this before any FreeRTOS critical
     * section, whose pre-scheduler BASEPRI remains masked until the first task. */
    if (HAL_CAN_ConfigFilter(can, &filter) != HAL_OK || HAL_CAN_Start(can) != HAL_OK)
        return ARM_BUS_ERROR;
    rx_queue = xQueueCreateStatic(CAN_RX_QUEUE_DEPTH, sizeof(CanFrame), rx_storage, &rx_control);
    if (rx_queue == NULL)
        return ARM_NOT_READY;
    if (HAL_CAN_ActivateNotification(can, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO0_OVERRUN |
                                              CAN_IT_ERROR | CAN_IT_BUSOFF | CAN_IT_ERROR_PASSIVE |
                                              CAN_IT_LAST_ERROR_CODE | CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
        return ARM_BUS_ERROR;
    return ARM_OK;
}
ArmResult BspCan_Receive(CanFrame *f, uint32_t wait_ms)
{
    if (f == NULL || rx_queue == NULL)
        return ARM_INVALID_ARGUMENT;
    return xQueueReceive(rx_queue, f, pdMS_TO_TICKS(wait_ms)) == pdPASS ? ARM_OK : ARM_TIMEOUT;
}
ArmResult BspCan_TrySend(const CanFrame *f)
{
    if (f == NULL || !f->extended || f->remote || f->dlc != 8 || f->ext_id > 0x1FFFFFFFU)
        return ARM_INVALID_ARGUMENT;
    CAN_HandleTypeDef *can = Can_GetHandle();
    if (HAL_CAN_GetTxMailboxesFreeLevel(can) == 0)
        return ARM_QUEUE_FULL;
    CAN_TxHeaderTypeDef header = {.ExtId = f->ext_id, .IDE = CAN_ID_EXT, .RTR = CAN_RTR_DATA, .DLC = 8};
    uint32_t mailbox;
    return HAL_CAN_AddTxMessage(can, &header, (uint8_t *)f->data, &mailbox) == HAL_OK ? ARM_OK
                                                                                      : ARM_BUS_ERROR;
}
void BspCan_AbortTx(void)
{
    CAN_HandleTypeDef *can = Can_GetHandle();
    if (can->State == HAL_CAN_STATE_LISTENING)
        (void)HAL_CAN_AbortTxRequest(can, CAN_TX_MAILBOX0 | CAN_TX_MAILBOX1 | CAN_TX_MAILBOX2);
}
void BspCan_GetDiagnostics(BspCanDiagnostics *d)
{
    if (d != NULL)
    {
        taskENTER_CRITICAL();
        *d = diagnostics;
        taskEXIT_CRITICAL();
    }
}
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *can)
{
    if (can == NULL || can->Instance != BOARD_CAN_INSTANCE || rx_queue == NULL)
        return;
    BaseType_t wake = pdFALSE;
    /* bxCAN FIFO has three entries. Bound each ISR invocation even under overload. */
    for (unsigned n = 0; n < 3 && HAL_CAN_GetRxFifoFillLevel(can, CAN_RX_FIFO0) > 0; ++n)
    {
        CAN_RxHeaderTypeDef header;
        CanFrame f = {0};
        if (HAL_CAN_GetRxMessage(can, CAN_RX_FIFO0, &header, f.data) != HAL_OK)
        {
            ++diagnostics.hal_errors;
            break;
        }
        f.ext_id = header.ExtId;
        f.dlc = (uint8_t)header.DLC;
        f.extended = header.IDE == CAN_ID_EXT;
        f.remote = header.RTR == CAN_RTR_REMOTE;
        f.tick = HAL_GetTick();
        if (!f.extended || f.remote || f.dlc != 8)
        {
            ++diagnostics.rx_invalid;
            continue;
        }
        if (xQueueSendFromISR(rx_queue, &f, &wake) != pdPASS)
            ++diagnostics.rx_overflow;
        else
            ++diagnostics.rx_received;
    }
    portYIELD_FROM_ISR(wake);
}
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *can)
{
    if (can != NULL && can->Instance == BOARD_CAN_INSTANCE)
    {
        ++diagnostics.hal_errors;
        diagnostics.last_hal_error = HAL_CAN_GetError(can);
    }
}
