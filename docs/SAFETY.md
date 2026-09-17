# Prototype 0 软件安全边界

## 默认不会运动

1. `ARM_ENABLE_MOTOR_OUTPUT=0` 在 transport 层拒绝所有主动 CAN 数据发送，包括 Enable；默认 ID 全部 UNASSIGNED。
2. 即便开发时打开编译开关，仍需六个唯一有效 ID、已确认sign、启动runtime坐标建立、已实测协议 profile、有限有效的控制增益/限幅，并得到六轴在线反馈。
3. 默认配置停在启动等待/失败。完整commissioning配置下，人工STOW后自动捕获本次offset并执行固定STOW→HOME；不会验证真实姿态是否确为STOW。只有StartupManager可用J3 hard→soft单调逃逸；其他运动始终soft。READY需要新反馈确认enabled及HOME完成。
4. `enabled` 来自反馈，不因发出 Enable 就直接设真。

## 故障路径

- 运行中任一轴反馈超时（默认 100 ms）、fault、温度达到开发 trip、非法反馈映射、硬限越界：停止当前轨迹、FAULT、撤销输出。
- CAN bus-off/error、RX overflow、TX 过期/失败、控制 heartbeat 超时：锁存故障、撤销输出；正常 motion 不再发送。
- 控制周期超过 10 ms（正常 5 ms）会在下一次调度时停入故障；监督任务另外检查 15 ms heartbeat。没有启用独立 IWDG，无法保证 MCU 完全失控时的停止时间。
- 撤销时清空 TX 队列，请求中止三个邮箱，再 best-effort 发已知 ID 的 Disable。已上总线的帧无法撤回；总线断开时 Disable 不保证送达，软件不能代替硬件急停。
- RX 带硬件接收时刻的 ms timestamp，入队延迟不刷新 online 寿命。32 位 tick 差值按模减法计算，100 ms 超时可跨回绕。
- `Error_Handler`、RTOS assert、栈溢出和 CPU fault 记录 static volatile fatal reason，立即禁中断并把 CAN 置初始化/中止状态后停住。不在 fatal 路径依赖 RTOS 对象完整性或打印。

FAULT 不会被普通 Disable 自动清掉。清除请求需健康且全部电机未使能，回到 DISABLED；manager 的电机 fault 和 transport 的总线 fault 仍锁存，须诊断并重新初始化/复位，不静默恢复运行。

## 限位和轨迹

hard=CAD provisional，soft=参数化内收开发余量。起点、终点、采样均检查。五次标量曲线单调且位于 [0,1]，因此各轴在两个合法端点之间；这只是独立关节范围保证，没有碰撞证明。STOW 被默认软限排除，禁止自动夹到 -148° 再假称“原 STOW”。

物理SoftStop取消业务并生成受限减速轨迹，保持最终姿态；录制软停丢弃，不落盘。按钮上电按住也锁止启动，按住不接受新命令；解除和确认不会自动续播。若停止距离不满足限位，则Fault。底层Trajectory_Stop本身只取消采样，不独立制动。RobotControl 的故障/Disable 会转电机禁能，重力负载可能下落。实物第一次测试要有支撑，并核验刹车/机械约束；这些硬件不在本轮实现范围。

## 测试能证明什么

Host 测试覆盖状态转换、超时全轴停、重复/未分配 ID、使能反馈确认、队列撤销、故障锁存及编译/运行门。生产 transport 源码分别在 gate=0 和 gate=1 下测试，RTOS 队列与 HAL 邮箱采用静态 host fake。

这些测试不证明 RTOS 在真实 MCU 的时序、电气收发器、力矩限制参数是否被实际电机采用、物理 stop 行为或独立急停能力。位置/力矩精度、温升、动力学、碰撞和任务看门狗仍需后续硬件验收。

## 外设与持久存储

轨迹先全量校验再播放，坏CRC/模型/超限拒绝；CCM禁止DMA。记录以实际反馈50Hz采样，超长/间隔异常终止丢弃。临时文件验证后才rename；FAT不是事务存储，物理掉电测试未完成。

压力未校准/过期/过载时invalid，启用force_required的Factory必须拒绝或软停。当前Factory=NONE，不依赖未连接压力传感器。UART拒绝运动；普通菜单无工程参数。Teach backend未验证，保持关闭。

硬件E-stop必须独立断开电机动力链，软件SoftStop不可替代。生产碰撞、力学/制动、硬件急停、IWDG与真实时序仍未认证。
