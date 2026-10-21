#ifndef ARM_HMI_DGUS_H
#define ARM_HMI_DGUS_H
#include "dgus_protocol.h"
#include "app_snapshot.h"
#include "local_ui.h"
#define HMI_VP_EVENT 0x5000U
#define HMI_VP_STATUS 0x5100U
#define HMI_VP_TEXT 0x5200U
#define HMI_TEXT_WORDS 32U
#define HMI_STATUS_WORDS 16U
typedef enum
{
    HMI_KEY_NONE, HMI_KEY_NEXT, HMI_KEY_PREVIOUS, HMI_KEY_CONFIRM,
    HMI_KEY_BACK, HMI_KEY_SOFT_STOP, HMI_KEY_RECORD_FINISH
} HmiKey;
typedef bool (*HmiWrite)(void *context, const uint8_t *bytes, size_t count);
typedef struct
{
    uint32_t page, selected, startup, robot, operation, command_revision, catalog_revision;
    uint32_t error, start_command;
    bool show_error, pending;
} HmiView;
typedef struct
{
    DgusParser parser;
    HmiView view;
    uint16_t token;
    HmiKey pending_key;
    bool pending_stop, have_view, exposed, touch_released;
    uint32_t stale_events, bad_events, tx_failures;
} HmiDgus;
void HmiDgus_Init(HmiDgus *hmi, bool crc);
void HmiDgus_Invalidate(HmiDgus *hmi); /* Task context: invalidate input after UI changes. */
void HmiDgus_ResetLink(HmiDgus *hmi); /* Also discard any partial frame. */
void HmiDgus_Sync(HmiDgus *hmi, const LocalUi *ui, const AppSnapshot *app, const StorageSnapshot *storage);
void HmiDgus_Push(HmiDgus *hmi, uint8_t byte, uint32_t now_ms);
bool HmiDgus_TakeEvent(HmiDgus *hmi, UiEvent *event, bool *soft_stop);
bool HmiDgus_Publish(HmiDgus *hmi, const LocalUi *ui, const AppSnapshot *app,
                     const StorageSnapshot *storage, const UiModel *model, HmiWrite write, void *context);
#endif
