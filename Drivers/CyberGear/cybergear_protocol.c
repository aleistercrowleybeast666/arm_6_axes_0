#include "cybergear_protocol.h"
#include <float.h>
#include <math.h>
#include <string.h>
_Static_assert(sizeof(float) == 4 && FLT_RADIX == 2 && FLT_MANT_DIG == 24, "Requires IEEE binary32");
bool CyberGear_IsValidId(uint8_t id)
{
    return id > 0 && id < 0x7FU;
} /* restricted commissioning subset */
float CyberGear_GetPositionRange(CyberGearProfile p)
{
    return p == CG_PROFILE_MANUAL_4PI ? 4.0f * ROBOT_PI : (p == CG_PROFILE_LEGACY_12_5 ? 12.5f : NAN);
}
static bool Protocol_IsKnownType(CyberGearFrameType t)
{
    switch (t)
    {
    case CG_DISCOVER:
    case CG_MOTION:
    case CG_FEEDBACK:
    case CG_ENABLE:
    case CG_DISABLE:
    case CG_SET_ZERO:
    case CG_SET_ID:
    case CG_READ_PARAM:
    case CG_WRITE_PARAM:
    case CG_FAULT_FEEDBACK:
        return true;
    default:
        return false;
    }
}
ArmResult CyberGear_EncodeId(CyberGearFrameType t, uint16_t data, uint8_t dest, uint32_t *id)
{
    if (id == NULL || !Protocol_IsKnownType(t))
        return ARM_INVALID_ARGUMENT;
    *id = ((uint32_t)t << 24) | ((uint32_t)data << 8) | dest;
    return ARM_OK;
}
ArmResult CyberGear_DecodeId(uint32_t id, CyberGearFrameType *t, uint16_t *data, uint8_t *dest)
{
    if (t == NULL || data == NULL || dest == NULL || id > 0x1FFFFFFFU)
        return ARM_INVALID_ARGUMENT;
    CyberGearFrameType type = (CyberGearFrameType)((id >> 24) & 0x1FU);
    if (!Protocol_IsKnownType(type))
        return ARM_INVALID_ARGUMENT;
    *t = type;
    *data = (uint16_t)(id >> 8);
    *dest = (uint8_t)id;
    return ARM_OK;
}
static ArmResult Protocol_InitFrame(uint8_t motor, uint8_t host, CyberGearFrameType type, CanFrame *f)
{
    if (f == NULL || !CyberGear_IsValidId(motor) || host > 0x7FU || motor == host)
        return ARM_INVALID_ARGUMENT;
    *f = (CanFrame){.dlc = 8, .extended = true};
    return CyberGear_EncodeId(type, host, motor, &f->ext_id);
}
ArmResult CyberGear_EncodeSimple(uint8_t motor, uint8_t host, CyberGearFrameType t, CanFrame *f)
{
    if (t != CG_ENABLE && t != CG_DISABLE && t != CG_SET_ZERO && t != CG_DISCOVER)
        return ARM_INVALID_ARGUMENT;
    ArmResult r = Protocol_InitFrame(motor, host, t, f);
    if (r == ARM_OK && t == CG_SET_ZERO)
        f->data[0] = 1;
    return r;
}
static uint16_t Protocol_MapFloat(float v, float lo, float hi)
{
    return (uint16_t)((RobotMath_Clamp(v, lo, hi) - lo) * 65535.0f / (hi - lo));
}
static void Protocol_WriteBe16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)v;
}
static uint16_t Protocol_ReadBe16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}
static float Protocol_Unmap(uint16_t v, float lo, float hi)
{
    return lo + (float)v * (hi - lo) / 65535.0f;
}
ArmResult CyberGear_EncodeMotion(uint8_t motor, CyberGearProfile profile, const CyberGearCommand *c,
                                 CanFrame *f)
{
    float range = CyberGear_GetPositionRange(profile);
    if (c == NULL || f == NULL || !CyberGear_IsValidId(motor) || !isfinite(range) ||
        !isfinite(c->position_rad) || !isfinite(c->velocity_rad_s) || !isfinite(c->torque_nm) ||
        !isfinite(c->kp) || !isfinite(c->kd) || !isfinite(c->current_a))
        return ARM_INVALID_ARGUMENT;
    /* Motion frames carry torque, NOT a separate current setpoint. */
    if (c->current_a != 0)
        return ARM_INVALID_ARGUMENT;
    *f = (CanFrame){.dlc = 8, .extended = true};
    (void)CyberGear_EncodeId(CG_MOTION, Protocol_MapFloat(c->torque_nm, -12, 12), motor, &f->ext_id);
    Protocol_WriteBe16(f->data, Protocol_MapFloat(c->position_rad, -range, range));
    Protocol_WriteBe16(f->data + 2, Protocol_MapFloat(c->velocity_rad_s, -30, 30));
    Protocol_WriteBe16(f->data + 4, Protocol_MapFloat(c->kp, 0, 500));
    Protocol_WriteBe16(f->data + 6, Protocol_MapFloat(c->kd, 0, 5));
    return ARM_OK;
}
ArmResult CyberGear_EncodeParameter(uint8_t motor, uint8_t host, CyberGearParameter p, float v, CanFrame *f)
{
    if (!isfinite(v))
        return ARM_INVALID_ARGUMENT;
    float lo = 0, hi = 0;
    switch (p)
    {
    case CG_PARAM_RUN_MODE:
        if (v < 0 || v > 3 || floorf(v) != v)
            return ARM_INVALID_ARGUMENT;
        break;
    case CG_PARAM_TORQUE_LIMIT:
        hi = CYBERGEAR_MAX_TORQUE_NM;
        break;
    case CG_PARAM_VELOCITY_LIMIT:
        hi = CYBERGEAR_MAX_VELOCITY_RAD_S;
        break;
    case CG_PARAM_CURRENT_LIMIT:
        hi = CYBERGEAR_MAX_CURRENT_A;
        break;
    case CG_PARAM_CURRENT:
        hi = CYBERGEAR_MAX_CURRENT_A;
        lo = -hi;
        break;
    case CG_PARAM_VELOCITY:
        hi = CYBERGEAR_MAX_VELOCITY_RAD_S;
        lo = -hi;
        break;
    case CG_PARAM_POSITION:
        hi = 4.0f * ROBOT_PI;
        lo = -hi;
        break;
    default:
        return ARM_INVALID_ARGUMENT;
    }
    ArmResult r = Protocol_InitFrame(motor, host, CG_WRITE_PARAM, f);
    if (r != ARM_OK)
        return r;
    f->data[0] = (uint8_t)p;
    f->data[1] = (uint8_t)((uint16_t)p >> 8);
    if (p == CG_PARAM_RUN_MODE)
        f->data[4] = (uint8_t)v;
    else
    {
        v = RobotMath_Clamp(v, lo, hi);
        uint32_t bits;
        memcpy(&bits, &v, sizeof(bits));
        for (unsigned i = 0; i < 4; ++i)
            f->data[4 + i] = (uint8_t)(bits >> (8U * i));
    }
    return ARM_OK;
}
ArmResult CyberGear_DecodeFeedback(const CanFrame *f, uint8_t motor, uint8_t host, CyberGearProfile profile,
                                   CyberGearState *s)
{
    float range = CyberGear_GetPositionRange(profile);
    if (f == NULL || s == NULL || !CyberGear_IsValidId(motor) || !isfinite(range) || f->dlc != 8 ||
        !f->extended || f->remote)
        return ARM_INVALID_ARGUMENT;
    CyberGearFrameType t;
    uint16_t data;
    uint8_t dest;
    if (CyberGear_DecodeId(f->ext_id, &t, &data, &dest) != ARM_OK || t != CG_FEEDBACK || dest != host ||
        (uint8_t)data != motor)
        return ARM_INVALID_ARGUMENT;
    uint8_t mode = (uint8_t)(data >> 14);
    float temperature = (float)Protocol_ReadBe16(f->data + 6) * 0.1f;
    if (mode > 2 || temperature > 200.0f)
        return ARM_INVALID_ARGUMENT; /* plausibility, not operating safety limit */
    *s = (CyberGearState){.position_rad = Protocol_Unmap(Protocol_ReadBe16(f->data), -range, range),
                          .velocity_rad_s = Protocol_Unmap(Protocol_ReadBe16(f->data + 2), -30, 30),
                          .torque_nm = Protocol_Unmap(Protocol_ReadBe16(f->data + 4), -12, 12),
                          .temperature_c = temperature,
                          .fault = (uint8_t)((data >> 8) & 0x3FU),
                          .mode = mode,
                          .last_rx_tick = f->tick,
                          .online = true,
                          .enabled = mode == 2};
    return ARM_OK;
}
