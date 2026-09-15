#ifndef __CAN_FRAME_H
#define __CAN_FRAME_H
#include <stdbool.h>
#include <stdint.h>
typedef struct
{
    uint32_t ext_id;
    uint8_t dlc;
    uint8_t data[8];
    uint32_t tick;
    bool extended, remote;
} CanFrame;
#endif
