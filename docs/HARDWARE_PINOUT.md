# Prototype 0 引脚

| 功能 | 引脚 | 配置 |
|---|---|---|
| CAN1_RX | PB8 | AF9；接外部收发器 RXD |
| CAN1_TX | PB9 | AF9；接外部收发器 TXD |
| HSE IN / OUT | PH0 / PH1 | 默认外部 8 MHz 晶振 |
| SWDIO / SWCLK | PA13 / PA14 | SWD 调试 |
| USART1 TX / RX | PA9 / PA10 | Future UART host，仅保留，不初始化 |
| USB FS DM / DP | PA11 / PA12 | Future USB FS Device，仅保留 |
| NRST | NRST | 建议接 CMSIS-DAP reset |

CAN_H/CAN_L 只能连接收发器总线侧；PB8/PB9 不能直接连差分总线。收发器逻辑侧必须兼容 MCU 3.3 V，并按具体器件处理 STB/EN。开发板型号及收发器型号尚未冻结，固件不驱动任何假定的收发器使能 GPIO。

CMSIS-DAP：SWDIO、SWCLK、GND、3V3 reference，建议 NRST；确认探针 3V3 引脚是电平参考还是供电输出，避免两个电源相互供电。USB/UART/按钮、电源、接触器不在本次范围。
