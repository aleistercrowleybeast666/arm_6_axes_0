#ifndef __ROBOT_COMMAND_H
#define __ROBOT_COMMAND_H
#include "robot_math_types.h"
typedef enum
{
    ROBOT_CMD_ENABLE,
    ROBOT_CMD_DISABLE,
    ROBOT_CMD_JOINT_MOVE,
    ROBOT_CMD_CLEAR_FAULT
} RobotCommandType;
typedef struct
{
    RobotCommandType type;
    JointVec6f target_rad;
    uint32_t sequence;
} RobotCommand;
/* Task context, nonblocking; ARM_OK means accepted to queue, not executed. */
ArmResult RobotCommand_Submit(const RobotCommand *command);
ArmResult RobotCommand_GetLastResult(uint32_t *sequence);
#endif
