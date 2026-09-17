# v0.0.2 构建与验收

日期：2026-09-18。起点固件 `0f9b6c76d7b931886a07c9db6b79f25b0fc2969f`；本报告随本轮提交保存，最终固件SHA见 Version_GetInfo / build/generated/version_build.h。机械SHA保持 `1b1cc4982e09ccf0d09f9921e68e24940a76d877`。

## 工具与生成

| 项目 | 实际结果 |
|---|---|
| CubeMX 6.15.0 / FW_F4 V1.28.3 | 实际生成SDIO/FatFs/I2C1/TIM3/GPIO/EXTI/UART/DMA，返回OK |
| integration script | 连续执行两次，生成文件hash相同；PASS |
| Arm GCC 14.3.Rel1 / 14.3.1 | CMake/Ninja clean firmware build PASS |
| EIDE 3.27.2 / unify_builder 3.11.1 | 实际配置驱动rebuild PASS，ELF/BIN/HEX |
| Host GCC 16.1.0 UCRT64 | clean build PASS，全部5组CTest PASS |
| Python 3.14.0 | 标准库Track工具、独立运动学reference PASS |
| OpenOCD 0.12.0 | init返回 unable to find a matching CMSIS-DAP device；未下载/运行 |

CubeMX本轮I2C Fast模式、SDIO DMA请求索引、FatFs卡检测/模板问题已修正。工具自身非阻塞RIF提示仍可能出现，见ARCHITECTURE，不宣称已消除此工具缺陷。未改HAL/CMSIS/RTOS/FatFs vendor源码。

## 验证命令

```text
python tools/integrate_cubemx.py
cmake --preset firmware
cmake --build --preset firmware --clean-first
cmake --build --preset host --clean-first
ctest --preset host --verbose
python tools/kinematics_reference.py
python tools/check_invariants.py
python tools/build_eide.py
```

自有C使用-Wall -Wextra -Werror；portable核心额外-Wdouble-promotion。第三方warning策略独立，没有为消除warning改写供应商源码。

## 内存

Debug -Og交叉构建，本轮最终源文件与dirty版本元数据，字节数可能随clean SHA字符串/链接顺序少量变化：

| 构建 | Flash | 主SRAM | CCM |
|---|---:|---:|---:|
| CMake | 84148 B / 512 KiB | 49856 B / 128 KiB | 49152 B / 64 KiB |
| EIDE | 84160 B / 512 KiB | 49864 B / 128 KiB | 49152 B / 64 KiB |

CCM Track Arena为4096×6×int16=49152 B，起址0x10000000，结束0x1000C000，NOLOAD，剩余16384 B。DMA staging在主SRAM。主SRAM包含七应用任务+Idle共32256 B静态栈、TCB/队列/缓冲以及4096 B MSP保留。无动态heap；ELF检查malloc及newlib重入allocator/_sbrk未链接；显示、日志无动态分配。

## Host coverage

1. **robot_foundation**：原四元数/FK/Jacobian/IK/限位/轨迹/Teach/Recipe/CyberGear/安全测试保留。FK最大误差0.000068665 mm；Jacobian250姿态/1500列，位移误差0.000253941 mm；IK104组最大0.049687 mm / 0.000268888 rad。RobotMath/RobotMotion/CyberGear协议源与起点未改。
2. **robot_business**：BIN版本/CRC/模型/长度/量化；Python生成BIN由C校验；50→200Hz Hermite连续速度及解析限位；Storage扫描/路径/删除/改名/列表上限；actual记录、4096边界、回绕与漏采样拒绝；临时保存各阶段故障注入；HX711符号扩展、校准CRC、迟滞消抖、过期/过载；正负sign offset、Startup逃逸、普通STOW拒绝；Custom保持/无hook、Factory hook；播放/记录软停、中止加载、Teach锁；停止速度/加速度/距离检查；校准存取及缺校准不损坏TF状态；UI菜单/删除确认/状态页/消抖/编码器回绕、UART运动拒绝。
3. **track_tool**：Python解析固定向量，CRC/模型/长度/损坏拒绝。
4. **transport_gate_0**：生产transport默认门关闭，不发送Enable/Disable/运动帧。
5. **transport_gate_1**：静态HAL/RTOS fake覆盖批量容量、邮箱忙、撤销、过期、回绕、总线故障锁存。

结果：**5/5 PASS，0 failed**。独立Python机械参考及几何源SHA检查PASS；不变量递归检查全部自有模块、backend隔离、FatFs调用边界、RTOS IRQ、CCM尺寸和默认输出锁。

## Hardware: NOT TESTED ON HARDWARE

没有发现CMSIS-DAP；本轮没有下载或在目标MCU执行。真实电机/CAN波形、SDIO卡与物理掉电、OLED、HX711力值/波形/采样率、编码器/按键、软停距离、硬件动力急停、栈高水位和控制WCET/抖动均未实测。Host mock和静态分析不等于Hardware PASS。

IDs/gains/output默认关闭；真实CyberGear Teach未配置；factory生产BIN和压力策略/量程/阈值待提供与测量；无碰撞规划或生产级力闭环。TF为启动挂载，异常后重新上电；FAT rename无事务掉电保证。详细边界见NEXT_STEPS、SAFETY、STORAGE。
