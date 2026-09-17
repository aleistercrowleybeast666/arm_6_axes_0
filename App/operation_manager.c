#include "operation_manager.h"
#include "robot_geometry.h"
#include <math.h>
#include <string.h>
#define OP_START_TOLERANCE_RAD (ROBOT_DEG_TO_RAD)
static JointVec6f Operation_Actual(uint32_t now, JointVec6f *velocity)
{
    MotorSnapshot s;
    MotorManager_GetSnapshot(now, &s);
    JointVec6f q;
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
    {
        q.q[i] = s.axes[i].position_rad;
        if (velocity)
            velocity->q[i] = s.axes[i].velocity_rad_s;
    }
    return q;
}
static bool Operation_AtHome(uint32_t now)
{
    JointVec6f q = Operation_Actual(now, NULL), h;
    RobotGeometry_GetFrozenPose(ROBOT_HOME, &h);
    for (unsigned i = 0; i < AXIS_COUNT; ++i)
        if (fabsf(q.q[i] - h.q[i]) > OP_START_TOLERANCE_RAD)
            return false;
    return true;
}
static ArmResult Operation_Home(RobotControl *c, uint32_t now)
{
    RobotCommand cmd = {.type = ROBOT_CMD_JOINT_MOVE};
    RobotGeometry_GetFrozenPose(ROBOT_HOME, &cmd.target_rad);
    c->external_command = false;
    return RobotControl_Execute(c, &cmd, now);
}
static ArmResult Operation_Error(OperationManager *o, ArmResult r, TrackResult t)
{
    o->snapshot.state = OP_ERROR;
    o->snapshot.result = r;
    o->snapshot.track_result = t;
    return r;
}
static ArmResult Operation_ExitTeach(RobotControl *c, uint32_t now)
{
    if (!c->teach_active)
        return ARM_OK;
    ArmResult r = MotorManager_ExitTeachMode();
    if (r != ARM_OK)
    {
        RobotControl_Fault(c, r);
        return r;
    }
    c->teach_active = false;
    c->hold = (RobotTrajectorySample){0};
    c->hold.q = Operation_Actual(now, &c->hold.dq_rad_s);
    return ARM_OK;
}
void OperationManager_Init(OperationManager *o, uint32_t now)
{
    if (o)
    {
        *o = (OperationManager){0};
        o->snapshot.factory_index = -1;
        StartupManager_Init(&o->startup, STARTUP_POSITION_ASSUME_STOW, now);
    }
}
static TrackResult Operation_Storage(OperationManager *o, RobotControl *c, StorageRequest *r)
{
    r->limits = c->soft;
    (void)MotorManager_GetMotionLimits(&r->motion);
    TrackResult t = StorageService_Request(r, &o->storage_ticket);
    if (t == TRACK_OK)
        o->storage_type = r->type;
    return t;
}
ArmResult OperationManager_Command(OperationManager *o, RobotControl *c, const AppCommand *cmd, uint32_t now)
{
    if (!o || !c || !cmd || cmd->source != CONTROL_SOURCE_LOCAL)
        return ARM_INVALID_ARGUMENT;
    if (cmd->type == CMD_SOFT_STOP)
    {
        OperationManager_SoftStop(o, c, now);
        return ARM_OK;
    }
    if (cmd->type == CMD_ACKNOWLEDGE)
    {
        if (o->storage_ticket || o->startup.state != STARTUP_READY)
            return ARM_NOT_READY;
        if (c->machine.state == ROBOT_SOFT_STOPPED)
        {
            ArmResult r = RobotControl_AcknowledgeStop(c);
            if (r != ARM_OK)
                return r;
        }
        if (c->machine.state != ROBOT_READY)
            return ARM_NOT_READY;
        o->snapshot.state = OP_IDLE;
        o->snapshot.aborted = false;
        return ARM_OK;
    }
    if (o->snapshot.state == OP_RECORDING &&
        (cmd->type == CMD_RECORD_FINISH || cmd->type == CMD_RECORD_CANCEL))
    {
        ArmResult e = Operation_ExitTeach(c, now);
        if (e != ARM_OK)
        {
            TrackRecorder_Abort(&o->recorder);
            return Operation_Error(o, e, TRACK_ABORTED);
        }
        /* End hand-guiding through a bounded stop; save is deferred until holding. */
        e = RobotControl_RequestSoftStop(c);
        if (e != ARM_OK)
        {
            TrackRecorder_Abort(&o->recorder);
            return Operation_Error(o, e, TRACK_ABORTED);
        }
        if (cmd->type == CMD_RECORD_CANCEL)
        {
            TrackRecorder_Abort(&o->recorder);
            o->snapshot.aborted = true;
            o->snapshot.state = OP_SOFT_STOPPING;
            return ARM_OK;
        }
        TrackResult t = TrackRecorder_Finish(&o->recorder);
        if (t != TRACK_OK)
        {
            TrackRecorder_Abort(&o->recorder);
            return Operation_Error(o, ARM_NOT_READY, t);
        }
        o->snapshot.state = OP_SAVING;
        return ARM_OK;
    }
    if (o->startup.state != STARTUP_READY || c->machine.state != ROBOT_READY || o->storage_ticket ||
        (o->snapshot.state != OP_IDLE && o->snapshot.state != OP_DONE && o->snapshot.state != OP_ERROR))
        return ARM_NOT_READY;
    o->snapshot = (OperationSnapshot){.state = OP_IDLE, .factory_index = -1};
    o->recipe = NULL;
    o->recipe_step = 0;
    o->moving_home = false;
    o->started_ms = now;
    if (cmd->type == CMD_GO_HOME)
    {
        ArmResult r = Operation_Home(c, now);
        if (r != ARM_OK)
            return Operation_Error(o, r, TRACK_NOT_READY);
        o->moving_home = true;
        o->snapshot.state = OP_EXECUTING;
        return ARM_OK;
    }
    if (cmd->type == CMD_RECORD_START)
    {
        StorageSnapshot storage;
        StorageService_GetSnapshot(&storage);
        if (!storage.mounted)
            return Operation_Error(o, ARM_NOT_READY, TRACK_NOT_READY);
        o->snapshot.state = OP_RECORD_PREPARE;
        if (!Operation_AtHome(now))
        {
            ArmResult r = Operation_Home(c, now);
            if (r != ARM_OK)
                return Operation_Error(o, r, TRACK_NOT_READY);
        }
        return ARM_OK;
    }
    StorageRequest r = {0};
    if (cmd->type >= CMD_EXEC_FACTORY_SMALL && cmd->type <= CMD_EXEC_FACTORY_LARGE)
    {
        unsigned i = cmd->type - CMD_EXEC_FACTORY_SMALL;
        o->recipe = FactoryRecipe_Get(i);
        o->snapshot.factory_index = (int)i;
        ForceSnapshot f;
        ForceSensor_GetSnapshot(now, &f);
        if (o->recipe->force_required && (!f.valid || !f.calibrated || f.stale || f.overload))
            return Operation_Error(o, ARM_NOT_READY, TRACK_NOT_READY);
        r.type = STORAGE_LOAD_TRACK;
        r.factory = true;
        r.factory_index = i;
    }
    else if (cmd->type == CMD_EXEC_CUSTOM)
    {
        r.type = STORAGE_LOAD_TRACK;
        memcpy(r.name, cmd->name, sizeof(r.name));
        memcpy(o->snapshot.name, cmd->name, sizeof(r.name));
    }
    else if (cmd->type == CMD_DELETE_CUSTOM)
    {
        r.type = STORAGE_DELETE_TRACK;
        memcpy(r.name, cmd->name, sizeof(r.name));
    }
    else
        return ARM_INVALID_ARGUMENT;
    TrackResult t = Operation_Storage(o, c, &r);
    if (t != TRACK_OK)
        return Operation_Error(o, ARM_NOT_READY, t);
    o->snapshot.state = OP_LOADING;
    return ARM_OK;
}
void OperationManager_SoftStop(OperationManager *o, RobotControl *c, uint32_t now)
{
    if (!o || !c)
        return;
    CommandManager_Clear();
    o->snapshot.aborted = true;
    o->pending_discard = o->storage_ticket != 0;
    TrackPlayer_Stop(&o->player);
    TrackRecorder_Abort(&o->recorder);
    (void)TrackBuffer_Transfer(TRACK_OWNER_READY, TRACK_OWNER_FREE);
    if (Operation_ExitTeach(c, now) != ARM_OK)
    {
        o->snapshot.state = OP_ERROR;
        return;
    }
    ArmResult r = RobotControl_RequestSoftStop(c);
    o->snapshot.result = r;
    o->snapshot.state = r == ARM_OK ? OP_SOFT_STOPPING : OP_ERROR;
}
static void Operation_RecipeStep(OperationManager *o, RobotControl *c, uint32_t now)
{
    if (!o->recipe)
    {
        o->snapshot.state = OP_DONE;
        return;
    }
    if (o->recipe_step >= 4)
    {
        (void)Operation_Error(o, ARM_INVALID_ARGUMENT, TRACK_BAD_FORMAT);
        return;
    }
    const RecipeStep *s = &o->recipe->steps[o->recipe_step];
    ForceSnapshot f;
    ForceSensor_GetSnapshot(now, &f);
    ArmResult r = ARM_NOT_CONFIGURED;
    if (s->type == RECIPE_STEP_DONE)
    {
        o->snapshot.state = OP_DONE;
        return;
    }
    if (s->type == RECIPE_STEP_MOVE_HOME)
    {
        r = Operation_Home(c, now);
        if (r == ARM_OK)
        {
            o->moving_home = true;
            o->snapshot.state = OP_EXECUTING;
            ++o->recipe_step;
            return;
        }
    }
    else if (s->type == RECIPE_STEP_FORCE_HOOK)
    {
        if (o->snapshot.state != OP_WAIT_FORCE)
        {
            ++o->snapshot.force_hook_calls;
            o->force_state = (ForceConditionState){.started_ms = now};
        }
        r = FactoryRecipe_ForceHook(o->recipe, &o->force_state, &f, now);
        if (r == ARM_FINISHED && o->recipe->end_action == FACTORY_END_MOVE_HOME)
        {
            ++o->recipe_step;
            r = Operation_Home(c, now);
            if (r == ARM_OK)
            {
                o->moving_home = true;
                o->snapshot.state = OP_EXECUTING;
                return;
            }
        }
        else if (r == ARM_FINISHED && o->recipe->end_action == FACTORY_END_RESERVED)
            r = ARM_NOT_CONFIGURED;
    }
    else if (s->type == RECIPE_STEP_WAIT_FORCE_CONDITION)
    {
        if (o->snapshot.state != OP_WAIT_FORCE)
            o->force_state = (ForceConditionState){.started_ms = now};
        r = ForceCondition_Evaluate(&s->condition, &o->force_state, &f, now);
    }
    if (r == ARM_FINISHED)
    {
        ++o->recipe_step;
        o->snapshot.state = OP_EXECUTING;
        return;
    }
    if (r == ARM_NOT_READY && f.valid)
    {
        o->snapshot.state = OP_WAIT_FORCE;
        return;
    }
    (void)Operation_Error(o, r, TRACK_NOT_READY);
}
void OperationManager_Step(OperationManager *o, RobotControl *c, uint32_t now)
{
    if (!o || !c)
        return;
    if (o->startup.state != STARTUP_READY)
    {
        (void)StartupManager_Step(&o->startup, c, now);
        if (o->startup.state == STARTUP_FAILED)
        {
            o->snapshot.state = OP_ERROR;
            o->snapshot.result = o->startup.result;
        }
        return;
    }
    if (c->machine.state == ROBOT_FAULT)
    {
        o->pending_discard = true;
        TrackPlayer_Stop(&o->player);
        TrackRecorder_Abort(&o->recorder);
        o->snapshot.state = OP_ERROR;
        o->snapshot.result = ARM_FAULT;
    }
    if (o->storage_ticket)
    {
        StorageResponse r;
        if (StorageService_TakeResult(o->storage_ticket, &r) == TRACK_OK)
        {
            o->storage_ticket = 0;
            if (o->pending_discard)
            {
                (void)TrackBuffer_Transfer(TRACK_OWNER_READY, TRACK_OWNER_FREE);
                o->pending_discard = false;
            }
            else if (r.result != TRACK_OK)
            {
                (void)Operation_Error(o, ARM_NOT_READY, r.result);
            }
            else if (o->storage_type == STORAGE_LOAD_TRACK)
            {
                JointVec6f actual = Operation_Actual(now, NULL);
                TrackResult t = TrackPlayer_Start(&o->player, &actual, OP_START_TOLERANCE_RAD);
                if (t != TRACK_OK)
                {
                    (void)TrackBuffer_Transfer(TRACK_OWNER_READY, TRACK_OWNER_FREE);
                    (void)Operation_Error(o, ARM_NOT_READY, t);
                }
                else
                {
                    o->snapshot.state = OP_EXECUTING;
                    o->started_ms = now;
                    c->external_command = true;
                }
            }
            else
            {
                o->snapshot.state = OP_DONE;
                memcpy(o->snapshot.name, r.name, sizeof(r.name));
            }
        }
    }
    if (c->machine.state == ROBOT_FAULT)
        return;
    if (o->snapshot.state == OP_SOFT_STOPPING)
    {
        if (!c->stopping.active)
            o->snapshot.state = OP_DONE;
        return;
    }
    if (o->snapshot.state == OP_RECORD_PREPARE && c->machine.state == ROBOT_READY)
    {
        if (!Operation_AtHome(now))
        {
            (void)Operation_Error(o, ARM_OUT_OF_LIMIT, TRACK_LIMIT_ERROR);
            return;
        }
        ArmResult r = MotorManager_EnterTeachMode();
        if (r != ARM_OK)
        {
            (void)Operation_Error(o, r, TRACK_NOT_READY);
            return;
        }
        c->teach_active = true;
        JointVec6f q = Operation_Actual(now, NULL);
        TrackResult t = TrackRecorder_Start(&o->recorder, &q, now);
        if (t != TRACK_OK)
        {
            (void)Operation_ExitTeach(c, now);
            (void)Operation_Error(o, ARM_NOT_READY, t);
            return;
        }
        o->snapshot.state = OP_RECORDING;
        o->started_ms = now;
    }
    if (o->snapshot.state == OP_RECORDING)
    {
        JointVec6f q = Operation_Actual(now, NULL);
        TrackResult t = TrackRecorder_Push(&o->recorder, &q, now, &c->soft);
        o->snapshot.samples = o->recorder.count;
        if (t != TRACK_OK)
        {
            OperationManager_SoftStop(o, c, now);
            o->snapshot.track_result = t;
        }
        o->snapshot.elapsed_ms = now - o->started_ms;
        return;
    }
    if (o->snapshot.state == OP_SAVING && !o->storage_ticket && !c->stopping.active)
    {
        if (c->machine.state == ROBOT_SOFT_STOPPED)
            (void)RobotControl_AcknowledgeStop(c);
        StorageRequest r = {.type = STORAGE_SAVE_TRACK};
        TrackResult t = Operation_Storage(o, c, &r);
        if (t != TRACK_OK)
        {
            TrackRecorder_Abort(&o->recorder);
            (void)Operation_Error(o, ARM_NOT_READY, t);
        }
        return;
    }
    if (o->snapshot.state == OP_EXECUTING)
    {
        if (o->moving_home)
        {
            if (c->machine.state == ROBOT_READY)
            {
                o->moving_home = false;
                if (o->recipe)
                    Operation_RecipeStep(o, c, now);
                else
                    o->snapshot.state = OP_DONE;
            }
            return;
        }
        if (o->player.active)
        {
            if (o->recipe && o->recipe->force_required)
            {
                ForceSnapshot f;
                ForceSensor_GetSnapshot(now, &f);
                if (!f.valid || f.stale || f.overload)
                {
                    OperationManager_SoftStop(o, c, now);
                    o->snapshot.result = ARM_FAULT;
                    return;
                }
            }
            RobotTrajectorySample sample;
            TrackResult t = TrackPlayer_Step(&o->player, 0.005f, &sample);
            if (t != TRACK_OK || RobotControl_SetExternal(c, &sample) != ARM_OK)
            {
                OperationManager_SoftStop(o, c, now);
                o->snapshot.track_result = t;
                return;
            }
            o->snapshot.progress_percent = (uint32_t)fminf(
                100, 100 * o->player.elapsed_s /
                         ((o->player.header.count - 1) * o->player.header.period_us * 0.000001f));
            if (o->player.finished)
            {
                c->external_command = false;
                c->machine.state = ROBOT_READY;
                if (o->recipe)
                    ++o->recipe_step;
                else
                    o->snapshot.state = OP_DONE;
            }
        }
        else if (o->recipe)
            Operation_RecipeStep(o, c, now);
    }
    else if (o->snapshot.state == OP_WAIT_FORCE)
        Operation_RecipeStep(o, c, now);
}
