# MotorBackend

静态 const function table，由 MotorBackend_Get 返回唯一实现。MotorManager 持有 static 六轴配置、joint_sign/runtime zero，业务只看关节坐标。CyberGear 编码/解码及总线配置事务移入 Motor/Backends/cybergear_backend.c；原协议源和数学源保持不变。

## 坐标约定

q_robot = sign × (q_motor - runtime_zero)，q_motor = runtime_zero + sign × q_robot。速度和力矩按同一方向映射。Startup 从六轴新鲜 raw 和冻结 STOW 计算 runtime_zero = raw - sign×STOW；仅 MCU RAM 有效，不发送设备机械置零、不改设备 NVM。Track 文件永远保存 robot q。

## 甲方电机需要实现

| 接口 | 责任 |
|---|---|
| init | 验证唯一ID、驱动配置、限幅和 profile |
| process_feedback / get_feedback | 有界解析，保留采样时刻、raw位置/速度/力矩/温度/fault/enabled |
| enable / disable | 有界使能配置事务、撤销与停止；enabled 必须反馈确认 |
| set_command | 下发原始电机坐标命令，验证范围及期限 |
| is_online / get_fault | 超时与故障状态 |
| enter_teach_mode / exit_teach_mode | 实测可控拖动及安全恢复保持 |
| arm_output / is_armed / has_bus_fault | 保留输出门和故障锁存语义 |

换 backend 不修改 Track、Recipe、UI、Force、RobotMath；若编码器绝对坐标策略不同，另实现 StartupPositionPolicy。目前 ABSOLUTE_ENCODER 和 HOMING_SENSOR 仅枚举，返回未配置。

当前 CyberGear 保留协议 profile差异检查、36帧有界配置预算、在线/温度/fault/使能确认和 transport安全门。enter/exit teach 均返回 NOT_CONFIGURED：没有已验证低刚度增益、重力补偿或负载保持方案，不把禁能当成安全拖动。Host mock 的 teach 可用仅用于验证完整记录流程，不代表真实 backend 已完成 commissioning。
