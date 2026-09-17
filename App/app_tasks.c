#include "app_main.h"
#include "app_snapshot.h"
#include "service_tasks.h"
#include "app_config.h"
#include "robot_control.h"
#include "robot_hw_config.h"
#include "bsp_can.h"
#include "can_transport.h"
#include "platform_lock.h"
#include "board_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx_hal.h"
static StaticTask_t control_tcb, can_tcb, supervisor_tcb;
static StackType_t control_stack[APP_CONTROL_STACK_WORDS], can_stack[APP_CAN_STACK_WORDS],
    supervisor_stack[APP_SUPERVISOR_STACK_WORDS];
static RobotControl robot;
static OperationManager operation;
static AppSnapshot app_snapshot;
static volatile bool soft_stop_latched;
static RobotState published_state;
static ArmResult pending_fault;
static uint32_t control_heartbeat;
static volatile AppFatalReason fatal_reason;
void Platform_EnterCritical(void)
{
    taskENTER_CRITICAL();
}
void Platform_ExitCritical(void)
{
    taskEXIT_CRITICAL();
}
AppFatalReason App_GetFatalReason(void)
{
    return fatal_reason;
}
void App_Fatal(AppFatalReason reason)
{
    __disable_irq();
    fatal_reason = reason;
    /* No RTOS access here: usable before scheduler, from faults, or corrupt stacks. */
    if ((RCC->APB1ENR & RCC_APB1ENR_CAN1EN) != 0)
    {
        CAN1->TSR = CAN_TSR_ABRQ0 | CAN_TSR_ABRQ1 | CAN_TSR_ABRQ2;
        CAN1->MCR |= CAN_MCR_INRQ;
    }
    for (;;)
    {
        __NOP();
    }
}
RobotState App_GetRobotState(void)
{
    taskENTER_CRITICAL();
    RobotState s = published_state;
    taskEXIT_CRITICAL();
    return s;
}
ArmResult RobotCommand_GetLastResult(uint32_t *sequence)
{
    if (sequence)
        *sequence = 0;
    return ARM_OUTPUT_DISABLED;
}
ArmResult RobotCommand_Submit(const RobotCommand *cmd)
{
    (void)cmd;
    return ARM_OUTPUT_DISABLED; /* v0.0.2: business authority only through CommandManager. */
}
void App_GetSnapshot(AppSnapshot *s)
{
    if (s)
    {
        taskENTER_CRITICAL();
        *s = app_snapshot;
        taskEXIT_CRITICAL();
    }
}
void App_RequestSoftStopFromIsr(void)
{
    soft_stop_latched = true;
}
static void App_RequestFault(ArmResult reason)
{
    taskENTER_CRITICAL();
    pending_fault = reason;
    MotorManager_DisableAll();
    taskEXIT_CRITICAL();
}
static void RobotControlTask(void *argument)
{
    (void)argument;
    TickType_t last = xTaskGetTickCount();
    uint32_t previous = HAL_GetTick();
    bool stop_was_held = false;
    for (;;)
    {
        uint32_t now = HAL_GetTick();
        taskENTER_CRITICAL();
        ArmResult fault = pending_fault;
        pending_fault = ARM_OK;
        bool soft_stop = soft_stop_latched;
        soft_stop_latched = false;
        taskEXIT_CRITICAL();
        bool stop_held = HAL_GPIO_ReadPin(BOARD_INPUT_PORT, BOARD_SOFT_STOP_PIN) == GPIO_PIN_RESET;
        soft_stop = soft_stop || (stop_held && !stop_was_held);
        stop_was_held = stop_held;
        if (fault != ARM_OK)
        {
            RobotControl_Fault(&robot, fault);
            CommandManager_Clear();
        }
        if ((uint32_t)(now - previous) > ROBOT_CONTROL_PERIOD_MS * 2U &&
            (robot.machine.state == ROBOT_RUNNING || robot.machine.state == ROBOT_READY ||
             MotorManager_IsArmed()))
            RobotControl_Fault(&robot, ARM_TIMEOUT);
        previous = now;
        if (soft_stop)
            OperationManager_SoftStop(&operation, &robot, now);
        AppCommand business;
        if (stop_held)
            CommandManager_Clear();
        if (!soft_stop && !stop_held && CommandManager_Take(&business))
        {
            ArmResult result = OperationManager_Command(&operation, &robot, &business, now);
            taskENTER_CRITICAL();
            app_snapshot.command_result = result;
            ++app_snapshot.command_revision;
            taskEXIT_CRITICAL();
        }
        OperationManager_Step(&operation, &robot, now);
        (void)RobotControl_Step(&robot, now);
        taskENTER_CRITICAL();
        published_state = robot.machine.state;
        app_snapshot.robot = robot.machine.state;
        app_snapshot.startup = operation.startup.state;
        app_snapshot.operation = operation.snapshot;
        control_heartbeat = now;
        taskEXIT_CRITICAL();
        vTaskDelayUntil(&last, pdMS_TO_TICKS(ROBOT_CONTROL_PERIOD_MS));
    }
}
static void CanRxTask(void *argument)
{
    (void)argument;
    for (;;)
    {
        CanFrame f;
        if (BspCan_Receive(&f, 1) == ARM_OK)
        {
            (void)MotorManager_ProcessFeedback(&f);
            /* Bounded burst, then give transport a chance to service mailboxes. */
            for (unsigned i = 0; i < 5 && BspCan_Receive(&f, 0) == ARM_OK; ++i)
                (void)MotorManager_ProcessFeedback(&f);
        }
        CanTransport_Pump(HAL_GetTick());
    }
}
static void SupervisorTask(void *argument)
{
    (void)argument;
    TickType_t last = xTaskGetTickCount();
    for (;;)
    {
        uint32_t now = HAL_GetTick();
        MotorSnapshot s;
        MotorManager_GetSnapshot(now, &s);
        BspCanDiagnostics d;
        BspCan_GetDiagnostics(&d);
        taskENTER_CRITICAL();
        uint32_t heartbeat = control_heartbeat;
        RobotState state = published_state;
        taskEXIT_CRITICAL();
        if (state == ROBOT_READY || state == ROBOT_RUNNING || CanTransport_IsArmed())
        {
            if (!MotorManager_IsHealthy(&s, false) || d.hal_errors || d.rx_overflow ||
                CanTransport_HasFault() || (uint32_t)(now - heartbeat) > APP_CONTROL_DEADLINE_MS)
                App_RequestFault(ARM_FAULT);
        }
        /* IWDG intentionally disabled; future feed only after all heartbeat checks. */
        vTaskDelayUntil(&last, pdMS_TO_TICKS(ROBOT_SUPERVISOR_PERIOD_MS));
    }
}
void App_Init(void)
{
    if (BspCan_Init() != ARM_OK)
        App_Fatal(APP_FATAL_HAL);
    const uint8_t ids[AXIS_COUNT] = {MOTOR_J1_ID, MOTOR_J2_ID, MOTOR_J3_ID,
                                     MOTOR_J4_ID, MOTOR_J5_ID, MOTOR_J6_ID};
    MotorAxisConfig configs[AXIS_COUNT] = {0};
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        configs[i].motor_id = ids[i];
        configs[i].map.joint_sign = 1;
        configs[i].profile = 0;
    }
    (void)MotorManager_Init(configs); /* UNASSIGNED is the expected safe boot configuration. */
    if (RobotControl_Init(&robot) != ARM_OK || CanTransport_Init() != ARM_OK)
        App_Fatal(APP_FATAL_RTOS_OBJECT);
    App_InitServices();
    OperationManager_Init(&operation, HAL_GetTick());
    published_state = ROBOT_DISABLED;
    control_heartbeat = HAL_GetTick();
}
void App_CreateTasks(void)
{
    App_CreateServiceTasks();
    if (xTaskCreateStatic(RobotControlTask, "RobotControl", APP_CONTROL_STACK_WORDS, NULL,
                          APP_CONTROL_PRIORITY, control_stack, &control_tcb) == NULL ||
        xTaskCreateStatic(CanRxTask, "CanRx", APP_CAN_STACK_WORDS, NULL, APP_CAN_PRIORITY, can_stack,
                          &can_tcb) == NULL ||
        xTaskCreateStatic(SupervisorTask, "Supervisor", APP_SUPERVISOR_STACK_WORDS, NULL,
                          APP_SUPERVISOR_PRIORITY, supervisor_stack, &supervisor_tcb) == NULL)
        App_Fatal(APP_FATAL_RTOS_OBJECT);
}
void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
    (void)task;
    (void)name;
    App_Fatal(APP_FATAL_STACK);
}
