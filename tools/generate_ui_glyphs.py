"""Generate a fixed-baseline OLED glyph subset from a user-supplied licensed font.

For the committed subset use Noto Sans Mono CJK SC Regular (Sans2.004, SIL OFL).
Firmware builds use the committed header and need neither Pillow nor a font.
"""
from pathlib import Path
import argparse
import hashlib
from PIL import Image, ImageDraw, ImageFont
import re

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--font', required=True)
args = parser.parse_args()
text = '\n'.join(p.read_text(encoding='utf-8') for p in sorted((ROOT/'UI').glob('*.c')))
chars = sorted(set(''.join(re.findall(r'"([^"\n]*)"', text))) | set(chr(i) for i in range(32, 127)))
font = ImageFont.truetype(args.font, 14)
rows = []
for ch in chars:
    if ord(ch) < 32:
        continue
    width = 8 if ord(ch) < 128 else 16
    bitmap = Image.new('L', (width, 16))
    draw = ImageDraw.Draw(bitmap)
    # One em size and baseline for letters, digits and CJK; never center each glyph vertically.
    draw.text((0 if width == 8 else 1, 12), ch, font=font, fill=255, anchor='ls')
    values = []
    for page in range(2):
        for x in range(16):
            values.append(sum((1 << y) if x < width and bitmap.getpixel((x, page*8+y)) >= 96 else 0
                              for y in range(8)))
    rows.append('{%d,%d,{%s}}' % (ord(ch), width, ','.join(str(v) for v in values)))
font_hash = hashlib.sha256(Path(args.font).read_bytes()).hexdigest()
header = ('/* OLED subset derived from Noto Sans Mono CJK SC Regular, SIL OFL 1.1.\n'
          ' * See FONT_LICENSE.txt and docs/THIRD_PARTY.md. Source SHA256: '+font_hash+' */\n'
          '#ifndef __UI_GLYPHS_H\n#define __UI_GLYPHS_H\n#include <stdint.h>\n'
          'typedef struct { uint32_t code; uint8_t width, pixels[32]; } UiGlyph;\n'
          'static const UiGlyph glyphs[]={\n' + ',\n'.join(rows) + '\n};\n#endif\n')
(ROOT/'UI/Backends/ui_glyphs.h').write_text(header, encoding='utf-8')
print(f'{len(rows)} glyphs; 16px cells, fixed baseline; font SHA256={font_hash}')
