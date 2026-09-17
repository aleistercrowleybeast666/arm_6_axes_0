#include "input_local.h"
static bool Input_Debounce(InputDebounce *b, bool pressed, uint32_t now)
{
    if (pressed != b->candidate)
    {
        b->candidate = pressed;
        b->since_ms = now;
    }
    if (b->candidate != b->stable && (uint32_t)(now - b->since_ms) >= INPUT_DEBOUNCE_MS)
    {
        b->stable = b->candidate;
        return b->stable;
    }
    return false;
}
UiEvent InputLocal_Update(LocalInput *i, uint16_t count, bool f, bool b, uint32_t now)
{
    if (!i)
        return UI_EVENT_NONE;
    if (!i->initialized)
    {
        i->last_count = count;
        i->initialized = true;
    }
    int delta = (int16_t)(count - i->last_count);
    i->last_count = count;
    i->remainder += delta;
    bool forward = Input_Debounce(&i->forward, f, now), back = Input_Debounce(&i->back, b, now);
    if (back)
        return BACK;
    if (forward)
        return FORWARD;
    if (i->remainder >= ENCODER_COUNTS_PER_DETENT)
    {
        i->remainder -= ENCODER_COUNTS_PER_DETENT;
        return ENCODER_NEXT;
    }
    if (i->remainder <= -ENCODER_COUNTS_PER_DETENT)
    {
        i->remainder += ENCODER_COUNTS_PER_DETENT;
        return ENCODER_PREV;
    }
    return UI_EVENT_NONE;
}
