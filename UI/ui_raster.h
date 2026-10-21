#ifndef __UI_RASTER_H
#define __UI_RASTER_H
#include "local_ui.h"
#define UI_DISPLAY_WIDTH 128U
#define UI_DISPLAY_HEIGHT 64U
#define UI_FRAMEBUFFER_BYTES (UI_DISPLAY_WIDTH * UI_DISPLAY_HEIGHT / 8U)
ArmResult UiRaster_Render(const UiModel *model, uint8_t framebuffer[UI_FRAMEBUFFER_BYTES]);
#endif
