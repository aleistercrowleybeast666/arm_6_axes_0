#include "command_manager.h"
#include "storage_service.h"
#include "platform_lock.h"
static AppCommand queue[APP_BUSINESS_QUEUE_DEPTH];
static unsigned head, count;
void CommandManager_Init(void)
{
    head = count = 0;
}
void CommandManager_Clear(void)
{
    Platform_EnterCritical();
    head = count = 0;
    Platform_ExitCritical();
}
ArmResult CommandManager_Submit(const AppCommand *c)
{
    if (!c || (unsigned)c->type > CMD_ACKNOWLEDGE)
        return ARM_INVALID_ARGUMENT;
    if (c->source != CONTROL_SOURCE_LOCAL)
        return ARM_OUTPUT_DISABLED;
    if ((c->type == CMD_EXEC_CUSTOM || c->type == CMD_DELETE_CUSTOM) && !Storage_IsSafeName(c->name))
        return ARM_INVALID_ARGUMENT;
    Platform_EnterCritical();
    if (count == APP_BUSINESS_QUEUE_DEPTH)
    {
        Platform_ExitCritical();
        return ARM_QUEUE_FULL;
    }
    queue[(head + count) % APP_BUSINESS_QUEUE_DEPTH] = *c;
    ++count;
    Platform_ExitCritical();
    return ARM_OK;
}
bool CommandManager_Take(AppCommand *c)
{
    if (!c)
        return false;
    Platform_EnterCritical();
    if (!count)
    {
        Platform_ExitCritical();
        return false;
    }
    *c = queue[head];
    head = (head + 1) % APP_BUSINESS_QUEUE_DEPTH;
    --count;
    Platform_ExitCritical();
    return true;
}
