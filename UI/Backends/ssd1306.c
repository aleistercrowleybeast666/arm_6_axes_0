#include "display_if.h"
#include "ui_raster.h"
#include "board_config.h"
#include "i2c.h"
#include <string.h>
static uint8_t framebuffer[UI_FRAMEBUFFER_BYTES];
static ArmResult Display_Write(const uint8_t *bytes, uint16_t count)
{
    return HAL_I2C_Master_Transmit(I2c_GetHandle(), BOARD_OLED_ADDRESS, (uint8_t *)bytes, count, 10) == HAL_OK
               ? ARM_OK
               : ARM_BUS_ERROR;
}
static ArmResult Ssd1306_Init(void)
{
    static const uint8_t commands[] = {0x00, 0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
                                       0x8D, 0x14, 0x20, 0x02, 0xA1, 0xC8, 0xDA, 0x12, 0x81,
                                       0x7F, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF};
    return Display_Write(commands, sizeof(commands));
}
static ArmResult Ssd1306_Render(const UiModel *m)
{
    ArmResult result = UiRaster_Render(m, framebuffer);
    if (result != ARM_OK)
        return result;
    for (unsigned page = 0; page < 8; ++page)
    {
        uint8_t command[] = {0, (uint8_t)(0xB0 + page), 0x00, 0x10};
        ArmResult r = Display_Write(command, sizeof(command));
        if (r != ARM_OK)
            return r;
        uint8_t packet[17] = {0x40};
        for (unsigned offset = 0; offset < 128; offset += 16)
        {
            memcpy(packet + 1, framebuffer + page * 128 + offset, 16);
            r = Display_Write(packet, sizeof(packet));
            if (r != ARM_OK)
                return r;
        }
    }
    return ARM_OK;
}
const DisplayBackend *Display_GetBackend(void)
{
    static const DisplayBackend backend = {Ssd1306_Init, Ssd1306_Render};
    return &backend;
}
