# 第一次调试

## 1. 工程与工具

打开 `arm_6_axes_0.code-workspace`。EIDE 3.27.2 可读取 `.eide/eide.yml`（schema 4.1），目标 Debug、GCC、Cortex-M4、single FPU、hard float ABI。给 EIDE 配置本机工具链路径，不把安装路径提交到 Git。

已验证 Arm GNU 14.3.Rel1、OpenOCD 0.12.0（2025-07-10 发行构建）。CMake/Ninja 是可复现的另一入口；具体命令见 README。EIDE 的 before-build task 调用 `python tools/version_header.py`，所以 Python 3 必须在 PATH。EIDE 输出 ELF/BIN/HEX 到 `build/eide/Debug`。

`.vscode/launch.json` 提供 CMake 产物和 EIDE 产物两个 Cortex-Debug 配置；EIDE 产物需先 Build。`openocd.cfg` 使用 CMSIS-DAP、SWD、4000 kHz、stm32f4x，默认 software reset（`reset_config none`），没有要求探针必须接 NRST。

本机 CubeMX 6.15.0 存在一条非阻塞 `VP_RIF_VS_RIF1` 导入提示，已定位到工具本身的无条件 restore 逻辑；F407 没有该引脚，ioc 中也未配置它。见 ARCHITECTURE。CAN/HSE/RTOS 参数错误已修正；遇到其他配置报错仍应停止生成并核对，不能把所有报错当成这个工具问题。

## 2. 上电前

核实开发板为 STM32F407VET6，板上实际 HSE=8 MHz。不同晶振必须改 `.ioc`/时钟/CAN timing，重新生成并执行集成脚本。检查供电、3.3 V 逻辑兼容性和复位。

探针接 SWDIO=PA13、SWCLK=PA14、GND、3V3 reference，建议 NRST。不要把探针参考引脚与外部电源随意并联。

## 3. 下载与验证

先保持机械电机电源断开，仅接 MCU：

```powershell
openocd -f openocd.cfg -c "init; reset halt; flash write_image erase build/firmware/arm_6_axes_0.elf; verify_image build/firmware/arm_6_axes_0.elf; reset run; shutdown"
```

也可用 EIDE OpenOCD uploader（cmsis-dap / stm32f4x / 0x08000000），或 VSCode 的 Flash CMSIS-DAP task。首次核对固件版本、`App_GetFatalReason()`、DISABLED 状态以及 RTOS 栈余量。硬件未连接时不将“编译成功”写成下载成功。

## 4. CAN 物理连接

PB9→CAN transceiver TXD，PB8←RXD，AF9；收发器逻辑电平 3.3 V 兼容。CAN_H 对 H、CAN_L 对 L，保证适当共地/共模范围，双绞线，避免长支线。两个总线物理末端各 120 Ω，**不是每个电机都加一个 120 Ω**；断电后 H/L 间常见约 60 Ω 只在恰好两个终端且无其他支路影响时成立，依器件实际电路核对。

`BspCan_GetDiagnostics` 可查看 RX 数、队列溢出和 HAL 错误。默认没有主动扫描/读取 CAN motor ID；可配合厂家工具或分析仪观察来源 ID。没有初始化 USART/USB，不依赖串口日志。

## 5. 单电机阶段

第一次不要接六台一起使能，更不能直接运行 HOME。只接一台 CyberGear，以外部工具核验 ID、反馈格式、角度 profile 和物理方向；优先仅观察 RX。

默认 robot manager 需要六轴全部配置和健康，**不会为了单电机实验跳过六轴安全规则**。单电机 Enable 应在独立 commissioning 调试程序/外部厂家工具内进行，由操作者明确开启，使用已确认的低扭矩/低速度/低电流限幅；本固件不在启动时调用任何 commissioning 命令。

单电机验证后记录：实际 ID、编码器零位、joint_sign、协议 profile、读回参数和控制增益。六轴都完成后再将已确认配置接入 `MotorManager_Init`，显式启用输出编译门并经 Command API 请求 Enable。实物支撑和重力下落风险必须先处理。

## 6. 再生成流程

1. CubeMX 打开仓库 `.ioc`，保留 STM32Cube FW_F4 V1.28.3、Makefile 工程及 static allocation。
2. Generate Code。
3. `python tools/integrate_cubemx.py` 恢复静态应用任务、封装 handle 和错误处理；本次已验证重复执行/再生成。
4. Clean Build、host tests、`python tools/check_invariants.py`。
5. 检查 Git diff；尤其 HSE、CAN pins/timing、FreeRTOS allocation/IRQ、startup/linker。

本次硬件结果：**NOT TESTED – PROBE NOT PRESENT**。OpenOCD 实际 `init` 返回 `unable to find a matching CMSIS-DAP device`。未执行 flash/reset run；CAN 和电机均未实测。
