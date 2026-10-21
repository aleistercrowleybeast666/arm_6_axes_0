#include "hmi_dgus.h"
#include "business_mock.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned received;
static uint16_t received_vp, received_word;
static uint8_t frames[5][DGUS_MAX_FRAME];
static size_t lengths[5];
static unsigned written, fail_at;
static void Receive(void *context, const DgusFrame *f)
{
    assert(context == &received);
    ++received;
    if (f->command == 0x83)
    {
        received_vp = f->vp;
        received_word = Dgus_GetWord(f->data);
    }
}
static bool Write(void *context, const uint8_t *bytes, size_t count)
{
    (void)context;
    assert(count <= DGUS_MAX_FRAME);
    if (written == fail_at)
        return false;
    assert(written < 5);
    lengths[written] = count;
    memcpy(frames[written++], bytes, count);
    return true;
}
static size_t Upload(uint8_t *out, bool crc, uint16_t vp, uint16_t token, uint16_t key)
{
    uint8_t bytes[] = {0x5A, 0xA5, 8, 0x83, (uint8_t)(vp >> 8), (uint8_t)vp, 2,
                       (uint8_t)(token >> 8), (uint8_t)token, (uint8_t)(key >> 8), (uint8_t)key};
    memcpy(out, bytes, sizeof(bytes));
    if (!crc)
        return sizeof(bytes);
    out[2] += 2;
    uint16_t value = Dgus_Crc16(out + 3, sizeof(bytes) - 3);
    out[sizeof(bytes)] = (uint8_t)value;
    out[sizeof(bytes) + 1] = (uint8_t)(value >> 8);
    return sizeof(bytes) + 2;
}
static void Push(DgusParser *p, const uint8_t *bytes, size_t n, uint32_t now)
{
    for (size_t i = 0; i < n; ++i)
        DgusParser_Push(p, bytes[i], now, Receive, &received);
}
static void HmiPush(HmiDgus *h, uint16_t vp, uint16_t token, uint16_t key)
{
    uint8_t bytes[16];
    size_t n = Upload(bytes, h->parser.crc, vp, token, key);
    for (size_t i = 0; i < n; ++i)
        HmiDgus_Push(h, bytes[i], 10);
}
static void Protocol(bool crc)
{
    DgusParser p;
    DgusParser_Init(&p, crc);
    uint8_t data[DGUS_MAX_FRAME];
    size_t n = Upload(data, crc, HMI_VP_EVENT, 0x5AA5, HMI_KEY_CONFIRM);
    received = 0;
    const uint8_t noise[] = {0xA5, 0, 0x5A, 0x5A, 0xA5, 0, 0x5A};
    Push(&p, noise, sizeof(noise), 0);
    for (size_t i = 0; i < n; ++i)
    {
        Push(&p, data + i, 1, 10);
        assert(received == (i + 1 == n ? 1U : 0U));
    }
    assert(received_vp == HMI_VP_EVENT && received_word == 0x5AA5);
    Push(&p, data, n, 11);
    Push(&p, data, n, 11);
    assert(received == 3); /* Concatenated frames. */
    if (crc)
    {
        data[n - 1] ^= 1;
        Push(&p, data, n, 12);
        assert(received == 3);
        data[n - 1] ^= 1;
    }
    uint8_t bad[16];
    size_t bad_n = Upload(bad, crc, HMI_VP_EVENT, 1, 1);
    bad[6] = 3; /* Valid CRC but inconsistent advertised word count. */
    if (crc)
    {
        uint16_t value = Dgus_Crc16(bad + 3, bad_n - 5);
        bad[bad_n - 2] = (uint8_t)value;
        bad[bad_n - 1] = (uint8_t)(value >> 8);
    }
    Push(&p, bad, bad_n, 13);
    Push(&p, data, n, 14);
    assert(received == 4 && p.rejected);
    const uint8_t partial[] = {0x5A, 0xA5, 90, 0x83};
    Push(&p, partial, sizeof(partial), UINT32_MAX - 20U);
    Push(&p, data, n, 40); /* Unsigned wrap-safe timeout. */
    assert(received == 5 && p.timeouts);
    DgusParser_Reset(&p);
    uint8_t ack[] = {0x5A, 0xA5, 3, 0x82, 'O', 'K', 0xA5, 0xEF};
    ack[2] = crc ? 5 : 3;
    Push(&p, ack, crc ? 8 : 6, 41);
    assert(received == 6);
    size_t length = Dgus_Read(data, sizeof(data), crc, 0x000F, 1);
    const uint8_t reference[] = {0x5A, 0xA5, 6, 0x83, 0, 0x0F, 1, 0xED, 0x90};
    if (crc)
        assert(length == sizeof(reference) && !memcmp(data, reference, length));
    else
        assert(length == 7 && data[2] == 4);
    assert(!Dgus_Read(data, 2, crc, 0, 1));
    assert(!Dgus_Read(data, sizeof(data), crc, 0xFFFF, 2));
    assert(!Dgus_Write(data, sizeof(data), crc, 0, data, 1));
    assert(!Dgus_Write(data, sizeof(data), crc, 0, data, SIZE_MAX));
    assert(!Dgus_Write(data, sizeof(data), crc, 0xFFFF, data, 4));
    /* Deterministic garbage, bounded buffering, followed by a clean frame after a gap. */
    struct { uint32_t before; DgusParser parser; uint32_t after; } guarded = {.before = 123, .after = 456};
    DgusParser_Init(&guarded.parser, crc);
    uint32_t random = 12345;
    for (unsigned i = 0; i < 100000; ++i)
    {
        random = random * 1664525U + 1013904223U;
        DgusParser_Push(&guarded.parser, (uint8_t)(random >> 24), i / 20, NULL, NULL);
        assert(guarded.parser.used <= DGUS_MAX_FRAME);
    }
    assert(guarded.before == 123 && guarded.after == 456);
    received = 0;
    n = Upload(data, crc, HMI_VP_EVENT, 2, 1);
    Push(&guarded.parser, data, n, 5100);
    assert(received == 1);
}
static void Publish(HmiDgus *h, LocalUi *u, AppSnapshot *a, StorageSnapshot *s)
{
    UiModel model;
    HmiDgus_Sync(h, u, a, s);
    LocalUi_Render(u, a->startup, a->robot, &a->operation, s, &model);
    written = 0;
    fail_at = 99;
    assert(HmiDgus_Publish(h, u, a, s, &model, Write, NULL));
    assert(written == 5 && h->exposed);
    assert(Dgus_GetWord(frames[0] + 4) == HMI_VP_TEXT);
    assert(Dgus_GetWord(frames[4] + 4) == HMI_VP_STATUS);
    assert(Dgus_GetWord(frames[4] + 8) == h->token);
    assert(lengths[0] == (h->parser.crc ? 72U : 70U));
}
static void Session(bool crc)
{
    HmiDgus h;
    HmiDgus_Init(&h, crc);
    LocalUi ui;
    LocalUi_Init(&ui);
    AppSnapshot app = {.startup = STARTUP_WAIT_MOTORS, .robot = ROBOT_DISABLED,
                       .operation = {.state = OP_IDLE}};
    StorageSnapshot storage = {0};
    UiEvent event;
    bool stop;
    HmiPush(&h, HMI_VP_EVENT, h.token, HMI_KEY_CONFIRM);
    assert(!HmiDgus_TakeEvent(&h, &event, &stop)); /* No events before the first published context. */
    Publish(&h, &ui, &app, &storage);
    uint16_t old = h.token;
    HmiPush(&h, HMI_VP_EVENT, old, HMI_KEY_CONFIRM);
    assert(!HmiDgus_TakeEvent(&h, &event, &stop)); /* A held key at startup first needs release. */
    HmiPush(&h, HMI_VP_EVENT, old, HMI_KEY_NONE);
    HmiPush(&h, HMI_VP_STATUS, old, HMI_KEY_CONFIRM);
    HmiPush(&h, HMI_VP_EVENT, old, 255);
    HmiPush(&h, HMI_VP_EVENT, old, HMI_KEY_RECORD_FINISH);
    assert(!HmiDgus_TakeEvent(&h, &event, &stop) && h.bad_events == 3);
    HmiPush(&h, HMI_VP_EVENT, old, HMI_KEY_CONFIRM);
    HmiPush(&h, HMI_VP_EVENT, old, HMI_KEY_CONFIRM);
    assert(HmiDgus_TakeEvent(&h, &event, &stop) && event == ENCODER_PRESS && !stop);
    assert(!HmiDgus_TakeEvent(&h, &event, &stop));
    AppCommand command;
    assert(!LocalUi_Handle(&ui, ENCODER_PRESS, app.startup, app.robot, &app.operation, &storage, &command));
    assert(ui.page == UI_PAGE_ACTIONS);
    Publish(&h, &ui, &app, &storage);
    HmiPush(&h, HMI_VP_EVENT, old, HMI_KEY_CONFIRM);
    assert(!HmiDgus_TakeEvent(&h, &event, &stop)); /* A previous page token is stale. */
    HmiPush(&h, HMI_VP_EVENT, h.token, HMI_KEY_CONFIRM);
    assert(!HmiDgus_TakeEvent(&h, &event, &stop)); /* A new token alone never rearms a held key. */
    HmiPush(&h, HMI_VP_EVENT, old, HMI_KEY_NONE);
    assert(!h.touch_released); /* An old queued release cannot arm a new page. */
    HmiPush(&h, HMI_VP_EVENT, h.token, HMI_KEY_NONE);
    HmiPush(&h, HMI_VP_EVENT, h.token, HMI_KEY_NEXT);
    app.robot = ROBOT_FAULT;
    HmiDgus_Sync(&h, &ui, &app, &storage);
    assert(!HmiDgus_TakeEvent(&h, &event, &stop)); /* State changes cancel a pending navigation key. */
    HmiPush(&h, HMI_VP_EVENT, old, HMI_KEY_SOFT_STOP);
    HmiDgus_ResetLink(&h);
    assert(HmiDgus_TakeEvent(&h, &event, &stop) && stop && event == UI_EVENT_NONE);
    assert(!HmiDgus_TakeEvent(&h, &event, &stop));
    app.robot = ROBOT_DISABLED;
    /* Even a syntactically valid UI sequence cannot start an unready robot. */
    assert(!LocalUi_Handle(&ui, ENCODER_PRESS, app.startup, app.robot, &app.operation, &storage, &command));
    assert(ui.page == UI_PAGE_START_CONFIRM && ui.selected == 1);
    assert(!LocalUi_Handle(&ui, ENCODER_PREV, app.startup, app.robot, &app.operation, &storage, &command));
    assert(!LocalUi_Handle(&ui, ENCODER_PRESS, app.startup, app.robot, &app.operation, &storage, &command));
    assert(ui.show_error);
    CommandManager_Init();
    assert(!CommandManager_Take(&command));
    /* Control rejects a crafted request as well as the menu gate. */
    RobotControl control;
    BusinessMock_Reset();
    MotorManager_DisableAll();
    assert(RobotControl_Init(&control) == ARM_OK);
    OperationManager operation;
    OperationManager_Init(&operation, 0);
    command = (AppCommand){.type = CMD_GO_HOME, .source = CONTROL_SOURCE_LOCAL};
    assert(OperationManager_Command(&operation, &control, &command, 0) != ARM_OK);
    assert(!control.trajectory.active && !MotorManager_IsArmed());
    UiModel model = {.rows = {"主菜单", "A", "", ""}};
    written = 0;
    fail_at = 99;
    assert(HmiDgus_Publish(&h, &ui, &app, &storage, &model, Write, NULL));
    assert(Dgus_GetWord(frames[0] + 6) == 0x4E3B && Dgus_GetWord(frames[0] + 8) == 0x83DC);
    assert(Dgus_GetWord(frames[0] + 12) == 0xFFFF);
    HmiDgus_Invalidate(&h);
    written = 0;
    fail_at = 2;
    assert(!HmiDgus_Publish(&h, &ui, &app, &storage, &model, Write, NULL));
    assert(!h.exposed && h.tx_failures);
    h.token = UINT16_MAX;
    HmiDgus_Invalidate(&h);
    assert(h.token == 1);
}
int main(void)
{
    Protocol(false);
    Protocol(true);
    Session(false);
    Session(true);
    puts("DGUSII CRC vectors/noise/fragmentation/length/timeout/wrap/fuzz/context/replay/release/stop/UTF16/backpressure/safety PASS");
    return 0;
}
