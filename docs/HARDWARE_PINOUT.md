# Prototype 0 引脚 v0.0.2

除既有 CAN/HSE/SWD 外，新外设引脚均为工程分配，需与实际 PCB/模块核对后使用。

| 功能 | 引脚 | 配置 |
|---|---|---|
| CAN1 RX / TX | PB8 / PB9 | AF9，1 Mbps，外部3.3V兼容CAN收发器 |
| HSE IN / OUT | PH0 / PH1 | 假设外部8 MHz |
| SWDIO / SWCLK | PA13 / PA14 | 调试；NRST建议接探针 |
| SDIO D0..D3 | PC8..PC11 | AF12，4-bit，外部上拉按板级设计确认 |
| SDIO CK / CMD | PC12 / PD2 | AF12，传输8 MHz |
| TF detect | PC7 | 暂定低有效、内部pull-up，需卡座开关 |
| OLED I2C1 SCL/SDA | PB6 / PB7 | AF4 open drain，400 kHz，需外部上拉 |
| Encoder TIM3 CH1/CH2 | PA6 / PA7 | AF2 TI12，16bit，4counts/detent假设 |
| Forward / Back | PE2 / PE3 | pull-up，低有效 |
| Software Stop | PE4 | pull-up，falling EXTI4，独立于菜单 |
| HX711 DOUT / SCK | PE0 / PE1 | falling EXTI0 / push-pull输出低初始 |
| UART1 TX/RX | PA9 / PA10 | AF7，115200 8N1，工程调试 |
| USB FS DM/DP | PA11 / PA12 | 仍预留，未实现USB |

HSE8→SYSCLK168 MHz，APB1/APB2=42/84 MHz，SDIO源48 MHz。SDIO DMA2 Stream3 RX / Stream6 TX channel4，SRAM DMA buffer。SD卡/OLED/HX711逻辑电平必须按模块确认，不能把5V逻辑直接接MCU；HX711 RATE硬件按80SPS预期接法核实。

## 中断优先级（越小越高）

CAN1 RX0/SCE、SDIO、DMA2 Stream3/6、EXTI0/4、USART1 均为5；configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY=5，FromISR调用符合数值>=5。TIM6 HAL tick=15，SysTick/PendSV=15，SVCall=0以启动首个任务。没有在高于RTOS阈值的IRQ调用FromISR。

CAN_H/L只能接收发器总线侧，末端终端电阻按拓扑配置。6S动力与逻辑供电、接触器、制动器、急停回路未由固件定义；硬件急停独立切断动力，不依赖GPIO、FreeRTOS或CAN报文。
