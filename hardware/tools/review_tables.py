"""Render deterministic human-readable tables from the schematic design source."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
design = json.loads((ROOT/'design/controller_revA.json').read_text(encoding='utf-8'))
parts = design['parts']

def escape(value):
    return str(value).replace('|', '\\|').replace('\n', ' ')

lines = ['# Rev.A 引脚与网络审图表', '',
         '由 `hardware/tools/review_tables.py` 生成。此表是设计意图，必须与客户端实际导出的网表比较；不是EDA网表导出。`NC` 表示明确不连接。', '']
for sheet in design['sheets']:
    lines += ['## '+sheet, '']
    for c in (item for item in design['components'] if item['sheet'] == sheet):
        p = parts[c['part']]
        lines += [f'### {c["ref"]} — {escape(p["value"])}', '',
                  f'{escape(p["mpn"])} / {p["lcsc"]} / {escape(p["package"])}', '',
                  '| 引脚 | 数据手册名称（如指定） | 网络 |', '| --- | --- | --- |']
        for pin, net in c['pins'].items():
            lines.append(f'| {escape(pin)} | {escape(c["pinNames"].get(pin,""))} | {escape(net or "NC")} |')
        if c['notes']:
            lines += ['', escape(c['notes'])]
        lines += ['']
lines += ['## 机械定义', '', '| RefDes | 孔径 | 金属化/网络 | keepout | X/Y |', '| --- | --- | --- | --- | --- |']
for h in design['mountingHoles']:
    lines.append(f'| {h["ref"]} | {h["diameter_mm"]}mm | NPTH / 无网络 | Ø{h["keepout_diameter_mm"]}mm | 待布局 |')
(ROOT/'docs/PIN_NET_REVIEW.md').write_text('\n'.join(lines)+'\n', encoding='utf-8')

lines = ['# Rev.A 器件库审核表', '',
         '厂家引脚、官方符号、封装焊盘为三个不同证据层。下面的状态来自采购清单；不能把商品页存在或封装名称相似等同于审核通过。', '',
         '运行后以 `runtime_audit.json` 中的实际 UUID、引脚读回、PAD 记录补齐。编号自动匹配后仍须检查尺寸、极性、固定片、隐藏引脚和实物插合方向。', '',
         '| 元件 | MPN / LCSC | 封装 | 库状态 | 数据手册 |',
         '| --- | --- | --- | --- | --- |']
for key,p in parts.items():
    refs = ', '.join(c['ref'] for c in design['components'] if c['part'] == key)
    lines.append(f'| {escape(refs)} | {escape(p["mpn"])} / {p["lcsc"]} | {escape(p["package"])} | {escape(p["libraryStatus"])} | [厂家文档]({p["datasheet"]}) |')
lines += ['', '## 重点审核', '',
          '- MCU：100脚；电源脚不得隐藏遗漏；PA/PB/PC/PD/PE和晶体脚逐号匹配。',
          '- LMR38020：EN=2，VIN=3，EP必须接地；不能套用其他Buck的通用8脚符号。',
          '- AP7361C-SP：OUT=1，EN=2，IN=8，GND=4，EP接地；不要混用SOT223/其他SO8版本。',
          '- SIT1042AQT/3：VCC=3，VIO=5，CANL=6，CANH=7，STB=8。',
          '- TF：触点1–8、Cd与全部金属壳脚；额外焊盘不可自动按无网络处理。',
          '- XT30：以实际塑壳+/-核对电气合同pin1/2与库焊盘，明确插合面。',
          '- JST PH/GH：精确MPN，SMT固定片的导电属性和pin1视图。',
          '- 肖特基、单向TVS、聚合物电容及蓝灯：阳/阴极、正/负端和丝印三方一致。',
          '- HSE：5032四焊盘，1/3晶体、2/4外壳地；实际库不能套用同封装不同引脚型号。',
          '- 四机械孔尚未创建PCB封装；本轮仅约束定义，不能报告已完成PCB尺寸验证。', '']
(ROOT/'docs/LIBRARY_AUDIT.md').write_text('\n'.join(lines), encoding='utf-8')
print('Generated PIN_NET_REVIEW.md and LIBRARY_AUDIT.md')
