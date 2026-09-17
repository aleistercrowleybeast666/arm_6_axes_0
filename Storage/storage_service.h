#ifndef __STORAGE_SERVICE_H
#define __STORAGE_SERVICE_H
#include "storage_backend.h"
#include "force_sensor.h"
#define MAX_CUSTOM_TRACKS 32U
#define STORAGE_QUEUE_DEPTH 4U
typedef enum
{
    STORAGE_MOUNT,
    STORAGE_SCAN_TRACKS,
    STORAGE_LOAD_TRACK,
    STORAGE_SAVE_TRACK,
    STORAGE_DELETE_TRACK,
    STORAGE_LOAD_FORCE_CAL,
    STORAGE_SAVE_FORCE_CAL
} StorageRequestType;
typedef struct
{
    StorageRequestType type;
    bool factory;
    unsigned factory_index;
    char name[STORAGE_NAME_BYTES];
    RobotLimits limits;
    RobotMotionLimits motion;
    ForceCalibration calibration;
} StorageRequest;
typedef struct
{
    TrackResult result;
    char name[STORAGE_NAME_BYTES];
    TrackHeader header;
    ForceCalibration calibration;
} StorageResponse;
typedef struct
{
    bool mounted, factory_available[3], list_full;
    unsigned count;
    char names[MAX_CUSTOM_TRACKS][STORAGE_NAME_BYTES];
    TrackResult last_error;
    uint32_t revision;
} StorageSnapshot;
bool Storage_IsSafeName(const char *name);
const char *Storage_GetFactoryPath(unsigned index);
void StorageService_Init(void);
TrackResult StorageService_Request(const StorageRequest *request, uint32_t *ticket);
bool StorageService_ProcessOne(void); /* StorageTask ONLY; may block. */
TrackResult StorageService_TakeResult(uint32_t ticket, StorageResponse *response);
void StorageService_GetSnapshot(StorageSnapshot *snapshot);
#endif
