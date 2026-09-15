# arm_0 Prototype 0 六轴机械臂控制固件

`arm_6_axes_0`，版本 **0.0.1**。STM32F407VET6 / CAN1 / 6× CyberGear / STM32 HAL / 全静态 FreeRTOS；包含独立 FK、Jacobian、数值 IK、关节限位及同步五次关节轨迹。

**上电进入 DISABLED，不自动使能、不自动 HOME。** `Motor/robot_hw_config.h` 的六个 ID 均为 UNASSIGNED，`ARM_ENABLE_MOTOR_OUTPUT=0`。校准、协议配置确认和控制增益也未配置；只修改 ID 或打开编译开关仍不能运行机器人。

## 当前范围

- 三个静态应用任务：控制 200 Hz、CAN 事件驱动、监督 100 Hz；RX/TX/命令队列均静态。
- CyberGear 显式 29 位 ID 和大小端编码、反馈校验、使能/停止/置零/运动/位置模式/限幅接口。
- 数学核心与 HAL/RTOS 解耦；内部长度 mm、角度 rad、速度 rad/s、时间 s，电机力矩 Nm。
- RAM 示教记录/回放核心、蛋糕尺寸和 slot 映射接口。
- CubeMX `.ioc`、EIDE 工程、CMake 构建、OpenOCD/CMSIS-DAP 调试配置。

尚未完成：UART host 协议、USB、按钮、示教持久存储、蛋糕自动工艺、实物 motor ID/零位/方向、PCB、电源及急停硬件。Recipe 的 S/M/L 全部 TBD。手拖模式的重力补偿未实现。

## 构建

依赖放在本机 PATH：Arm GNU Toolchain（已测试 14.3.Rel1）、CMake ≥3.20、Ninja、Python 3。Host 测试另需本机 GCC 或兼容编译器。HAL/CMSIS/FreeRTOS 必要源码随仓库提交，新 clone 不需要下载子模块或机械仓库。

```powershell
cmake --preset firmware
cmake --build --preset firmware
cmake --preset host
cmake --build --preset host
ctest --preset host --verbose
python tools/kinematics_reference.py
python tools/check_invariants.py
```

产物：`build/firmware/arm_6_axes_0.elf`、`.bin`、`.hex`、`.map`。版本头在构建时生成，记录 HEAD（未提交修改加 `-dirty`）及机械 SHA。

EIDE：打开 `arm_6_axes_0.code-workspace`，选择 Debug / GCC，设置本机 Arm GCC 和 OpenOCD 路径。Build、Clean、Rebuild 由 EIDE 提供；产物在 `build/eide/Debug`。可通过 `python tools/build_eide.py` 检查已安装 EIDE 的真实构建后端（该附加脚本需要 PyYAML）；无需此脚本也能在 EIDE 构建。

## 硬件假设

HSE **8 MHz** → SYSCLK **168 MHz**，APB1 42 MHz、APB2 84 MHz。**HSE 频率依赖实际开发板**；若板上不是 8 MHz，先修改 `.ioc`、重新生成并核算 CAN timing。不得把 HSI 当作最终 CAN 时钟方案。

CAN1：PB8 RX / PB9 TX，AF9，1 Mbps。PA9/PA10 预留 USART1；PA11/PA12 预留 USB FS。必须使用外部 CAN 收发器。

## 机械基线与已知限制

机械 SHA：`1b1cc4982e09ccf0d09f9921e68e24940a76d877`，尺寸链 **180/320/290/60/86/90 mm**。CAKE_APPROACH 的 TCP 为 **[500, 0, 210] mm**。

所有关节 hard limit 都是 `CAD_PROVISIONAL_LIMIT`，没有完成实物验证。软限默认向内收 2°，仅为可配置开发值。STOW 的 J3=-150° 位于 CAD 下限，因此能进行 FK/硬限数学测试，但默认软限运动和回放会拒绝 STOW。不能用裁剪姿态来掩盖该差异。

CyberGear 手册 ±4π 与部分开源实现 ±12.5 的差异见协议文档；必须按电机固件实测后选择 profile。关节限位不能替代自碰撞/环境碰撞检查，数值 IK 收敛也不代表路径安全。

## 文档

- [构建与测试验收](docs/TEST_REPORT.md)
- [首次硬件调试](docs/BRINGUP.md)
- [机械基线](docs/MECHANICAL_BASELINE.md)、[运动学](docs/KINEMATICS.md)、[IK](docs/IK.md)
- [架构与并发](docs/ARCHITECTURE.md)、[软件安全边界](docs/SAFETY.md)
- [引脚](docs/HARDWARE_PINOUT.md)、[CAN timing](docs/CAN_TIMING.md)、[CAN 带宽](docs/CAN_BANDWIDTH.md)
- [CyberGear 协议与来源](docs/CYBERGEAR_PROTOCOL.md)、[后续工作](docs/NEXT_STEPS.md)

硬件状态：OpenOCD 可解析配置，但未发现 CMSIS-DAP。下载、CAN 波形及电机功能均 **NOT TESTED – PROBE/MOTOR NOT PRESENT**；host 测试与交叉编译不能代替硬件验收。
