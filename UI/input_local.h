#ifndef __INPUT_LOCAL_H
#define __INPUT_LOCAL_H
#include "local_ui.h"
#define INPUT_DEBOUNCE_MS 25U
#define ENCODER_COUNTS_PER_DETENT 4
typedef struct
{
    bool stable, candidate, needs_release;
    uint32_t since_ms;
} InputDebounce;
typedef struct
{
    InputDebounce forward, back, encoder_key;
    uint16_t last_count;
    int remainder;
    bool initialized;
} LocalInput;
void InputLocal_Discard(LocalInput *input, uint16_t encoder_count);
UiEvent InputLocal_Update(LocalInput *input, uint16_t encoder_count, bool forward_low, bool back_low,
                          uint32_t now_ms);
UiEvent InputLocal_UpdateWithKey(LocalInput *input, uint16_t encoder_count, bool forward_low,
                                bool back_low, bool encoder_key_low, uint32_t now_ms);
#endif
