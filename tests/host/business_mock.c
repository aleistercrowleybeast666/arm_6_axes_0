#include "app_snapshot.h"
#include "business_mock.h"
#include "bsp_can.h"
#include <string.h>
#define MOCK_FILE_COUNT 48U
#define MOCK_FILE_BYTES 50000U
typedef struct
{
    char path[64];
    uint8_t bytes[MOCK_FILE_BYTES];
    size_t length;
    bool used;
} MockFile;
static MockFile files[MOCK_FILE_COUNT];
static int opened = -1, fail_after = -1;
static size_t offset;
static unsigned list_index, writes;
static bool mounted = true, armed, online = true, teach_allowed = true, teaching;
static MotorSnapshot motor;
static AppSnapshot app;
static bool Mock_Fail(void)
{
    if (fail_after < 0)
        return false;
    if (fail_after == 0)
    {
        fail_after = -1;
        return true;
    }
    --fail_after;
    return false;
}
void BusinessMock_FailAfter(int n)
{
    fail_after = n;
}
static int Mock_Find(const char *path)
{
    for (unsigned i = 0; i < MOCK_FILE_COUNT; ++i)
        if (files[i].used && !strcmp(path, files[i].path))
            return (int)i;
    return -1;
}
bool BusinessMock_FileExists(const char *p)
{
    return Mock_Find(p) >= 0;
}
TrackResult BusinessMock_PutFile(const char *p, const uint8_t *b, size_t n)
{
    if (n > MOCK_FILE_BYTES || strlen(p) >= 64)
        return TRACK_BAD_FORMAT;
    int i = Mock_Find(p);
    if (i < 0)
        for (unsigned k = 0; k < MOCK_FILE_COUNT; ++k)
            if (!files[k].used)
            {
                i = (int)k;
                break;
            }
    if (i < 0)
        return TRACK_LIST_FULL;
    strcpy(files[i].path, p);
    memcpy(files[i].bytes, b, n);
    files[i].length = n;
    files[i].used = true;
    return TRACK_OK;
}
void BusinessMock_Reset(void)
{
    memset(files, 0, sizeof(files));
    opened = -1;
    fail_after = -1;
    writes = 0;
    mounted = online = teach_allowed = true;
    armed = teaching = false;
    motor = (MotorSnapshot){0};
    app = (AppSnapshot){0};
}
void BusinessMock_SetApp(const AppSnapshot *s)
{
    app = *s;
}
void App_GetSnapshot(AppSnapshot *s)
{
    *s = app;
}
void BspCan_GetDiagnostics(BspCanDiagnostics *d)
{
    *d = (BspCanDiagnostics){0};
}
void BusinessMock_SetMounted(bool v)
{
    mounted = v;
}
void BusinessMock_SetTeachAllowed(bool v)
{
    teach_allowed = v;
}
void BusinessMock_SetOnline(bool v)
{
    online = v;
}
unsigned BusinessMock_GetWrites(void)
{
    return writes;
}
void BusinessMock_Tick(uint32_t now)
{
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        motor.axes[i].last_rx_tick = now;
        motor.axes[i].online = online;
    }
}
void BusinessMock_SetJoint(const JointVec6f *q)
{
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        motor.axes[i].position_rad = q->q[i];
}
static ArmResult Mock_Init(const MotorAxisConfig c[AXIS_COUNT])
{
    motor = (MotorSnapshot){.configured = true, .coordinates_ready = true};
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        motor.axes[i].motor_id = c[i].motor_id;
    return ARM_OK;
}
static ArmResult Mock_Feedback(const CanFrame *f)
{
    (void)f;
    return ARM_OK;
}
static void Mock_Snapshot(uint32_t now, MotorSnapshot *s)
{
    *s = motor;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        s->axes[i].online = online && (uint32_t)(now - s->axes[i].last_rx_tick) <= 100;
}
static ArmResult Mock_Enable(uint32_t now)
{
    (void)now;
    if (!armed)
        return ARM_OUTPUT_DISABLED;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        motor.axes[i].enabled = true;
    return ARM_OK;
}
static void Mock_Disable(void)
{
    armed = false;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        motor.axes[i].enabled = false;
}
static ArmResult Mock_Command(const RobotTrajectorySample *s, uint32_t now)
{
    (void)now;
    if (!armed)
        return ARM_OUTPUT_DISABLED;
    if (!teaching)
        for (unsigned i = 0; i < AXIS_COUNT; ++i)
        {
            motor.axes[i].position_rad = s->q.q[i];
            motor.axes[i].velocity_rad_s = s->dq_rad_s.q[i];
        }
    return ARM_OK;
}
static ArmResult Mock_EnterTeach(void)
{
    if (!teach_allowed)
        return ARM_NOT_CONFIGURED;
    teaching = true;
    return ARM_OK;
}
static ArmResult Mock_ExitTeach(void)
{
    teaching = false;
    return ARM_OK;
}
static bool Mock_Online(uint32_t now)
{
    (void)now;
    return online;
}
static uint32_t Mock_Fault(uint32_t now)
{
    (void)now;
    return 0;
}
static ArmResult Mock_Arm(void)
{
    armed = true;
    return ARM_OK;
}
static bool Mock_IsArmed(void)
{
    return armed;
}
static bool Mock_BusFault(void)
{
    return false;
}
const MotorBackend *MotorBackend_Get(void)
{
    static const MotorBackend b = {Mock_Init,    Mock_Feedback,   Mock_Snapshot,  Mock_Enable, Mock_Disable,
                                   Mock_Command, Mock_EnterTeach, Mock_ExitTeach, Mock_Online, Mock_Fault,
                                   Mock_Arm,     Mock_IsArmed,    Mock_BusFault};
    return &b;
}
static TrackResult Mock_Mount(void)
{
    return mounted ? TRACK_OK : TRACK_NOT_READY;
}
static TrackResult Mock_Open(const char *p, bool create)
{
    if (Mock_Fail())
        return TRACK_IO_ERROR;
    int i = Mock_Find(p);
    if (create)
    {
        if (i >= 0)
            return TRACK_IO_ERROR;
        TrackResult r = BusinessMock_PutFile(p, (const uint8_t *)"", 0);
        if (r != TRACK_OK)
            return r;
        i = Mock_Find(p);
    }
    if (i < 0)
        return TRACK_IO_ERROR;
    opened = i;
    offset = 0;
    return TRACK_OK;
}
static TrackResult Mock_Read(uint8_t *b, size_t n, size_t *actual)
{
    if (opened < 0 || Mock_Fail())
        return TRACK_IO_ERROR;
    MockFile *f = &files[opened];
    *actual = f->length - offset;
    if (*actual > n)
        *actual = n;
    memcpy(b, f->bytes + offset, *actual);
    offset += *actual;
    return TRACK_OK;
}
static TrackResult Mock_Write(const uint8_t *b, size_t n)
{
    if (opened < 0 || Mock_Fail() || offset + n > MOCK_FILE_BYTES)
        return TRACK_IO_ERROR;
    memcpy(files[opened].bytes + offset, b, n);
    offset += n;
    files[opened].length = offset;
    ++writes;
    return TRACK_OK;
}
static TrackResult Mock_Sync(void)
{
    return Mock_Fail() ? TRACK_IO_ERROR : TRACK_OK;
}
static TrackResult Mock_Close(void)
{
    opened = -1;
    return Mock_Fail() ? TRACK_IO_ERROR : TRACK_OK;
}
static TrackResult Mock_Rename(const char *a, const char *b)
{
    if (Mock_Fail())
        return TRACK_IO_ERROR;
    int i = Mock_Find(a);
    if (i < 0 || Mock_Find(b) >= 0)
        return TRACK_IO_ERROR;
    strcpy(files[i].path, b);
    return TRACK_OK;
}
static TrackResult Mock_Remove(const char *p)
{
    if (Mock_Fail())
        return TRACK_IO_ERROR;
    int i = Mock_Find(p);
    if (i < 0)
        return TRACK_IO_ERROR;
    files[i].used = false;
    return TRACK_OK;
}
static TrackResult Mock_Exists(const char *p, bool *v)
{
    if (Mock_Fail())
        return TRACK_IO_ERROR;
    *v = Mock_Find(p) >= 0;
    return TRACK_OK;
}
static TrackResult Mock_ListBegin(void)
{
    list_index = 0;
    return TRACK_OK;
}
static TrackResult Mock_ListNext(char name[STORAGE_NAME_BYTES], bool *end)
{
    *end = false;
    while (list_index < MOCK_FILE_COUNT)
    {
        const MockFile *f = &files[list_index++];
        if (f->used && !strncmp(f->path, "/tracks/custom/", 15))
        {
            strcpy(name, f->path + 15);
            return TRACK_OK;
        }
    }
    *end = true;
    return TRACK_OK;
}
static void Mock_ListEnd(void)
{
}
const StorageBackend *StorageBackend_Get(void)
{
    static const StorageBackend b = {Mock_Mount,  Mock_Open,      Mock_Read,     Mock_Write,
                                     Mock_Sync,   Mock_Close,     Mock_Rename,   Mock_Remove,
                                     Mock_Exists, Mock_ListBegin, Mock_ListNext, Mock_ListEnd};
    return &b;
}
