#ifndef ARM_DGUS_PROTOCOL_H
#define ARM_DGUS_PROTOCOL_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define DGUS_MAX_BODY 96U
#define DGUS_MAX_FRAME (DGUS_MAX_BODY + 3U)
#define DGUS_FRAME_TIMEOUT_MS 50U
typedef struct
{
    uint8_t command, words;
    uint16_t vp;
    const uint8_t *data; /* Callback lifetime only; big-endian words. */
} DgusFrame;
typedef void (*DgusReceive)(void *context, const DgusFrame *frame);
typedef struct
{
    uint8_t bytes[DGUS_MAX_FRAME];
    size_t used;
    uint32_t last_byte_ms, accepted, rejected, timeouts;
    bool crc;
} DgusParser;
void DgusParser_Init(DgusParser *parser, bool crc);
void DgusParser_Reset(DgusParser *parser);
void DgusParser_Push(DgusParser *parser, uint8_t byte, uint32_t now_ms,
                     DgusReceive receive, void *context);
uint16_t Dgus_Crc16(const uint8_t *bytes, size_t count);
uint16_t Dgus_GetWord(const uint8_t *bytes);
size_t Dgus_Write(uint8_t *out, size_t capacity, bool crc, uint16_t vp,
                  const uint8_t *data, size_t bytes);
size_t Dgus_Read(uint8_t *out, size_t capacity, bool crc, uint16_t vp, uint8_t words);
#endif
