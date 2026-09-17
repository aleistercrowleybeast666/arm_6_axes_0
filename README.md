# arm_0 Prototype 0 六轴机械臂控制固件

`arm_6_axes_0`，版本 **0.0.2**。STM32F407VET6 / CAN1 / 6× CyberGear / STM32 HAL / 全静态 FreeRTOS；包含独立 FK、Jacobian、数值 IK、关节限位及同步五次关节轨迹。

**默认不会运动**：六轴ID未分配、增益/限幅未 commissioning，`ARM_ENABLE_MOTOR_OUTPUT=0`。完成实物配置后启动策略为：人工摆到冻结 STOW → 捕获本次 runtime offset → 自动固定 STOW→HOME → 主菜单。无 Homing、无设备机械置零。

## 当前范围

- 保留原 HAL/CAN/FK/Jacobian/IK/限位/同步关节轨迹；新增 Startup/Command/Operation、静态 MotorBackend。
- SDIO4bit + FatFs 单任务所有者；BIN v1；48 KiB CCM、4096点、50Hz实际反馈录制，200Hz连续速度播放。先整条加载RAM再执行，录制结束才写TF。
- Factory三类独立Recipe、Force Hook（当前NONE，工艺TBD）；Custom播放结束保持、不自动HOME、不调用Factory Hook。
- ForceSensor N抽象 + HX711、校准持久化；OLED本地两项主菜单、编码器/FWD/BACK、独立软停、只读UART调试与空闲校准。
- 七个静态应用任务；全部静态内存；真实CyberGear手拖尚未调试，明确返回NOT_CONFIGURED。

未提供真实生产轨迹、蛋糕尺寸、压力阈值或安全增益。USB、Pi GUI、J7、挤奶油、Homing、动力学补偿和生产级力闭环不在本轮实现范围。

## 构建

依赖放在本机 PATH：Arm GNU Toolchain（已测试 14.3.Rel1）、CMake ≥3.20、Ninja、Python 3。Host 测试另需本机 GCC 或兼容编译器。HAL/CMSIS/FreeRTOS/FatFs 必要源码随仓库提交，新 clone 不需要下载子模块或机械仓库。

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

CAN1：PB8 RX / PB9 TX，AF9，1 Mbps。PA9/PA10 为115200 UART调试；PA11/PA12 仍预留USB。新增引脚见 HARDWARE_PINOUT。必须使用外部 CAN 收发器。

## 机械基线与已知限制

机械 SHA：`1b1cc4982e09ccf0d09f9921e68e24940a76d877`，尺寸链 **180/320/290/60/86/90 mm**。CAKE_APPROACH 的 TCP 为 **[500, 0, 210] mm**。

所有关节 hard limit 都是 `CAD_PROVISIONAL_LIMIT`，没有完成实物验证。软限默认向内收 2°，仅为可配置开发值。STOW 的 J3=-150° 位于 CAD 下限，只有 StartupManager 的固定、单调 STOW→HOME 使用 hard→soft 逃逸；普通运动和回放仍拒绝 STOW。不能用裁剪姿态来掩盖该差异。

CyberGear 手册 ±4π 与部分开源实现 ±12.5 的差异见协议文档；必须按电机固件实测后选择 profile。关节限位不能替代自碰撞/环境碰撞检查，数值 IK 收敛也不代表路径安全。

## 文档

- [整机架构与RTOS](docs/SYSTEM_ARCHITECTURE.md)、[普通用户流程](docs/USER_WORKFLOW.md)
- [轨迹格式与PC工具](docs/TRACK_FORMAT.md)、[TF所有权与保存](docs/STORAGE.md)
- [压力与校准](docs/FORCE_SENSOR.md)、[本地菜单](docs/UI_LOCAL.md)、[电机替换接口](docs/MOTOR_BACKEND.md)
- [测试验收](docs/TEST_REPORT.md)、[首次调试](docs/BRINGUP.md)、[安全边界](docs/SAFETY.md)
- [引脚/IRQ](docs/HARDWARE_PINOUT.md)、[生成维护/CubeMX提示](docs/ARCHITECTURE.md)、[后续工作](docs/NEXT_STEPS.md)
- [机械基线](docs/MECHANICAL_BASELINE.md)、[FK](docs/KINEMATICS.md)、[IK](docs/IK.md)、[CyberGear](docs/CYBERGEAR_PROTOCOL.md)

本轮软件结果见测试报告；**NOT TESTED ON HARDWARE**。Host和cross build不能替代实际电机、TF、OLED、HX711、按键或急停验收。
