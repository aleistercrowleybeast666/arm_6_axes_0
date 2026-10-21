#include "ui_raster.h"
#include "Backends/ui_glyphs.h"
#include <string.h>
static const UiGlyph *UiRaster_Glyph(uint32_t code)
{
    for (unsigned i = 0; i < sizeof(glyphs) / sizeof(glyphs[0]); ++i)
        if (glyphs[i].code == code)
            return &glyphs[i];
    return NULL;
}
static uint32_t UiRaster_Decode(const unsigned char **p)
{
    uint32_t c = *(*p)++;
    if (c < 128)
        return c;
    unsigned n = c >= 0xC2 && c <= 0xDF ? 1 : (c >= 0xE0 && c <= 0xEF ? 2 : 0);
    if (!n)
        return '?';
    uint32_t code = c & ((1U << (6 - n)) - 1);
    for (unsigned i = 0; i < n; ++i)
    {
        if ((**p & 0xC0) != 0x80)
            return '?';
        code = (code << 6) | (*(*p)++ & 0x3F);
    }
    return code;
}
ArmResult UiRaster_Render(const UiModel *m, uint8_t framebuffer[UI_FRAMEBUFFER_BYTES])
{
    if (!m || !framebuffer)
        return ARM_INVALID_ARGUMENT;
    memset(framebuffer, 0, UI_FRAMEBUFFER_BYTES);
    for (unsigned row = 0; row < 4; ++row)
    {
        unsigned x = 0;
        const unsigned char *p = (const unsigned char *)m->rows[row];
        /* Only selected ASCII filenames use a bounded character offset. */
        if (m->scroll_selected && m->has_selection && row == m->selected)
            for (unsigned skip = 0; skip < m->scroll_offset && *p && *p < 128; ++skip)
                ++p;
        while (*p && x < UI_DISPLAY_WIDTH)
        {
            const UiGlyph *g = UiRaster_Glyph(UiRaster_Decode(&p));
            if (!g)
                g = UiRaster_Glyph('?');
            if (!g || x + g->width > UI_DISPLAY_WIDTH)
                break;
            for (unsigned page = 0; page < 2; ++page)
                for (unsigned col = 0; col < g->width; ++col)
                    framebuffer[(row * 2 + page) * UI_DISPLAY_WIDTH + x + col] = g->pixels[page * 16 + col];
            x += g->width;
        }
        if (m->has_title && row == 0)
            for (unsigned col = 0; col < UI_DISPLAY_WIDTH; ++col)
                framebuffer[UI_DISPLAY_WIDTH + col] |= 0x80; /* Fixed title separator, same text size. */
        if (m->has_selection && row == m->selected)
            for (unsigned col = 0; col < UI_DISPLAY_WIDTH * 2; ++col)
                framebuffer[row * UI_DISPLAY_WIDTH * 2 + col] ^= 0xFF;
    }
    return ARM_OK;
}
