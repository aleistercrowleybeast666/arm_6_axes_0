# 机械数据来源

读取时间：2026-09-15。源：[arm_0/main](https://github.com/aleistercrowleybeast666/arm_0)。本次实际获取的 HEAD 为 `1b1cc4982e09ccf0d09f9921e68e24940a76d877`。

读取并保存了三个小型基线文件（`docs/mechanical_source/`），未 vendor 整个机械仓库：

- [arm_0_parameters.json](https://github.com/aleistercrowleybeast666/arm_0/blob/1b1cc4982e09ccf0d09f9921e68e24940a76d877/mechanical/arm_0_parameters.json)
- [joint_datums.csv](https://github.com/aleistercrowleybeast666/arm_0/blob/1b1cc4982e09ccf0d09f9921e68e24940a76d877/mechanical/docs/joint_datums.csv)
- [arm_0_v3_review.md](https://github.com/aleistercrowleybeast666/arm_0/blob/1b1cc4982e09ccf0d09f9921e68e24940a76d877/mechanical/docs/arm_0_v3_review.md)

JSON 内 `freeze_review_baseline_commit` 是机械仓库自身记录的旧审图来源，不能替代本次实际 HEAD。

| 数据 | 实际值 |
|---|---|
| BaseHeight / UpperArm / Forearm | 180 / 320 / 290 mm |
| Wrist / Flange / Tool | 60 / **86** / 90 mm |
| J1 | base → yaw，+Z |
| J2 | yaw → upper，父局部 -Y |
| J3 | upper → fore，父局部 -Y |
| J4 | fore → roll，父局部 +X |
| J5 | roll → pitch，父局部 -Y |
| J6 | pitch → tool，父局部 +X |

J4 电机实体在 J3 后的位置不参与 FK 原点计算；数学 J4 原点始终位于 290 mm 小臂末端。UpperLane/FoldLane 均为 0。

| 关节 | CAD provisional hard limit / deg | 默认开发 soft limit / deg |
|---|---|---|
| J1 | -170 … 170 | -168 … 168 |
| J2 | -15 … 100 | -13 … 98 |
| J3 | -150 … 100 | -148 … 98 |
| J4 | -180 … 180 | -178 … 178 |
| J5 | -120 … 120 | -118 … 118 |
| J6 | -180 … 180 | -178 … 178 |

`RobotLimits_MakeCad` 接收软限余量（rad）；默认 2° 不是最终实物安全余量。限位明确标为 `CAD_PROVISIONAL_LIMIT`。

| 姿态 | q / deg | 双精度 TCP / mm |
|---|---|---|
| STOW | 0,55,-150,0,95,0 | 329.039949671, 0, 93.460509840 |
| HOME | 0,52,-142,0,90,0 | 373.011672104, 0, 82.163441154 |
| SAFE_UNFOLD | 0,50,-110,0,60,0 | 556.692035100, 0, 122.025330474 |
| CAKE_APPROACH | 0,60.4965754675011,-72.4522811069283,0,-78.0442943605728,0 | 500,0,210 |

与用户 prompt 的预期数据一致，没有机械版本差异。STOW 被默认软限排除，这是保守软限和原始 CAD 姿态之间的明确冲突，未改写冻结姿态。

Python 参考按矩阵链独立计算，并逐项对照 CAD HOME CSV 的六轴原点和世界轴（double 容差 1e-8）。生成的 JSON 保存 q、各关节原点/轴、四元数和 TCP；C host 测试逐项比较。
