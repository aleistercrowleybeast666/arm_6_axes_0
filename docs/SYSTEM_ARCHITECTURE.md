# 整机架构 v0.0.2

```text
Encoder / FWD / BACK -> LocalUi -> CommandManager [LOCAL authority]
                                      |
                                      v
StartupManager ----------------> OperationManager
                                      |
                          Recipe / TrackPlayer / TrackRecorder
                                      |
                           Motion / ForceCondition
                                      |
                               RobotControl (200 Hz)
                                      |
                                MotorManager
                                      |
                              static MotorBackend
                                      |
                          CyberGear -> CAN transport -> HAL

StorageTask -> StorageService -> static StorageBackend -> FatFs -> SDIO DMA
    |                    |
    +-- SRAM staging ----+-- CCM Track Arena (one owner)
SensorTask -> HX711 backend -> ForceSensor snapshot (N)
UiTask -> Display interface -> SSD1306 backend
DebugUartTask -> read-only snapshots / idle-only force calibration
SoftStop EXTI -> latch -> RobotControlTask -> controlled stop
Hardware E-stop -> power contactor / drive power chain (independent of MCU)
```

## 实时与所有权

RobotControlTask 独占 RobotControl、StartupManager、OperationManager、Player 和 Recorder 状态；5 ms 一次。不访问 FatFs、OLED 或 HX711，不做轨迹整表验证，不等待设备 IO。播放前 StorageTask 完整加载、校验 CRC、模型及连续曲线位置/速度/加速度极值，关文件后才发布 READY。播放仅从 CCM 读取。

| Task | 优先级 | 栈 words / bytes | 调度与工作 |
|---|---:|---:|---|
| RobotControl | 7 | 1536 / 6144 | 200 Hz；业务、运动和软停 |
| CanRx | 6 | 768 / 3072 | RX 事件及有界突发，泵送 TX |
| Supervisor | 5 | 512 / 2048 | 100 Hz；反馈、总线、控制心跳 |
| Sensor | 4 | 512 / 2048 | HX711 通知，20 ms 超时轮询 |
| UI | 3 | 1536 / 6144 | 输入 10 ms；OLED 20 Hz |
| Storage | 2 | 2048 / 8192 | 通知唤醒；独占所有 FatFs 调用 |
| DebugUart | 1 | 1024 / 4096 | 2 ms 轮询静态串口 ring |
| Idle | 0 | 128 / 512 | FreeRTOS 静态 idle |

总任务栈 8064 words / 32256 B；另有 4096 B MSP 保留区。栈高水位、WCET 和 200 Hz 抖动仍需上板测量。所有任务、CAN 队列、业务命令队列、存储请求/响应槽、UART ring 均静态；无 heap。业务命令为 8 项静态 ring；存储为 4 项带 ticket 的静态 mailbox，短临界区保护，结果消费前不复用槽。通知只负责唤醒，不承载请求内容。

可阻塞操作限于低优先级 Storage/UI：FatFs、带超时的 SDIO、I2C。HX711 在 SensorTask 读 24 bit，每个高脉冲短暂屏蔽中断，脉冲之间恢复；必须用示波器确认脉宽与最坏延迟。CAN ISR 仅入队；HX711 EXTI 仅通知；软停 EXTI 仅锁存。

## Startup 和安全

ASSUME_STOW：人工摆到 STOW → 等六轴 configured/online/fresh/no-fault → 捕获 motor raw → MCU runtime offset → 固定 STOW→HOME → READY。不是 Homing，也不会发送机械置零。仅该固定启动轨迹允许 J3 从 hard 下限单调进入 soft 区；进入后立即恢复 soft 检查。未来 ABSOLUTE_ENCODER / HOMING_SENSOR 目前明确返回未配置。

默认 IDs 未分配、增益/限幅未 commissioning、ARM_ENABLE_MOTOR_OUTPUT=0，因此真实机器不会自动动作。上电已按住软停也能被电平检查识别；按住期间清空/拒绝新命令。启动中止后需要重新正确摆位并重新启动。

SoftStop 在控制任务内取消业务，清空排队命令，按已配置加速度生成速度连续的减速曲线，检查停止位置；保持电机 enabled 的末端姿态，确认后也不自动续播。停止距离无法留在限位内、反馈/总线故障走 Fault 撤销输出。软件停止不保证断电后的重力保持。

## 更换硬件和后续扩展

更换电机实现 MotorBackend；更换 ADC 实现 ForceBackend；更换显示器实现 DisplayBackend；业务均只用标准关节坐标、N 和 UiModel。没有运行时插件、动态注册或依赖注入。旧 Teach/RecipeBook 数学接口保留作基础测试，整机操作使用新的 Track/FactoryRecipe。

Raspberry Pi 未来只能通过明确的新命令源/协议进入 CommandManager；当前只接受 LOCAL。J7、挤奶油 AUX、生产级力闭环未实现。6S 动力、电机电源、逻辑降压、接触器和硬件急停由电气方案确认；不要把 MCU soft stop GPIO 当作动力切断链。
