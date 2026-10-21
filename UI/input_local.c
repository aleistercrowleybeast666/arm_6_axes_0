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
        if (b->stable && !b->needs_release)
        {
            b->needs_release = true;
            return true;
        }
    }
    if (!b->candidate && !b->stable && (uint32_t)(now - b->since_ms) >= INPUT_DEBOUNCE_MS)
        b->needs_release = false;
    return false;
}
void InputLocal_Discard(LocalInput *i, uint16_t count)
{
    if (!i)
        return;
    i->last_count = count;
    i->remainder = 0;
    InputDebounce *buttons[] = {&i->forward, &i->back, &i->encoder_key};
    for (unsigned n = 0; n < 3; ++n)
        if (buttons[n]->stable)
            buttons[n]->needs_release = true;
    /* Preserve an unconsumed debouncing press across READY -> RUNNING, so a fresh Stop is not lost. */
}
UiEvent InputLocal_UpdateWithKey(LocalInput *i, uint16_t count, bool f, bool b, bool key, uint32_t now)
{
    if (!i)
        return UI_EVENT_NONE;
    if (!i->initialized)
    {
        i->last_count = count;
        i->initialized = true;
        i->forward = (InputDebounce){.stable = f, .candidate = f, .needs_release = f, .since_ms = now};
        i->back = (InputDebounce){.stable = b, .candidate = b, .needs_release = b, .since_ms = now};
        i->encoder_key = (InputDebounce){.stable = key, .candidate = key, .needs_release = key, .since_ms = now};
        return UI_EVENT_NONE; /* A key held at boot must first be released. */
    }
    int delta = (int16_t)(count - i->last_count);
    i->last_count = count;
    i->remainder += delta;
    bool forward = Input_Debounce(&i->forward, f, now), back = Input_Debounce(&i->back, b, now);
    bool confirm = Input_Debounce(&i->encoder_key, key, now);
    if (confirm)
        return ENCODER_PRESS;
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
UiEvent InputLocal_Update(LocalInput *i, uint16_t count, bool f, bool b, uint32_t now)
{
    return InputLocal_UpdateWithKey(i, count, f, b, false, now);
}
