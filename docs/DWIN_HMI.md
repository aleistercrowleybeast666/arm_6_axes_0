# DWIN DGUSII 串口屏软件第一阶段

## 基线和范围

仓库 `arm_6_axes_0`，既有 origin 为 `https://github.com/aleistercrowleybeast666/arm_6_axes_0.git`。
串口屏前基线提交 `b8e050bcc8370b706c4f6ccbc4ae8f9c073de41a`，annotated tag
`pre-hmi-baseline-2026-10-21`，标签对象 `c9e158d2ad694165a5273ac2c4f8a6756a20520d`。
2026-10-21 已推送 main 和此标签；远程标签解引用和 main 均核验为上述基线提交。
基线包含之前未提交的 CubeMX 接线集成、OLED 菜单/编码器操作、离线 TF 管理、字体许可及相关测试和文档。
构建目录、日志、ELF/BIN/HEX、机器路径缓存未纳入提交。基线验证：5 项 CTest、独立运动学参考、静态不变量、STM32F407 交叉编译通过。
当前 HMI 工作从该标签的独立 `feature/dwin-dgusii-hmi` 分支开始。

目标屏：用户已定 **DMG80480T070_09WTC / 800×480 / DGUSII**。
接口按用户给定的默认 RS232 继续等待客服确认。本次只实现 MCU 软件候选，未修改原理图、电气接口、网络名或 CH340。
STM32 管脚是 3.3V 逻辑，RS232 接口不能直接与 PA9/PA10 相连；收发器、接头、供电和引脚序号以确认后的硬件为准。

## 现有功能和首版菜单假设

现有软件包括：六轴 CyberGear CAN、FK/Jacobian/IK、硬/软限位、同步轨迹、启动摆位/零偏流程、
Factory 三类 Recipe、Custom 回放、手拖录制框架、TF 文件和压力校准存储、HX711、OLED/编码器/按钮及 USART2 调试。
真实手拖后端仍未 commissioning；电机 ID、增益和输出门维持现有默认。

首版复用 `LocalUi`，主菜单为「执行动作、记录轨迹、TF 卡、设备状态」。
沿用现有小/中/大工厂 Recipe、Custom 文件、回待机、启动确认、删除默认否、运行/录制状态、停止确认流程。
菜单允许离线浏览；开始、记录、回放仍由 LocalUi、CommandManager、OperationManager、RobotControl 和 MotorManager 逐级校验。
屏端不提供任意 AppCommand、关节目标、限位修改、输出使能、故障解除或生产参数写入接口。
该页划分从当前 UI 推导；800×480 图稿、触控排版、字体资源和具体页号可在用户确认后继续制作。

## 分层和默认开关

- `UI/dgus_protocol.*`：HAL 无关字节流协议，静态 99 字节缓存；重叠帧头、噪声、分片/粘包、CRC、准确字数、VP 边界和 50ms 帧间隙检查。
- `UI/hmi_dgus.*`：允许列表、界面上下文令牌、防重复确认、优先软停、只读状态及 UTF-16BE 文本。
- `BSP/hmi_uart.*`：独立 USART1 候选适配；RX 256/TX 512 字节队列、非阻塞中断发送、每次 UI 最多读取 64 字节、溢出/错误/发送超时重建会话。
- `App/service_tasks.c`：同一个 UiTask 拥有实体和触控输入，通过原有 `LocalUi_Handle` 和命令通路工作。已有调试回调分发至 USART1 与 USART2 各自模块。

`ARM_ENABLE_DGUS_HMI=0` 为默认；关闭时不配置 USART1、不打开其 IRQ、不访问候选管脚。
USART2 PA2/PA3 115200 工程调试保持现用配置。当前 `.ioc` 中 PA9/PA10 未分配，候选 USART1 使用 PA9 TX/PA10 RX AF7、115200、8N1、无流控。
HAL 适配置于 BSP；本阶段不修改 `.ioc`。`tools/check_invariants.py` 检查候选脚/IRQ 未与 CubeMX 新分配冲突。
后续若纳入 CubeMX 生成，须先迁移 USART1 唯一所有者和回调，不能并存第二个句柄或 IRQ 实现。

