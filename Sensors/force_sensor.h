#ifndef __FORCE_SENSOR_H
#define __FORCE_SENSOR_H
#include "robot_math_types.h"
#define HX711_EXPECTED_RATE_HZ 80U
#define FORCE_STALE_TIMEOUT_MS 150U
#define FORCE_CAL_BYTES 40U
typedef struct
{
    int32_t zero_raw;
    float newton_per_count, full_scale_n, overload_n;
    uint32_t sample_rate_hz;
    bool valid;
} ForceCalibration;
typedef struct
{
    int32_t raw;
    float force_n;
    uint32_t timestamp_ms;
    bool valid, calibrated, contact, overload, stale;
} ForceSnapshot;
typedef struct
{
    float on_n, off_n;
    uint32_t stable_samples;
    bool configured;
} ContactConfig;
typedef struct
{
    ContactConfig config;
    uint32_t count;
    bool contact;
} ContactDetector;
typedef enum
{
    FORCE_CONDITION_NONE,
    FORCE_CONDITION_CONTACT,
    FORCE_CONDITION_RELEASE,
    FORCE_CONDITION_STABLE,
    FORCE_CONDITION_RIPPLE_BELOW,
    FORCE_CONDITION_RESERVED
} ForceConditionType;
typedef struct
{
    ForceConditionType type;
    float threshold_n, hysteresis_n;
    uint32_t stable_ms, timeout_ms;
    bool configured;
} ForceCondition;
typedef struct
{
    bool candidate;
    uint32_t started_ms, since_ms;
} ForceConditionState;
ArmResult ContactDetector_Configure(ContactDetector *detector, const ContactConfig *config);
bool ContactDetector_Update(ContactDetector *detector, float force_n);
ArmResult ForceCondition_Evaluate(const ForceCondition *condition, ForceConditionState *state,
                                  const ForceSnapshot *snapshot, uint32_t now_ms);
void ForceSensor_Init(void);
void ForceSensor_PushRaw(int32_t raw, uint32_t now_ms);
void ForceSensor_GetSnapshot(uint32_t now_ms, ForceSnapshot *snapshot);
ArmResult ForceSensor_SetCalibration(const ForceCalibration *calibration);
void ForceSensor_GetCalibration(ForceCalibration *calibration);
ArmResult ForceSensor_ConfigureContact(const ContactConfig *config);
ArmResult ForceSensor_Tare(uint32_t now_ms);
ArmResult ForceSensor_Calibrate(float known_n, uint32_t now_ms);
void ForceCalibration_Encode(const ForceCalibration *calibration, uint8_t bytes[FORCE_CAL_BYTES]);
ArmResult ForceCalibration_Decode(const uint8_t bytes[FORCE_CAL_BYTES], ForceCalibration *calibration);
#endif
