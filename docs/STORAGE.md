# TF / Storage

SDIO 4-bit，HAL DMA，FatFs，独占 StorageTask。上电只设置 SD handle；真正 HAL_SD_Init / wide bus / FatFs mount 在 StorageTask 中执行，无卡不会卡死 main。48 MHz SDIO clock 源、ClockDiv=4，对应传输时钟 8 MHz；初始化阶段 HAL 使用低速。实际卡、连线和吞吐尚未实测。

PC7 卡检测暂定低有效。DMA2 Stream3 RX / Stream6 TX，channel4，主 SRAM 512 B 对齐 sector buffer；FatFs 到 CCM 的数据始终经 SRAM 1024 B staging 复制。禁止 DMA 直接访问 CCM。DMA 完成标志后等待 card TRANSFER，1000 ms 有界超时并 abort。

## 所有权

4 个静态 request/response slots 加 ticket，非阻塞提交，任务通知唤醒，完整消费结果才释放槽。只有 StorageTask 调用 ProcessOne 和 FatFs；`_FS_REENTRANT=0` 合法的前提就是唯一所有者。`_USE_LFN=1` 静态 LFN，max31，codepage437，无动态 LFN heap。

Arena：FREE→STORAGE_LOAD→READY→PLAYER→FREE；FREE→RECORDER→STORAGE_SAVE→FREE。所有者发布由临界区保护，整条轨迹加载、验证且关闭文件后才 READY。运行中无轨迹文件读取；录制只写 RAM，结束并完成受控停止后才提交保存。48 KiB NOLOAD 区不依赖上电清零，writer 必须写满 header.count 对应样本后才能发布。

## 目录与命名

- Factory 固定 `/tracks/factory/small.bin`、`medium.bin`、`large.bin`；菜单常驻，仅缺文件不可执行；没有 factory 删除命令。
- Custom `/tracks/custom/*.bin`；文件名含后缀最多31 ASCII字节，stem只允许字母、数字、下划线、短横线，拒绝路径穿越和其他后缀。
- 扫描最多4096目录项，展示最多32有效头文件，按名字排序；更多记录报告 list_full。目录头检查只用于展示，播放前仍验证完整 payload。
- 显示名为文件 stem。电脑上改名后下次启动扫描自动采用新名称；长文件名由 LFN 支持，不支持中文文件名。
- 自动命名 Custom_001.bin..Custom_999.bin，跳过已存在的 bin 和 tmp，不覆盖用户轨迹。只允许删除 custom，菜单再次确认。
- `/config/force_cal.bin` 为可选独立校准文件；缺失不会把已挂载的卡判坏。

## 保存与断电

新建 `.tmp`（CREATE_NEW）→写显式 header/payload→sync→close→重新打开，完整 CRC/精确长度校验→close→rename `.bin`。任一步失败不发布新的可执行 bin，孤立 tmp 扫描忽略。mock 对保存过程各阶段注入错误；不是对真实 FAT 断电行为的认证。

FAT 本身不是事务文件系统；写目录/rename 时掉电仍可能破坏文件系统。校准覆盖先验证 tmp，再删除旧 bin、重命名，掉电可能丢失旧校准；缺失/损坏时保持 calibrated=false，不加载临时文件。不要声称保证保存成功或旧文件永不丢失。

不自动格式化、不创建伪 factory 动作、不提供在线换卡保证。卡在启动时挂载；IO 错误后标记 unavailable，检查并重新上电。文件时间固定2026-01-01（没有 RTC）；不作为真实记录时间。CubeMX 的 FATFS/App 和 Targets 保留生成证据但不参与编译，实际 backend 使用自有单所有者 disk adapter 与原始 FatFs core。
