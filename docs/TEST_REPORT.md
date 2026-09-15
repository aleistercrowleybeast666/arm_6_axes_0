# 0.0.1 构建与验收记录

日期：2026-09-15。机械源 HEAD：`1b1cc4982e09ccf0d09f9921e68e24940a76d877`。代码验收对应本报告所在提交；最终固件 HEAD 由 `Version_GetInfo()` / 构建头查询。

## Environment

| 项目 | 实际版本 / 配置 |
|---|---|
| STM32CubeMX | 6.15.0，DB.6.0.150，实际生成并再生成 |
| STM32Cube | FW_F4 V1.28.3 |
| FreeRTOS | 10.3.1，ARM_CM4F port，static only |
| Arm GCC | 14.3.Rel1 / GCC 14.3.1 20250623 |
| Host GCC | 16.1.0，Windows UCRT64 |
| Python | 3.14.0，参考计算只用标准库 |
| EIDE | 3.27.2，schema 4.1，真实 unify_builder 3.11.1 后端编译 |
| OpenOCD | 0.12.0，2025-07-10 构建；CMSIS-DAP / SWD |

## Firmware

STM32F407VET6，HSE 8 MHz → 168 MHz，APB1/APB2 42/84 MHz。CAN1 PB8/PB9 AF9，1 Mbps，3 / 11TQ / 2TQ / SJW1，sample point=85.714%。

| 验证 | 结果 |
|---|---|
| CubeMX 真正生成 HAL / startup / linker / FreeRTOS | PASS |
| CubeMX 最终参数 CAN/HSE/RTOS/RTC 检查 | PASS；仍有下述工具自身的 RIF 提示 |
| CMake/Ninja Arm GCC clean build | PASS，ELF / BIN / HEX |
| EIDE 配置驱动的真实 backend rebuild | PASS，ELF / BIN / HEX |
| 自有代码 `-Wall -Wextra -Werror` | PASS，0 warning |
| 单精度 math core `-Wdouble-promotion` | PASS |
| 静态任务/队列及 ELF 无 allocator 检查 | PASS |
| OpenOCD 配置解析（noinit / shutdown） | PASS |

EIDE 仅对第三方 HAL/RTOS 的 unused-parameter warning 设置豁免；未为消除 warning 改动第三方源文件。CMake 也分别编译第三方和自有代码。最终 Debug 使用 `-Og`、单精度 FPU hard ABI。

链接时保留 `RobotFK_Solve`、`RobotJacobian_Compute`、`RobotIK_Solve` 和 `Version_GetInfo`，可在实际 ELF 中查到；其他未调用的纯核心接口可被 linker GC，完整实现仍参与交叉编译及 host tests。

内存报告（含构建时版本元数据，SHA 字符串长度变化会使 Flash 小幅变化）：

| 区域 | 占用 | 容量 |
|---|---|---|
| Flash | 约 30.5 KiB | 512 KiB |
| 主 SRAM | 20,928 B（15.97%） | 128 KiB |
| CCM | 0 B | 64 KiB |

主 SRAM 占用已包括 4 KiB 主栈保留区、三个应用任务栈、idle 栈、静态控制块及队列。heap=0，不编译 heap_N/sysmem；`malloc/calloc/realloc/free/pvPortMalloc/vPortFree/_sbrk/ucHeap` 未进入 ELF。实际各任务栈高水位尚未实测。

## Mechanical / kinematics

独立 Python double 的矩阵链，与 CAD HOME 六轴原点/轴逐项一致（检查容差 1e-8）。C 使用 float 四元数，不复用 Python 算法。

| FK 姿态 | C float TCP / mm |
|---|---|
| STOW | [329.039948, 0, 93.460533] |
| HOME | [373.011627, 0, 82.163414] |
| SAFE_UNFOLD | [556.692017, 0, 122.025398] |
| CAKE_APPROACH | [500.000000, 0, 210.000000] |
| ZERO | [846.000000, 0, 180.000000] |

