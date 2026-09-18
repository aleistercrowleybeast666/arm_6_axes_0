# Rev.A CAN

参考来源为用户描述的“野火小智CAN通信模块 V1.1”。本轮未取得该参考图的原始文件，元件脚位/电气参数以所选厂家手册重新核对，没有把该模块的验证结果等同于本板验证。

## 固定连接

SIT1042AQT/3 / C5382542，SOP8：1TXD=PB9、2GND、3VCC=5V、4RXD=PB8、5VIO=3V3、6CANL、7CANH、8STB。STB经0Ω接地，厂家确认低电平为Normal/High-Speed；不分配MCU脚。

VCC与VIO各配置100nF+1µF0805并联去耦，100nF最靠近相应供电脚，1µF紧邻。VCC工作4.5–5.5V，VIO工作2.8–5.5V；总线绝对最大值±58V。依据 [SIT厂家手册，立创存档](https://www.lcsc.com/datasheet/C5382542.pdf)，Rev1.7（2024-09）。

**Main controller contains fixed 120 Ω termination.** R_TERM为120Ω0805、1%、0.125W，直接CANH↔CANL；无开关、跳帽、焊桥或TERM_EN。本板必须位于物理末端，另一末端配置匹配终端，中间节点不再重复加终端。3V差分时此电阻耗散75mW；持续故障电压、环境降额需单独审查。

J_CAN为GH1.25-3P：1GND、2CAN_H、3CAN_L。总线双绞、短支线，地线作信号参考，不作电机大电流回流。接插件pin1视图仍待库与厂家图纸复核。

## A：本版采用的保护

CANH、CANL分别经一只 **Diodes SMAJ13CA-13-F / C134951** 双向TVS到GND。VRWM13V、击穿14.4–15.9V、额定脉冲钳位21.5V@18.6A，400W（10/1000µs）。数值来源：[Diodes器件手册](https://www.lcsc.com/datasheet/C134951.pdf)。

优势是保留指定方案、SMA易焊，额定测试点的钳位低于SIT总线±58V绝对最大值。限制：通用功率TVS的结电容会加载总线，且不同脉冲、布局寄生和温度下钳位不同。13V反向工作电压也意味着它不能无限承受总线误接24V的持续故障；必须测试/限制故障能量。

目前没有本板1Mbps眼图/波形，不以“参考板可用”判定通过。保持本版SMAJ13CA，不擅自换料。

## B：备选 CAN 专用 TVS（未装入BOM）

| 方案 | 优点 | 代价/约束 |
| --- | --- | --- |
| Nexperia PESD2CAN，SOT23双线阵列 | 24V VRWM；0V/1MHz时电容典型25pF、最大30pF；8/20µs、5A下钳位最大41V | 脉冲条件与400W SMAJ不同，不能直接用瓦数排名；共地小封装需重审焊盘和布局；官网当前标注Not for Design In，不推荐作为新设计冻结料 |
| Nexperia在产CAN-FD专用系列 | 可从低电容、低动态钳位产品中寻找适配器件 | 尚未冻结完整MPN、LCSC、封装或验证±58V收发器的钳位裕量；只能作为Rev.B选型方向 |

来源：[PESD2CAN官方数据手册](https://assets.nexperia.com/documents/data-sheet/PESD2CAN.pdf)、[生命周期状态](https://www.nexperia.com/product/PESD2CAN)、[官方CAN保护产品族](https://www.nexperia.com/products/esd-protection-tvs-filtering-and-signal-conditioning/family/lin-can-fd-flexray)。

## C：必须完成的验证

1. 实际线长、支线、全部节点和两端120Ω条件下，1Mbps观察CANH/CANL差分、共模、边沿、过冲和误码。
2. 装/不装TVS的对比只作为台架定位方法；最终按装TVS配置验收，不能为了通过而未经审图移除保护。
3. 检查TVS最短回流路径、收发器旁路、连接器到TVS走线；浪涌/ESD目标由整机环境确定后验证。
4. SIT和GH连接器的库存快照为0，SMAJ13CA库存10；保留指定MPN，采购前确认供货渠道。替代品需要新的电气和库审核。
