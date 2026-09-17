"""Idempotent application integration after CubeMX generation (no HAL/vendor edits)."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]

def edit(name, transform):
    path = ROOT / name
    path.write_text(transform(path.read_text(encoding='utf-8')), encoding='utf-8', newline='\n')

def block(text, tag, content):
    pattern = rf'(/\* USER CODE BEGIN {re.escape(tag)} \*/).*?(/\* USER CODE END {re.escape(tag)} \*/)'
    result, n = re.subn(pattern, lambda m: m[1]+'\n'+content+'\n'+m[2], text, flags=re.S)
    if n != 1:
        raise RuntimeError(f'Missing/duplicate CubeMX user block: {tag}')
    return result

def main_source(s):
    s = block(s, 'Includes', '#include "app_main.h"\n#include "board_config.h"')
    s = block(s, '2', '  App_Init();')
    s = block(s, 'WHILE', '  App_Fatal(APP_FATAL_RTOS_OBJECT);\n  while (1)\n  {')
    s = block(s, 'Error_Handler_Debug', '  App_Fatal(APP_FATAL_HAL);')
    s = s.replace('if (htim->Instance == TIM6)', 'if (htim != NULL && htim->Instance == BOARD_HAL_TICK_INSTANCE)')
    return s

def can_source(s):
    s = re.sub(r'(?m)^(?:static )?CAN_HandleTypeDef hcan1;', 'static CAN_HandleTypeDef hcan1;', s)
    return block(s, '1', 'CAN_HandleTypeDef *Can_GetHandle(void) { return &hcan1; }')

def can_header(s):
    s = re.sub(r'(?m)^extern CAN_HandleTypeDef hcan1;\n?', '', s)
    return block(s, 'Prototypes', 'CAN_HandleTypeDef *Can_GetHandle(void);')

def irq_source(s):
    s = re.sub(r'(?m)^extern CAN_HandleTypeDef hcan1;\n?', '', s)
    s = s.replace('HAL_CAN_IRQHandler(&hcan1)', 'HAL_CAN_IRQHandler(Can_GetHandle())')
    s = block(s, 'Includes', '#include "can.h"\n#include "app_main.h"')
    for handler in ['HardFault_IRQn','MemoryManagement_IRQn','BusFault_IRQn','UsageFault_IRQn','NonMaskableInt_IRQn']:
        s = block(s, handler+' 0', '  App_Fatal(APP_FATAL_CPU);')
    return s

def rtos_config(s):
    for macro, value in {'configSUPPORT_STATIC_ALLOCATION':'1','configSUPPORT_DYNAMIC_ALLOCATION':'0',
        'INCLUDE_vTaskDelayUntil':'1','configUSE_MUTEXES':'0','configTOTAL_HEAP_SIZE':'0','configMAX_PRIORITIES':'9'}.items():
        s = re.sub(r'(?m)^#define '+macro+r'\s+.*$', '#define '+macro+' '+value, s)
    s = block(s, 'Includes', '#include "app_main.h"')
    s = block(s, '1', '#define configASSERT(x) do { if (!(x)) App_Fatal(APP_FATAL_ASSERT); } while (0)')
    s = block(s, 'Defines', '#define configCHECK_FOR_STACK_OVERFLOW 2\n#define configUSE_TIMERS 0')
    return s

def msp_source(s):
    # SVC must be above BASEPRI=5 before FreeRTOS starts its first task.
    return block(s, 'MspInit 1', '  HAL_NVIC_SetPriority(SVCall_IRQn, 0, 0);')

edit('Core/Src/main.c', main_source)
edit('Core/Src/can.c', can_source)
edit('Core/Inc/can.h', can_header)
edit('Core/Src/stm32f4xx_it.c', irq_source)
edit('Core/Inc/FreeRTOSConfig.h', rtos_config)
edit('Core/Src/stm32f4xx_hal_msp.c', msp_source)
# The CubeMX placeholder task is deliberately replaced by the native static API.
rtos = ROOT / 'Core/Src/freertos.c'
header = rtos.read_text(encoding='utf-8').split('/* USER CODE END Header */')[0]+'/* USER CODE END Header */\n'
rtos.write_text(header+'''#include "FreeRTOS.h"
#include "task.h"
#include "app_main.h"
static StaticTask_t idle_tcb;
static StackType_t idle_stack[configMINIMAL_STACK_SIZE];
void MX_FREERTOS_Init(void) { App_CreateTasks(); }
void vApplicationGetIdleTaskMemory(StaticTask_t **tcb, StackType_t **stack, uint32_t *size)
{ *tcb=&idle_tcb; *stack=idle_stack; *size=configMINIMAL_STACK_SIZE; }
''', encoding='utf-8', newline='\n')
# Keep CubeMX's original Makefile accessible only as generated evidence in cache;
# repository Makefile delegates to the supported CMake source manifest.
(ROOT/'Makefile').write_text('''.PHONY: all clean rebuild host
all:
\tcmake --preset firmware
\tcmake --build --preset firmware
clean:
\tcmake --build --preset firmware --target clean
rebuild: clean all
host:
\tcmake --preset host
\tcmake --build --preset host
\tctest --preset host
''', encoding='utf-8', newline='\n')
print('CubeMX static application integration complete')

# Own generated handles behind accessors; keep HAL sources untouched.
peripherals=[('sdio',[('SD_HandleTypeDef','hsd','Sd_GetHandle'),('DMA_HandleTypeDef','hdma_sdio_rx','Sd_GetRxDma'),('DMA_HandleTypeDef','hdma_sdio_tx','Sd_GetTxDma')]),
 ('i2c',[('I2C_HandleTypeDef','hi2c1','I2c_GetHandle')]),('tim',[('TIM_HandleTypeDef','htim3','Encoder_GetHandle')]),('usart',[('UART_HandleTypeDef','huart1','Uart_GetHandle')])]
for module, handles in peripherals:
    c=ROOT/f'Core/Src/{module}.c'; h=ROOT/f'Core/Inc/{module}.h'
    if not c.exists():continue
    source=c.read_text(encoding='utf-8');header=h.read_text(encoding='utf-8');accessors=[];prototypes=[]
    for typ,var,getter in handles:
        source=re.sub(r'(?m)^(?:static )?'+typ+' '+var+';', 'static '+typ+' '+var+';',source)
        header=re.sub(r'(?m)^extern '+typ+' '+var+';\n?', '', header)
        accessors.append(f'{typ} *{getter}(void) {{ return &{var}; }}');prototypes.append(f'{typ} *{getter}(void);')
    c.write_text(block(source,'1','\n'.join(accessors)),encoding='utf-8',newline='\n')
    h.write_text(block(header,'Prototypes','\n'.join(prototypes)),encoding='utf-8',newline='\n')
irq=ROOT/'Core/Src/stm32f4xx_it.c';s=irq.read_text(encoding='utf-8')
for module,handles in peripherals:
    for typ,var,getter in handles:
        s=re.sub(r'(?m)^extern '+typ+' '+var+';\n?', '',s);s=s.replace('&'+var,getter+'()')
s=block(s,'Includes','#include "can.h"\n#include "sdio.h"\n#include "usart.h"\n#include "app_main.h"')
irq.write_text(s,encoding='utf-8',newline='\n')
main=ROOT/'Core/Src/main.c';s=main.read_text(encoding='utf-8').replace('  MX_FATFS_Init();','  /* StorageTask owns FatFs initialization. */')
main.write_text(s,encoding='utf-8',newline='\n')
linker=ROOT/'STM32F407XX_FLASH.ld';s=linker.read_text(encoding='utf-8').replace('.ccmram :','.ccmram (NOLOAD) :').replace('} >CCMRAM AT> FLASH','} >CCMRAM')
linker.write_text(s,encoding='utf-8',newline='\n')
# Normalize trailing generator padding in the peripheral wrappers owned above.
for module in ['dma','i2c','sdio','tim','usart']:
    for folder,suffix in [('Inc','h'),('Src','c')]:
        path=ROOT/f'Core/{folder}/{module}.{suffix}'
        path.write_text(path.read_text(encoding='utf-8').rstrip()+'\n',encoding='utf-8',newline='\n')
