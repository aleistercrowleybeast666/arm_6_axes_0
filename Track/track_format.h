#ifndef __TRACK_FORMAT_H
#define __TRACK_FORMAT_H
#include "robot_trajectory.h"
#define TRACK_MAX_SAMPLES 4096U
#define TRACK_HEADER_BYTES 40U
#define TRACK_SAMPLE_BYTES 12U
#define TRACK_RECORD_PERIOD_US 20000U
typedef enum
{
    TRACK_OK,
    TRACK_BAD_FORMAT,
    TRACK_UNSUPPORTED_VERSION,
    TRACK_BAD_CRC,
    TRACK_WRONG_MODEL,
    TRACK_BUFFER_FULL,
    TRACK_LIMIT_ERROR,
    TRACK_BUSY,
    TRACK_IO_ERROR,
    TRACK_NOT_READY,
    TRACK_INVALID_NAME,
    TRACK_LIST_FULL,
    TRACK_ABORTED
} TrackResult;
typedef struct
{
    int16_t q_cdeg[AXIS_COUNT];
} TrackSample;
typedef struct
{
    uint32_t period_us, count, model_id, payload_crc;
} TrackHeader;
uint32_t TrackCrc_Update(uint32_t state, const uint8_t *bytes, size_t length);
uint32_t TrackCrc_Compute(const uint8_t *bytes, size_t length);
uint32_t TrackFormat_GetModelId(void);
void TrackFormat_EncodeHeader(const TrackHeader *header, uint8_t bytes[TRACK_HEADER_BYTES]);
TrackResult TrackFormat_DecodeHeader(const uint8_t bytes[TRACK_HEADER_BYTES], TrackHeader *header);
void TrackFormat_EncodeSample(const TrackSample *sample, uint8_t bytes[TRACK_SAMPLE_BYTES]);
void TrackFormat_DecodeSample(const uint8_t bytes[TRACK_SAMPLE_BYTES], TrackSample *sample);
TrackResult TrackFormat_Quantize(const JointVec6f *q, TrackSample *sample);
void TrackFormat_ToJoint(const TrackSample *sample, JointVec6f *q);
#endif
