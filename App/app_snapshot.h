#ifndef __APP_SNAPSHOT_H
#define __APP_SNAPSHOT_H
#include "operation_manager.h"
typedef struct
{
    StartupState startup;
    RobotState robot;
    OperationSnapshot operation;
    uint32_t command_revision;
    ArmResult command_result;
} AppSnapshot;
void App_GetSnapshot(AppSnapshot *snapshot);
void App_RequestSoftStopFromIsr(void);
void App_NotifySensorFromIsr(void);
#endif
