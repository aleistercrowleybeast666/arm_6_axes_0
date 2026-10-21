#include "dgus_protocol.h"
#include <string.h>
uint16_t Dgus_GetWord(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] << 8 | p[1]);
}
uint16_t Dgus_Crc16(const uint8_t *p, size_t n)
{
    uint16_t crc = 0xFFFFU;
    for (size_t i = 0; i < n; ++i)
    {
        crc ^= p[i];
        for (unsigned bit = 0; bit < 8; ++bit)
            crc = (uint16_t)((crc >> 1) ^ ((crc & 1U) ? 0xA001U : 0U));
    }
    return crc;
}
void DgusParser_Init(DgusParser *p, bool crc)
{
    if (p)
        *p = (DgusParser){.crc = crc};
}
void DgusParser_Reset(DgusParser *p)
{
    if (p)
        p->used = 0;
}
static void Dgus_Drop(DgusParser *p, size_t n)
{
    p->used -= n;
    memmove(p->bytes, p->bytes + n, p->used);
}
void DgusParser_Push(DgusParser *p, uint8_t byte, uint32_t now, DgusReceive receive, void *context)
{
    if (!p)
        return;
    if (p->used && (uint32_t)(now - p->last_byte_ms) >= DGUS_FRAME_TIMEOUT_MS)
    {
        ++p->timeouts;
        p->used = 0;
    }
    p->last_byte_ms = now;
    if (p->used == sizeof(p->bytes))
    {
        ++p->rejected;
        p->used = 0;
    }
    p->bytes[p->used++] = byte;
    while (p->used)
    {
        if (p->bytes[0] != 0x5A || (p->used >= 2 && p->bytes[1] != 0xA5))
        {
            Dgus_Drop(p, 1);
            continue;
        }
        if (p->used < 3)
            return;
        size_t body = p->bytes[2], overhead = p->crc ? 2U : 0U;
        if (body < 3U + overhead || body > DGUS_MAX_BODY)
        {
            ++p->rejected;
            Dgus_Drop(p, 1);
            continue;
        }
        size_t total = body + 3U;
        if (p->used < total)
            return;
        size_t payload = body - overhead;
        const uint8_t *b = p->bytes + 3;
        bool valid = true;
        if (p->crc)
        {
            uint16_t crc = Dgus_Crc16(b, payload);
            valid = b[payload] == (uint8_t)crc && b[payload + 1] == (uint8_t)(crc >> 8);
        }
        DgusFrame frame = {.command = b[0]};
        if (valid && b[0] == 0x82)
            valid = payload == 3 && b[1] == 'O' && b[2] == 'K';
        else if (valid && b[0] == 0x83 && payload >= 6)
        {
            frame.vp = Dgus_GetWord(b + 1);
            frame.words = b[3];
            frame.data = b + 4;
            valid = frame.words && payload == 4U + 2U * frame.words &&
                    (uint32_t)frame.vp + frame.words <= 0x10000UL;
        }
        else
            valid = false;
        if (valid)
        {
            ++p->accepted;
            if (receive)
                receive(context, &frame);
            Dgus_Drop(p, total);
        }
        else
        {
            ++p->rejected;
            Dgus_Drop(p, 1); /* Search the bounded buffer for the next header. */
        }
    }
}
static size_t Dgus_Finish(uint8_t *out, size_t payload, bool crc)
{
    out[0] = 0x5A;
    out[1] = 0xA5;
    out[2] = (uint8_t)(payload + (crc ? 2U : 0U));
    if (crc)
    {
        uint16_t value = Dgus_Crc16(out + 3, payload);
        out[3 + payload] = (uint8_t)value;
        out[4 + payload] = (uint8_t)(value >> 8);
    }
    return 3U + out[2];
}
size_t Dgus_Write(uint8_t *out, size_t cap, bool crc, uint16_t vp, const uint8_t *data, size_t n)
{
    size_t overhead = crc ? 2U : 0U;
    if (!out || !data || !n || n % 2 || n > DGUS_MAX_BODY - 3U - overhead ||
        cap < n + 6U + overhead || (uint32_t)vp + n / 2U > 0x10000UL)
        return 0;
    out[3] = 0x82;
    out[4] = (uint8_t)(vp >> 8);
    out[5] = (uint8_t)vp;
    memcpy(out + 6, data, n);
    return Dgus_Finish(out, n + 3U, crc);
}
size_t Dgus_Read(uint8_t *out, size_t cap, bool crc, uint16_t vp, uint8_t words)
{
    if (!out || !words || words > (DGUS_MAX_BODY - 4U - (crc ? 2U : 0U)) / 2U ||
        cap < 7U + (crc ? 2U : 0U) || (uint32_t)vp + words > 0x10000UL)
        return 0;
    out[3] = 0x83;
    out[4] = (uint8_t)(vp >> 8);
    out[5] = (uint8_t)vp;
    out[6] = words;
    return Dgus_Finish(out, 4, crc);
}
