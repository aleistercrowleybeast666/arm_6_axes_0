#include "storage_backend.h"
#include "storage_disk.h"
#include "ff.h"
#include <string.h>
static FATFS filesystem;
static FIL file;
static DIR directory;
static char volume_path[4];
static bool driver_linked;
static TrackResult FatFs_Mount(void)
{
    if (!driver_linked)
    {
        if (FATFS_LinkDriver(StorageDisk_GetDriver(), volume_path) != 0)
            return TRACK_IO_ERROR;
        driver_linked = true;
    }
    if (f_mount(&filesystem, volume_path, 1) != FR_OK)
        return TRACK_NOT_READY;
    const char *paths[] = {"/tracks", "/tracks/factory", "/tracks/custom", "/config"};
    for (unsigned i = 0; i < 4; ++i)
    {
        FRESULT r = f_mkdir(paths[i]);
        if (r != FR_OK && r != FR_EXIST)
            return TRACK_IO_ERROR;
    }
    return TRACK_OK;
}
static TrackResult FatFs_Open(const char *p, bool create)
{
    return f_open(&file, p, create ? (FA_WRITE | FA_CREATE_NEW) : FA_READ) == FR_OK ? TRACK_OK
                                                                                    : TRACK_IO_ERROR;
}
static TrackResult FatFs_Read(uint8_t *b, size_t n, size_t *actual)
{
    UINT read = 0;
    FRESULT r = f_read(&file, b, (UINT)n, &read);
    *actual = read;
    return r == FR_OK ? TRACK_OK : TRACK_IO_ERROR;
}
static TrackResult FatFs_Write(const uint8_t *b, size_t n)
{
    UINT written = 0;
    return f_write(&file, b, (UINT)n, &written) == FR_OK && written == n ? TRACK_OK : TRACK_IO_ERROR;
}
static TrackResult FatFs_Sync(void)
{
    return f_sync(&file) == FR_OK ? TRACK_OK : TRACK_IO_ERROR;
}
static TrackResult FatFs_Close(void)
{
    return f_close(&file) == FR_OK ? TRACK_OK : TRACK_IO_ERROR;
}
static TrackResult FatFs_Rename(const char *a, const char *b)
{
    return f_rename(a, b) == FR_OK ? TRACK_OK : TRACK_IO_ERROR;
}
static TrackResult FatFs_Remove(const char *p)
{
    return f_unlink(p) == FR_OK ? TRACK_OK : TRACK_IO_ERROR;
}
static TrackResult FatFs_Exists(const char *p, bool *exists)
{
    FILINFO info;
    FRESULT r = f_stat(p, &info);
    *exists = r == FR_OK;
    return r == FR_OK || r == FR_NO_FILE || r == FR_NO_PATH ? TRACK_OK : TRACK_IO_ERROR;
}
static TrackResult FatFs_ListBegin(void)
{
    return f_opendir(&directory, "/tracks/custom") == FR_OK ? TRACK_OK : TRACK_IO_ERROR;
}
static TrackResult FatFs_ListNext(char name[STORAGE_NAME_BYTES], bool *end)
{
    FILINFO info;
    FRESULT r = f_readdir(&directory, &info);
    if (r != FR_OK)
        return TRACK_IO_ERROR;
    *end = !info.fname[0];
    name[0] = 0;
    if (!(info.fattrib & AM_DIR) && strlen(info.fname) < STORAGE_NAME_BYTES)
        strcpy(name, info.fname);
    return TRACK_OK;
}
static void FatFs_ListEnd(void)
{
    (void)f_closedir(&directory);
}
const StorageBackend *StorageBackend_Get(void)
{
    static const StorageBackend b = {FatFs_Mount,  FatFs_Open,      FatFs_Read,     FatFs_Write,
                                     FatFs_Sync,   FatFs_Close,     FatFs_Rename,   FatFs_Remove,
                                     FatFs_Exists, FatFs_ListBegin, FatFs_ListNext, FatFs_ListEnd};
    return &b;
}
DWORD get_fattime(void)
{
    return ((DWORD)(2026 - 1980) << 25) | ((DWORD)1 << 21) |
           ((DWORD)1 << 16); /* No RTC: explicit fixed date. */
}
