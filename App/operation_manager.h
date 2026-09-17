#ifndef __OPERATION_MANAGER_H
#define __OPERATION_MANAGER_H
#include "startup_manager.h"
#include "command_manager.h"
#include "track_player.h"
#include "track_recorder.h"
#include "factory_recipes.h"
#include "storage_service.h"
typedef enum
{
    OP_IDLE,
    OP_LOADING,
    OP_EXECUTING,
    OP_RECORD_PREPARE,
    OP_RECORDING,
    OP_SAVING,
    OP_WAIT_FORCE,
    OP_SOFT_STOPPING,
    OP_DONE,
    OP_ERROR
} OperationState;
typedef struct
{
    OperationState state;
    ArmResult result;
    TrackResult track_result;
    char name[STORAGE_NAME_BYTES];
    uint32_t samples, elapsed_ms, progress_percent, force_hook_calls;
    bool aborted;
    int factory_index; /* -1 for Custom/other; UI translates fixed products. */
} OperationSnapshot;
typedef struct
{
    StartupManager startup;
    OperationSnapshot snapshot;
    TrackPlayer player;
    TrackRecorder recorder;
    const FactoryRecipe *recipe;
    unsigned recipe_step;
    uint32_t storage_ticket, started_ms;
    bool moving_home, pending_discard;
    ForceConditionState force_state;
    StorageRequestType storage_type;
} OperationManager;
void OperationManager_Init(OperationManager *operation, uint32_t now_ms);
ArmResult OperationManager_Command(OperationManager *operation, RobotControl *control,
                                   const AppCommand *command, uint32_t now_ms);
void OperationManager_Step(OperationManager *operation, RobotControl *control, uint32_t now_ms);
void OperationManager_SoftStop(OperationManager *operation, RobotControl *control, uint32_t now_ms);
#endif
