# 架构与CubeMX维护

当前业务架构、任务/队列、所有权和坐标边界见 [SYSTEM_ARCHITECTURE](SYSTEM_ARCHITECTURE.md)。v0.0.1的显式Enable命令入口由LOCAL CommandManager替代；RobotCommand_Submit保留符号但拒绝命令，避免绕过Operation。

## 生成流程

CubeMX6.15.0 / FW_F4 V1.28.3实际生成；新增SDIO/FatFs/I2C1/TIM3/USART1/GPIO/EXTI/DMA。Generate Code后必须运行 `python tools/integrate_cubemx.py`，再clean build、host tests及invariants。脚本封装HAL handles、接入静态任务和错误路径、设置SVC/RTOS配置、CCM NOLOAD、移除main中的FatFs初始化；HAL/RTOS/FatFs vendor源保持原样。脚本可重复执行。

本轮修正I2C400k需Fast模式、SDIO DMA请求索引、FatFs SD卡检测/模板配置，最终实际Generate Code返回OK。生成的FATFS占位文件不参与构建，FatFs由StorageTask及自有disk adapter唯一拥有。

CubeMX 6.15.0 的已知本机提示：加载时仍可能出现 `Pin8 (VP_RIF_VS_RIF1) cannot be retrieved for this MCU`。当前ioc不包含RIF；外设引脚数量已随v0.0.2增加。读取本机 `com.st.microxplorer.mcu.Mcu` 的字节码确认：导入逻辑只在 DIE505 检测已有 RIF，却在未找到时对所有 die 尝试 restore RIF，导致 F407 的非阻塞提示。实际生成返回 OK；未修改 CubeMX 安装文件、未把 MCU 伪装成其他芯片。该工具提示需由 CubeMX 版本修复；不能宣称本轮消除了它。初次 CAN/HSE/静态分配字段错误已修正，未使用的 RTC HSE 派生分频也已设为 /8，避免超过其 1 MHz 上限。

禁止直接下载未经集成的Cube占位应用。CubeMX安装文件未修改；工具自身RIF提示仍可能出现，不等于工程生成失败，也不能将其他报错当作该提示忽略。
