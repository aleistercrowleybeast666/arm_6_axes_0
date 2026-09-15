# 架构、任务与所有权

```text
CAN1 RX IRQ → static RX queue → CanRxTask → cybergear_protocol → motor_manager
                                                              ↓ snapshot
future UART/USB/buttons → RobotCommand_Submit → RobotControlTask → trajectory
                                                              ↓ 6-axis batch
CAN1 mailboxes ← bsp_can ← can_transport static TX queue ← motor_manager

SupervisorTask → health/timeout/overflow/heartbeat → revoke output + latched fault request
```

| 任务 | 频率 / 唤醒 | FreeRTOS 优先级 | 静态栈 |
|---|---|---|---|
| RobotControlTask | vTaskDelayUntil，5 ms / 200 Hz | 5（最高应用） | 1536 words / 6144 bytes |
| CanRxTask | RX queue 事件，最多等待 1 ms 后服务 TX | 4 | 768 words / 3072 bytes |
| SupervisorTask | vTaskDelayUntil，10 ms / 100 Hz | 3 | 512 words / 2048 bytes |
| FreeRTOS idle | 内核 | 0 | 128 words / 512 bytes |

Tick=1000 Hz；无软件 timer task，`configUSE_TIMERS=0`。`configSUPPORT_STATIC_ALLOCATION=1`，`configSUPPORT_DYNAMIC_ALLOCATION=0`。不编译 heap_N.c / sysmem.c，不调用 malloc / pvPortMalloc。ELF 用 nm 检查没有 allocator 符号；链接脚本 heap=0。所有任务与 RX32/TX64/command8 队列静态创建。

## 所有权和并发

- 控制器和轨迹只由 RobotControlTask 修改。监督任务只写短临界区保护的 fault request，并立即撤销 transport 输出。
- motor_manager 的六轴数组是文件内 static；所有读写用短的可嵌套任务临界区。控制每次获得一致的六轴 snapshot；snapshot 是内存一致，不意味着六电机物理采样同时发生。
- CAN 队列使用 FreeRTOS 静态 API。RX ISR 最多读取三帧并赋 timestamp/入队；不解析 CyberGear、不打印、不求 IK、不阻塞。
- CAN IRQ=5，符合 `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY=5`；HAL TIM6=15；SysTick/PendSV=15；SVCall=0，必须高于 BASEPRI=5，才能启动首个任务。TIM6 是 HAL 时基，SysTick 给 FreeRTOS。
- TX 批量检查和提交由临界区保护；transport 串行访问 HAL mailbox。失败、撤销会清空软件队列和请求中止三个邮箱。每次 pump 最多填 3 帧，没有无限忙等。
- CAN 的 HAL 启动先于任何 FreeRTOS 对象/临界区创建，确保启动超时可以由 TIM6 驱动；随后建队列、开通知，再启动调度器。

## 命令接口

`RobotCommand_Submit` 面向未来 UART/USB/buttons/host，仅任务上下文，非阻塞；支持 Enable、Disable、JointMove、ClearFault。返回 `ARM_OK` 表示接受；最终处理状态通过 `RobotCommand_GetLastResult(&sequence)` 查询，这是一个“最近命令结果”接口，不是可靠的多请求事务协议。

Disable 单独锁存，不受 command queue 满影响；立即撤销输出并清空积压命令。其他命令以固定队列传给控制任务。启用动作需显式命令，校验六轴配置/在线/软限后才允许；发 mode/限幅/零增益中性 motion/Enable，等待新反馈确认六轴 enabled，才能 READY。默认配置永远无法通过该链。

READY 用最近测量姿态作保持目标；RUNNING 使用同步五次关节轨迹；结束回 READY。命令中的 torque feedforward 暂为 0，没有机器人动力学或重力补偿。此基础不包含实际负载下已调好的控制参数。

## Math / teach / recipe

RobotMath 完全不包含 HAL/RTOS。RobotMotion 的纯 C 轨迹器按速度/加速度极值同步选时。Teach 的固定 RAM64 样本用于接口验证；Recorder 参数必须来自新鲜实际反馈。Player 自有样本副本，按原时间使用停到停的五次段插值，段间速度/加速度为 0；超出给定速度/加速度限的记录会拒绝加载。尚未与运行命令/UI/Flash 接通。

RecipeBook 由调用者持有，没有全局可变 recipe；S/M/L 未配置，Custom 只验证有限正尺寸和用户提供的 slot。尺寸有效不等于机器人可达或工艺可执行。

## 生成与构建维护

`.ioc` 由本机 CubeMX 6.15.0 实际生成并再次加载/生成验证。HAL/CMSIS 来自 STM32Cube FW_F4 V1.28.3，FreeRTOS 内核 V10.3.1。CubeMX 的默认空闲应用任务由 `tools/integrate_cubemx.py` 转为三个原生静态任务。该脚本还封装 CAN handle、接入 fatal reason、补齐 FreeRTOS 检查、改写生成 Makefile 为 CMake 入口；不改 HAL/vendor 源码。

CubeMX 6.15.0 的已知本机提示：加载时仍可能出现 `Pin8 (VP_RIF_VS_RIF1) cannot be retrieved for this MCU`。仓库 ioc 只有 8 个有效引脚（索引 0..7），不包含 RIF。读取本机 `com.st.microxplorer.mcu.Mcu` 的字节码确认：导入逻辑只在 DIE505 检测已有 RIF，却在未找到时对所有 die 尝试 restore RIF，导致 F407 的非阻塞提示。实际生成返回 OK；未修改 CubeMX 安装文件、未把 MCU 伪装成其他芯片。该工具提示需由 CubeMX 版本修复；不能宣称本轮消除了它。初次 CAN/HSE/静态分配字段错误已修正，未使用的 RTC HSE 派生分频也已设为 /8，避免超过其 1 MHz 上限。

再生成：CubeMX 打开 `.ioc` → Generate Code → `python tools/integrate_cubemx.py` → 构建和 host tests → `python tools/check_invariants.py`。每次都必须执行集成脚本，不能直接下载 CubeMX 的占位应用。启动文件和 linker 均来源于实际生成，未伪造 HAL 工程。

MCU：512 KiB Flash，128 KiB 主 SRAM（0x20000000），64 KiB CCM（0x10000000）。当前 CCM 未使用，CAN buffer/队列都在主 SRAM；DMA 不能访问 CCM。主栈保留 4096 bytes，FreeRTOS 各任务栈另计。硬件 stack high-water、任务 WCET 和周期抖动尚未测量。
