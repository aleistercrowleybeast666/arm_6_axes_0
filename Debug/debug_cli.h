#ifndef __DEBUG_CLI_H
#define __DEBUG_CLI_H
#include "robot_math_types.h"
#define DEBUG_LINE_BYTES 96U
void DebugCli_Execute(const char *line, uint32_t now_ms, char *response, size_t capacity);
bool DebugCli_Poll(char *response, size_t capacity);
#endif
