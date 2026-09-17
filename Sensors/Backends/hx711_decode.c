#include "force_backend.h"
int32_t Hx711_SignExtend(uint32_t v)
{
    v &= 0xFFFFFFU;
    return v & 0x800000U ? (int32_t)v - 16777216 : (int32_t)v;
}
