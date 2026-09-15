#ifndef __APP_MAIN_H
#define __APP_MAIN_H
#include "robot_state_machine.h"
typedef enum
{
    APP_FATAL_HAL = 1,
    APP_FATAL_RTOS_OBJECT,
    APP_FATAL_STACK,
    APP_FATAL_ASSERT,
    APP_FATAL_CPU
} AppFatalReason;
void App_Init(void);
void App_CreateTasks(void);
void App_Fatal(AppFatalReason reason);
AppFatalReason App_GetFatalReason(void);
RobotState App_GetRobotState(void);
#endif
