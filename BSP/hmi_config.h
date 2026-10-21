#ifndef ARM_HMI_CONFIG_H
#define ARM_HMI_CONFIG_H
/* Software candidate only. Enable after confirming the screen interface and board wiring. */
#ifndef ARM_ENABLE_DGUS_HMI
#define ARM_ENABLE_DGUS_HMI 0
#endif
#ifndef ARM_DGUS_CRC
#define ARM_DGUS_CRC 1
#endif
#define HMI_UART_BAUD 115200U
#define HMI_PUBLISH_PERIOD_MS 100U
#endif
