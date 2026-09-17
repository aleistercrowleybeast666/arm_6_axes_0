#ifndef __STORAGE_BACKEND_H
#define __STORAGE_BACKEND_H
#include "track_format.h"
#define STORAGE_NAME_BYTES 32U
typedef struct
{
    TrackResult (*mount)(void);
    TrackResult (*open)(const char *path, bool create_new);
    TrackResult (*read)(uint8_t *bytes, size_t count, size_t *actual);
    TrackResult (*write)(const uint8_t *bytes, size_t count);
    TrackResult (*sync)(void);
    TrackResult (*close)(void);
    TrackResult (*rename)(const char *old_path, const char *new_path);
    TrackResult (*remove)(const char *path);
    TrackResult (*exists)(const char *path, bool *exists);
    TrackResult (*list_begin)(void);
    TrackResult (*list_next)(char name[STORAGE_NAME_BYTES], bool *end);
    void (*list_end)(void);
} StorageBackend;
const StorageBackend *StorageBackend_Get(void);
#endif
