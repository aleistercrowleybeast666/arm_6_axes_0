"""Fail a build review if static allocation, safety defaults, or source separation drift."""
from pathlib import Path
import re
import subprocess
import hashlib
ROOT=Path(__file__).resolve().parents[1]
def require(condition,message):
    if not condition:raise SystemExit(message)
config=(ROOT/'Core/Inc/FreeRTOSConfig.h').read_text(encoding='utf-8')
for name,value in [('configSUPPORT_STATIC_ALLOCATION','1'),('configSUPPORT_DYNAMIC_ALLOCATION','0'),('INCLUDE_vTaskDelayUntil','1'),('configMAX_PRIORITIES','9')]:
    require(re.search(r'#define\s+'+name+r'\s+'+value+r'\b',config),name+' is incorrect')
# These pins were read from U1 pad nets in the supplied 六轴机械臂.epro2.
# Check all three layers: CubeMX, generated pin labels, and the business BSP.
ioc=dict(line.split('=',1) for line in (ROOT/'arm_6_axes_0.ioc').read_text(encoding='utf-8').splitlines() if '=' in line)
board=(ROOT/'BSP/board_config.h').read_text(encoding='utf-8')
main_header=(ROOT/'Core/Inc/main.h').read_text(encoding='utf-8')
for pin,signal in {'PA2':'USART2_TX','PA3':'USART2_RX','PB10':'I2C2_SCL','PB11':'I2C2_SDA',
                   'PB8':'CAN1_RX','PB9':'CAN1_TX','PA6':'S_TIM3_CH1','PA7':'S_TIM3_CH2',
                   'PC8':'SDIO_D0','PC9':'SDIO_D1','PC10':'SDIO_D2','PC11':'SDIO_D3',
                   'PC12':'SDIO_CK','PD2':'SDIO_CMD'}.items():
    require(ioc.get(pin+'.Signal')==signal,'Wiring baseline mismatch: '+pin)
for pin,label,signal in [('PD8','HX711_DOUT','GPXTI8'),('PD9','HX711_SCK','GPIO_Output'),
                         ('PE9','UI_START','GPIO_Input'),('PE10','UI_BACK','GPIO_Input'),
                         ('PE11','SOFT_STOP','GPXTI11'),('PC7','TF_DETECT','GPIO_Input'),
                         ('PC4','ENCODER_KEY','GPIO_Input'),('PE2','DCDC_PG','GPIO_Input')]:
    require(ioc.get(pin+'.Signal')==signal and ioc.get(pin+'.GPIO_Label')==label,'GPIO baseline mismatch: '+pin)
    for name,value in [(label+'_Pin','GPIO_PIN_'+pin[2:]),(label+'_GPIO_Port','GPIO'+pin[1])]:
        require(re.search(r'#define\s+'+name+r'\s+'+value+r'\b',main_header),'Generated pin mismatch: '+name)
for name,value in {'BOARD_UART_INSTANCE':'USART2','BOARD_I2C_INSTANCE':'I2C2','BOARD_HAL_TICK_INSTANCE':'TIM1',
                   'BOARD_HX_PORT':'GPIOD','BOARD_HX_DOUT_PIN':'GPIO_PIN_8','BOARD_HX_SCK_PIN':'GPIO_PIN_9',
                   'BOARD_INPUT_PORT':'GPIOE','BOARD_FORWARD_PIN':'GPIO_PIN_9','BOARD_BACK_PIN':'GPIO_PIN_10',
                   'BOARD_SOFT_STOP_PIN':'GPIO_PIN_11','BOARD_TF_DETECT_PORT':'GPIOC','BOARD_TF_DETECT_PIN':'GPIO_PIN_7'}.items():
    require(re.search(r'#define\s+'+name+r'\s+'+value+r'\b',board),'BSP baseline mismatch: '+name)
irq=(ROOT/'Core/Src/stm32f4xx_it.c').read_text(encoding='utf-8')
for module,typ,var,getter,handler in [('i2c','I2C','hi2c2','I2c_GetHandle',None),
                                   ('usart','UART','huart2','Uart_GetHandle','USART2_IRQHandler')]:
    source=(ROOT/f'Core/Src/{module}.c').read_text(encoding='utf-8')
    require(f'static {typ}_HandleTypeDef {var};' in source and f'return &{var};' in source,
            'Run tools/integrate_cubemx.py: stale/unprotected '+module+' handle')
    if handler:
        require(re.search(r'void '+handler+r'\(void\).*?HAL_UART_IRQHandler\('+getter+r'\(\)\)',irq,re.S),
                'UART IRQ must use the accessor')
