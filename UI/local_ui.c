#include "local_ui.h"
#include "ui_strings.h"
#include "text_format.h"
#include <string.h>
void LocalUi_Init(LocalUi *u)
{
    if (u)
        *u = (LocalUi){.page = UI_PAGE_MAIN};
}
static unsigned Ui_Count(const LocalUi *u, const StorageSnapshot *s)
{
    switch (u->page)
    {
    case UI_PAGE_MAIN:
        return 2;
    case UI_PAGE_ACTIONS:
        return 4 + s->count;
    case UI_PAGE_FACTORY:
        return 2;
    case UI_PAGE_CUSTOM:
        return 3;
    case UI_PAGE_DELETE_CONFIRM:
        return 2;
    case UI_PAGE_RECORD:
        return 2;
    default:
        return 0;
    }
}
static bool Ui_IsBusy(OperationState s)
{
    return s != OP_IDLE && s != OP_DONE && s != OP_ERROR;
}
void LocalUi_SetCommandResult(LocalUi *u, ArmResult r)
{
    if (u && r != ARM_OK)
    {
        u->pending_command = false;
        u->show_error = true;
        u->error = UI_TEXT_NOT_CONFIGURED;
    }
}
bool LocalUi_Handle(LocalUi *u, UiEvent e, StartupState startup, RobotState robot, const OperationSnapshot *o,
                    const StorageSnapshot *s, AppCommand *cmd)
{
    if (!u || !o || !s || !cmd)
        return false;
    *cmd = (AppCommand){.source = CONTROL_SOURCE_LOCAL};
    if (startup != STARTUP_READY)
        return false;
    if (u->show_error)
    {
        if (e == BACK || e == FORWARD)
            u->show_error = false;
        return false;
    }
    if (s->revision != u->catalog_revision)
    {
        u->catalog_revision = s->revision;
        if (u->page == UI_PAGE_CUSTOM || u->page == UI_PAGE_DELETE_CONFIRM)
        {
            u->page = UI_PAGE_ACTIONS;
            u->selected = 0;
        }
    }
    if (Ui_IsBusy(o->state) || robot == ROBOT_SOFT_STOPPED || o->state == OP_DONE || o->state == OP_ERROR ||
        u->pending_command)
    {
        if (Ui_IsBusy(o->state))
            u->pending_command = false;
        if (o->state == OP_RECORDING && (e == FORWARD || e == BACK))
        {
            cmd->type = e == FORWARD ? CMD_RECORD_FINISH : CMD_RECORD_CANCEL;
            return true;
        }
        if ((o->state == OP_DONE || o->state == OP_ERROR || robot == ROBOT_SOFT_STOPPED) &&
            (e == FORWARD || e == BACK))
        {
            cmd->type = CMD_ACKNOWLEDGE;
            u->page = UI_PAGE_MAIN;
            u->selected = 0;
            u->pending_command = false;
            return true;
        }
        return false;
    }
    unsigned n = Ui_Count(u, s);
    if (e == ENCODER_NEXT && n)
    {
        u->selected = (u->selected + 1) % n;
        return false;
    }
    if (e == ENCODER_PREV && n)
    {
        u->selected = (u->selected + n - 1) % n;
        return false;
    }
    if (e == BACK)
    {
        u->page = u->page == UI_PAGE_ACTIONS || u->page == UI_PAGE_RECORD ? UI_PAGE_MAIN : UI_PAGE_ACTIONS;
        u->selected = 0;
        return false;
    }
    if (e != FORWARD)
        return false;
    if (u->page == UI_PAGE_MAIN)
    {
        u->page = u->selected ? UI_PAGE_RECORD : UI_PAGE_ACTIONS;
        u->selected = 0;
        return false;
    }
    if (u->page == UI_PAGE_ACTIONS)
    {
        u->action_index = u->selected;
        if (u->selected == 0)
        {
            cmd->type = CMD_GO_HOME;
            u->pending_command = true;
            return true;
        }
        if (u->selected <= 3)
            u->page = UI_PAGE_FACTORY;
        else
        {
            unsigned i = u->selected - 4;
            if (i >= s->count)
                return false;
            strcpy(u->custom_name, s->names[i]);
            u->page = UI_PAGE_CUSTOM;
        }
        u->selected = 0;
        return false;
    }
    if (u->page == UI_PAGE_FACTORY)
    {
        if (u->selected)
        {
            u->page = UI_PAGE_ACTIONS;
            u->selected = 0;
            return false;
        }
        if (!s->mounted || !s->factory_available[u->action_index - 1])
        {
            u->show_error = true;
            u->error = s->mounted ? UI_TEXT_TRACK_UNAVAILABLE : UI_TEXT_STORAGE_MISSING;
            return false;
        }
        cmd->type = (AppCommandType)(CMD_EXEC_FACTORY_SMALL + u->action_index - 1);
        u->pending_command = true;
        return true;
    }
    if (u->page == UI_PAGE_CUSTOM)
    {
        if (u->selected == 2)
        {
            u->page = UI_PAGE_ACTIONS;
            u->selected = 0;
            return false;
        }
        if (u->selected == 1)
        {
            u->page = UI_PAGE_DELETE_CONFIRM;
            u->selected = 1;
            return false;
        }
        cmd->type = CMD_EXEC_CUSTOM;
        strcpy(cmd->name, u->custom_name);
        u->pending_command = true;
        return true;
    }
    if (u->page == UI_PAGE_DELETE_CONFIRM)
    {
        if (u->selected)
        {
            u->page = UI_PAGE_CUSTOM;
            u->selected = 0;
            return false;
        }
        cmd->type = CMD_DELETE_CUSTOM;
        strcpy(cmd->name, u->custom_name);
        u->pending_command = true;
        return true;
    }
    if (u->page == UI_PAGE_RECORD)
    {
        if (u->selected)
        {
            u->page = UI_PAGE_MAIN;
            u->selected = 0;
            return false;
        }
        if (!s->mounted)
        {
            u->show_error = true;
            u->error = UI_TEXT_STORAGE_MISSING;
            return false;
        }
        cmd->type = CMD_RECORD_START;
        u->pending_command = true;
        return true;
    }
    return false;
}
static void Ui_Row(UiModel *m, unsigned row, const char *text)
{
    (void)Text_Format(m->rows[row], sizeof(m->rows[row]), "%s", text);
}
void LocalUi_Render(const LocalUi *u, StartupState startup, RobotState robot, const OperationSnapshot *o,
                    const StorageSnapshot *s, UiModel *m)
{
    if (!u || !o || !s || !m)
        return;
    *m = (UiModel){0};
    if (startup != STARTUP_READY)
    {
        UiText text = startup == STARTUP_BOOT
                          ? UI_TEXT_INITIALIZING
                          : (startup == STARTUP_TO_HOME
                                 ? UI_TEXT_TO_HOME
                                 : (startup == STARTUP_FAILED ? UI_TEXT_NOT_CONFIGURED : UI_TEXT_CONNECTING));
        Ui_Row(m, 0, UiStrings_Get(text));
        return;
    }
    if (u->show_error)
    {
        Ui_Row(m, 0, UiStrings_Get(u->error));
        return;
    }
    if (Ui_IsBusy(o->state) || o->state == OP_DONE || o->state == OP_ERROR || robot == ROBOT_SOFT_STOPPED ||
        u->pending_command)
    {
        UiText text = UI_TEXT_LOADING;
        switch (o->state)
        {
        case OP_RECORDING:
            text = UI_TEXT_RECORDING;
            break;
        case OP_RECORD_PREPARE:
            text = UI_TEXT_TO_HOME;
            break;
        case OP_SAVING:
            text = UI_TEXT_SAVING;
            break;
        case OP_EXECUTING:
        case OP_WAIT_FORCE:
            text = UI_TEXT_EXECUTING;
            break;
        case OP_DONE:
            text = UI_TEXT_DONE;
            break;
        case OP_ERROR:
            text = o->result == ARM_NOT_CONFIGURED
                       ? UI_TEXT_NOT_CONFIGURED
                       : (o->track_result == TRACK_NOT_READY && !s->mounted ? UI_TEXT_STORAGE_MISSING
                                                                            : UI_TEXT_ERROR);
            break;
        default:
            break;
        }
        if (robot == ROBOT_SOFT_STOPPED || o->aborted)
            text = UI_TEXT_STOPPED;
        if (o->track_result == TRACK_BUFFER_FULL)
            text = UI_TEXT_TOO_LONG;
        Ui_Row(m, 0, UiStrings_Get(text));
        if (o->state == OP_RECORDING)
        {
            (void)Text_Format(m->rows[1], 64, "%02lu:%02lu", (unsigned long)(o->elapsed_ms / 60000),
                              (unsigned long)(o->elapsed_ms / 1000 % 60));
            (void)Text_Format(m->rows[2], 64, "%lu/4096", (unsigned long)o->samples);
        }
        else
        {
            char stem[STORAGE_NAME_BYTES];
            strcpy(stem, o->name);
            char *dot = strrchr(stem, '.');
            if (dot)
                *dot = 0;
            Ui_Row(m, 1,
                   o->factory_index >= 0 && o->factory_index < 3 && o->state == OP_EXECUTING
                       ? UiStrings_Get((UiText)(UI_TEXT_SMALL + o->factory_index))
                       : stem);
            if (o->state == OP_EXECUTING)
                (void)Text_Format(m->rows[2], 64, "%lu%%", (unsigned long)o->progress_percent);
        }
        return;
    }
    unsigned count = Ui_Count(u, s), base = (u->selected / 4) * 4;
    m->has_selection = true;
    m->selected = u->selected - base;
    for (unsigned row = 0; row < 4 && base + row < count; ++row)
    {
        unsigned i = base + row;
        UiText t = UI_TEXT_RETURN;
        switch (u->page)
        {
        case UI_PAGE_MAIN:
            t = i ? UI_TEXT_RECORD : UI_TEXT_EXECUTE;
            break;
        case UI_PAGE_ACTIONS:
            if (i >= 4)
            {
                char stem[STORAGE_NAME_BYTES];
                strcpy(stem, s->names[i - 4]);
                stem[strlen(stem) - 4] = 0;
                Ui_Row(m, row, stem);
                continue;
            }
            t = (UiText)(UI_TEXT_HOME + i);
            break;
        case UI_PAGE_FACTORY:
            t = i ? UI_TEXT_RETURN : UI_TEXT_START;
            break;
        case UI_PAGE_CUSTOM:
            t = i == 0 ? UI_TEXT_START : (i == 1 ? UI_TEXT_DELETE : UI_TEXT_RETURN);
            break;
        case UI_PAGE_DELETE_CONFIRM:
            t = i ? UI_TEXT_NO : UI_TEXT_YES;
            break;
        case UI_PAGE_RECORD:
            t = i ? UI_TEXT_RETURN : UI_TEXT_RECORD_START;
            break;
        default:
            break;
        }
        Ui_Row(m, row, UiStrings_Get(t));
    }
    if (u->page == UI_PAGE_DELETE_CONFIRM)
    {
        for (int i = 2; i >= 0; --i)
            memcpy(m->rows[i + 1], m->rows[i], 64);
        Ui_Row(m, 0, UiStrings_Get(UI_TEXT_CONFIRM_DELETE));
        ++m->selected;
    }
}
