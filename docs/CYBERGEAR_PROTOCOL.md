# CyberGear 协议实现与来源

## 交叉检查来源

- [小米 CyberGear 使用手册英文镜像，§4.1/4.2](https://rosmo-robot.github.io/assets/img/CyberGear%E5%BE%AE%E7%94%B5%E6%9C%BA%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8E%E4%B9%A6%20en_compressed.pdf)：ID 位域、命令、参数索引、反馈物理量范围。
- [cybergear-robotics/cybergear](https://github.com/cybergear-robotics/cybergear/tree/ee4ce43ec659773314b053fb95b6889281117bb3)：实际读取 `cybergear.c` / `cybergear_defs.h`。
- [DanielKalicki/Xiaomi_CyberGear_Arduino](https://github.com/DanielKalicki/Xiaomi_CyberGear_Arduino/tree/c495e2797eecf27ace15f100fe74e20d2141c63d)：实际读取 driver.cpp / defs.h。

独立编写本项目驱动，没有复制参考业务源码。开源实现存在 option 参数被 uint8 截断、浮点 clamp 后仍复制原值或 clamp 次序错误等问题，因此没有直接采用其实现。

## 数据格式

29 位 ID：`type[28:24] | data[23:8] | destination[7:0]`。显式移位、掩码，不用 C bit-field；拒绝超过 29 位 ID。CAN 必须扩展数据帧、DLC=8；本协议没有额外应用 CRC，不添加 CRC。

Motion 的 16 位 data 是 torque 映射，不能塞入 host ID。payload：position、velocity、Kp、Kd 四个 big-endian uint16。反馈同样 big-endian：position、velocity、torque、temperature×10。

参数写的 index 是 little-endian uint16；bytes 2..3=0，byte4 开始为模式 uint8 或 little-endian IEEE-754 binary32。浮点通过 memcpy 获取位模式再显式字节序列化，不依赖 CPU endianness。NaN/Inf 拒绝；有限越界编码按协议上下界 clamp。高层 MotorManager 在编码前拒绝越过已配置控制限幅的指令，防止静默饱和掩盖规划错误。

## 协议范围差异

手册给出 position **±4π**，参考开源代码给出 **±12.5**。两者不等价，不能混用。`CyberGearProfile` 显式提供 `CG_PROFILE_MANUAL_4PI`（默认）和 `CG_PROFILE_LEGACY_12_5`；TX/RX 使用同一个 profile。配置 `protocol_confirmed=false` 时高层不能使能。实际电机固件必须用已知角度交叉校准后才能确认，host tests 不证明哪种匹配实物。

其余 motion 范围：velocity ±30 rad/s、torque ±12 Nm、Kp 0..500、Kd 0..5。电流 API 上限保守限定到 23 A，实际 commissioning 必须给出更低的已确认值。MotorManager 不自动指定生产控制增益或限幅。

## 已实现能力

| 帧 / API | 状态 |
|---|---|
| 0 discover | 独立编码器支持；无上电扫描/ID 分配 |
| 1 SendMotionCommand | position/velocity/torque/Kp/Kd；current_a 必须为 0（独立电流模式不与 motion 混用） |
| 2 ProcessFeedback | host、motor ID、type、DLC、EXT/RTR、mode、温度合理性校验 |
| 3 Enable / 4 Disable | 请求入队；enabled 只由新反馈确认，停止不自动清故障 |
| 6 SetMechanicalZero | 显式接口；已使能状态拒绝；仅电机掉电丢失的零位命令，不能代替机器人标定 |
| 18 SetPositionMode / SetLimits | 模式、速度/扭矩/电流限幅；参数编码还支持位置/速度/电流给定 |
| 21 fault feedback | 核对消息头后保守锁存原始故障存在，不猜测完整详细 payload 字节布局 |
| IsOnline / GetState | wrap-safe 超时检查与状态复制 |

Type 7 / 17 有明确 enum 和 ID 编解码，尚无完整 ID 改写/参数读回事务。位置模式接口当前不接入机器人控制器，机器人轨迹使用 motion mode。所有写入成功只表示入队，未实现按索引的参数读回确认事务。

反馈 bits 16..21 为故障；bits 22..23 为状态（0 reset / 1 calibration / 2 running，3 拒绝）。温度字段 >200°C 当作坏帧；实际软件 trip 默认为更低的 60°C 开发值。任何实际故障在 manager 锁存，未因下一帧恢复为 0 而清除。

本轮没有电机/总线实测。上述命令的物理行为、参数固件兼容性、反馈周期、温度以及角度 profile 全部 **NOT TESTED ON HARDWARE**。
