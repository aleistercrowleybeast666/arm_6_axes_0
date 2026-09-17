#ifndef __COMMAND_MANAGER_H
#define __COMMAND_MANAGER_H
#include "robot_math_types.h"
#include "storage_backend.h"
#define DEBUG_MOTION_COMMANDS 0
#define APP_BUSINESS_QUEUE_DEPTH 8U
typedef enum
{
    CONTROL_SOURCE_LOCAL,
    CONTROL_SOURCE_DEBUG,
    CONTROL_SOURCE_FUTURE_HOST
} ControlSource;
typedef enum
{
    CMD_GO_HOME,
    CMD_EXEC_FACTORY_SMALL,
    CMD_EXEC_FACTORY_MEDIUM,
    CMD_EXEC_FACTORY_LARGE,
    CMD_EXEC_CUSTOM,
    CMD_DELETE_CUSTOM,
    CMD_RECORD_START,
    CMD_RECORD_FINISH,
    CMD_RECORD_CANCEL,
    CMD_SOFT_STOP,
    CMD_ACKNOWLEDGE
} AppCommandType;
typedef struct
{
    AppCommandType type;
    ControlSource source;
    char name[STORAGE_NAME_BYTES];
} AppCommand;
void CommandManager_Init(void);
ArmResult CommandManager_Submit(const AppCommand *command);
bool CommandManager_Take(AppCommand *command);
void CommandManager_Clear(void);
#endif
