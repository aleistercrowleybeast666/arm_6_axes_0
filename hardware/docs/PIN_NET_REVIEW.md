# Rev.A 引脚与网络审图表

由 `hardware/tools/review_tables.py` 生成。此表是设计意图，必须与客户端实际导出的网表比较；不是EDA网表导出。`NC` 表示明确不连接。

## 01_POWER

### J_POWER — XT30 2P

XT30PW-M30.G.Y / C431092 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 24V_IN |
| 2 |  | GND |

PIN1 +24V_LOGIC; PIN2 GND. Logic branch only; verify moulded +/− against library pads.

### F1 — 1A 125V

0451001.MRL / C3099 / 2410

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 24V_IN |
| 2 |  | VIN_FUSED |

1A fast fuse; upstream battery branch fuse required; interrupt rating coordination pending.

### D_REVERSE — SS310 100V 3A

SS310 / C2848687 / SMA(DO-214AC)

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 | K | VIN_PROTECTED |
| 2 | A | VIN_FUSED |

100V/3A series Schottky; cathode downstream.

### D_INPUT — SMBJ33A

SMBJ33A / C173526 / DO-214AA(SMB)

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 | K | VIN_PROTECTED |
| 2 | A | GND |

SMBJ33A, VRWM33V, VC53.3V at rated pulse.

### C_IN_BULK — 47uF

63SXV47M / C178377 / SMD,D10xL12.6mm

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | VIN_PROTECTED |
| 2 |  | GND |

47uF/63V polymer. Pin1 positive.

### C_IN — 4.7uF

12101C475K4T2A / C2325876 / 1210

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | VIN_PROTECTED |
| 2 |  | GND |

4.7uF/100V X7R; immediately across VIN/GND.

### C_IN_HF — 100nF

CL21B104KCFSFNE / C398816 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | VIN_PROTECTED |
| 2 |  | GND |

100nF/100V; minimum hot-loop area.

### U_BUCK — 5V 2A Buck

LMR38020FDDAR / C5149193 / SO-PowerPAD-8

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 | GND | GND |
| 2 | EN | VIN_PROTECTED |
| 3 | VIN | VIN_PROTECTED |
| 4 | RT/SYNC | BUCK_RT |
| 5 | FB | BUCK_FB |
| 6 | PG | BUCK_PG |
| 7 | BOOT | BUCK_BOOT |
| 8 | SW | BUCK_SW |
| 9 | EP | GND |

CRITICAL LAYOUT AREA. EN=VIN; exposed pad GND. No external VCC pin.

### R_RT — 64.9k

0805W8F6492T5E / C17788 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | BUCK_RT |
| 2 |  | GND |

400kHz, TI Table 8-1.

### C_BOOT — 100nF

CL21B104KBCNNNC / C1711 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | BUCK_BOOT |
| 2 |  | BUCK_SW |

100nF directly BOOT to SW, not GND.

### L1 — 15uH

SMDRI127-150MT / C40000 / SMD,12.3x12.3mm

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | BUCK_SW |
| 2 |  | 5V |

15uH, 4.5A thermal/8A saturation at specified test conditions.

### C_OUT1 — 22uF

CL32B226KAJNNNE / C309062 / 1210

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 5V |
| 2 |  | GND |

22uF/25V X7R, 1210; verify DC-bias effective capacitance.

### C_OUT2 — 22uF

CL32B226KAJNNNE / C309062 / 1210

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 5V |
| 2 |  | GND |

22uF/25V X7R, 1210; verify DC-bias effective capacitance.

### C_OUT3 — 22uF

CL32B226KAJNNNE / C309062 / 1210

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 5V |
| 2 |  | GND |

22uF/25V X7R, 1210; verify DC-bias effective capacitance.

### R_FB_TOP — 100k

0805W8F1003T5E / C149504 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 5V |
| 2 |  | BUCK_FB |

### R_FB_BOTTOM — 24.9k

0805W8F2492T5E / C17571 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | BUCK_FB |
| 2 |  | GND |

Vout=1V*(1+100k/24.9k)=5.016V.

### R_PG — 100k

0805W8F1003T5E / C149504 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | BUCK_PG |

Open drain PG pulled to 3V3; debug test point only.

### U_LDO — 3V3 LDO

AP7361C-33SP-13 / C4943338 / ESOP-8

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 | OUT | 3V3 |
| 2 | EN | 5V |
| 3 | NC | NC |
| 4 | GND | GND |
| 5 | NC | NC |
| 6 | NC | NC |
| 7 | NC | NC |
| 8 | IN | 5V |
| 9 | EP | GND |