## 帧和 CRC

帧头 `5A A5`，长度包含指令、数据和可选 CRC；VP/word 为大端，CRC 为 Modbus CRC16（初值 FFFF、多项式 A001、低字节先发送），校验不含帧头和长度。
仅接收准确的 `82 4F 4B` 写应答和 `83 VP_H VP_L WORD_COUNT DATA...`。应答不生成按键；未知指令/VP/字数直接丢弃。
帧体上限 96 字节，覆盖本版变量与 32 字 Unicode 行文本；不宣称支持协议最大包。
CRC 默认开启（`ARM_DGUS_CRC=1`），屏端 CFG 的 `0x05.7` 必须一致；也可显式构建 CRC 关闭版。解析器不会尝试自动降级 CRC。
官方参考向量：读版本 `5A A5 06 83 00 0F 01 ED 90`；写应答 `5A A5 05 82 4F 4B A5 EF`。

## VP 合同 v1.0（本项目分配）

地址以 16 位 word 为单位，范围在常用曲线缓冲 1000–4FFF 之外。屏工程不得把下列区域同时用作 SP 或其他对象。

| VP | 方向 | 内容 |
|---|---|---|
| 5000–5001 | 屏 → MCU | 必须一帧上传 2 words：当前上下文令牌、按键码 |
| 5100–510F | MCU → 屏 | 16 words 状态块，定义见下表 |
| 5200–521F | MCU → 屏 | 32 words Unicode 标题/第 0 行 |
| 5220–523F | MCU → 屏 | 第 1 行 |
| 5240–525F | MCU → 屏 | 第 2 行 |
| 5260–527F | MCU → 屏 | 第 3 行 |

| 状态块 word 偏移 | 内容 |
|---|---|
| 0,1 | 协议 0100、当前上下文令牌 |
| 2,3 | LocalUi 页枚举、渲染行选中编号（0–3，是否有效见 flags） |
| 4,5,6,7 | StartupState、RobotState、OperationState、最近 ArmResult |
| 8 | flags：bit0 选中有效，1 错误页，2 命令等待，3 TF 已挂载，4 输出 armed，5 电机配置完成，6 CAN ready，7 压力 valid |
| 9,10,11 | 进度百分比、Custom 数量、TF 错误枚举 |
| 12,13 | command_revision 低/高 word |
| 14,15 | 在线电机数量、录制样本数 |

文本为 UTF-16BE/BMP，空余填 FFFF，屏端配置 Unicode 字库和相同字数；不可解码字符替换问号。
页枚举表示复用菜单状态，**不是已分配的 DGUS 物理图页编号**。状态/文本每 100ms 更新；文本成功入队后发布状态块，后者使当前令牌可接收输入。
同一瞬间仍受串口串行传输限制，不是屏幕原子刷新协议。

按键码：0 触摸已释放、1 下一个、2 上一个、3 编码器确认、4 返回、5 软停、6 结束录制并保存（仅 OP_RECORDING 接受）。
确认在运动时沿用编码器软停语义；录制取消使用返回。结束录制和软停有不同按钮，防止确认造成意外保存。

屏端须把最新 5101 令牌与当前按键组合为 `83 5000 02 TOKEN KEY` 自动上传帧。
每个普通事件只接收一次并推进令牌；屏端等到状态返回新令牌后，下一次新的触摸才能发送。
每个新令牌都先要求与之匹配的 key=0 释放/空闲报告，然后才允许普通按下。屏端应报告当前触摸状态，空闲时随新令牌报告释放，禁用长按自动重发；不能在尚未松手时伪造释放。
启动按住、长按跨页面及旧令牌的排队释放均不能把普通输入重新使能。
页面、选中项、运行状态、目录版本、命令结果、错误页或实体输入变化会作废旧令牌；不缓存待自动重试的启动事件。
重复、过期和未发布令牌不生效；传输溢出/错误作废半帧和普通事件。
有效软停帧优先于普通输入，可使用旧令牌；已解析的软停跨会话复位保留，随后调用现有 App_RequestSoftStop 锁存并清空业务队列。
串口屏软停不替代独立物理软停或硬件急停。令牌用于操作时序，具有 16 位回绕，不能作为认证或安全等级保证。

