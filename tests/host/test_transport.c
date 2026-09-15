#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "can_transport.h"
#include "bsp_can.h"
#include "robot_hw_config.h"
static uint32_t tick, sent, aborted;
static unsigned nesting;
static bool busy;
static BspCanDiagnostics diagnostics;
void Fake_EnterCritical(void)
{
    ++nesting;
}
void Fake_ExitCritical(void)
{
    assert(nesting > 0);
    --nesting;
}
uint32_t HAL_GetTick(void)
{
    return tick;
}
QueueHandle_t xQueueCreateStatic(UBaseType_t n, UBaseType_t size, uint8_t *storage, StaticQueue_t *q)
{
    *q = (StaticQueue_t){.storage = storage, .item_size = size, .capacity = n};
    return q;
}
BaseType_t xQueueReset(QueueHandle_t q)
{
    q->count = q->read_index = q->write_index = 0;
    return pdPASS;
}
UBaseType_t uxQueueSpacesAvailable(QueueHandle_t q)
{
    return (UBaseType_t)(q->capacity - q->count);
}
BaseType_t xQueueSend(QueueHandle_t q, const void *item, TickType_t wait)
{
    (void)wait;
    if (q->count == q->capacity)
        return pdFAIL;
    memcpy(q->storage + q->write_index * q->item_size, item, q->item_size);
    q->write_index = (q->write_index + 1) % q->capacity;
    ++q->count;
    return pdPASS;
}
BaseType_t xQueuePeek(QueueHandle_t q, void *item, TickType_t wait)
{
    (void)wait;
    if (q->count == 0)
        return pdFAIL;
    memcpy(item, q->storage + q->read_index * q->item_size, q->item_size);
    return pdPASS;
}
BaseType_t xQueueReceive(QueueHandle_t q, void *item, TickType_t wait)
{
    BaseType_t r = xQueuePeek(q, item, wait);
    if (r == pdPASS)
    {
        q->read_index = (q->read_index + 1) % q->capacity;
        --q->count;
    }
    return r;
}
ArmResult BspCan_TrySend(const CanFrame *f)
{
    assert(f != NULL);
    if (busy)
        return ARM_QUEUE_FULL;
    ++sent;
    return ARM_OK;
}
void BspCan_AbortTx(void)
{
    ++aborted;
}
void BspCan_GetDiagnostics(BspCanDiagnostics *d)
{
    *d = diagnostics;
}
int main(void)
{
    assert(CanTransport_Init() == ARM_OK);
    assert(!CanTransport_IsArmed());
    CanFrame f = {.dlc = 8, .extended = true, .ext_id = 0x03000001U};
    assert(CanTransport_Submit(&f, 1) == ARM_OUTPUT_DISABLED);
    if (!ARM_ENABLE_MOTOR_OUTPUT)
    {
        assert(CanTransport_ArmOutput() == ARM_OUTPUT_DISABLED);
        f.ext_id = 0x04000001;
        assert(CanTransport_Submit(&f, 1) == ARM_OUTPUT_DISABLED);
        CanTransport_Pump(0);
        assert(sent == 0);
        puts("Production transport default compile gate PASS: no CAN TX, including Enable and Disable");
        return 0;
    }
    assert(CanTransport_ArmOutput() == ARM_OK);
    assert(CanTransport_Submit(&f, 1) == ARM_OK);
    busy = true;
    CanTransport_Pump(0);
    assert(sent == 0);
    busy = false;
    CanTransport_Pump(1);
    assert(sent == 1);
    CanFrame batch[64];
    for (unsigned i = 0; i < 64; ++i)
        batch[i] = f;
    assert(CanTransport_Submit(batch, 64) == ARM_OK);
    assert(CanTransport_Submit(&f, 1) == ARM_QUEUE_FULL);
    CanTransport_RevokeOutput();
    CanTransport_Pump(2);
    assert(sent == 1 && aborted > 0);
    assert(CanTransport_Submit(&f, 1) == ARM_OUTPUT_DISABLED);
    f.ext_id = 0x04000001;
    assert(CanTransport_Submit(&f, 1) == ARM_OK);
    CanTransport_Pump(3);
    assert(sent == 2);
    assert(CanTransport_ArmOutput() == ARM_OK);
    f.ext_id = 0x01000001;
    assert(CanTransport_Submit(&f, 1) == ARM_OK);
    CanTransport_Pump(21);
    assert(CanTransport_HasFault() && !CanTransport_IsArmed() && sent == 2);
    assert(CanTransport_ArmOutput() == ARM_NOT_READY);
    assert(CanTransport_Init() == ARM_OK);
    tick = UINT32_MAX - 10U;
    assert(CanTransport_ArmOutput() == ARM_OK);
    assert(CanTransport_Submit(&f, 1) == ARM_OK);
    CanTransport_Pump(5);
    assert(sent == 3);
    diagnostics.hal_errors = 1;
    assert(CanTransport_Submit(&f, 1) == ARM_OK);
    CanTransport_Pump(6);
    assert(sent == 3 && !CanTransport_IsArmed());
    assert(CanTransport_ArmOutput() == ARM_NOT_READY);
    assert(nesting == 0);
    puts("Production transport armed-path PASS: batch capacity, mailbox busy, revoke, stale queue, wrap, bus "
         "error latch");
    return 0;
}