- **FK tests PASS**：TCP、关节原点、世界轴、关节及 TCP 四元数；最大位置参考差 **0.000068665 mm**。
- **Jacobian finite difference PASS**：250 个随机合法姿态 × 6 列，中心差分约 1e-4 rad。最大位置增量误差 **0.000253941 mm**，最大旋转向量增量误差 **4.69841893e-7 rad**。
- **IK roundtrip PASS**：4 个冻结姿态 + 100 个随机姿态，每个验证原 seed 和扰动 seed。最大最终位置误差 **0.049687 mm**、姿态误差 **0.000268888 rad**；均满足 0.05 mm / 0.0005 rad 容差。
- **IK 非正常输入 PASS**：不可达/奇异附近不输出 NaN，迭代有界；NaN target、越界 seed 拒绝。IK 成功不是全工作空间收敛保证。
- **Limits PASS**：STOW 在 CAD hard limit 合法、在默认 soft limit 被拒绝；边界距离、非有限值拒绝。
- **Trajectory PASS**：起点/终点速度和加速度为零，同步结束，1000 次采样均满足软限和指定 v/a 极值；非法起点/终点、零位移、Stop 后行为。

## Protocol / motion / safety

- CyberGear golden ID / payload 向量、big-endian motion、little-endian float 参数、clamp、NaN、错误 DLC/RTR/29-bit ID/mode、零位方向双向映射：PASS。
- RAM teach recorder/player 时间插值、样本单调性、容量满、速度/加速度不可满足时拒绝：PASS。
- Recipe S/M/L 未配置、Custom 几何校验及 teach/trajectory slot 映射：PASS。
- 控制器 safe boot、未分配/重复 ID、36 帧配置/中性命令/Enable 序列、新反馈确认 READY、超时全轴停止、FAULT 锁存、tick wrap：PASS。
- 生产 transport 源码 gate=0 测试：Enable/Disable 都被拒绝，实际 mock HAL TX 次数为零：PASS。
- 生产 transport gate=1 测试：64 帧队列容量、邮箱忙、撤销后不再发旧帧、允许 best-effort stop、过期队列、tick wrap、bus-error 锁存：PASS。

CTest 共 3 个测试程序，全部 PASS。控制测试使用模拟 CAN transport；transport 两个测试使用静态队列/邮箱 fake。没有假称 RTOS 并发或真实物理电机已经通过。

## Hardware test

- CMSIS-DAP：**NOT TESTED – PROBE NOT PRESENT**。实际 OpenOCD `init` 返回 `unable to find a matching CMSIS-DAP device`。
- reset halt / flash write / verify / reset run：未执行；没有硬件成功记录。
- CAN：初始化与 IRQ/队列已交叉编译，协议已 host 测试；收发器、电气时序、总线负载未实测。
- Motors：未使能、未运动、未做实物 ID 或 profile 假设。编译默认 `ARM_ENABLE_MOTOR_OUTPUT=0`。

## CubeMX 弹窗调查

本次修复了最初的 CAN 字段名（BS1/BS2/SJW）、HSE 引脚名、FreeRTOS MEMORY_ALLOCATION 枚举、未使用 RTC 分频，以及 SVC 优先级。最终正常生成返回 OK。

仍有 CubeMX 6.15.0 自身的非阻塞 `Pin8 (VP_RIF_VS_RIF1) cannot be retrieved for this MCU`：ioc 没有 RIF。读取本机 Mcu 类导入逻辑，确认其在缺失 RIF 标志时对 F407 也尝试 restore 仅适用其他 die 的 RIF。没有修改安装程序或伪造 MCU；保留该限制，详情见 ARCHITECTURE。

## 重现

```powershell
python tools/kinematics_reference.py
cmake --preset firmware
cmake --build --preset firmware --clean-first
cmake --preset host
cmake --build --preset host --clean-first
ctest --preset host --verbose
python tools/check_invariants.py
python tools/build_eide.py
```
