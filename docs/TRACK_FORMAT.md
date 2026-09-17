# Track BIN v1

所有整数显式 little-endian；禁止直接 dump C struct。扩展名小写 `.bin`，魔数 `A6TR`。头固定 40 B，样本固定 12 B。

| Offset | Bytes | 字段 | v1 |
|---:|---:|---|---|
| 0 | 4 | magic | ASCII A6TR |
| 4 | 2 | version | 1 |
| 6 | 2 | header_bytes | 40 |
| 8 | 1 | joint_count | 6 |
| 9 | 1 | encoding | 1 = q_cdeg int16 |
| 10 | 2 | flags | 0 |
| 12 | 4 | sample_period_us | 10000 / 20000 / 40000 |
| 16 | 4 | sample_count | 2..4096 |
| 20 | 4 | model_id | 当前 0x6683CDEB |
| 24 | 4 | payload_bytes | count × 12 |
| 28 | 4 | payload_crc32 | 只覆盖 payload |
| 32 | 4 | reserved | 0 |
| 36 | 4 | header_crc32 | 覆盖 bytes 0..35 |

CRC-32/IEEE reflected polynomial `0xEDB88320`，初值和终值 XOR 均 `0xFFFFFFFF`；`123456789` → `0xCBF43926`，与 Python zlib.crc32 相同。

样本顺序 J1..J6，每项 signed int16，小端、0.01°，使用机器人关节坐标，不是电机原始角度。转换 rad→deg×100 后四舍五入，NaN/Inf/超 int16 范围拒绝，不截断、不饱和到合法区。文件长度必须恰好 40+12×count，不接受尾部垃圾。

## 时基、插值与容量

默认录制 50 Hz（20000 us）；格式兼容 25/50/100 Hz。控制 200 Hz。样本时间 t=i×period；4096 个 50 Hz 点覆盖 0..81.90 s，容量预算为 81.92 s；25/100 Hz 分别覆盖 163.80/40.95 s。CCM arena 固定 49152 B，头及元数据另放主 SRAM。最大文件 49192 B。

播放用单调 cubic Hermite，相邻同向割线取调和平均切线，拐点切线 0，首尾切线 0；段间速度连续。StorageTask 解析每段位置驻点、速度极值、加速度端点，检查 soft/velocity/acceleration 后才允许执行。加速度在节点处不保证连续，不宣称 jerk 限制。记录间隔异常会中止，不以压缩时间掩盖丢采样。

## 稳定模型 ID

CRC 输入为顺序 LE int32：6；每轴的 parent translation 三项（mm×1000，单位 um）及 local axis 三项（×1000000）；最后 tool_length_um=90000。关节原始行如下，前三列 mm、后三列单位向量：

```text
0   0 0   0  0  1
0   0 180 0 -1  0
320 0 0   0 -1  0
290 0 0   1  0  0
60  0 0   0 -1  0
86  0 0   1  0  0
```

模型 ID 不包含固件 Git SHA；数学几何变化会改变 ID。未知版本、编码、flags、非零 reserved、错误模型、CRC、长度或限位一律拒绝，无自动迁移。

## PC 检查工具

```text
python tools/track_tool.py info tests/vectors/home_track_v1.bin
python tools/track_tool.py verify track.bin
python tools/track_tool.py verify track.bin --velocity 1 --acceleration 2
python tools/track_tool.py dump track.bin --csv track.csv
```

工具只依赖 Python 标准库。示例限幅只是调用参数，不是设备默认安全值。无参数 verify 检查格式/CRC/模型/hard limits 并报告曲线峰值；设备仍按 commissioning 后的 soft/速度/加速度验证。固定 HOME test vector 由 Python 编码，C 和 Python 都解码校验。
