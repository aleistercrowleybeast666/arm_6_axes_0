#include "track_buffer.h"
#include "platform_lock.h"
/* Explicitly initialized by the writer. NOLOAD; never a DMA destination. */
static TrackSample arena[TRACK_MAX_SAMPLES] __attribute__((section(".ccmram"), aligned(4)));
_Static_assert(sizeof(arena) == 49152, "CCM arena size");
static TrackOwner current_owner;
static TrackHeader metadata;
TrackOwner TrackBuffer_GetOwner(void)
{
    Platform_EnterCritical();
    TrackOwner o = current_owner;
    Platform_ExitCritical();
    return o;
}
TrackResult TrackBuffer_Transfer(TrackOwner expected, TrackOwner next)
{
    bool allowed = next == TRACK_OWNER_FREE ||
                   (expected == TRACK_OWNER_FREE &&
                    (next == TRACK_OWNER_RECORDER || next == TRACK_OWNER_STORAGE_LOAD)) ||
                   (expected == TRACK_OWNER_STORAGE_LOAD && next == TRACK_OWNER_READY) ||
                   (expected == TRACK_OWNER_READY && next == TRACK_OWNER_PLAYER) ||
                   (expected == TRACK_OWNER_RECORDER && next == TRACK_OWNER_STORAGE_SAVE);
    if (!allowed)
        return TRACK_BUSY;
    Platform_EnterCritical();
    TrackResult r = TRACK_BUSY;
    if (current_owner == expected)
    {
        current_owner = next;
        r = TRACK_OK;
        if (next == TRACK_OWNER_FREE)
            metadata = (TrackHeader){0};
    }
    Platform_ExitCritical();
    return r;
}
TrackResult TrackBuffer_Write(TrackOwner o, uint32_t i, const TrackSample *s)
{
    if (!s || i >= TRACK_MAX_SAMPLES)
        return TRACK_BUFFER_FULL;
    if (o != TRACK_OWNER_RECORDER && o != TRACK_OWNER_STORAGE_LOAD)
        return TRACK_BUSY;
    if (TrackBuffer_GetOwner() != o)
        return TRACK_BUSY;
    arena[i] = *s;
    return TRACK_OK;
}
const TrackSample *TrackBuffer_GetSamples(TrackOwner o)
{
    return TrackBuffer_GetOwner() == o ? arena : NULL;
}
TrackHeader TrackBuffer_GetHeader(void)
{
    Platform_EnterCritical();
    TrackHeader h = metadata;
    Platform_ExitCritical();
    return h;
}
TrackResult TrackBuffer_SetHeader(TrackOwner o, const TrackHeader *h)
{
    if (!h || h->count > TRACK_MAX_SAMPLES)
        return TRACK_BAD_FORMAT;
    Platform_EnterCritical();
    TrackResult r = TRACK_BUSY;
    if (current_owner == o && (o == TRACK_OWNER_RECORDER || o == TRACK_OWNER_STORAGE_LOAD))
    {
        metadata = *h;
        r = TRACK_OK;
    }
    Platform_ExitCritical();
    return r;
}