AP7361C SO-8EP (SP), EN=IN. Thermal design limits continuous current.

### C_LDO_IN — 10uF

CL21A106KAYNNNE / C15850 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 5V |
| 2 |  | GND |

### C_LDO_OUT — 10uF

CL21A106KAYNNNE / C15850 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |

### R_LED — 470R

0805W8F4700T5E / C17710 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | LED_A |

About 1mA at Vf=2.83V; verify LED batch at low current.

### LED_PWR — BLUE

ORH-B35A / C205441 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 | K | GND |
| 2 | A | LED_A |

BLUE 0805; silk PWR.

## 02_MCU

### U_MCU — STM32F407VET6

STM32F407VET6 / C28730 / LQFP-100(14x14)

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 | PE2 | BTN_FORWARD |
| 2 | PE3 | BTN_BACK |
| 3 | PE4 | BTN_SOFTSTOP |
| 4 | PE5 | NC |
| 5 | PE6 | NC |
| 6 | VBAT | 3V3 |
| 7 | PC13 | NC |
| 8 | PC14 | NC |
| 9 | PC15 | NC |
| 10 | VSS | GND |
| 11 | VDD | 3V3 |
| 12 | PH0 | HSE_IN |
| 13 | PH1 | HSE_OUT |
| 14 | NRST | NRST |
| 15 | PC0 | NC |
| 16 | PC1 | NC |
| 17 | PC2 | NC |
| 18 | PC3 | NC |
| 19 | VDD | 3V3 |
| 20 | VSSA | GND |
| 21 | VREF+ | VDDA |
| 22 | VDDA | VDDA |
| 23 | PA0 | NC |
| 24 | PA1 | NC |
| 25 | PA2 | NC |
| 26 | PA3 | NC |
| 27 | VSS | GND |
| 28 | VDD | 3V3 |
| 29 | PA4 | NC |
| 30 | PA5 | NC |
| 31 | PA6 | ENC_A |
| 32 | PA7 | ENC_B |
| 33 | PC4 | NC |
| 34 | PC5 | NC |
| 35 | PB0 | NC |
| 36 | PB1 | NC |
| 37 | PB2 | BOOT1 |
| 38 | PE7 | NC |
| 39 | PE8 | NC |
| 40 | PE9 | NC |
| 41 | PE10 | NC |
| 42 | PE11 | NC |
| 43 | PE12 | NC |
| 44 | PE13 | NC |
| 45 | PE14 | NC |
| 46 | PE15 | NC |
| 47 | PB10 | NC |
| 48 | PB11 | NC |
| 49 | VCAP_1 | VCAP1 |
| 50 | VDD | 3V3 |
| 51 | PB12 | NC |
| 52 | PB13 | NC |
| 53 | PB14 | NC |
| 54 | PB15 | NC |
| 55 | PD8 | NC |
| 56 | PD9 | NC |
| 57 | PD10 | NC |
| 58 | PD11 | NC |
| 59 | PD12 | NC |
| 60 | PD13 | NC |
| 61 | PD14 | NC |
| 62 | PD15 | NC |
| 63 | PC6 | NC |
| 64 | PC7 | SD_DETECT |
| 65 | PC8 | SD_D0 |
| 66 | PC9 | SD_D1 |
| 67 | PA8 | NC |
| 68 | PA9 | UART_TX |
| 69 | PA10 | UART_RX |
| 70 | PA11 | USB_DM_RESERVED |
| 71 | PA12 | USB_DP_RESERVED |
| 72 | PA13 | SWDIO |
| 73 | VCAP_2 | VCAP2 |
| 74 | VSS | GND |
| 75 | VDD | 3V3 |
| 76 | PA14 | SWCLK |
| 77 | PA15 | NC |
| 78 | PC10 | SD_D2 |
| 79 | PC11 | SD_D3 |
| 80 | PC12 | SD_CLK_MCU |
| 81 | PD0 | NC |
| 82 | PD1 | NC |
| 83 | PD2 | SD_CMD |
| 84 | PD3 | NC |
| 85 | PD4 | NC |
| 86 | PD5 | NC |
| 87 | PD6 | NC |
| 88 | PD7 | NC |
| 89 | PB3 | NC |
| 90 | PB4 | NC |
| 91 | PB5 | NC |
| 92 | PB6 | OLED_SCL |
| 93 | PB7 | OLED_SDA |
| 94 | BOOT0 | BOOT0 |
| 95 | PB8 | CAN_RX |
| 96 | PB9 | CAN_TX |
| 97 | PE0 | HX_DOUT_MCU |
| 98 | PE1 | HX_SCK_MCU |
| 99 | VSS | GND |
| 100 | VDD | 3V3 |

