# 有界 DLS 数值逆运动学

API：`RobotIK_Solve(target, seed, options, solution)`；所有存储固定大小、无递归、无动态分配。调用方可把上一次成功的 q 作为下一次 seed，以保留附近的解分支；不保证跨越奇异点仍保持分支。

使用 `(JᵀJ + λ²I) Δq = Jᵀe`，6×6 带主元高斯消元，不求通用逆。位置和姿态的误差与 Jacobian 使用相同权重：默认 position_weight=0.01/mm、orientation_weight=1/rad。各权重先乘对应行，因此正规方程自然包含权重平方。

默认：200 次迭代、位置容差 0.05 mm、姿态容差 0.0005 rad、每次最大单轴增量 0.15 rad、λ=0.02。所有选项检查 finite/正值，迭代上限硬封顶 2000。

使用整体步长缩放，最多 8 次回溯线搜索；候选关节裁剪到传入 limits。失败时逐步增大阻尼（最大 100），成功时逐步回到初始阻尼。regularized 正规矩阵的最小/最大消元主元比作为 condition proxy，**不是 Jacobian 的真实 condition number 或 SVD**。极小主元、病态代理或停滞可返回 `IK_SINGULAR_OR_POOR_CONDITION`。

返回 enum：`IK_OK`、`IK_MAX_ITER`、`IK_OUT_OF_LIMIT`、`IK_SINGULAR_OR_POOR_CONDITION`、`IK_INVALID_TARGET`、`IK_NUMERIC_ERROR`。输出包含最后合法迭代 q、误差、迭代数、主元比和已用阻尼。只有 `IK_OK` 可作为成功结果；其他结果的 q 仅供诊断。越界 seed 明确拒绝；`IK_OUT_OF_LIMIT` 也可能表示受到边界约束导致本次局部搜索失败，并非不可达的数学证明。

默认 IK 使用软限。为了验证原始 STOW，host 的冻结姿态测试显式传 CAD hard limit。测试同时覆盖原解 seed（应当零步完成）和扰动 seed；验收按 `FK(result)` 的位置/姿态误差，不要求每个关节精确回到源 q。

IK 不是硬实时控制周期的一部分：本轮命令队列接收关节目标。后续应在规划上下文执行 IK，验证成功和轨迹安全后再提交。没有动力学、碰撞或奇异点路径速度规划。
