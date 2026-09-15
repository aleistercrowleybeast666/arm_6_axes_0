# 坐标系与运动学

单位：长度 mm，角度 rad，速度 rad/s，角加速度 rad/s²；电机力矩 Nm。`RobotMath_DegToRad` / `RobotMath_RadToDeg` 仅用于边界转换。

父到子变换：先沿父坐标系平移，再绕父局部指定单位轴旋转。齐次矩阵链等价于：

```text
Rz(q1) Tz(180) Ry(-q2) Tx(320) Ry(-q3) Tx(290)
Rx(q4) Tx(60) Ry(-q5) Tx(86) Rx(q6) Tx(90)
```

C 使用归一化四元数逐级组合；Python 参考使用 double 的 4×4 矩阵乘法和独立的矩阵转四元数。没有猜测 DH 参数。`RobotFK_ComputeFrames` 输出六个旋转后的关节姿态、旋转前变换到世界系的轴、TCP。`RobotFK_Solve` 只输出 TCP。

Jacobian 在世界坐标系表示：上三行 `axis × (tcp - joint_origin)`，单位 mm/rad；下三行 `axis`，单位 rad/rad。姿态误差为 `target * conjugate(actual)` 的最短旋转向量，与 Jacobian 在同一世界系。

四元数顺序为 `[w,x,y,z]`。Conjugate 是单位四元数的逆；Normalize 对零、NaN/Inf 返回错误。姿态计算不用 Euler 相减。

`robot_joint_map` 唯一负责机器人与编码器映射：

```text
q_robot = joint_sign * (q_motor - motor_zero_offset_rad)
q_motor = joint_sign * q_robot + motor_zero_offset_rad
dq_robot = joint_sign * dq_motor
torque_robot = joint_sign * torque_motor
```

sign 必须为 ±1。配置校准未确认时不能使能。代码没有额外假设外部传动减速比；若实物加入外部传动，应先扩展此唯一映射模块。

验证：有限差分对 250 个可复现随机姿态的全部 6 列做中心差分，步长约 1e-4 rad；位置差和旋转向量差均与 `J × Δq` 比较。单精度原点约 1e-4 mm 的消减误差决定了差分测试的绝对容差。数值见 TEST_REPORT。
