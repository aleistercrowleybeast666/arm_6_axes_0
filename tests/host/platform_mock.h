#ifndef __PLATFORM_MOCK_H
#define __PLATFORM_MOCK_H
#include "can_frame.h"
#include <stddef.h>
void Mock_Reset(void);
size_t Mock_GetPending(void);
const CanFrame *Mock_GetFrame(size_t index);
#endif
