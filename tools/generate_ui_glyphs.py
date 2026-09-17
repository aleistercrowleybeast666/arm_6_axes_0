"""Optional development tool: rasterize only the menu glyphs (Pillow + an installed font).

Generated bitmaps are committed; firmware builds do not need Pillow or a font.
Usage: python tools/generate_ui_glyphs.py --font <CJK TTF/TTC file>
"""
from pathlib import Path
import argparse
from PIL import Image, ImageDraw, ImageFont
import re

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--font', required=True)
args = parser.parse_args()
text = (ROOT/'UI/ui_strings.c').read_text(encoding='utf-8')
chars = sorted(set(''.join(re.findall(r'"([^"\n]*)"', text))) | set(chr(i) for i in range(32, 127)))
font = ImageFont.truetype(args.font, 14)
rows = []
for ch in chars:
    if ord(ch)<32: continue
    width = 8 if ord(ch)<128 else 16
    image = Image.new('1',(width,16));draw=ImageDraw.Draw(image)
    bbox=draw.textbbox((0,0),ch,font=font)
    draw.text(((width-(bbox[2]-bbox[0]))//2-bbox[0],(16-(bbox[3]-bbox[1]))//2-bbox[1]),ch,font=font,fill=1)
    values=[]
    for page in range(2):
        for x in range(16):
            values.append(sum((1<<y) if x<width and image.getpixel((x,page*8+y)) else 0 for y in range(8)))
    rows.append('{%d,%d,{%s}}'%(ord(ch),width,','.join(str(v) for v in values)))
dest=ROOT/'UI/Backends/ui_glyphs.h';dest.parent.mkdir(parents=True,exist_ok=True)
dest.write_text('#ifndef __UI_GLYPHS_H\n#define __UI_GLYPHS_H\n#include <stdint.h>\n'
               'typedef struct { uint32_t code; uint8_t width, pixels[32]; } UiGlyph;\n'
               'static const UiGlyph glyphs[]={\n'+',\n'.join(rows)+'\n};\n#endif\n',encoding='utf-8')
print(f'{len(rows)} glyphs, no runtime font dependency')