All unused GPIO explicitly no-connect; no pin reassignment.

### C_VDD11 — 100nF

CL21B104KBCNNNC / C1711 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |

Place directly at U_MCU pin 11.

### C_VDD19 — 100nF

CL21B104KBCNNNC / C1711 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |

Place directly at U_MCU pin 19.

### C_VDD28 — 100nF

CL21B104KBCNNNC / C1711 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |

Place directly at U_MCU pin 28.

### C_VDD50 — 100nF

CL21B104KBCNNNC / C1711 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |

Place directly at U_MCU pin 50.

### C_VDD75 — 100nF

CL21B104KBCNNNC / C1711 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |

Place directly at U_MCU pin 75.

### C_VDD100 — 100nF

CL21B104KBCNNNC / C1711 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |

Place directly at U_MCU pin 100.

### C_MCU_BULK — 10uF

CL21A106KAYNNNE / C15850 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |

### C_VBAT — 100nF

CL21B104KBCNNNC / C1711 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |

VBAT tied to 3V3; no battery backup.

### C_VCAP1 — 2.2uF

CL21A225KAFNNNE / C87994 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | VCAP1 |
| 2 |  | GND |

2.2uF low-ESR <=2ohm. NEVER connect VCAP to 3V3.

### C_VCAP2 — 2.2uF

CL21A225KAFNNNE / C87994 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | VCAP2 |
| 2 |  | GND |

2.2uF low-ESR <=2ohm. NEVER connect VCAP to 3V3.

### R_AVDD — 0R

0805W8F0000T5E / C17477 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | VDDA |

0R feed; footprint can accept ferrite only after current/impedance review.

### C_AVDD_HF — 100nF

CL21B104KBCNNNC / C1711 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | VDDA |
| 2 |  | GND |

### C_AVDD_BULK — 1uF

CL21B105KBFNNNE / C28323 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | VDDA |
| 2 |  | GND |

### R_BOOT0 — 10k

0805W8F1002T5E / C17414 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | BOOT0 |
| 2 |  | GND |

### R_BOOT1 — 10k

0805W8F1002T5E / C17414 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | BOOT1 |
| 2 |  | GND |

PB2 boot strap low; currently unused by firmware.

### R_RESET — 10k

0805W8F1002T5E / C17414 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | NRST |

### C_RESET — 100nF

CL21B104KBCNNNC / C1711 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | NRST |
| 2 |  | GND |

SWD reset access; no optional reset button fitted.

### Y1 — 8MHz CL12pF

5032 8M 12PF 20PPM 4P / C5181482 / SMD5032-4P

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | HSE_IN |
| 2 |  | GND |
| 3 |  | HSE_OUT |
| 4 |  | GND |

8MHz CL12pF ESR80ohm max; 5032. Pads 2/4 case ground.

### C_HSE_IN — 18pF

0805CG180J500NT / C1797 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | HSE_IN |
| 2 |  | GND |

C1=C2=18pF with estimated Cstray=3pF gives CL=12pF; verify startup/drive.

### C_HSE_OUT — 18pF

0805CG180J500NT / C1797 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | HSE_OUT |
| 2 |  | GND |

## 03_CAN

### U_CAN — CAN 5V / 3V3

SIT1042AQT/3 / C5382542 / SOP-8

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 | TXD | CAN_TX |
| 2 | GND | GND |
| 3 | VCC | 5V |
| 4 | RXD | CAN_RX |
| 5 | VIO | 3V3 |
| 6 | CANL | CAN_L |
| 7 | CANH | CAN_H |
| 8 | STB | CAN_STB |

SIT1042AQT/3 SOP8; STB LOW normal/high-speed; CAN1 1Mbps.

### R_STB — 0R

0805W8F0000T5E / C17477 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | CAN_STB |
| 2 |  | GND |

### C_CAN_VCC_HF — 100nF

CL21B104KBCNNNC / C1711 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 5V |
| 2 |  | GND |

### C_CAN_VCC — 1uF

CL21B105KBFNNNE / C28323 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 5V |
| 2 |  | GND |

### C_CAN_VIO_HF — 100nF

CL21B104KBCNNNC / C1711 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |

### C_CAN_VIO — 1uF

CL21B105KBFNNNE / C28323 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |

### R_TERM — 120R

0805W8F1200T5E / C17437 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | CAN_H |
| 2 |  | CAN_L |

FIXED 120ohm. Main controller is a physical bus end. No switch/jumper.

### D_CAN_H — SMAJ13CA

