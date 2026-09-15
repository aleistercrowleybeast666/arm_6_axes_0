cmake_minimum_required(VERSION 3.20)
project(arm_6_axes_0 VERSION 0.0.1 LANGUAGES C ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(ARM_BUILD_FIRMWARE "Build STM32F407 target" OFF)
set(APP_DIRS RobotMath RobotMotion Teach Task Motor Safety App BSP Drivers/CyberGear)
set(PORTABLE_SOURCES
    RobotMath/robot_math_types.c RobotMath/robot_geometry.c RobotMath/robot_fk.c
    RobotMath/robot_jacobian.c RobotMath/robot_ik.c RobotMath/robot_limits.c
    RobotMotion/robot_trajectory.c Teach/teach_recorder.c Teach/teach_player.c Task/recipe.c
    Motor/robot_joint_map.c Motor/motor_manager.c Safety/robot_state_machine.c App/robot_control.c
    Drivers/CyberGear/cybergear_protocol.c Drivers/CyberGear/cybergear_motor.c)
add_library(robot_core STATIC ${PORTABLE_SOURCES})
target_include_directories(robot_core PUBLIC ${APP_DIRS})
target_compile_options(robot_core PRIVATE -Wall -Wextra -Werror -Wdouble-promotion -ffunction-sections -fdata-sections)
if(ARM_BUILD_FIRMWARE)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_custom_target(version_metadata
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/version_header.py
        BYPRODUCTS ${CMAKE_SOURCE_DIR}/build/generated/version_build.h)
    set(HAL_DIR Drivers/STM32F4xx_HAL_Driver)
    set(RTOS_DIR Middlewares/Third_Party/FreeRTOS/Source)
    file(GLOB HAL_SOURCES CONFIGURE_DEPENDS ${HAL_DIR}/Src/*.c)
    add_library(st_platform STATIC ${HAL_SOURCES}
        ${RTOS_DIR}/tasks.c ${RTOS_DIR}/queue.c ${RTOS_DIR}/list.c
        ${RTOS_DIR}/portable/GCC/ARM_CM4F/port.c
        ${RTOS_DIR}/CMSIS_RTOS/cmsis_os.c)
    set(PLATFORM_INCLUDES Core/Inc ${HAL_DIR}/Inc ${HAL_DIR}/Inc/Legacy
        Drivers/CMSIS/Include Drivers/CMSIS/Device/ST/STM32F4xx/Include
        ${RTOS_DIR}/include ${RTOS_DIR}/portable/GCC/ARM_CM4F ${RTOS_DIR}/CMSIS_RTOS)
    target_include_directories(st_platform PUBLIC ${PLATFORM_INCLUDES} ${APP_DIRS})
    target_compile_definitions(st_platform PUBLIC USE_HAL_DRIVER STM32F407xx)
    target_compile_options(st_platform PRIVATE -ffunction-sections -fdata-sections)
    add_executable(arm_6_axes_0
        Core/Src/main.c Core/Src/can.c Core/Src/gpio.c Core/Src/freertos.c
        Core/Src/stm32f4xx_it.c Core/Src/stm32f4xx_hal_msp.c Core/Src/stm32f4xx_hal_timebase_tim.c
        Core/Src/system_stm32f4xx.c startup_stm32f407xx.s
        App/app_tasks.c App/version.c BSP/bsp_can.c BSP/can_transport.c BSP/syscall_stubs.c)
    add_dependencies(arm_6_axes_0 version_metadata)
    target_include_directories(arm_6_axes_0 PRIVATE build/generated)
    target_compile_options(arm_6_axes_0 PRIVATE -Wall -Wextra -Werror -ffunction-sections -fdata-sections)
    target_link_libraries(arm_6_axes_0 PRIVATE robot_core st_platform m)
    target_link_options(arm_6_axes_0 PRIVATE -T${CMAKE_SOURCE_DIR}/STM32F407XX_FLASH.ld
        --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections
        -Wl,-Map=${CMAKE_BINARY_DIR}/arm_6_axes_0.map -Wl,--print-memory-usage
        -Wl,--undefined=RobotIK_Solve -Wl,--undefined=RobotFK_Solve -Wl,--undefined=RobotJacobian_Compute -Wl,--undefined=Version_GetInfo)
    set_target_properties(arm_6_axes_0 PROPERTIES SUFFIX .elf)
    find_program(ARM_OBJCOPY arm-none-eabi-objcopy REQUIRED)
    find_program(ARM_SIZE arm-none-eabi-size REQUIRED)
    add_custom_command(TARGET arm_6_axes_0 POST_BUILD
        COMMAND ${ARM_OBJCOPY} -O binary $<TARGET_FILE:arm_6_axes_0> ${CMAKE_BINARY_DIR}/arm_6_axes_0.bin
        COMMAND ${ARM_OBJCOPY} -O ihex $<TARGET_FILE:arm_6_axes_0> ${CMAKE_BINARY_DIR}/arm_6_axes_0.hex
        COMMAND ${ARM_SIZE} $<TARGET_FILE:arm_6_axes_0>)
else()
    enable_testing()
    add_executable(host_tests tests/host/test_main.c tests/host/platform_mock.c)
    target_link_libraries(host_tests PRIVATE robot_core m)
    target_include_directories(host_tests PRIVATE tests/vectors tests/host)
    target_compile_options(host_tests PRIVATE -Wall -Wextra -Werror -UNDEBUG)
    add_test(NAME robot_foundation COMMAND host_tests)
    foreach(gate RANGE 0 1)
        add_executable(transport_test_${gate} tests/host/test_transport.c BSP/can_transport.c)
        target_include_directories(transport_test_${gate} PRIVATE tests/host/fakes ${APP_DIRS})
        target_compile_definitions(transport_test_${gate} PRIVATE ARM_ENABLE_MOTOR_OUTPUT=${gate})
        target_compile_options(transport_test_${gate} PRIVATE -Wall -Wextra -Werror -UNDEBUG)
        add_test(NAME transport_gate_${gate} COMMAND transport_test_${gate})
    endforeach()
endif()
