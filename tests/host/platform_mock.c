#include "platform_mock.h"
#include "can_transport.h"
#include "platform_lock.h"
#include "cybergear_types.h"
/* Deliberately permits explicit arming to exercise commissioned control in host tests.
 * The production transport independently tests the compile-time closed gate. */
static CanFrame frames[64];
static size_t count;
static bool armed;
void Platform_EnterCritical(void)
{
}
void Platform_ExitCritical(void)
{
}
void Mock_Reset(void)
{
    count = 0;
    armed = false;
}
size_t Mock_GetPending(void)
{
    return count;
}
const CanFrame *Mock_GetFrame(size_t i)
{
    return i < count ? &frames[i] : NULL;
}
ArmResult CanTransport_Init(void)
{
    Mock_Reset();
    return ARM_OK;
}
ArmResult CanTransport_ArmOutput(void)
{
    armed = true;
    return ARM_OK;
}
bool CanTransport_IsArmed(void)
{
    return armed;
}
bool CanTransport_HasFault(void)
{
    return false;
}
void CanTransport_RevokeOutput(void)
{
    armed = false;
    count = 0;
}
void CanTransport_Pump(uint32_t now)
{
    (void)now;
    count = 0;
}
ArmResult CanTransport_Submit(const CanFrame *f, size_t n)
{
    if (f == NULL || n == 0 || n > 64)
        return ARM_INVALID_ARGUMENT;
    if (count + n > 64)
        return ARM_QUEUE_FULL;
    for (size_t i = 0; i < n; ++i)
        if (!armed && (f[i].ext_id >> 24) != CG_DISABLE)
            return ARM_OUTPUT_DISABLED;
    for (size_t i = 0; i < n; ++i)
        frames[count++] = f[i];
    return ARM_OK;
}

void Platform_NotifyStorage(void) {}
