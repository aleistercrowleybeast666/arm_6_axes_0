# 后续工作

1. 确认开发板晶振、电源、收发器、终端电阻，接探针做 MCU/HAL tick/RTOS/clock/CAN 位时序验收。
2. 单电机确认实际 ID、profile（±4π 或 ±12.5）、zero/sign、温度字段和故障。加入参数读回事务，验证 mode/限幅落地；不能把成功入队等同于电机已采用参数。
3. 六轴标定、真实 hard/soft limit、低速安全裕量和增益/力矩/电流限幅。确认外部传动；目前假设 joint 与电机输出一比一。
4. 量测 CPU/WCET、200 Hz jitter、CAN worst-case latency、任务栈高水位；按测量决定调度和 buffer，完成由 Supervisor 健康判定后喂 IWDG 的设计。
5. 增加碰撞约束、动力学/重力补偿和失能后负载支撑策略，才进入手拖实际示教。Recorder 输入必须来自实际反馈。
6. 选择 SPI NOR Flash 等持久介质，接入样本流接口、校验/断电恢复；RAM64 buffer 不是最终存储。
7. 确认蛋糕 S/M/L 尺寸、刀具/TCP、转台和工艺，填入 Recipe 配置，再规划自动轨迹。
8. 首版上位机使用 PA9/PA10 USART1，统一调用 Command API；设计正式 framing、事务应答、状态遥测，再扩展 USB/按钮/触屏。

本轮明确不实现 UART packet framing、USB CDC、按钮去抖、GUI、生产蛋糕路径、PCB、电源控制、接触器或硬件急停。
