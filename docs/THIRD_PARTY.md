# 第三方来源

硬件工程由 STM32CubeMX **6.15.0** 实际生成，使用本机已安装的 **STM32Cube FW_F4 V1.28.3**。

- STM32 HAL：`Drivers/STM32F4xx_HAL_Driver/LICENSE.txt`，保持原始 vendor source。
- CMSIS core：`Drivers/CMSIS/LICENSE.txt`。
- CMSIS device：`Drivers/CMSIS/Device/ST/STM32F4xx/LICENSE.txt`。
- FreeRTOS V10.3.1：`Middlewares/Third_Party/FreeRTOS/Source/LICENSE`，保留版权及许可文件。
- `Core/` / linker / startup 包含 ST 生成的版权标头，应用集成仅修改生成的用户层文件，不改 HAL/CMSIS/FreeRTOS vendor 源码。

EIDE 配置参考本机已安装 3.27.2 的 schema 及附近正常工程的工具链/构建/uploader 字段；没有复制该工程业务模块。`tools/build_eide.py` 的附加 CLI 检查需要 PyYAML；固件本身没有此依赖。

CyberGear 资料链接、固定参考 SHA 和差异见 CYBERGEAR_PROTOCOL；参考仓库只用于交叉验证，没有 vendor 其业务源码。机械文件只保留三个必要的基线数据文件。

v0.0.2新增FatFs原始源码：Middlewares/Third_Party/FatFs/src，许可/版权见ff.c和相关文件头（ChaN及ST适配标头），未改第三方算法。HAL新增SD/UART/I2C等仍来自上述Cube包。

OLED菜单仅提交所需字符位图，不分发字体文件；当前位图由本机Microsoft YaHei渲染。tools/generate_ui_glyphs.py接受自行提供的字体，可替换为适合产品分发授权的字库；正式产品字库授权需另确认。
