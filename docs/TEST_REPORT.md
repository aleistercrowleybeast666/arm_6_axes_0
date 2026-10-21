# v0.0.2 构建与验收

## 2026-10-14 单编码器交互复验

- 用户反馈前一版OLED型号和菜单已实物确认；本轮未连接任何硬件、未烧录/复位、未运行电机或读写真实TF卡。
- 子菜单末尾返回、静态返回栈恢复选择和滚动；动作启动默认返回，蛋糕/记录复用原页面，回待机/个人轨迹共享确认页。
- 运行中编码器按下经直接软停锁存，先于菜单焦点/错误/状态页；旧前进记录保存、后退取消与专用软停路径保留。停止确认不续跑。
- Host CTest 5/5通过；新覆盖全部子菜单单编码器路径、空文件列表返回、默认取消、各种启动类型、动作前离线拒绝、长按/按下和释放弹跳、启动时按住、状态切换旧输入丢弃与新软停按下保留、运行时旋转无效、软停清空业务队列/锁存/不自动重启、原按键回归。
- CMake/Ninja firmware及host构建通过，-Wall -Wextra -Werror保留；不变量检查通过。Flash 89488 B，主SRAM 49856 B，CCM 49152 B。
- 生产C帧缓冲预览 `build/host/ui_encoder_preview.png` 已目视检查默认返回反白和“按下:软停”。预览不是实物照片。
- 新交互真实按键到软停延迟、栈余量、停止距离和负载保持仍未实测；软停没有被改称硬件急停。没有参数编辑新增、安装依赖或推送发布。

## 2026-10-14 离线OLED/TF菜单复验

- 本次CMake/Ninja firmware和host增量构建通过；自有源码继续-Wall -Wextra -Werror。ELF/BIN/HEX已更新，未上传/烧录。
- 最新Host CTest 5/5通过。新增覆盖：未接电机与启动超时后浏览、离线运动/录制拒绝、TF刷新与删除默认否/完整文件名确认、离线存储失败回传、运行态维护拒绝、压力状态区分、编码器按键独立消抖、生产UiRaster帧缓冲越界保护。
- 默认输出门0及六轴UNASSIGNED保留；不变量检查通过。CAN Start失败改为锁存总线故障且保留菜单服务，10ms vendor超时；该初始化降级只完成源码审查和交叉编译，未在真实CAN/MCU验证。
- Flash 88680 B / 512 KiB，主SRAM 49856 B / 128 KiB，CCM 49152 B / 64 KiB。菜单200字形子集静态约8000 B，不分发完整字体。
- 生产C渲染输出的主菜单、TF页和删除确认PBM转为 `build/host/ui_preview.png` 进行目视检查；统一16px单元和基线、标题+三行内容、反白及完整文件名确认可见。此预览不是实物OLED照片。
- 字体替换为Noto Sans Mono CJK SC Regular（Sans2.004，SIL OFL）；许可与版权在UI/Backends/FONT_LICENSE.txt。旧UI文档的PE2/PE3/PE4及CAN文档TIM6描述已对齐当前PE9/PE10/PE11和TIM1；编码器PC4按下确认已接入。
- 没有安装依赖、连接串口/SWD、发送硬件命令、烧录/复位、操控电机或读写真实TF卡；Hardware仍为NOT TESTED。实物屏幕控制器/地址、输入方向、TF卡行为和时序仍待确认。

## 2026-10-14 接线基准与编译修复复验

按用户提供的 `六轴机械臂.epro2` 中 U1 焊盘网络核对 IOC：I2C2 PB10/PB11、USART2 PA2/PA3、HX711 PD8/PD9、按键 PE9/PE10/PE11；完整对应见 [HARDWARE_PINOUT](HARDWARE_PINOUT.md)。本次使用工作区已有 CubeMX 生成文件修复集成，没有重新运行 CubeMX 生成器。

修复旧 I2C/UART getter、静态句柄及 IRQ 路由、旧 BSP 引脚与外设过滤，恢复七个静态应用任务、`vTaskDelayUntil` 和足够的任务优先级；FatFs 继续由 StorageTask 唯一初始化。HAL tick 配置统一为已有生成代码使用的 TIM1。

- CMake/Ninja `firmware --clean-first`：95步完整重编译通过，生成 ELF/BIN/HEX。
- EIDE 3.27.2 实际 `unify_builder --rebuild`：89个C文件、1个汇编文件通过，生成 ELF/BIN/HEX。直接使用工作区 `builder.params`，未安装附加脚本所需的 PyYAML。
- Host：5/5 CTest通过；独立运动学参考通过。
- 集成脚本重复运行：生成文件逐字节哈希一致。
- 扩展后的不变量检查：IOC / 生成引脚 / BSP 接线基准、应用集成、IRQ、静态内存、默认输出锁和链接符号检查通过。

CMake Flash 84228 B，主SRAM 49856 B，CCM 49152 B。未烧录或实机测试。

以下为先前版本的验收记录。

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