SMAJ13CA-13-F / C134951 / SMA

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | CAN_H |
| 2 |  | GND |

Bidirectional SMAJ13CA; verify capacitive loading at 1Mbps.

### D_CAN_L — SMAJ13CA

SMAJ13CA-13-F / C134951 / SMA

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | CAN_L |
| 2 |  | GND |

Bidirectional SMAJ13CA; no substitution in Rev.A.

### J_CAN — GH1.25 3P

SM03B-GHS-TB(LF)(SN) / C514175 / SMD,P=1.25mm,Surface Mount,Right Angle

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | GND |
| 2 |  | CAN_H |
| 3 |  | CAN_L |

JST GH 1.25mm; numbered pin order is harness contract.

## 04_STORAGE

### J_TF — microSD + CD

TF-01A / C91145 / SMD

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 | DAT2 | SD_D2 |
| 2 | DAT3 | SD_D3 |
| 3 | CMD | SD_CMD |
| 4 | VDD | 3V3 |
| 5 | CLK | SD_CLK |
| 6 | VSS | GND |
| 7 | DAT0 | SD_D0 |
| 8 | DAT1 | SD_D1 |
| 9 | CD | SD_DETECT |

HRO TF-01A: contact Cd=9, closes to grounded shell when inserted. All shell pins GND; library pad audit required.

### R_SD_D0 — 47k

0805W8F4702T5E / C17713 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | SD_D0 |

SD pull-up 47k in 10k..100k range; CLK has none.

### R_SD_D1 — 47k

0805W8F4702T5E / C17713 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | SD_D1 |

SD pull-up 47k in 10k..100k range; CLK has none.

### R_SD_D2 — 47k

0805W8F4702T5E / C17713 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | SD_D2 |

SD pull-up 47k in 10k..100k range; CLK has none.

### R_SD_D3 — 47k

0805W8F4702T5E / C17713 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | SD_D3 |

SD pull-up 47k in 10k..100k range; CLK has none.

### R_SD_CMD — 47k

0805W8F4702T5E / C17713 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | SD_CMD |

SD pull-up 47k in 10k..100k range; CLK has none.

### R_SD_CLK — 33R

0805W8F330JT5E / C17634 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | SD_CLK_MCU |
| 2 |  | SD_CLK |

Place near MCU PC12; 4-bit SDIO currently 8MHz.

### R_SD_DETECT — 10k

0805W8F1002T5E / C17414 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | SD_DETECT |

Active-low mechanical detect independent of DAT3.

### C_SD_HF — 100nF

CL21B104KBCNNNC / C1711 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |

### C_SD_BULK — 10uF

CL21A106KAYNNNE / C15850 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |

## 05_UI_IO

### J_OLED — PH2.0 4P

S4B-PH-SM4-TB(LF)(SN) / C265102 / SMD,P=2mm,Surface Mount,Right Angle

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |
| 3 |  | OLED_SCL |
| 4 |  | OLED_SDA |

SSD1306 1.3in 128x64, I2C address0x3C; short harness.

### R_SCL — 4.7k

0805W8F4701T5E / C17673 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | OLED_SCL |

Default fitted; DNP only if module already has suitable strong pull-ups.

### R_SDA — 4.7k

0805W8F4701T5E / C17673 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | OLED_SDA |

Default fitted; verify 400kHz rise time and total pull-up resistance.

### J_ENCODER — PH2.0 3P

S3B-PH-SM4-TB(LF)(SN) / C265101 / SMD,P=2mm,Surface Mount,Right Angle

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | GND |
| 2 |  | ENC_A |
| 3 |  | ENC_B |

Mechanical encoder A/B only; no push switch.

### R_ENC_A — 10k

0805W8F1002T5E / C17414 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | ENC_A |

### R_ENC_B — 10k

0805W8F1002T5E / C17414 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | ENC_B |

### J_FORWARD — PH2.0 2P

S2B-PH-SM4-TB(LF)(SN) / C295747 / SMD,P=2mm,Surface Mount,Right Angle

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | BTN_FORWARD |
| 2 |  | GND |

Active LOW. SoftStop is NOT hardware E-STOP.

### R_FORWARD — 10k

0805W8F1002T5E / C17414 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | BTN_FORWARD |

Software debounce; no default RC capacitor.

### J_BACK — PH2.0 2P

S2B-PH-SM4-TB(LF)(SN) / C295747 / SMD,P=2mm,Surface Mount,Right Angle

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | BTN_BACK |
| 2 |  | GND |

Active LOW. SoftStop is NOT hardware E-STOP.

### R_BACK — 10k

0805W8F1002T5E / C17414 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | BTN_BACK |

