#include "hmi_dgus.h"
#include <string.h>
static void Hmi_Advance(HmiDgus *h)
{
    if (++h->token == 0)
        ++h->token;
    h->exposed = false;
    h->touch_released = false;
}
void HmiDgus_Init(HmiDgus *h, bool crc)
{
    if (h)
    {
        *h = (HmiDgus){.token = 1};
        DgusParser_Init(&h->parser, crc);
    }
}
void HmiDgus_Invalidate(HmiDgus *h)
{
    if (h)
    {
        h->pending_key = HMI_KEY_NONE;
        Hmi_Advance(h);
    }
}
void HmiDgus_ResetLink(HmiDgus *h)
{
    if (h)
    {
        DgusParser_Reset(&h->parser);
        HmiDgus_Invalidate(h);
    }
}
void HmiDgus_Sync(HmiDgus *h, const LocalUi *u, const AppSnapshot *a, const StorageSnapshot *s)
{
    if (!h || !u || !a || !s)
        return;
    HmiView v = {.page = u->page, .selected = u->selected, .startup = a->startup, .robot = a->robot,
                 .operation = a->operation.state, .command_revision = a->command_revision,
                 .catalog_revision = s->revision, .error = u->error, .start_command = u->start_command,
                 .show_error = u->show_error, .pending = u->pending_command};
    HmiView *o = &h->view;
    if (!h->have_view || o->page != v.page || o->selected != v.selected || o->startup != v.startup ||
        o->robot != v.robot || o->operation != v.operation || o->command_revision != v.command_revision ||
        o->catalog_revision != v.catalog_revision || o->error != v.error || o->start_command != v.start_command ||
        o->show_error != v.show_error || o->pending != v.pending)
        HmiDgus_Invalidate(h);
    h->view = v;
    h->have_view = true;
}
static void Hmi_Receive(void *context, const DgusFrame *f)
{
    HmiDgus *h = context;
    if (f->command == 0x82)
        return; /* A write acknowledgement never represents a key. */
    if (f->vp != HMI_VP_EVENT || f->words != 2)
    {
        ++h->bad_events;
        return;
    }
    uint16_t token = Dgus_GetWord(f->data), key = Dgus_GetWord(f->data + 2);
    if (key == HMI_KEY_SOFT_STOP)
    {
        h->pending_stop = true; /* A valid stop frame does not wait for a matching menu token. */
        HmiDgus_Invalidate(h);
        return;
    }
    if (key > HMI_KEY_RECORD_FINISH)
    {
        ++h->bad_events;
        return;
    }
    if (!h->have_view || !h->exposed || token != h->token || h->pending_key || h->pending_stop)
    {
        ++h->stale_events;
        return;
    }
    if (key == HMI_KEY_NONE)
    {
        h->touch_released = true;
        return;
    }
    if (!h->touch_released)
    {
        ++h->stale_events;
        return;
    }
    if (key == HMI_KEY_RECORD_FINISH && h->view.operation != OP_RECORDING)
    {
        ++h->bad_events;
        return;
    }
    h->pending_key = (HmiKey)key;
    Hmi_Advance(h); /* Identical frames, held touches and queued old confirms cannot replay. */
}
void HmiDgus_Push(HmiDgus *h, uint8_t byte, uint32_t now)
{
    if (h)
        DgusParser_Push(&h->parser, byte, now, Hmi_Receive, h);
}
bool HmiDgus_TakeEvent(HmiDgus *h, UiEvent *event, bool *stop)
{
    if (!h || !event || !stop)
        return false;
    *event = UI_EVENT_NONE;
    *stop = h->pending_stop;
    if (h->pending_stop)
    {
        h->pending_stop = false;
        h->pending_key = HMI_KEY_NONE;
        return true;
    }
    const UiEvent events[] = {UI_EVENT_NONE, ENCODER_NEXT, ENCODER_PREV, ENCODER_PRESS,
                              BACK, UI_EVENT_NONE, FORWARD};
    HmiKey key = h->pending_key;
    h->pending_key = HMI_KEY_NONE;
    *event = events[key];
    return key != HMI_KEY_NONE;
}
static void Hmi_Word(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)v;
}
static void Hmi_Text(const char *text, uint8_t out[HMI_TEXT_WORDS * 2U])
{
    memset(out, 0xFF, HMI_TEXT_WORDS * 2U);
    const unsigned char *p = (const unsigned char *)text;
    for (unsigned i = 0; i < HMI_TEXT_WORDS && *p; ++i)
    {
        uint32_t code = *p++;
        if (code >= 128)
        {
            unsigned extra = code >= 0xC2 && code <= 0xDF ? 1 : (code >= 0xE0 && code <= 0xEF ? 2 : 0);
            uint32_t lead = code;
            code = extra ? code & (extra == 1 ? 0x1FU : 0x0FU) : '?';
            bool valid = extra != 0;
            for (unsigned n = 0; n < extra; ++n)
            {
                if ((*p & 0xC0U) != 0x80U)
                {
                    valid = false;
                    break;
                }
                code = (code << 6) | (*p++ & 0x3FU);
            }
            if (!valid || (lead <= 0xDF && code < 0x80) || (lead >= 0xE0 && code < 0x800) ||
                (code >= 0xD800 && code <= 0xDFFF))
                code = '?';
        }
        Hmi_Word(out + 2U * i, (uint16_t)code);
    }
}
bool HmiDgus_Publish(HmiDgus *h, const LocalUi *u, const AppSnapshot *a, const StorageSnapshot *s,
                     const UiModel *m, HmiWrite write, void *context)
{
    if (!h || !u || !a || !s || !m || !write)
        return false;
    uint8_t frame[DGUS_MAX_FRAME], data[HMI_TEXT_WORDS * 2U];
    for (unsigned row = 0; row < 4; ++row)
    {
        Hmi_Text(m->rows[row], data);
        size_t n = Dgus_Write(frame, sizeof(frame), h->parser.crc,
                              (uint16_t)(HMI_VP_TEXT + row * HMI_TEXT_WORDS), data, sizeof(data));
        if (!n || !write(context, frame, n))
        {
            ++h->tx_failures;
            return false;
        }
    }
    uint16_t flags = (m->has_selection ? 1U : 0U) | (u->show_error ? 2U : 0U) |
                     (u->pending_command ? 4U : 0U) | (s->mounted ? 8U : 0U) |
                     (u->device.output_armed ? 16U : 0U) | (u->device.motors_configured ? 32U : 0U) |
                     (u->device.can_ready ? 64U : 0U) | (u->device.force_valid ? 128U : 0U);
    const uint16_t status[HMI_STATUS_WORDS] = {
        0x0100, h->token, (uint16_t)u->page, (uint16_t)m->selected,
        (uint16_t)a->startup, (uint16_t)a->robot, (uint16_t)a->operation.state, (uint16_t)a->command_result,
        flags, (uint16_t)a->operation.progress_percent, (uint16_t)s->count, (uint16_t)s->last_error,
        (uint16_t)a->command_revision, (uint16_t)(a->command_revision >> 16),
        (uint16_t)u->device.online_motors, (uint16_t)a->operation.samples};
    for (unsigned i = 0; i < HMI_STATUS_WORDS; ++i)
        Hmi_Word(data + i * 2U, status[i]);
    size_t n = Dgus_Write(frame, sizeof(frame), h->parser.crc, HMI_VP_STATUS, data, HMI_STATUS_WORDS * 2U);
    if (!n || !write(context, frame, n))
    {
        ++h->tx_failures;
        return false;
    }
    h->exposed = true;
    return true;
}
