#include "storage_service.h"
#include "track_player.h"
#include "platform_lock.h"
#include "text_format.h"
#include <string.h>
typedef struct
{
    StorageRequest request;
    StorageResponse response;
    uint32_t ticket;
    unsigned state;
} StorageSlot;
static StorageSlot slots[STORAGE_QUEUE_DEPTH];
static uint32_t next_ticket;
static StorageSnapshot published;
static uint8_t staging[1024] __attribute__((aligned(4)));
static const char *const factory_paths[3] = {"/tracks/factory/small.bin", "/tracks/factory/medium.bin",
                                             "/tracks/factory/large.bin"};
const char *Storage_GetFactoryPath(unsigned i)
{
    return i < 3 ? factory_paths[i] : NULL;
}
bool Storage_IsSafeName(const char *n)
{
    if (!n)
        return false;
    size_t len = 0;
    while (len < STORAGE_NAME_BYTES && n[len])
        ++len;
    if (len < 5 || len >= STORAGE_NAME_BYTES || strcmp(n + len - 4, ".bin"))
        return false;
    for (size_t i = 0; i < len - 4; ++i)
    {
        char c = n[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' ||
              c == '-'))
            return false;
    }
    return true;
}
static void Storage_CustomPath(char path[64], const char *name)
{
    (void)Text_Format(path, 64, "/tracks/custom/%s", name);
}
void StorageService_Init(void)
{
    memset(slots, 0, sizeof(slots));
    published = (StorageSnapshot){0};
    next_ticket = 0;
}
void StorageService_GetSnapshot(StorageSnapshot *s)
{
    if (s)
    {
        Platform_EnterCritical();
        *s = published;
        Platform_ExitCritical();
    }
}
TrackResult StorageService_Request(const StorageRequest *r, uint32_t *ticket)
{
    if (!r || !ticket || (unsigned)r->type > STORAGE_SAVE_FORCE_CAL)
        return TRACK_BAD_FORMAT;
    if ((r->type == STORAGE_LOAD_TRACK || r->type == STORAGE_DELETE_TRACK) &&
        (r->factory ? (r->factory_index >= 3 || r->type == STORAGE_DELETE_TRACK)
                    : !Storage_IsSafeName(r->name)))
        return TRACK_INVALID_NAME;
    Platform_EnterCritical();
    unsigned i;
    for (i = 0; i < STORAGE_QUEUE_DEPTH; ++i)
        if (!slots[i].state)
            break;
    if (i == STORAGE_QUEUE_DEPTH)
    {
        Platform_ExitCritical();
        return TRACK_BUSY;
    }
    TrackResult t = TRACK_OK;
    if (r->type == STORAGE_LOAD_TRACK)
        t = TrackBuffer_Transfer(TRACK_OWNER_FREE, TRACK_OWNER_STORAGE_LOAD);
    if (r->type == STORAGE_SAVE_TRACK)
        t = TrackBuffer_Transfer(TRACK_OWNER_RECORDER, TRACK_OWNER_STORAGE_SAVE);
    if (t == TRACK_OK)
    {
        slots[i] = (StorageSlot){.request = *r, .ticket = ++next_ticket, .state = 1};
        if (!slots[i].ticket)
            slots[i].ticket = ++next_ticket;
        *ticket = slots[i].ticket;
    }
    Platform_ExitCritical();
    if (t == TRACK_OK)
        Platform_NotifyStorage();
    return t;
}
TrackResult StorageService_TakeResult(uint32_t ticket, StorageResponse *r)
{
    if (!ticket || !r)
        return TRACK_BAD_FORMAT;
    Platform_EnterCritical();
    for (unsigned i = 0; i < STORAGE_QUEUE_DEPTH; ++i)
        if (slots[i].ticket == ticket && slots[i].state == 3)
        {
            *r = slots[i].response;
            slots[i].state = 0;
            Platform_ExitCritical();
            return TRACK_OK;
        }
    Platform_ExitCritical();
    return TRACK_NOT_READY;
}
static TrackResult Storage_ReadExact(uint8_t *b, size_t n)
{
    size_t actual = 0;
    TrackResult r = StorageBackend_Get()->read(b, n, &actual);
    return r == TRACK_OK && actual != n ? TRACK_BAD_FORMAT : r;
}
static TrackResult Storage_CheckEof(void)
{
    uint8_t b;
    size_t n = 0;
    TrackResult r = StorageBackend_Get()->read(&b, 1, &n);
    return r == TRACK_OK && n ? TRACK_BAD_FORMAT : r;
}
static TrackResult Storage_ReadTrack(const char *path, bool load, TrackHeader *h)
{
    const StorageBackend *b = StorageBackend_Get();
    TrackResult r = b->open(path, false);
    if (r != TRACK_OK)
        return r;
    r = Storage_ReadExact(staging, TRACK_HEADER_BYTES);
    if (r == TRACK_OK)
        r = TrackFormat_DecodeHeader(staging, h);
    uint32_t crc = UINT32_MAX;
    if (r == TRACK_OK)
        for (uint32_t i = 0; i < h->count;)
        {
            unsigned n = h->count - i;
            if (n > sizeof(staging) / TRACK_SAMPLE_BYTES)
                n = sizeof(staging) / TRACK_SAMPLE_BYTES;
            r = Storage_ReadExact(staging, n * TRACK_SAMPLE_BYTES);
            if (r != TRACK_OK)
                break;
            crc = TrackCrc_Update(crc, staging, n * TRACK_SAMPLE_BYTES);
            if (load)
                for (unsigned k = 0; k < n; ++k)
                {
                    TrackSample s;
                    TrackFormat_DecodeSample(staging + k * TRACK_SAMPLE_BYTES, &s);
                    r = TrackBuffer_Write(TRACK_OWNER_STORAGE_LOAD, i + k, &s);
                    if (r != TRACK_OK)
                        break;
                }
            if (r != TRACK_OK)
                break;
            i += n;
        }
    if (r == TRACK_OK && (crc ^ UINT32_MAX) != h->payload_crc)
        r = TRACK_BAD_CRC;
    if (r == TRACK_OK)
        r = Storage_CheckEof();
    TrackResult close = b->close();
    return r == TRACK_OK ? close : r;
}
static TrackResult Storage_CheckHeader(const char *path)
{
    const StorageBackend *b = StorageBackend_Get();
    TrackResult r = b->open(path, false);
    if (r != TRACK_OK)
        return r;
    r = Storage_ReadExact(staging, TRACK_HEADER_BYTES);
    TrackHeader h;
    if (r == TRACK_OK)
        r = TrackFormat_DecodeHeader(staging, &h);
    TrackResult c = b->close();
    return r == TRACK_OK ? c : r;
}
static TrackResult Storage_Scan(void)
{
    StorageSnapshot s = {.mounted = true};
    const StorageBackend *b = StorageBackend_Get();
    for (unsigned i = 0; i < 3; ++i)
        s.factory_available[i] = Storage_CheckHeader(factory_paths[i]) == TRACK_OK;
    TrackResult r = b->list_begin();
    if (r != TRACK_OK)
        return r;
    for (unsigned entries = 0; entries < 4096; ++entries)
    {
        char name[STORAGE_NAME_BYTES] = {0};
        bool end = false;
        r = b->list_next(name, &end);
        if (r != TRACK_OK || end)
            break;
        if (!Storage_IsSafeName(name))
            continue;
        char path[64];
        Storage_CustomPath(path, name);
        if (Storage_CheckHeader(path) != TRACK_OK)
            continue;
        if (s.count == MAX_CUSTOM_TRACKS)
        {
            s.list_full = true;
            continue;
        }
        strcpy(s.names[s.count++], name);
    }
    b->list_end();
    for (unsigned i = 0; i < s.count; ++i)
        for (unsigned j = i + 1; j < s.count; ++j)
            if (strcmp(s.names[i], s.names[j]) > 0)
            {
                char temp[STORAGE_NAME_BYTES];
                strcpy(temp, s.names[i]);
                strcpy(s.names[i], s.names[j]);
                strcpy(s.names[j], temp);
            }
    s.last_error = r != TRACK_OK ? r : (s.list_full ? TRACK_LIST_FULL : TRACK_OK);
    Platform_EnterCritical();
    s.revision = published.revision + 1;
    published = s;
    Platform_ExitCritical();
    return s.last_error;
}
static TrackResult Storage_SaveTrack(StorageResponse *response, const StorageRequest *req)
{
    const TrackSample *samples = TrackBuffer_GetSamples(TRACK_OWNER_STORAGE_SAVE);
    TrackHeader h = TrackBuffer_GetHeader();
    TrackResult r = TrackPlayer_Validate(samples, &h, &req->limits, &req->motion);
    if (r != TRACK_OK)
        return r;
    uint32_t crc = UINT32_MAX;
    for (unsigned i = 0; i < h.count; ++i)
    {
        TrackFormat_EncodeSample(&samples[i], staging);
        crc = TrackCrc_Update(crc, staging, TRACK_SAMPLE_BYTES);
    }
    h.payload_crc = crc ^ UINT32_MAX;
    const StorageBackend *b = StorageBackend_Get();
    char path[64], temp[64];
    bool found = false;
    for (unsigned i = 1; i <= 999; ++i)
    {
        (void)Text_Format(response->name, sizeof(response->name), "Custom_%03u.bin", i);
        Storage_CustomPath(path, response->name);
        strcpy(temp, path);
        strcpy(temp + strlen(temp) - 3, "tmp");
        bool exists = false;
        r = b->exists(path, &exists);
        if (r != TRACK_OK)
            return r;
        if (exists)
            continue;
        r = b->exists(temp, &exists);
        if (r != TRACK_OK)
            return r;
        if (!exists)
        {
            found = true;
            break;
        }
    }
    if (!found)
        return TRACK_LIST_FULL;
    r = b->open(temp, true);
    if (r != TRACK_OK)
        return r;
    TrackFormat_EncodeHeader(&h, staging);
    r = b->write(staging, TRACK_HEADER_BYTES);
    for (unsigned i = 0; r == TRACK_OK && i < h.count;)
    {
        unsigned n = h.count - i;
        if (n > sizeof(staging) / TRACK_SAMPLE_BYTES)
            n = sizeof(staging) / TRACK_SAMPLE_BYTES;
        for (unsigned k = 0; k < n; ++k)
            TrackFormat_EncodeSample(&samples[i + k], staging + k * TRACK_SAMPLE_BYTES);
        r = b->write(staging, n * TRACK_SAMPLE_BYTES);
        i += n;
    }
    if (r == TRACK_OK)
        r = b->sync();
    TrackResult close = b->close();
    if (r == TRACK_OK)
        r = close;
    TrackHeader verified;
    if (r == TRACK_OK)
        r = Storage_ReadTrack(temp, false, &verified);
    if (r == TRACK_OK)
        r = b->rename(temp, path);
    response->header = h;
    return r;
}
static TrackResult Storage_Calibration(const StorageRequest *q, StorageResponse *r)
{
    const StorageBackend *b = StorageBackend_Get();
    TrackResult t;
    if (q->type == STORAGE_LOAD_FORCE_CAL)
    {
        bool exists = false;
        t = b->exists("/config/force_cal.bin", &exists);
        if (t != TRACK_OK)
            return t;
        if (!exists)
            return TRACK_NOT_READY; /* Optional file absent does not invalidate a mounted card. */
        t = b->open("/config/force_cal.bin", false);
        if (t != TRACK_OK)
            return t;
        t = Storage_ReadExact(staging, FORCE_CAL_BYTES);
        if (t == TRACK_OK)
            t = Storage_CheckEof();
        TrackResult close = b->close();
        if (t == TRACK_OK)
            t = close;
        if (t == TRACK_OK && ForceCalibration_Decode(staging, &r->calibration) != ARM_OK)
            t = TRACK_BAD_FORMAT;
        return t;
    }
    ForceCalibration_Encode(&q->calibration, staging);
    ForceCalibration check;
    if (ForceCalibration_Decode(staging, &check) != ARM_OK)
        return TRACK_BAD_FORMAT;
    /* Fixed temporary file: orphan may be removed; never expose unverified data as .bin. */
    bool exists = false;
    t = b->exists("/config/force_cal.tmp", &exists);
    if (t != TRACK_OK)
        return t;
    if (exists && b->remove("/config/force_cal.tmp") != TRACK_OK)
        return TRACK_IO_ERROR;
    t = b->open("/config/force_cal.tmp", true);
    if (t != TRACK_OK)
        return t;
    t = b->write(staging, FORCE_CAL_BYTES);
    if (t == TRACK_OK)
        t = b->sync();
    TrackResult close = b->close();
    if (t == TRACK_OK)
        t = close;
    if (t != TRACK_OK)
        return t;
    t = b->open("/config/force_cal.tmp", false);
    if (t != TRACK_OK)
        return t;
    t = Storage_ReadExact(staging, FORCE_CAL_BYTES);
    if (t == TRACK_OK)
        t = Storage_CheckEof();
    close = b->close();
    if (t == TRACK_OK)
        t = close;
    if (t == TRACK_OK && ForceCalibration_Decode(staging, &check) != ARM_OK)
        t = TRACK_BAD_CRC;
    if (t != TRACK_OK)
        return t;
    t = b->exists("/config/force_cal.bin", &exists);
    if (t != TRACK_OK)
        return t;
    if (exists && b->remove("/config/force_cal.bin") != TRACK_OK)
        return TRACK_IO_ERROR;
    return b->rename("/config/force_cal.tmp", "/config/force_cal.bin");
}
bool StorageService_ProcessOne(void)
{
    unsigned i;
    Platform_EnterCritical();
    for (i = 0; i < STORAGE_QUEUE_DEPTH; ++i)
        if (slots[i].state == 1)
            break;
    if (i == STORAGE_QUEUE_DEPTH)
    {
        Platform_ExitCritical();
        return false;
    }
    slots[i].state = 2;
    Platform_ExitCritical();
    const StorageRequest *q = &slots[i].request;
    StorageResponse r = {0};
    const StorageBackend *b = StorageBackend_Get();
    if (q->type == STORAGE_MOUNT)
    {
        r.result = b->mount();
        Platform_EnterCritical();
        published.mounted = r.result == TRACK_OK;
        Platform_ExitCritical();
        if (r.result == TRACK_OK)
            r.result = Storage_Scan();
    }
    else if (!published.mounted)
        r.result = TRACK_NOT_READY;
    else if (q->type == STORAGE_SCAN_TRACKS)
        r.result = Storage_Scan();
    else if (q->type == STORAGE_LOAD_TRACK)
    {
        char path[64];
        if (q->factory)
            strcpy(path, factory_paths[q->factory_index]);
        else
            Storage_CustomPath(path, q->name);
        r.result = Storage_ReadTrack(path, true, &r.header);
        if (r.result == TRACK_OK)
            r.result = TrackPlayer_Validate(TrackBuffer_GetSamples(TRACK_OWNER_STORAGE_LOAD), &r.header,
                                            &q->limits, &q->motion);
        if (r.result == TRACK_OK)
            r.result = TrackBuffer_SetHeader(TRACK_OWNER_STORAGE_LOAD, &r.header);
    }
    else if (q->type == STORAGE_SAVE_TRACK)
    {
        r.result = Storage_SaveTrack(&r, q);
        if (r.result == TRACK_OK)
            (void)Storage_Scan();
    }
    else if (q->type == STORAGE_DELETE_TRACK)
    {
        char path[64];
        Storage_CustomPath(path, q->name);
        r.result = b->remove(path);
        if (r.result == TRACK_OK)
            (void)Storage_Scan();
    }
    else
        r.result = Storage_Calibration(q, &r);
    if (q->type == STORAGE_LOAD_TRACK)
        (void)TrackBuffer_Transfer(TRACK_OWNER_STORAGE_LOAD,
                                   r.result == TRACK_OK ? TRACK_OWNER_READY : TRACK_OWNER_FREE);
    if (q->type == STORAGE_SAVE_TRACK)
        (void)TrackBuffer_Transfer(TRACK_OWNER_STORAGE_SAVE, TRACK_OWNER_FREE);
    Platform_EnterCritical();
    slots[i].response = r;
    slots[i].state = 3;
    published.last_error = r.result;
    if (r.result == TRACK_IO_ERROR)
        published.mounted = false;
    Platform_ExitCritical();
    return true;
}
