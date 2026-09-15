#ifndef __CYBERGEAR_PROTOCOL_H
#define __CYBERGEAR_PROTOCOL_H
#include "cybergear_types.h"
bool CyberGear_IsValidId(uint8_t motor_id);
float CyberGear_GetPositionRange(CyberGearProfile profile);
ArmResult CyberGear_EncodeId(CyberGearFrameType type, uint16_t data, uint8_t destination, uint32_t *id);
ArmResult CyberGear_DecodeId(uint32_t id, CyberGearFrameType *type, uint16_t *data, uint8_t *destination);
ArmResult CyberGear_EncodeSimple(uint8_t motor, uint8_t host, CyberGearFrameType type, CanFrame *frame);
ArmResult CyberGear_EncodeMotion(uint8_t motor, CyberGearProfile profile, const CyberGearCommand *command,
                                 CanFrame *frame);
ArmResult CyberGear_EncodeParameter(uint8_t motor, uint8_t host, CyberGearParameter parameter, float value,
                                    CanFrame *frame);
ArmResult CyberGear_DecodeFeedback(const CanFrame *frame, uint8_t motor, uint8_t host,
                                   CyberGearProfile profile, CyberGearState *state);
#endif
