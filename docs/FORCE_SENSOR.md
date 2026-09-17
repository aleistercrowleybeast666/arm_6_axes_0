# ForceSensor / HX711

业务统一使用 N，与 HX711 raw 解耦。snapshot：raw、force_n、timestamp_ms、valid、calibrated、contact、overload、stale。默认无有效校准、无生产接触阈值；缺传感器不妨碍 Custom 或 force policy=NONE 的 Factory。

## Backend 和时序

当前 HX711 A 通道 gain128，24 bit signed，读后第25个脉冲选择下一次 A128。DOUT EXTI 只通知 SensorTask；任务先检查 ready 才读取。DWT 计时，高/低脉冲目标各1 us，每个高电平短暂关中断防止抢占把 SCK 拉高到 power-down 时间，低电平恢复中断。读取不在 ISR / ControlTask。RATE 接法尚未确认，工程预期80 SPS；校准格式可记录10/80，软件不会凭空改变 RATE 引脚。

原始数值 median3→IIR alpha0.2→按 zero 和 N/count 转换。150 ms 未更新视为 stale；ADC rails、非有限值、瞬时未滤波 N 或滤波 N 超过已配置 overload_n 都置 invalid/overload。接触检测用 on/off hysteresis + 连续稳定样本数；无效或过期数据清接触状态。过滤延迟、噪声、饱和、采样率、极性均待实测。

ForceCondition 支持 NONE（立即完成）、CONTACT、RELEASE、STABLE 的阈值/迟滞/持续时间/超时结构；RIPPLE_BELOW、RESERVED 明确 NOT_CONFIGURED，未实现奶油质量判断。三个 Factory 独立 force hook，当前 NONE/HOLD，实际压力阈值和结束动作仍 TBD。

## 校准与调试

UART：`force raw`、`force`、`force tare`、`force cal <known_force_N>`、`force save`。tare/cal/save 仅在启动完成且机器人 idle 状态允许；不会发运动命令。tare 记录 zero 并使旧校准失效，cal 用已知正负载 N 计算 N/count；返回值和 valid 状态明确显示。

**首次工程校准还必须通过 ForceSensor_SetCalibration 配置实测 full_scale_n、overload_n、sample_rate_hz 和有效初始比例，或加载工程生成的有效校准文件。**当前 UART 不提供这些量程设置项。仅输入已知负载不能把未知量程的设备变成 calibrated=true。默认全空时 cal 返回 NOT_CONFIGURED；不能为了让菜单可用编造量程。校准保存异步交给 StorageTask。

文件 `/config/force_cal.bin` 固定40 B，LE：0 magic A6FC[4]；4 version u16=1；6 size u16=40；8 zero_raw i32；12 newton_per_count IEEE754 binary32；16 full_scale_n f32；20 overload_n f32；24 sample_rate_hz u32；28 valid u8=1；29..35 reserved=0；36 CRC32前36字节（同 Track CRC）。zero必须24bit范围，scale有限非0，0<overload<=full_scale，rate为10/80。损坏/缺失拒绝，不自动套用默认 calibration。

参考：[Avia HX711 datasheet](https://akizukidenshi.com/goodsaffix/hx711.pdf)。上述接口/超时为本工程实现；电气和力学效果 NOT TESTED ON HARDWARE。
