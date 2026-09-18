# Rev.A 接口与线束合同

方向均相对主控板。针号是电气合同，**不是未审核的正面/背面视图**。线束制作前必须以厂家图纸和实际库焊盘核实 pin 1 位置；不可按屏幕左右顺序猜测。

| RefDes | 板端 MPN / 类型 | Pin → 信号 | 电平、方向、外设 |
| --- | --- | --- | --- |
| J_POWER | AMASS XT30PW-M30.G.Y，XT30 2P | 1=+24V_LOGIC (`24V_IN`)，2=GND | 输入；6S 电池 22.2V 标称/25.2V 满电，未来 24V PSU；逻辑支路 |
| J_CAN | JST SM03B-GHS-TB(LF)(SN)，GH1.25 3P SMT | 1=GND，2=CAN_H，3=CAN_L | CAN 总线双向；本板是固定终端之一 |
| J_OLED | JST S4B-PH-SM4-TB(LF)(SN)，PH2.0 4P SMT | 1=3V3，2=GND，3=OLED_SCL，4=OLED_SDA | 电源输出；PB6 时钟、PB7 双向数据；SSD1306 1.3寸128×64，地址0x3C |
| J_ENCODER | JST S3B-PH-SM4-TB(LF)(SN)，PH2.0 3P SMT | 1=GND，2=ENC_A，3=ENC_B | PA6/PA7 输入，10k 上拉；机械编码器，无按压信号 |
| J_FORWARD | JST S2B-PH-SM4-TB(LF)(SN)，PH2.0 2P SMT | 1=BTN_FORWARD，2=GND | PE2 输入，低有效，10k 上拉 |
| J_BACK | 同上 | 1=BTN_BACK，2=GND | PE3 输入，低有效，10k 上拉 |
| J_SOFTSTOP | 同上 | 1=BTN_SOFTSTOP，2=GND | PE4 EXTI 输入，低有效，10k 上拉；软件停止 |
| J_HX711 | JST S4B-PH-SM4-TB(LF)(SN)，PH2.0 4P SMT | 1=3V3，2=GND，3=HX_DOUT，4=HX_SCK | 远端供电输出；PE0 EXTI 输入、PE1 输出，各串33Ω；远端 HX711 靠近 Load Cell |
| J_UART | JST S3B-PH-SM4-TB(LF)(SN)，PH2.0 3P SMT | 1=GND，2=UART_TX，3=UART_RX | PA9 输出/PA10 输入，3.3V TTL；接适配器时交叉 TX/RX；丝印 UART 3V3 |
| J_SWD | JST S5B-PH-SM4-TB(LF)(SN)，PH2.0 5P SMT | 1=3V3_REF，2=GND，3=SWDIO，4=SWCLK，5=NRST | 1仅目标电压参考；PA13 双向、PA14 时钟输入、复位；不从调试器反灌供电 |

## 板载 TF 座

HRO TF-01A / C91145，贴片、带 Card Detect。

| 触点 | 信号 | MCU / 处理 |
| --- | --- | --- |
| 1 | DAT2 | PC10，47k 上拉 |
| 2 | DAT3 | PC11，47k 上拉 |
| 3 | CMD | PD2，47k 上拉 |
| 4 | VDD | 3V3；局部100nF+10µF |
| 5 | CLK | PC12 经33Ω，无上拉 |
| 6 | VSS | GND |
| 7 | DAT0 | PC8，47k 上拉 |
| 8 | DAT1 | PC9，47k 上拉 |
| Cd（设计号9） | SD_DETECT | PC7，10k 上拉，插卡接地低有效；实际接触开关和外壳回路待实物/库确认 |
| 金属壳焊脚 | GND | 所有实际壳脚均应接地，不得遗漏或误当信号脚 |

## 接口约束

- OLED 上拉默认安装 4.7k；模块若已有上拉，测量总并联阻值和波形后可 DNP 主板两只电阻。必须确认模块上拉接3.3V。
- 400kHz I²C 的 RC 上升时间估算 `tr≈0.8473×Rp×Cbus`。4.7k、300ns 对应约75pF，总量包含线束、插头和引脚。外壳线束不能在未测量时认定满足该限制。[NXP UM10204 §7.1](https://www.nxp.com/docs/en/user-guide/UM10204.pdf)
- HX711 的 DOUT 不允许输出5V。主板33Ω不能替代长线驱动/ESD保护；确认线长、回流、干扰和 SCK 高电平时间。远端板须有本地去耦。
- 三按钮采用软件去抖；SoftStop 不具备硬件急停功能，硬件 E-STOP 在独立主动力接触器回路。
- 六个电机电源不经过 J_POWER 或主控 PCB。CAN GND 是信号参考，不承载电机回流。
- PA11/PA12 仅预留 USB_DM/DP 测试点；没有 USB 接口、VBUS 或完整 USB 电路。
- 后续丝印、压接图必须与本表保持一致，明确板端插合面、线端视图和 pin 1 标记；XT30 的 moulded +/− 尤其需要复核。
