#ifndef __FORCE_BACKEND_H
#define __FORCE_BACKEND_H
#include "robot_math_types.h"
typedef struct
{
    ArmResult (*read_raw)(int32_t *raw);
} ForceBackend;
const ForceBackend *ForceBackend_Get(void);
int32_t Hx711_SignExtend(uint32_t value);
#endif
