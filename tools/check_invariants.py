"""Fail a build review if static allocation, safety defaults, or source separation drift."""
from pathlib import Path
import re
import subprocess
ROOT=Path(__file__).resolve().parents[1]
def require(condition,message):
    if not condition:raise SystemExit(message)
config=(ROOT/'Core/Inc/FreeRTOSConfig.h').read_text()
for name,value in [('configSUPPORT_STATIC_ALLOCATION','1'),('configSUPPORT_DYNAMIC_ALLOCATION','0'),('INCLUDE_vTaskDelayUntil','1')]:
    require(re.search(r'#define\s+'+name+r'\s+'+value+r'\b',config),name+' is incorrect')
hw=(ROOT/'Motor/robot_hw_config.h').read_text()
msp=(ROOT/'Core/Src/stm32f4xx_hal_msp.c').read_text()
require(re.search(r'HAL_NVIC_SetPriority\(SVCall_IRQn,\s*0,\s*0\)',msp),'SVC must be above BASEPRI to start the first task')
require(re.search(r'#define\s+ARM_ENABLE_MOTOR_OUTPUT\s+0\b',hw),'Default motor output must remain disabled')
for folder in ['RobotMath','RobotMotion','Teach','Task','Motor','Safety','App','BSP','Drivers/CyberGear']:
    for path in (ROOT/folder).glob('*.[ch]'):
        content=path.read_text()
        require(not re.search(r'\b(malloc|calloc|realloc|free|pvPortMalloc|vPortFree)\s*\(',content),f'Dynamic allocation: {path}')
        if folder=='RobotMath':require(not re.search(r'#include.*(stm32|FreeRTOS|task\.h)',content),f'Nonportable math: {path}')
elf=ROOT/'build/firmware/arm_6_axes_0.elf'
if elf.exists():
    symbols=subprocess.check_output(['arm-none-eabi-nm',str(elf)],text=True)
    require(not re.search(r'\b(malloc|calloc|realloc|free|pvPortMalloc|vPortFree|_sbrk|ucHeap)$',symbols,re.M),'Dynamic allocation linked into ELF')
    for name in ['RobotIK_Solve','RobotFK_Solve','RobotJacobian_Compute','Version_GetInfo']:
        require(re.search(r'\b'+name+r'$',symbols,re.M),name+' missing from ELF')
print('Static memory / safe defaults / HAL-free math / linked symbols PASS')
