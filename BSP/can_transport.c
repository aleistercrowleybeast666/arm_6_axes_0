#include "can_transport.h"
#include "bsp_can.h"
#include "board_config.h"
#include "robot_hw_config.h"
#include "cybergear_types.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "stm32f4xx_hal.h"
static StaticQueue_t tx_control;
static uint8_t tx_storage[CAN_TX_QUEUE_DEPTH * sizeof(CanFrame)];
static QueueHandle_t tx_queue;
static bool armed, failed;
ArmResult CanTransport_Init(void)
{
    tx_queue = xQueueCreateStatic(CAN_TX_QUEUE_DEPTH, sizeof(CanFrame), tx_storage, &tx_control);
    armed = false;
    failed = false;
    return tx_queue != NULL ? ARM_OK : ARM_NOT_READY;
}
bool CanTransport_IsArmed(void)
{
    taskENTER_CRITICAL();
    bool v = armed;
    taskEXIT_CRITICAL();
    return v;
}
bool CanTransport_HasFault(void)
{
    BspCanDiagnostics d;
    BspCan_GetDiagnostics(&d);
    taskENTER_CRITICAL();
    bool v = failed;
    taskEXIT_CRITICAL();
    return v || d.hal_errors != 0 || d.rx_overflow != 0;
}
ArmResult CanTransport_ArmOutput(void)
{
    taskENTER_CRITICAL();
    ArmResult r = ARM_OK;
    if (!ARM_ENABLE_MOTOR_OUTPUT)
        r = ARM_OUTPUT_DISABLED;
    else if (CanTransport_HasFault() || tx_queue == NULL)
        r = ARM_NOT_READY;
    else
        armed = true;
    taskEXIT_CRITICAL();
    return r;
}
void CanTransport_RevokeOutput(void)
{
    taskENTER_CRITICAL();
    armed = false;
    if (tx_queue != NULL)
        (void)xQueueReset(tx_queue);
    BspCan_AbortTx();
    taskEXIT_CRITICAL();
}
ArmResult CanTransport_Submit(const CanFrame *frames, size_t n)
{
    if (frames == NULL || n == 0 || n > CAN_TX_QUEUE_DEPTH)
        return ARM_INVALID_ARGUMENT;
    taskENTER_CRITICAL();
    ArmResult r = ARM_OK;
    if (!ARM_ENABLE_MOTOR_OUTPUT)
        r = ARM_OUTPUT_DISABLED;
    else if (tx_queue == NULL)
        r = ARM_NOT_READY;
    else if (uxQueueSpacesAvailable(tx_queue) < n)
        r = ARM_QUEUE_FULL;
    for (size_t i = 0; i < n && r == ARM_OK; ++i)
    {
        const CanFrame *f = &frames[i];
        if (f->dlc != 8 || !f->extended || f->remote || f->ext_id > 0x1FFFFFFFU)
            r = ARM_INVALID_ARGUMENT;
        else if ((!armed || failed) && (f->ext_id >> 24) != CG_DISABLE)
            r = ARM_OUTPUT_DISABLED;
    }
    if (r == ARM_OK)
        for (size_t i = 0; i < n; ++i)
        {
            CanFrame f = frames[i];
            f.tick = HAL_GetTick();
            if (xQueueSend(tx_queue, &f, 0) != pdPASS)
            {
                failed = true;
                r = ARM_QUEUE_FULL;
                break;
            }
        }
    taskEXIT_CRITICAL();
    return r;
}
void CanTransport_Pump(uint32_t now)
{
    taskENTER_CRITICAL();
    if (tx_queue == NULL)
    {
        taskEXIT_CRITICAL();
        return;
    }
    if (armed && CanTransport_HasFault())
    {
        failed = true;
        armed = false;
        (void)xQueueReset(tx_queue);
        BspCan_AbortTx();
        taskEXIT_CRITICAL();
        return;
    }
    for (unsigned n = 0; n < 3; ++n)
    {
        CanFrame f;
        if (xQueuePeek(tx_queue, &f, 0) != pdPASS)
            break;
        if ((uint32_t)(now - f.tick) > CAN_TX_MAX_AGE_MS ||
            ((!armed || failed) && (f.ext_id >> 24) != CG_DISABLE))
        {
            failed = true;
            armed = false;
            (void)xQueueReset(tx_queue);
            BspCan_AbortTx();
            break;
        }
        ArmResult r = BspCan_TrySend(&f);
        if (r == ARM_QUEUE_FULL)
            break;
        (void)xQueueReceive(tx_queue, &f, 0);
        if (r != ARM_OK)
        {
            failed = true;
            armed = false;
            (void)xQueueReset(tx_queue);
            BspCan_AbortTx();
            break;
        }
    }
    taskEXIT_CRITICAL();
}
