# Prototype 0 主控板 Rev.A — 原理图设计包

## 当前状态（2026-09-19）

已完成六页、99 个元件、38 种采购料号的引脚级设计源、BOM、官方 API 生成器和审图资料。**尚未生成客户端原生 `.epro` 工程，尚未执行 EasyEDA ERC。** 网关已连接、客户端已切在线模式；工程记录已创建，但图页初始化失败，随后API报告`typeSymbols`错误。最后回读图页列表为空；不能宣称原理图已在客户端画好。当前交付为待客户端执行与审查的设计包。

| 层级 | 结果 |
| --- | --- |
| 电路源与 CubeMX 引脚核对 | 10 项离线检查通过 |
| 生成器的引脚、封装解析及保护逻辑 | 7 项单元测试通过；模拟 API，不是客户端验收 |
| 官方库的实际符号与封装 | 38种已查到官方目录；符号/焊盘待实际放置审核 |
| 原生可编辑工程、实际图面检查、网表比对 | 待执行 |
| EasyEDA ERC | **NOT RUN** |
| PCB | **NOT STARTED — waiting schematic review** |

固件、IOC 和已有业务架构未修改。机械孔只定义尺寸和约束，未创建 PCB。

## 文件入口

- [原理图生成源](tools/design.py)：元件、引脚、网络、图页位置。
- [展开后的设计](design/controller_revA.json)：生成器直接读取的完整数据。
- [采购信息快照](design/parts.json)：厂家、完整 MPN、LCSC、规格、数据手册、库存时间。
- [BOM](bom/arm_6_axes_0_revA_bom.csv)：99 个安装元件行，另含 4 个非采购机械孔行。
- [可安装扩展](easyeda/arm-six-axes-reva-schematic.eext)及[源码](easyeda/extension/src/generator.js)。
- [接口表](docs/CONNECTORS.md)、[电源预算](docs/POWER_BUDGET.md)、[CAN](docs/CAN_DESIGN.md)、[机械约束](docs/MECHANICAL_PCB.md)。
- [ERC 清单](docs/ERC_CHECKLIST.md)、[审图报告](docs/DESIGN_REVIEW.md)、[库审核表](docs/LIBRARY_AUDIT.md)、[全部引脚连接](docs/PIN_NET_REVIEW.md)。

## 六张原理图

| 页 | 内容 |
| --- | --- |
| 01_POWER | XT30、保险、反接、TVS、LMR38020、AP7361C、蓝色电源灯 |
| 02_MCU | STM32F407VET6、全部供电、VCAP、HSE、BOOT、RESET |
| 03_CAN | SIT1042AQT/3、双电源去耦、SMAJ13CA、固定 120Ω、GH |
| 04_STORAGE | 带检测 TF 座、4-bit SDIO、上拉、CLK 串阻、去耦 |
| 05_UI_IO | OLED、编码器、三按钮、HX711、PH 接口 |
| 06_DEBUG | UART、SWD、18 个测试点、4 孔机械定义 |

## 在官方 API 中生成

### 方式 A：现有 run-api-gateway（推荐）

使用官方 [API 文档仓库](https://github.com/easyeda/easyeda-api-skill)中的 `scripts/bridge-server.mjs`，以及官方 [run-api-gateway 扩展](https://jlc-ext.com/item/oshwhub/run-api-gateway)。本次桥接服务已启动于 `127.0.0.1:49620`。客户端扩展需真正加载并连接该服务；仅切换“已启用”而无连接不足以调用 API。不要重复启动占用同一端口的服务。

在已保存旧工程后，准备一个**没有打开工程的新窗口**。生成器拒绝切换已有工程，防止官方 `openProject` 丢弃未保存更改。

从仓库根目录运行：

```powershell
python hardware/easyeda/run_bridge.py --status
python hardware/easyeda/run_bridge.py --generate
```

`--status` 只读探测官方规定的 49620–49629 端口。多窗口时必须提供 `--port 49620 --window-id <实际ID>`，不随意选择。生成时按料号查询官方系统库，要求完整 MPN、LCSC、符号及封装关联匹配；没有可靠匹配就停止，不猜 UUID、不替换器件。

成功时新建独立工程，放置六页元件，按实际符号引脚端点连线并设置网络端口，未用引脚标 NC，读取封装编号审核，保存、调用 `sch_Drc.check`，导出 `.epro`、每页 SVG/网表及 `runtime_audit.json` 到 `hardware/easyeda/output/<UTC时间>/`。保存本地文件要求客户端扩展的外部交互权限。

运行失败会保留检查记录及客户端中的部分工程；**不自动重试写操作，不把部分工程当成完成品**。先检查报告、修正具体库差异，在新的空窗口重新生成。实际导出目录默认被 Git 忽略，审图通过后再明确加入版本控制。

### 方式 B：本地扩展

在专业版扩展管理中安装本包 `.eext`，从空窗口的 `Arm Rev.A → Generate Rev.A in empty window` 运行。成功后官方保存接口提示导出审计 JSON 和可编辑 `.epro`。此方式也未在当前断开的客户端中实测。菜单操作由用户完成；本任务没有使用 GUI 自动点击。

### 修改后重建

```powershell
python hardware/tools/design.py
python hardware/tools/check_design.py
# 在 hardware/easyeda/extension 中：
pnpm install --frozen-lockfile --ignore-scripts
node build.mjs
node --test tests/generator.test.mjs
# 回到仓库根目录：
python hardware/tools/package_extension.py
python hardware/tools/review_tables.py
```

Python 仅使用标准库。扩展锁定 `esbuild 0.24.2`、`@jlceda/pro-api-types 0.4.23`；不上传数据手册、第三方库缓存、个人配置或依赖目录。官方 API 参考版本：文档仓库 `ccfaf28a577b61a09ebc907f0a943d1e6c782def`，SDK `2fb050c7889ac5ab64388472f9ba29f9d21b0631`。

## 交付边界

当前 BOM 的 `Footprint` 已记录官方目录封装名，附 `[UNVERIFIED]` 标记。采购料号是真实查询结果，但封装 UUID、尺寸和极性尚未在客户端完成三方核对；因此这不是可直接下单生产的版本。详见审图报告，不进入 PCB 阶段。

## 恢复工作

已创建工程记录 `arm_6_axes_0_Prototype0_RevA_20260918164505853`，UUID `6c9d811095b7492da48fa71b8e2a6dc2`。最后回读图页列表为空；请先在客户端确认此工程能够正常打开并初始化原理图文档。不要直接重复新建工程。用户要求遇到必须人工处理的阻塞就停止，本轮已保留 [现场记录](review/session_status.json) 和 [38种器件的官方目录对应](review/library_catalog_audit.json)。

确认原有操作已结束、该工程确实为空且可以正常操作后，才可使用 `--resume-empty-project <上述UUID>` 恢复。已有图页的工程会被此选项拒绝，需先检查内容和实际状态。当前`.eext`与脚本包含上述客户端问题的防护，但尚未完整执行成功；不得按已验收脚本直接进入PCB。
