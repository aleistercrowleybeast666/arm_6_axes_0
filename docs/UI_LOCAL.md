# 本地 UI

纯状态机 LocalUi 的输入为 ENCODER_NEXT / PREV / FORWARD / BACK，输出 UiModel 四行 UTF-8 文本、选择行和 AppCommand。它不调用电机或 FatFs。DisplayBackend 静态接口接收模型；当前 SSD1306 I2C 128×64，7bit地址0x3C、400 kHz（模块型号/地址待确认），固定字形和1 KiB framebuffer，无heap。

```text
主菜单
  执行动作
    回待机
    小蛋糕 / 中蛋糕 / 大蛋糕 -> 开始 / 返回
    Custom文件名 -> 开始 / 删除 / 返回
      删除 -> 确定删除？是 / 否（默认否）
  记录轨迹 -> 开始记录 / 返回
```

状态页覆盖初始化、连接电机、前往待机、加载、执行/进度、记录/时间/容量、保存、完成、失败、停止。工厂项常驻但文件不存在则提示轨迹不可用。Custom显示文件stem，不显示工程参数。命令处理结果通过 snapshot revision 回传；列表变化后不继续使用失效的详情名。

PA6/PA7 TIM3 TI12 encoder，uint16回绕处理，每4计数一个detent（需匹配实物编码器）。FWD=PE2、BACK=PE3，低有效，25 ms消抖，只发按下事件；不使用 encoder push，不定义长按/隐藏工程菜单。记录中 FWD 完成保存，BACK取消。PE4独立物理软停不经过菜单/命令队列；上电按住和持续按住同样阻止新命令。硬件急停另走动力链。

显示20 Hz，输入10 ms；I2C有超时，只阻塞UI任务。显示器掉线重试不阻塞控制任务。字形由 tools/generate_ui_glyphs.py 对菜单字符与ASCII生成，构建直接使用提交的位图；替换字形后需验证实际OLED可读性。尚未实测OLED、电平、编码器方向、按键抖动、软停机械效果。