**尚未交付屏端 DWprj.hmi/DWIN_SET**。本阶段提供 MCU 接口合同和离线帧工具。
屏端需要配置能回传相邻两词（当前令牌+键码）的控件逻辑或 DGUS OS 辅助逻辑；具体控件配置需在 DGUS 工具验证，不能直接用普通单 word 按键回传代替。
没有生成猜测的屏端脚本或声称页面可下载。先完成屏工程和电气核对，再在禁止电机输出条件下做独立通信验收。

## 构建、离线验证和回滚

```powershell
cmake --preset host
cmake --build --preset host
ctest --preset host --output-on-failure
python tools/kinematics_reference.py
python tools/check_invariants.py
cmake --preset firmware
cmake --build --preset firmware
# 仅编译候选 USART1 实现；不执行、不烧录
cmake --preset firmware -B build/firmware-dgus -DARM_ENABLE_DGUS_HMI=ON -DARM_DGUS_CRC=ON
cmake --build build/firmware-dgus
python tools/dgus_hmi_frame.py --token 2 confirm
```

EIDE 递归包含 BSP/UI 源码，默认 HMI 关闭；需要候选编译时在其 defines 显式加 `ARM_ENABLE_DGUS_HMI=1`、`ARM_DGUS_CRC=1`。
无需重写 CubeMX 生成文件或安装串口库。帧工具只打印字节，从不枚举/打开 COM 或触碰硬件。
Host 测试覆盖 CRC 官方向量、噪声/分片/粘包、格式错误、超时回绕、两种 CRC 模式、各 10 万字节伪随机流、令牌/重复/旧页面事件、软停优先、Unicode、发送回压和未就绪运动拒绝。
补充覆盖：首次释放、跨页长按和旧释放帧不能解锁新上下文。
UART fake 测试覆盖开关两种配置、候选脚和 8N1、USART2 隔离、整帧回压、RX 溢出、接收/发送失败、丢失发送完成中断的 100ms 恢复。
现有 host 业务测试继续验证限位、软停、停止后不自动恢复等路径。软件测试不代表硬件或真实停止性能已验收。

2026-10-21 实际验证结果：8/8 CTest 通过；默认 OFF 与显式 ON+CRC 的 STM32 交叉编译均通过；静态不变量与独立运动学参考通过。
开启版 ELF 已核验包含 USART1_IRQHandler、HmiUart_Poll、HmiDgus_Push/Publish，CCM 仍为 48KiB，未链接动态分配符号。
默认关闭版 RAM 49,856B / Flash 89,520B；开启版 RAM 50,976B / Flash 92,828B（Debug 构建，后续代码/版本元数据可导致小幅变化）。
对应 host 结果保存在忽略的 `build/host/Testing/Temporary/LastTest.log`；固件产物位于 `build/firmware` 和 `build/firmware-dgus`，均不提交。

回滚选择：保持默认 HMI OFF，或在工作区干净时切回 `main`/基线标签。未来转台应从上述基线另开分支。
不得强制复位/清理用户新改动；若有其他未提交内容，先保存到其所属分支再切换。

## 官方依据

- [DWIN 工具下载](https://www.dwin-global.com/tool-page/)
- [T5L DGUSII Application Development Guide V2.921](https://ecdn6.globalso.com/upload/p/1355/source/2026-03/T5L_DGUSII-Application-Development-Guide-V2.921.pdf)，已下载核对：印刷页 33–35（CRC/波特率），44–45（通信协议/CRC 向量）。

本次没有刷固件、打开真实串口、启动电机、改变网络设置或改动原理图。硬件接口确认、屏端项目与实机通信/安全验收仍待进行。
