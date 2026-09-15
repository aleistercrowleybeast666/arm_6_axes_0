#ifndef __QUEUE_H
#define __QUEUE_H
#include "FreeRTOS.h"
typedef StaticQueue_t *QueueHandle_t;
QueueHandle_t xQueueCreateStatic(UBaseType_t capacity, UBaseType_t item_size, uint8_t *storage,
                                 StaticQueue_t *queue);
BaseType_t xQueueReset(QueueHandle_t queue);
UBaseType_t uxQueueSpacesAvailable(QueueHandle_t queue);
BaseType_t xQueueSend(QueueHandle_t queue, const void *item, TickType_t wait);
BaseType_t xQueueReceive(QueueHandle_t queue, void *item, TickType_t wait);
BaseType_t xQueuePeek(QueueHandle_t queue, void *item, TickType_t wait);
#endif
