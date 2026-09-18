# Rev.A 器件库审核表

厂家引脚、官方符号、封装焊盘为三个不同证据层。下面的状态来自采购清单；不能把商品页存在或封装名称相似等同于审核通过。

运行后以 `runtime_audit.json` 中的实际 UUID、引脚读回、PAD 记录补齐。编号自动匹配后仍须检查尺寸、极性、固定片、隐藏引脚和实物插合方向。

| 元件 | MPN / LCSC | 封装 | 库状态 | 数据手册 |
| --- | --- | --- | --- | --- |
| U_MCU | STM32F407VET6 / C28730 | LQFP-100(14x14) | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/eded9b3eda904d8bb3a84cf2887d6a5f.pdf?productCode=C28730) |
| U_BUCK | LMR38020FDDAR / C5149193 | SO-PowerPAD-8 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/ec6587c13899eda138625fa2c78da98a.pdf?productCode=C5149193) |
| U_LDO | AP7361C-33SP-13 / C4943338 | ESOP-8 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/ede62dc0e0b0cdea60995fdce24a9165.pdf?productCode=C4943338) |
| U_CAN | SIT1042AQT/3 / C5382542 | SOP-8 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/54c3cce6c41d58bd4bf29d90c12accc3.pdf?productCode=C5382542) |
| J_TF | TF-01A / C91145 | SMD | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/7650eac21a54674b08ad4ba21e17fceb.pdf?productCode=C91145) |
| J_POWER | XT30PW-M30.G.Y / C431092 | - | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/f40c503d729038bd14858056d73ccb74.pdf?productCode=C431092) |
| J_FORWARD, J_BACK, J_SOFTSTOP | S2B-PH-SM4-TB(LF)(SN) / C295747 | SMD,P=2mm,Surface Mount,Right Angle | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/2474e9261ed7c5f86b55d373e724589d.pdf?productCode=C295747) |
| J_ENCODER, J_UART | S3B-PH-SM4-TB(LF)(SN) / C265101 | SMD,P=2mm,Surface Mount,Right Angle | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/6c6fce4367275d84ec6a99bf09f1250f.pdf?productCode=C265101) |
| J_OLED, J_HX711 | S4B-PH-SM4-TB(LF)(SN) / C265102 | SMD,P=2mm,Surface Mount,Right Angle | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/6c6fce4367275d84ec6a99bf09f1250f.pdf?productCode=C265102) |
| J_SWD | S5B-PH-SM4-TB(LF)(SN) / C265104 | SMD,P=2mm,Surface Mount,Right Angle | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/d87682fbfaf1a1bc6a21af6a6a17fd27.pdf?productCode=C265104) |
| J_CAN | SM03B-GHS-TB(LF)(SN) / C514175 | SMD,P=1.25mm,Surface Mount,Right Angle | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/895c7a62b0771fc9d108814674460a56.pdf?productCode=C514175) |
| Y1 | 5032 8M 12PF 20PPM 4P / C5181482 | SMD5032-4P | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/4ede9566961a29c7940ef999c46fe124.pdf?productCode=C5181482) |
| F1 | 0451001.MRL / C3099 | 2410 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/00c9748756ca418792b5b22a2bd74996.pdf?productCode=C3099) |
| D_REVERSE | SS310 / C2848687 | SMA(DO-214AC) | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/99efa47dcb98c002cc80beb74904af65.pdf?productCode=C2848687) |
| D_INPUT | SMBJ33A / C173526 | DO-214AA(SMB) | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/2fdb55289569ebfc0bc21eab3853afe6.pdf?productCode=C173526) |
| D_CAN_H, D_CAN_L | SMAJ13CA-13-F / C134951 | SMA | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/3c9d77b4a0a6f95a3a2d0836916a6837.pdf?productCode=C134951) |
| L1 | SMDRI127-150MT / C40000 | SMD,12.3x12.3mm | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/aa59274d1174eb16f1491f7db2812ea7.pdf?productCode=C40000) |
| LED_PWR | ORH-B35A / C205441 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/4ccd793e0537a8e53b7b3f15c5c9c51e.pdf?productCode=C205441) |
| TP_24V_IN, TP_5V, TP_3V3, TP_GND, TP_CAN_TX, TP_CAN_RX, TP_CAN_H, TP_CAN_L, TP_UART_TX, TP_UART_RX, TP_NRST, TP_BOOT0, TP_SD_CLK, TP_SWDIO, TP_SWCLK, TP_USB_DM_RESERVED, TP_USB_DP_RESERVED, TP_BUCK_PG | 5015 / C2906768 | - | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/92094172d6d10050dfee88ee891cc96e.pdf?productCode=C2906768) |
| C_BOOT, C_VDD11, C_VDD19, C_VDD28, C_VDD50, C_VDD75, C_VDD100, C_VBAT, C_AVDD_HF, C_RESET, C_CAN_VCC_HF, C_CAN_VIO_HF, C_SD_HF, C_HX_LOCAL | CL21B104KBCNNNC / C1711 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/02336ea48ea44ca18c72517dd3cb7b47.pdf?productCode=C1711) |
| C_IN_HF | CL21B104KCFSFNE / C398816 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/027b5a29c43d158ed795bf73acd3fcbc.pdf?productCode=C398816) |
| C_AVDD_BULK, C_CAN_VCC, C_CAN_VIO | CL21B105KBFNNNE / C28323 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/02336ea48ea44ca18c72517dd3cb7b47.pdf?productCode=C28323) |
| C_VCAP1, C_VCAP2 | CL21A225KAFNNNE / C87994 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/cc213578a492448787051d6bc0e87012.pdf?productCode=C87994) |
| C_LDO_IN, C_LDO_OUT, C_MCU_BULK, C_SD_BULK | CL21A106KAYNNNE / C15850 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/02336ea48ea44ca18c72517dd3cb7b47.pdf?productCode=C15850) |
| C_OUT1, C_OUT2, C_OUT3 | CL32B226KAJNNNE / C309062 | 1210 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/02336ea48ea44ca18c72517dd3cb7b47.pdf?productCode=C309062) |
| C_IN | 12101C475K4T2A / C2325876 | 1210 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://www.lcsc.com/datasheet/C2325876.pdf) |
| C_IN_BULK | 63SXV47M / C178377 | SMD,D10xL12.6mm | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/128402af54cf4e6ebab28b0ce30e07fb.pdf?productCode=C178377) |
| C_HSE_IN, C_HSE_OUT | 0805CG180J500NT / C1797 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/09fe2daf22580a99fa7fea93ccb20fb7.pdf?productCode=C1797) |
| R_AVDD, R_STB | 0805W8F0000T5E / C17477 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17477) |
| R_BOOT0, R_BOOT1, R_RESET, R_SD_DETECT, R_ENC_A, R_ENC_B, R_FORWARD, R_BACK, R_SOFTSTOP | 0805W8F1002T5E / C17414 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17414) |
| R_FB_TOP, R_PG | 0805W8F1003T5E / C149504 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C149504) |
| R_RT | 0805W8F6492T5E / C17788 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17788) |
| R_FB_BOTTOM | 0805W8F2492T5E / C17571 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17571) |
| R_SCL, R_SDA | 0805W8F4701T5E / C17673 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17673) |
| R_SD_D0, R_SD_D1, R_SD_D2, R_SD_D3, R_SD_CMD | 0805W8F4702T5E / C17713 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17713) |
| R_TERM | 0805W8F1200T5E / C17437 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17437) |
| R_SD_CLK, R_HX_DOUT, R_HX_SCK | 0805W8F330JT5E / C17634 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17634) |
| R_LED | 0805W8F4700T5E / C17710 | 0805 | OFFICIAL_CATALOG_MATCH; SYMBOL_AND_PAD_AUDIT_PENDING | [厂家文档](https://datasheet.lcsc.com/datasheet/pdf/0a975aaa49b7c97f38a963127be4a823.pdf?productCode=C17710) |

## 重点审核

- MCU：100脚；电源脚不得隐藏遗漏；PA/PB/PC/PD/PE和晶体脚逐号匹配。
- LMR38020：EN=2，VIN=3，EP必须接地；不能套用其他Buck的通用8脚符号。
- AP7361C-SP：OUT=1，EN=2，IN=8，GND=4，EP接地；不要混用SOT223/其他SO8版本。
- SIT1042AQT/3：VCC=3，VIO=5，CANL=6，CANH=7，STB=8。
- TF：触点1–8、Cd与全部金属壳脚；额外焊盘不可自动按无网络处理。
- XT30：以实际塑壳+/-核对电气合同pin1/2与库焊盘，明确插合面。
- JST PH/GH：精确MPN，SMT固定片的导电属性和pin1视图。
- 肖特基、单向TVS、聚合物电容及蓝灯：阳/阴极、正/负端和丝印三方一致。
- HSE：5032四焊盘，1/3晶体、2/4外壳地；实际库不能套用同封装不同引脚型号。
- 四机械孔尚未创建PCB封装；本轮仅约束定义，不能报告已完成PCB尺寸验证。
