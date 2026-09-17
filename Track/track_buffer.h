#ifndef __TRACK_BUFFER_H
#define __TRACK_BUFFER_H
#include "track_format.h"
typedef enum
{
    TRACK_OWNER_FREE,
    TRACK_OWNER_STORAGE_LOAD,
    TRACK_OWNER_READY,
    TRACK_OWNER_PLAYER,
    TRACK_OWNER_RECORDER,
    TRACK_OWNER_STORAGE_SAVE
} TrackOwner;
TrackOwner TrackBuffer_GetOwner(void);
TrackResult TrackBuffer_Transfer(TrackOwner expected, TrackOwner next);
TrackResult TrackBuffer_Write(TrackOwner owner, uint32_t index, const TrackSample *sample);
const TrackSample *TrackBuffer_GetSamples(TrackOwner owner);
TrackHeader TrackBuffer_GetHeader(void);
TrackResult TrackBuffer_SetHeader(TrackOwner owner, const TrackHeader *header);
#endif
