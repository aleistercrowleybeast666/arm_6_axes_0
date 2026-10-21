# 架构与CubeMX维护

当前业务架构、任务/队列、所有权和坐标边界见 [SYSTEM_ARCHITECTURE](SYSTEM_ARCHITECTURE.md)。v0.0.1的显式Enable命令入口由LOCAL CommandManager替代；RobotCommand_Submit保留符号但拒绝命令，避免绕过Operation。

## 生成流程

CubeMX6.15.0 / FW_F4 V1.28.3生成；当前接线使用SDIO/FatFs/I2C2/TIM3/USART2/GPIO/EXTI/DMA，HAL时基为TIM1，详见 [HARDWARE_PINOUT](HARDWARE_PINOUT.md)。Generate Code后必须运行 `python tools/integrate_cubemx.py`，再clean build、host tests及invariants。脚本封装HAL handles、接入静态任务和错误路径、设置SVC/RTOS配置、CCM NOLOAD、移除main中的FatFs初始化；HAL/RTOS/FatFs vendor源保持原样。脚本可重复执行。

2026-10-14 修复：重新生成会保留用户区中的旧 `hi2c1/huart1` getter，却生成 `hi2c2/huart2` 声明；只消除编译错误还会留下旧 BSP 引脚、串口回调过滤和软停/HX711 中断分发。集成脚本现使用接线基准的句柄，检查声明是否存在，并统一 HAL tick 回调。恢复 `App_CreateTasks()`、`INCLUDE_vTaskDelayUntil=1` 和9级优先级（控制任务使用7级，Cube默认7级仅允许0..6），避免编译成功后仍无法运行应用。`python tools/check_invariants.py` 现检查 IOC、生成代码和 BSP 的基准一致性，未集成的占位应用会直接报错。

本轮修正I2C400k需Fast模式、SDIO DMA请求索引、FatFs SD卡检测/模板配置，最终实际Generate Code返回OK。生成的FATFS占位文件不参与构建，FatFs由StorageTask及自有disk adapter唯一拥有。

CubeMX 6.15.0 的已知本机提示：加载时仍可能出现 `Pin8 (VP_RIF_VS_RIF1) cannot be retrieved for this MCU`。当前ioc不包含RIF；外设引脚数量已随v0.0.2增加。读取本机 `com.st.microxplorer.mcu.Mcu` 的字节码确认：导入逻辑只在 DIE505 检测已有 RIF，却在未找到时对所有 die 尝试 restore RIF，导致 F407 的非阻塞提示。实际生成返回 OK；未修改 CubeMX 安装文件、未把 MCU 伪装成其他芯片。该工具提示需由 CubeMX 版本修复；不能宣称本轮消除了它。初次 CAN/HSE/静态分配字段错误已修正，未使用的 RTC HSE 派生分频也已设为 /8，避免超过其 1 MHz 上限。

禁止直接下载未经集成的Cube占位应用。CubeMX安装文件未修改；工具自身RIF提示仍可能出现，不等于工程生成失败，也不能将其他报错当作该提示忽略。
