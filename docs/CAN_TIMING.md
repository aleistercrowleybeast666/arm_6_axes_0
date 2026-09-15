# CAN1 bit timing

CubeMX 6.15.0 实际生成 STM32F407VET6：HSE=8 MHz，PLLM=4、PLLN=168、PLLP=2、PLLQ=7。

```text
PLL input = 8 / 4 = 2 MHz
VCO = 2 × 168 = 336 MHz
SYSCLK = 336 / 2 = 168 MHz
PCLK1 = 168 / 4 = 42 MHz
PCLK2 = 168 / 2 = 84 MHz
CAN TQ = prescaler / PCLK1 = 3 / 42 MHz = 71.428571 ns
bit length = 1(Sync) + 11(BS1) + 2(BS2) = 14 TQ
bitrate = 42 MHz / (3 × 14) = 1,000,000 bit/s
sample point = (1 + 11) / 14 = 85.714286%
SJW = 1 TQ
```

APB1 的定时器频率是 84 MHz，但 CAN 输入时钟是 **42 MHz PCLK1**，不能按 APB timer doubling 计算 CAN bitrate。

Auto retransmission=DISABLE，Auto bus-off recovery=DISABLE，防止故障后无界重发过期运动目标或静默恢复。TX 邮箱完成/错误中断开启以记录失败；软件故障恢复需要显式诊断。

HSE 与实际板载晶振必须一致。变更 HSE 后，在 CubeMX 重算 PLL/CAN 并示波器或 CAN 分析仪实测 bit timing，不能只改宏。TIM6 给 HAL 提供 1 ms tick，SysTick 属于 FreeRTOS。
