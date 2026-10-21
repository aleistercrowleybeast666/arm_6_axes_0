# Prototype 0 引脚 v0.0.2

接线基准为用户提供的 `六轴机械臂.epro2`（2026-10-14，SHA256 `0153ec948997332d1ade3581a3cd030378ddf28d7d4b03d83fe14eb9a29403f1`）。已从工程内 U1（STM32F407VET6）的 PCB 焊盘网络及符号引脚编号核对以下信号；该基准优先于 `hardware/design/` 中的旧 Rev.A 设计意图。

| 功能 | 引脚 | 配置 |
|---|---|---|
| CAN1 RX / TX | PB8 / PB9 | AF9，1 Mbps，外部3.3V兼容CAN收发器 |
| HSE IN / OUT | PH0 / PH1 | 假设外部8 MHz |
| SWDIO / SWCLK | PA13 / PA14 | 调试；NRST建议接探针 |
| SDIO D0..D3 | PC8..PC11 | AF12，4-bit，外部上拉按板级设计确认 |
| SDIO CK / CMD | PC12 / PD2 | AF12，传输8 MHz |
| TF detect | PC7 | 暂定低有效、内部pull-up，需卡座开关 |
| OLED I2C2 SCL/SDA | PB10 / PB11 | OLED_SCL / OLED_SDA，AF4 open drain，400 kHz |
| Encoder TIM3 CH1/CH2 | PA6 / PA7 | AF2 TI12，16bit，4counts/detent假设 |
| Forward / Back | PE9 / PE10 | KEY_F / KEY_B，pull-up，低有效；Cube 标签 UI_START / UI_BACK |
| Encoder key | PC4 | KEY_E，pull-up输入；菜单中按下进入/确认，运行时按下软件软停；25ms消抖，无新GPIO/IRQ配置 |
| Software Stop | PE11 | KEY_S，pull-up，falling EXTI15_10，独立于菜单 |
| HX711 DOUT / SCK | PD8 / PD9 | EADC_DOUT / EADC_SCK，falling EXTI9_5 / push-pull输出低初始 |
| UART2 TX/RX | PA2 / PA3 | HC_MT / HC_MR，AF7，115200 8N1 |
| DCDC power good | PE2 | DCDC_PG，无内部上拉输入；当前业务未使用 |
| USB FS DM/DP | PA11 / PA12 | 仍预留，未实现USB |

HSE8→SYSCLK168 MHz，APB1/APB2=42/84 MHz，SDIO源48 MHz。SDIO DMA2 Stream3 RX / Stream6 TX channel4，SRAM DMA buffer。SD卡/OLED/HX711逻辑电平必须按模块确认，不能把5V逻辑直接接MCU；HX711 RATE硬件按80SPS预期接法核实。

## 中断优先级（越小越高）

CAN1 RX0/SCE、SDIO、DMA2 Stream3/6、EXTI9_5/15_10、USART2 均为5；configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY=5，FromISR调用符合数值>=5。TIM1 HAL tick=15，SysTick/PendSV=15，SVCall=0以启动首个任务。没有在高于RTOS阈值的IRQ调用FromISR。

CAN_H/L只能接收发器总线侧，末端终端电阻按拓扑配置。6S动力与逻辑供电、接触器、制动器、急停回路未由固件定义；硬件急停独立切断动力，不依赖GPIO、FreeRTOS或CAN报文。