Software debounce; no default RC capacitor.

### J_SOFTSTOP — PH2.0 2P

S2B-PH-SM4-TB(LF)(SN) / C295747 / SMD,P=2mm,Surface Mount,Right Angle

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | BTN_SOFTSTOP |
| 2 |  | GND |

Active LOW. SoftStop is NOT hardware E-STOP.

### R_SOFTSTOP — 10k

0805W8F1002T5E / C17414 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | BTN_SOFTSTOP |

Software debounce; no default RC capacitor.

### J_HX711 — PH2.0 4P

S4B-PH-SM4-TB(LF)(SN) / C265102 / SMD,P=2mm,Surface Mount,Right Angle

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |
| 3 |  | HX_DOUT |
| 4 |  | HX_SCK |

Remote HX711 close to load cell. 3.3V only; DOUT must not be 5V.

### R_HX_DOUT — 33R

0805W8F330JT5E / C17634 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | HX_DOUT |
| 2 |  | HX_DOUT_MCU |

### R_HX_SCK — 33R

0805W8F330JT5E / C17634 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | HX_SCK_MCU |
| 2 |  | HX_SCK |

### C_HX_LOCAL — 100nF

CL21B104KBCNNNC / C1711 / 0805

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |

Board-side connector decoupling; remote HX711 requires its own local capacitors.

## 06_DEBUG

### J_UART — PH2.0 3P

S3B-PH-SM4-TB(LF)(SN) / C265101 / SMD,P=2mm,Surface Mount,Right Angle

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | GND |
| 2 |  | UART_TX |
| 3 |  | UART_RX |

UART 3V3 TTL 115200; TX/RX named from controller viewpoint.

### J_SWD — PH2.0 5P

S5B-PH-SM4-TB(LF)(SN) / C265104 / SMD,P=2mm,Surface Mount,Right Angle

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |
| 2 |  | GND |
| 3 |  | SWDIO |
| 4 |  | SWCLK |
| 5 |  | NRST |

3V3 is target voltage reference, not external power input.

### TP_24V_IN — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 24V_IN |

Top-access SMT test point; keep high-speed stubs short.

### TP_5V — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 5V |

Top-access SMT test point; keep high-speed stubs short.

### TP_3V3 — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | 3V3 |

Top-access SMT test point; keep high-speed stubs short.

### TP_GND — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | GND |

Top-access SMT test point; keep high-speed stubs short.

### TP_CAN_TX — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | CAN_TX |

Top-access SMT test point; keep high-speed stubs short.

### TP_CAN_RX — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | CAN_RX |

Top-access SMT test point; keep high-speed stubs short.

### TP_CAN_H — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | CAN_H |

Top-access SMT test point; keep high-speed stubs short.

### TP_CAN_L — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | CAN_L |

Top-access SMT test point; keep high-speed stubs short.

### TP_UART_TX — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | UART_TX |

Top-access SMT test point; keep high-speed stubs short.

### TP_UART_RX — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | UART_RX |

Top-access SMT test point; keep high-speed stubs short.

### TP_NRST — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | NRST |

Top-access SMT test point; keep high-speed stubs short.

### TP_BOOT0 — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | BOOT0 |

Top-access SMT test point; keep high-speed stubs short.

### TP_SD_CLK — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | SD_CLK |

Top-access SMT test point; keep high-speed stubs short.

### TP_SWDIO — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | SWDIO |

Top-access SMT test point; keep high-speed stubs short.

### TP_SWCLK — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | SWCLK |

Top-access SMT test point; keep high-speed stubs short.

### TP_USB_DM_RESERVED — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | USB_DM_RESERVED |

Top-access SMT test point; keep high-speed stubs short.

### TP_USB_DP_RESERVED — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | USB_DP_RESERVED |

Top-access SMT test point; keep high-speed stubs short.

### TP_BUCK_PG — SMT test point

5015 / C2906768 / -

| 引脚 | 数据手册名称（如指定） | 网络 |
| --- | --- | --- |
| 1 |  | BUCK_PG |

Top-access SMT test point; keep high-speed stubs short.

## 机械定义

| RefDes | 孔径 | 金属化/网络 | keepout | X/Y |
| --- | --- | --- | --- | --- |
| MH1 | 3.2mm | NPTH / 无网络 | Ø6mm | 待布局 |
| MH2 | 3.2mm | NPTH / 无网络 | Ø6mm | 待布局 |
| MH3 | 3.2mm | NPTH / 无网络 | Ø6mm | 待布局 |
| MH4 | 3.2mm | NPTH / 无网络 | Ø6mm | 待布局 |
