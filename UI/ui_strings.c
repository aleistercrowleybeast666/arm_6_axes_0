#include "ui_strings.h"
static const char *const strings[UI_TEXT_COUNT] = {
    "执行动作",     "记录轨迹", "回待机",       "小蛋糕",     "中蛋糕",     "大蛋糕",      "开始",
    "返回",         "删除",     "确定删除？",   "是",         "否",         "开始记录",    "正在记录",
    "正在保存",     "正在加载", "正在执行",     "操作完成",   "动作已停止", "操作失败",    "正在初始化",
    "正在连接电机", "前往待机", "存储卡未就绪", "轨迹不可用", "轨迹过长",   "功能尚未就绪"};
const char *UiStrings_Get(UiText t)
{
    return (unsigned)t < UI_TEXT_COUNT ? strings[t] : "";
}
