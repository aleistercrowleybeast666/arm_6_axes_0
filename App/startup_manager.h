#ifndef __STARTUP_MANAGER_H
#define __STARTUP_MANAGER_H
#include "robot_control.h"
typedef enum
{
    STARTUP_BOOT,
    STARTUP_WAIT_MOTORS,
    STARTUP_CAPTURE_STOW,
    STARTUP_TO_HOME,
    STARTUP_READY,
    STARTUP_FAILED
} StartupState;
typedef enum
{
    STARTUP_POSITION_ASSUME_STOW,
    STARTUP_POSITION_ABSOLUTE_ENCODER,
    STARTUP_POSITION_HOMING_SENSOR
} StartupPositionPolicy;
typedef struct
{
    StartupState state;
    StartupPositionPolicy policy;
    uint32_t began_ms;
    ArmResult result;
} StartupManager;
void StartupManager_Init(StartupManager *startup, StartupPositionPolicy policy, uint32_t now_ms);
ArmResult StartupManager_Step(StartupManager *startup, RobotControl *control, uint32_t now_ms);
#endif