main=(ROOT/'Core/Src/main.c').read_text(encoding='utf-8')
require('htim != NULL && htim->Instance == BOARD_HAL_TICK_INSTANCE' in main,'HAL tick callback/board mismatch')
require(ioc.get('NVIC.TimeBaseIP')=='TIM1','IOC HAL tick must match TIM1')
require('MX_FATFS_Init();' not in main,'FatFs must be initialized by StorageTask')
rtos=(ROOT/'Core/Src/freertos.c').read_text(encoding='utf-8')
require('App_CreateTasks();' in rtos and 'osThreadCreate(' not in rtos,'Run tools/integrate_cubemx.py: CubeMX placeholder task remains')
hw=(ROOT/'Motor/robot_hw_config.h').read_text(encoding='utf-8')
msp=(ROOT/'Core/Src/stm32f4xx_hal_msp.c').read_text(encoding='utf-8')
require(re.search(r'HAL_NVIC_SetPriority\(SVCall_IRQn,\s*0,\s*0\)',msp),'SVC must be above BASEPRI to start the first task')
require(re.search(r'#define\s+ARM_ENABLE_MOTOR_OUTPUT\s+0\b',hw),'Default motor output must remain disabled')
for folder in ['RobotMath','RobotMotion','Teach','Task','Motor','Safety','App','BSP','Drivers/CyberGear','Track','Storage','Sensors','UI','Debug']:
    for path in (ROOT/folder).rglob('*.[ch]'):
        content=path.read_text(encoding='utf-8')
        require(not re.search(r'\b(malloc|calloc|realloc|free|pvPortMalloc|vPortFree)\s*\(',content),f'Dynamic allocation: {path}')
        if folder=='RobotMath':require(not re.search(r'#include.*(stm32|FreeRTOS|task\.h)',content),f'Nonportable math: {path}')
        if folder in ('Track','Task','UI','App'):
            require(not re.search(r'\b(f_open|f_read|f_write|f_sync|f_close|f_rename)\s*\(',content),f'FatFs outside StorageTask: {path}')
        if folder in ('Track','Task','UI','App','Sensors'):
            require(not re.search(r'\bCyberGear_\w+\s*\(',content),f'Backend leakage: {path}')
for axis in range(1,7):
    require(re.search(r'#define\s+MOTOR_J'+str(axis)+r'_ID\s+MOTOR_UNASSIGNED_ID',hw),'Assigned default motor ID')
geometry=(ROOT/'RobotMath/robot_geometry.c').read_text(encoding='utf-8')
require(hashlib.sha256(geometry.encode()).hexdigest()=='9743ae2a37aed52e913af4ab4bafd64f35ae55d4410c7b932b475caef5164af1','Frozen geometry/poses changed')
require((ROOT/'docs/mechanical_source/SOURCE_SHA').read_text().strip()=='1b1cc4982e09ccf0d09f9921e68e24940a76d877','Mechanical baseline SHA changed')
ff=(ROOT/'FATFS/Target/ffconf.h').read_text(encoding='utf-8')
for name,value in [('_USE_LFN','1'),('_MAX_LFN','31'),('_FS_REENTRANT','0')]:
    require(re.search(r'#define\s+'+name+r'\s+'+value+r'\b',ff),'FatFs static owner/LFN config: '+name)
linker=(ROOT/'STM32F407XX_FLASH.ld').read_text(encoding='utf-8')
require('.ccmram (NOLOAD)' in linker and '_Min_Heap_Size = 0x0;' in linker,'CCM NOLOAD / heap=0 required')
for source in (ROOT/'Core/Src').glob('*.c'):
    text=source.read_text(encoding='utf-8')
    for irq,priority in re.findall(r'HAL_NVIC_SetPriority\((\w+),\s*(\d+),\s*\d+\)',text):
        if irq in ('CAN1_RX0_IRQn','CAN1_SCE_IRQn','SDIO_IRQn','DMA2_Stream3_IRQn','DMA2_Stream6_IRQn','EXTI9_5_IRQn','EXTI15_10_IRQn','USART2_IRQn'):
            require(int(priority)>=5,'Unsafe RTOS IRQ priority: '+irq)
elf=ROOT/'build/firmware/arm_6_axes_0.elf'
if elf.exists():
    symbols=subprocess.check_output(['arm-none-eabi-nm',str(elf)],text=True)
    require(not re.search(r'\b(malloc|calloc|realloc|free|pvPortMalloc|vPortFree|_sbrk|_sbrk_r|_malloc_r|_calloc_r|_realloc_r|_free_r|ucHeap)$',symbols,re.M),'Dynamic allocation linked into ELF')
    for name in ['RobotIK_Solve','RobotFK_Solve','RobotJacobian_Compute','Version_GetInfo']:
        require(re.search(r'\b'+name+r'$',symbols,re.M),name+' missing from ELF')
    addresses={name:int(address,16) for address,name in re.findall(r'^([0-9a-fA-F]+)\s+\w\s+(_[se]ccmram)$',symbols,re.M)}
    require(addresses.get('_sccmram')==0x10000000 and addresses.get('_eccmram')==0x1000C000,'Track CCM must occupy exactly 49152 B')
print('Wiring baseline / CubeMX integration / static memory / safe defaults / frozen geometry / HAL-free math / backend boundaries / IRQ / CCM / linked symbols PASS')
