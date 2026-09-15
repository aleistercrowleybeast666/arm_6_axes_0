#ifndef __TEACH_RECORDER_H
#define __TEACH_RECORDER_H
#include "teach_types.h"
ArmResult TeachRecorder_Init(TeachRecorder *recorder, const RobotLimits *limits);
ArmResult TeachRecorder_Start(TeachRecorder *recorder);
/* Caller MUST supply a coherent, fresh measured joint snapshot, never commanded q. */
ArmResult TeachRecorder_PushSample(TeachRecorder *recorder, const TeachSample *actual_feedback);
void TeachRecorder_Stop(TeachRecorder *recorder);
#endif
