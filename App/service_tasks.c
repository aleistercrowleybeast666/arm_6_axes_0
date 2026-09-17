#include "service_tasks.h"
#include "app_snapshot.h"
#include "app_main.h"
#include "force_backend.h"
#include "display_if.h"
#include "input_local.h"
#include "debug_uart.h"
#include "board_config.h"
#include "platform_lock.h"
#include "tim.h"
#include "FreeRTOS.h"
#include "task.h"
static StaticTask_t sensor_tcb, ui_tcb, storage_tcb, debug_tcb;
static StackType_t sensor_stack[512], ui_stack[1536], storage_stack[2048], debug_stack[1024];
static TaskHandle_t sensor_task, storage_task;
void Platform_NotifyStorage(void)
{
    if (storage_task)
        xTaskNotifyGive(storage_task);
}
static void SensorTask(void *argument)
{
    (void)argument;
    for (;;)
    {
        (void)ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(20));
        int32_t raw;
        if (ForceBackend_Get()->read_raw(&raw) == ARM_OK)
            ForceSensor_PushRaw(raw, HAL_GetTick());
    }
}
void App_NotifySensorFromIsr(void)
{
    if (sensor_task)
    {
        BaseType_t wake = pdFALSE;
        vTaskNotifyGiveFromISR(sensor_task, &wake);
        portYIELD_FROM_ISR(wake);
    }
}
static void UiTask(void *argument)
{
    (void)argument;
    LocalUi ui;
    LocalUi_Init(&ui);
    LocalInput input = {0};
    uint32_t last_render = 0, revision = 0;
    bool display_ready = false;
    for (;;)
    {
        uint32_t now = HAL_GetTick();
        AppSnapshot a;
        App_GetSnapshot(&a);
        StorageSnapshot s;
        StorageService_GetSnapshot(&s);
        if (a.command_revision != revision)
        {
            revision = a.command_revision;
            LocalUi_SetCommandResult(&ui, a.command_result);
        }
        UiEvent event =
            InputLocal_Update(&input, (uint16_t)__HAL_TIM_GET_COUNTER(Encoder_GetHandle()),
                              HAL_GPIO_ReadPin(BOARD_INPUT_PORT, BOARD_FORWARD_PIN) == GPIO_PIN_RESET,
                              HAL_GPIO_ReadPin(BOARD_INPUT_PORT, BOARD_BACK_PIN) == GPIO_PIN_RESET, now);
        AppCommand cmd;
        if (LocalUi_Handle(&ui, event, a.startup, a.robot, &a.operation, &s, &cmd))
            LocalUi_SetCommandResult(&ui, CommandManager_Submit(&cmd));
        if ((uint32_t)(now - last_render) >= 50)
        {
            UiModel model;
            LocalUi_Render(&ui, a.startup, a.robot, &a.operation, &s, &model);
            if (!display_ready)
                display_ready = Display_GetBackend()->init() == ARM_OK;
            if (display_ready)
                display_ready = Display_GetBackend()->render(&model) == ARM_OK;
            last_render = now;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
static void StorageTask(void *argument)
{
    (void)argument;
    StorageRequest request = {.type = STORAGE_MOUNT};
    uint32_t ticket = 0;
    (void)StorageService_Request(&request, &ticket);
    bool load_cal = true;
    for (;;)
    {
        if (!StorageService_ProcessOne())
            (void)ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100));
        if (ticket)
        {
            StorageResponse response;
            if (StorageService_TakeResult(ticket, &response) == TRACK_OK)
            {
                ticket = 0;
                if (load_cal)
                {
                    load_cal = false;
                    if (response.result == TRACK_OK || response.result == TRACK_LIST_FULL)
                    {
                        request.type = STORAGE_LOAD_FORCE_CAL;
                        (void)StorageService_Request(&request, &ticket);
                    }
                }
                else if (response.result == TRACK_OK)
                    (void)ForceSensor_SetCalibration(&response.calibration);
            }
        }
    }
}
static void DebugUartTask(void *argument)
{
    (void)argument;
    (void)DebugUart_Init();
    for (;;)
    {
        DebugUart_Poll(HAL_GetTick());
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}
void App_InitServices(void)
{
    ForceSensor_Init();
    StorageService_Init();
    CommandManager_Init();
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    if (HAL_TIM_Encoder_Start(Encoder_GetHandle(), TIM_CHANNEL_ALL) != HAL_OK)
        App_Fatal(APP_FATAL_HAL);
}
void App_CreateServiceTasks(void)
{
    sensor_task = xTaskCreateStatic(SensorTask, "Sensor", 512, NULL, 4, sensor_stack, &sensor_tcb);
    if (!sensor_task || !xTaskCreateStatic(UiTask, "UI", 1536, NULL, 3, ui_stack, &ui_tcb) ||
        !(storage_task =
              xTaskCreateStatic(StorageTask, "Storage", 2048, NULL, 2, storage_stack, &storage_tcb)) ||
        !xTaskCreateStatic(DebugUartTask, "DebugUart", 1024, NULL, 1, debug_stack, &debug_tcb))
        App_Fatal(APP_FATAL_RTOS_OBJECT);
}
void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
    if (pin == BOARD_SOFT_STOP_PIN)
        App_RequestSoftStopFromIsr();
    else if (pin == BOARD_HX_DOUT_PIN)
        App_NotifySensorFromIsr();
}
