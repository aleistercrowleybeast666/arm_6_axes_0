#ifndef __TEACH_TYPES_H
#define __TEACH_TYPES_H
#include "robot_trajectory.h"
#define TEACH_CAPACITY 64U
typedef struct
{
    uint32_t time_ms;
    JointVec6f q_rad;
} TeachSample;
typedef struct
{
    TeachSample samples[TEACH_CAPACITY];
    size_t count;
    bool recording;
    RobotLimits limits;
} TeachRecorder;
typedef struct
{
    TeachSample samples[TEACH_CAPACITY];
    size_t count, segment;
    float elapsed_s;
    bool active;
    RobotLimits limits;
    RobotMotionLimits motion;
} TeachPlayer;
#endif
