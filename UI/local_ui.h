#ifndef __LOCAL_UI_H
#define __LOCAL_UI_H
#include "operation_manager.h"
#include "ui_strings.h"
typedef enum
{
    UI_EVENT_NONE,
    ENCODER_NEXT,
    ENCODER_PREV,
    ENCODER_PRESS,
    FORWARD,
    BACK
} UiEvent;
typedef enum
{
    UI_PAGE_MAIN,
    UI_PAGE_ACTIONS,
    UI_PAGE_FACTORY,
    UI_PAGE_CUSTOM,
    UI_PAGE_DELETE_CONFIRM,
    UI_PAGE_RECORD,
    UI_PAGE_TF,
    UI_PAGE_FILES,
    UI_PAGE_TF_STATUS,
    UI_PAGE_DEVICE,
    UI_PAGE_SYSTEM,
    UI_PAGE_START_CONFIRM,
    UI_PAGE_STATUS
} UiPage;
typedef struct
{
    char rows[4][64];
    unsigned selected;
    bool has_selection;
    bool has_title, scroll_selected;
    unsigned scroll_offset;
} UiModel;
typedef struct
{
    unsigned online_motors;
    bool motors_configured, output_armed, force_online, force_calibrated, force_valid, can_ready;
    char version[16];
    uint32_t now_ms;
} UiDeviceStatus;
#define UI_NAV_DEPTH 5U
typedef struct
{
    UiPage page;
    unsigned selected;
    uint32_t scroll_elapsed_ms;
} UiNavigation;
typedef struct
{
    UiPage page;
    unsigned selected, action_index;
    char custom_name[STORAGE_NAME_BYTES];
    uint32_t catalog_revision;
    UiText error;
    bool show_error, pending_command;
    UiPage custom_parent;
    unsigned return_selection;
    uint32_t selected_since_ms;
    UiDeviceStatus device;
    UiNavigation navigation[UI_NAV_DEPTH];
    unsigned navigation_depth;
    AppCommandType start_command;
} LocalUi;
bool LocalUi_MotionActive(StartupState startup, RobotState robot, const OperationSnapshot *operation);
void LocalUi_SetDeviceStatus(LocalUi *ui, const UiDeviceStatus *status);
void LocalUi_Init(LocalUi *ui);
bool LocalUi_Handle(LocalUi *ui, UiEvent event, StartupState startup, RobotState robot,
                    const OperationSnapshot *operation, const StorageSnapshot *storage, AppCommand *command);
void LocalUi_Render(const LocalUi *ui, StartupState startup, RobotState robot,
                    const OperationSnapshot *operation, const StorageSnapshot *storage, UiModel *model);
void LocalUi_SetCommandResult(LocalUi *ui, ArmResult result);
#endif
