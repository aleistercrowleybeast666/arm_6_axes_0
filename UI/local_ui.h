#ifndef __LOCAL_UI_H
#define __LOCAL_UI_H
#include "operation_manager.h"
#include "ui_strings.h"
typedef enum
{
    UI_EVENT_NONE,
    ENCODER_NEXT,
    ENCODER_PREV,
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
    UI_PAGE_STATUS
} UiPage;
typedef struct
{
    char rows[4][64];
    unsigned selected;
    bool has_selection;
} UiModel;
typedef struct
{
    UiPage page;
    unsigned selected, action_index;
    char custom_name[STORAGE_NAME_BYTES];
    uint32_t catalog_revision;
    UiText error;
    bool show_error, pending_command;
} LocalUi;
void LocalUi_Init(LocalUi *ui);
bool LocalUi_Handle(LocalUi *ui, UiEvent event, StartupState startup, RobotState robot,
                    const OperationSnapshot *operation, const StorageSnapshot *storage, AppCommand *command);
void LocalUi_Render(const LocalUi *ui, StartupState startup, RobotState robot,
                    const OperationSnapshot *operation, const StorageSnapshot *storage, UiModel *model);
void LocalUi_SetCommandResult(LocalUi *ui, ArmResult result);
#endif
