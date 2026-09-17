#ifndef __DISPLAY_IF_H
#define __DISPLAY_IF_H
#include "local_ui.h"
typedef struct
{
    ArmResult (*init)(void);
    ArmResult (*render)(const UiModel *model);
} DisplayBackend;
const DisplayBackend *Display_GetBackend(void);
#endif
