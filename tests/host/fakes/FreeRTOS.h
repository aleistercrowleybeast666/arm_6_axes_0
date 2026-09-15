#ifndef __FREERTOS_H
#define __FREERTOS_H
#include <stdint.h>
#include <stddef.h>
typedef int BaseType_t;
typedef unsigned UBaseType_t;
typedef uint32_t TickType_t;
typedef struct
{
    uint8_t *storage;
    size_t item_size, capacity, count, read_index, write_index;
} StaticQueue_t;
#define pdPASS 1
#define pdFAIL 0
void Fake_EnterCritical(void);
void Fake_ExitCritical(void);
#define taskENTER_CRITICAL() Fake_EnterCritical()
#define taskEXIT_CRITICAL() Fake_ExitCritical()
#endif
