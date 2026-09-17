#include "display_if.h"
#include "ui_glyphs.h"
#include "board_config.h"
#include "i2c.h"
#include <string.h>
#define DISPLAY_WIDTH 128U
#define DISPLAY_HEIGHT 64U
static uint8_t framebuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];
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
static const UiGlyph *Display_Glyph(uint32_t code)
{
    for (unsigned i = 0; i < sizeof(glyphs) / sizeof(glyphs[0]); ++i)
        if (glyphs[i].code == code)
            return &glyphs[i];
    return NULL;
}
static uint32_t Display_Decode(const unsigned char **p)
{
    uint32_t c = *(*p)++;
    if (c < 128)
        return c;
    unsigned n = c < 224 ? 1 : 2;
    uint32_t code = c & ((1U << (6 - n)) - 1);
    for (unsigned i = 0; i < n; ++i)
    {
        if ((**p & 0xC0) != 0x80)
            return '?';
        code = (code << 6) | (*(*p)++ & 0x3F);
    }
    return code;
}
static ArmResult Ssd1306_Render(const UiModel *m)
{
    if (!m)
        return ARM_INVALID_ARGUMENT;
    memset(framebuffer, 0, sizeof(framebuffer));
    for (unsigned row = 0; row < 4; ++row)
    {
        unsigned x = 0;
        const unsigned char *p = (const unsigned char *)m->rows[row];
        while (*p && x < DISPLAY_WIDTH)
        {
            uint32_t code = Display_Decode(&p);
            const UiGlyph *g = Display_Glyph(code);
            if (!g)
                g = Display_Glyph('?');
            if (!g || x + g->width > DISPLAY_WIDTH)
                break;
            for (unsigned page = 0; page < 2; ++page)
                for (unsigned col = 0; col < g->width; ++col)
                    framebuffer[(row * 2 + page) * DISPLAY_WIDTH + x + col] = g->pixels[page * 16 + col];
            x += g->width;
        }
        if (m->has_selection && row == m->selected)
            for (unsigned i = 0; i < 256; ++i)
                framebuffer[row * 256 + i] ^= 0xFF;
    }
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
