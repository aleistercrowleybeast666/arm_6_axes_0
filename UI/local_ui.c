#include "local_ui.h"
#include "ui_strings.h"
#include "text_format.h"
#include <string.h>
#define UI_CONTENT_ROWS 3U
void LocalUi_Init(LocalUi *u)
{
    if (u)
        *u = (LocalUi){.page = UI_PAGE_MAIN, .custom_parent = UI_PAGE_ACTIONS};
}
void LocalUi_SetDeviceStatus(LocalUi *u, const UiDeviceStatus *s)
{
    if (u && s)
        u->device = *s;
}
static unsigned Ui_Count(const LocalUi *u, const StorageSnapshot *s)
{
    switch (u->page)
    {
    case UI_PAGE_MAIN: return 4;
    case UI_PAGE_ACTIONS: return 5 + s->count;
    case UI_PAGE_FACTORY: return 2;
    case UI_PAGE_CUSTOM: return 3;
    case UI_PAGE_DELETE_CONFIRM: return 2;
    case UI_PAGE_RECORD: return 2;
    case UI_PAGE_TF: return 4;
    case UI_PAGE_FILES: return s->count + 1;
    case UI_PAGE_DEVICE: return 2;
    case UI_PAGE_SYSTEM: case UI_PAGE_TF_STATUS: return 1;
    case UI_PAGE_START_CONFIRM: return 2;
    default: return 0;
    }
}
static bool Ui_IsBusy(OperationState s)
{
    return s != OP_IDLE && s != OP_DONE && s != OP_ERROR;
}
static bool Ui_StatusVisible(StartupState startup, RobotState robot, const OperationSnapshot *o)
{
    return (startup == STARTUP_READY || o->storage_only) &&
           (Ui_IsBusy(o->state) || o->state == OP_DONE || o->state == OP_ERROR || robot == ROBOT_SOFT_STOPPED);
}
void LocalUi_SetCommandResult(LocalUi *u, ArmResult r)
{
    if (u && r != ARM_OK)
    {
        u->pending_command = false;
        u->show_error = true;
        u->error = r == ARM_NOT_READY && u->page != UI_PAGE_TF && u->page != UI_PAGE_DELETE_CONFIRM
                       ? UI_TEXT_MOTOR_OFFLINE : UI_TEXT_NOT_CONFIGURED;
    }
}
static void Ui_Page(LocalUi *u, UiPage page, unsigned selected)
{
    u->page = page;
    u->selected = selected;
    u->selected_since_ms = u->device.now_ms;
}
static bool Ui_RequireMotion(LocalUi *u, StartupState startup, RobotState robot)
{
    if (startup == STARTUP_READY && robot == ROBOT_READY)
        return true;
    u->show_error = true;
    u->error = UI_TEXT_MOTOR_OFFLINE;
    return false;
}
static void Ui_Enter(LocalUi *u, UiPage page, unsigned selected)
{
    if (u->navigation_depth == UI_NAV_DEPTH)
    {
        u->show_error = true;
        u->error = UI_TEXT_NOT_CONFIGURED;
        return;
    }
    u->navigation[u->navigation_depth++] = (UiNavigation){
        .page = u->page, .selected = u->selected,
        .scroll_elapsed_ms = u->device.now_ms - u->selected_since_ms};
    Ui_Page(u, page, selected);
}
static void Ui_Back(LocalUi *u)
{
    if (u->navigation_depth)
    {
        const UiNavigation *parent = &u->navigation[--u->navigation_depth];
        u->page = parent->page;
        u->selected = parent->selected;
        u->selected_since_ms = u->device.now_ms - parent->scroll_elapsed_ms;
        return;
    }
    switch (u->page)
    {
    case UI_PAGE_MAIN: break;
    case UI_PAGE_ACTIONS: Ui_Page(u, UI_PAGE_MAIN, 0); break;
    case UI_PAGE_RECORD: Ui_Page(u, UI_PAGE_MAIN, 1); break;
    case UI_PAGE_TF: Ui_Page(u, UI_PAGE_MAIN, 2); break;
    case UI_PAGE_DEVICE: Ui_Page(u, UI_PAGE_MAIN, 3); break;
    case UI_PAGE_SYSTEM: Ui_Page(u, UI_PAGE_DEVICE, 0); break;
    case UI_PAGE_FILES: Ui_Page(u, UI_PAGE_TF, 0); break;
    case UI_PAGE_TF_STATUS: Ui_Page(u, UI_PAGE_TF, 1); break;
    case UI_PAGE_DELETE_CONFIRM: Ui_Page(u, UI_PAGE_CUSTOM, 1); break;
    case UI_PAGE_CUSTOM: Ui_Page(u, u->custom_parent, u->return_selection); break;
    default: Ui_Page(u, UI_PAGE_ACTIONS, u->action_index); break;
    }
}
static void Ui_OpenStart(LocalUi *u, AppCommandType type)
{
    u->start_command = type;
    Ui_Enter(u, UI_PAGE_START_CONFIRM, 1); /* Return is always selected; opening never starts. */
}
static bool Ui_IsStartPage(UiPage page)
{
    return page == UI_PAGE_FACTORY || page == UI_PAGE_RECORD || page == UI_PAGE_START_CONFIRM;
}
bool LocalUi_MotionActive(StartupState startup, RobotState robot, const OperationSnapshot *o)
{
    if (!o || o->state == OP_SOFT_STOPPING || robot == ROBOT_SOFT_STOPPED)
        return false;
    return startup == STARTUP_TO_HOME || robot == ROBOT_RUNNING || robot == ROBOT_STARTUP ||
           (!o->storage_only && (o->state == OP_LOADING || o->state == OP_EXECUTING ||
             o->state == OP_WAIT_FORCE || o->state == OP_RECORD_PREPARE || o->state == OP_RECORDING));
}
bool LocalUi_Handle(LocalUi *u, UiEvent e, StartupState startup, RobotState robot,
                    const OperationSnapshot *o, const StorageSnapshot *s, AppCommand *cmd)
{
    if (!u || !o || !s || !cmd)
        return false;
    *cmd = (AppCommand){.source = CONTROL_SOURCE_LOCAL};
    bool motion_pending = u->pending_command && Ui_IsStartPage(u->page);
    /* Encoder stop precedes menu focus, errors and startup/status pages. */
    if (e == ENCODER_PRESS && (LocalUi_MotionActive(startup, robot, o) || motion_pending))
    {
        cmd->type = CMD_SOFT_STOP;
        u->pending_command = false;
        return true;
    }
    if (e == ENCODER_PRESS)
        e = FORWARD;
    if (startup == STARTUP_TO_HOME)
        return false;
    if (u->show_error)
    {
        if (e == BACK || e == FORWARD)
        {
            u->show_error = false;
            if (Ui_IsStartPage(u->page))
                Ui_Back(u);
            if (o->state == OP_ERROR && (startup == STARTUP_READY || o->storage_only))
            {
                cmd->type = CMD_ACKNOWLEDGE;
                return true;
            }
        }
        return false;
    }
    if (s->revision != u->catalog_revision)
    {
        u->catalog_revision = s->revision;
        if (u->page == UI_PAGE_CUSTOM || u->page == UI_PAGE_DELETE_CONFIRM ||
            (u->page == UI_PAGE_START_CONFIRM && u->start_command == CMD_EXEC_CUSTOM))
        {
            do { Ui_Back(u); } while (u->navigation_depth && u->page != u->custom_parent);
        }
        unsigned n = Ui_Count(u, s);
        if (n && u->selected >= n)
            u->selected = n - 1;
    }
    if (Ui_StatusVisible(startup, robot, o) || u->pending_command)
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
            u->pending_command = false;
            if (Ui_IsStartPage(u->page))
                Ui_Back(u); /* A fresh Start/Return confirmation is required for any later run. */
            return true;
        }
        return false;
    }
    unsigned n = Ui_Count(u, s);
    if ((e == ENCODER_NEXT || e == ENCODER_PREV) && n)
    {
        u->selected = (u->selected + (e == ENCODER_NEXT ? 1 : n - 1)) % n;
        u->selected_since_ms = u->device.now_ms;
        return false;
    }
    if (e == BACK)
    {
        Ui_Back(u);
        return false;
    }
    if (e != FORWARD)
        return false;
    if (u->page == UI_PAGE_MAIN)
    {
        const UiPage pages[] = {UI_PAGE_ACTIONS, UI_PAGE_RECORD, UI_PAGE_TF, UI_PAGE_DEVICE};
        if (u->selected == 1)
            u->start_command = CMD_RECORD_START;
        Ui_Enter(u, pages[u->selected], u->selected == 1 ? 1 : 0);
        return false;
    }
    if (u->page == UI_PAGE_DEVICE)
    {
        if (u->selected)
            Ui_Back(u);
        else
            Ui_Enter(u, UI_PAGE_SYSTEM, 0);
        return false;
    }
    if (u->page == UI_PAGE_SYSTEM || u->page == UI_PAGE_TF_STATUS)
    {
        Ui_Back(u);
        return false;
    }
    if (u->page == UI_PAGE_TF)
    {
        if (u->selected == 3)
        {
            Ui_Back(u);
            return false;
        }
        if (u->selected < 2)
        {
            Ui_Enter(u, u->selected ? UI_PAGE_TF_STATUS : UI_PAGE_FILES, 0);
            return false;
        }
        if (!s->mounted)
        {
            u->show_error = true;
            u->error = UI_TEXT_STORAGE_MISSING;
            return false;
        }
        cmd->type = CMD_SCAN_TRACKS;
    }
    else if (u->page == UI_PAGE_ACTIONS || u->page == UI_PAGE_FILES)
    {
        if (u->selected == n - 1)
        {
            Ui_Back(u);
            return false;
        }
        if (u->page == UI_PAGE_ACTIONS && u->selected == 0)
        {
            Ui_OpenStart(u, CMD_GO_HOME);
            return false;
        }
        u->action_index = u->selected;
        if (u->page == UI_PAGE_ACTIONS && u->selected <= 3)
        {
            u->start_command = (AppCommandType)(CMD_EXEC_FACTORY_SMALL + u->selected - 1);
            Ui_Enter(u, UI_PAGE_FACTORY, 1);
        }
        else
        {
            unsigned i = u->page == UI_PAGE_FILES ? u->selected : u->selected - 4;
            if (i >= s->count)
                return false;
            u->custom_parent = u->page;
            u->return_selection = u->selected;
            strcpy(u->custom_name, s->names[i]);
            Ui_Enter(u, UI_PAGE_CUSTOM, 0);
        }
        return false;
    }
    else if (u->page == UI_PAGE_CUSTOM)
    {
        if (u->selected == 2)
            Ui_Back(u);
        else if (u->selected == 1)
            Ui_Enter(u, UI_PAGE_DELETE_CONFIRM, 1);
        else
            Ui_OpenStart(u, CMD_EXEC_CUSTOM);
        return false;
    }
    else if (u->page == UI_PAGE_DELETE_CONFIRM)
    {
        if (u->selected)
        {
            Ui_Back(u);
            return false;
        }
        if (!s->mounted)
        {
            u->show_error = true;
            u->error = UI_TEXT_STORAGE_MISSING;
            return false;
        }
        cmd->type = CMD_DELETE_CUSTOM;
        strcpy(cmd->name, u->custom_name);
    }
    else if (Ui_IsStartPage(u->page))
    {
        if (u->selected)
        {
            Ui_Back(u);
            return false;
        }
        /* Recheck current dependencies only at the explicit Start press. Control checks them again. */
        if (!Ui_RequireMotion(u, startup, robot))
            return false;
        if (u->start_command != CMD_GO_HOME && !s->mounted)
        {
            u->show_error = true;
            u->error = UI_TEXT_STORAGE_MISSING;
            return false;
        }
        if (u->start_command >= CMD_EXEC_FACTORY_SMALL && u->start_command <= CMD_EXEC_FACTORY_LARGE &&
            !s->factory_available[u->start_command - CMD_EXEC_FACTORY_SMALL])
        {
            u->show_error = true;
            u->error = UI_TEXT_TRACK_UNAVAILABLE;
            return false;
        }
        cmd->type = u->start_command;
        if (cmd->type == CMD_EXEC_CUSTOM)
            strcpy(cmd->name, u->custom_name);
    }
    else
        return false;
    u->pending_command = true;
    return true;
}
static void Ui_Row(UiModel *m, unsigned row, const char *text)
{
    (void)Text_Format(m->rows[row], sizeof(m->rows[row]), "%s", text);
}
static UiText Ui_Title(const LocalUi *u, StartupState startup)
{
    switch (u->page)
    {
    case UI_PAGE_MAIN: return startup == STARTUP_READY ? UI_TEXT_MAIN : UI_TEXT_OFFLINE;
    case UI_PAGE_ACTIONS: return UI_TEXT_EXECUTE;
    case UI_PAGE_FACTORY: return (UiText)(UI_TEXT_SMALL + u->action_index - 1);
    case UI_PAGE_CUSTOM: return UI_TEXT_FILES;
    case UI_PAGE_DELETE_CONFIRM: return UI_TEXT_CONFIRM_DELETE;
    case UI_PAGE_RECORD: return UI_TEXT_RECORD;
    case UI_PAGE_TF: return UI_TEXT_TF;
    case UI_PAGE_FILES: return UI_TEXT_FILES;
    case UI_PAGE_TF_STATUS: return UI_TEXT_TF_STATUS;
    case UI_PAGE_DEVICE: return UI_TEXT_DEVICE;
    case UI_PAGE_SYSTEM: return UI_TEXT_SYSTEM;
    case UI_PAGE_START_CONFIRM: return UI_TEXT_CONFIRM_START;
    default: return UI_TEXT_MAIN;
    }
}
void LocalUi_Render(const LocalUi *u, StartupState startup, RobotState robot, const OperationSnapshot *o,
                    const StorageSnapshot *s, UiModel *m)
{
    if (!u || !o || !s || !m)
        return;
    *m = (UiModel){.has_title = true};
    Ui_Row(m, 0, UiStrings_Get(Ui_Title(u, startup)));
    if (u->show_error)
    {
        Ui_Row(m, 1, UiStrings_Get(u->error));
        Ui_Row(m, 3, UiStrings_Get(UI_TEXT_BACK_HINT));
        return;
    }
    if (startup == STARTUP_TO_HOME)
    {
        Ui_Row(m, 0, UiStrings_Get(UI_TEXT_TO_HOME));
        Ui_Row(m, 3, UiStrings_Get(UI_TEXT_ENCODER_STOP));
        return;
    }
    if (Ui_StatusVisible(startup, robot, o) || u->pending_command)
    {
        UiText text = UI_TEXT_LOADING;
        switch (o->state)
        {
        case OP_RECORDING: text = UI_TEXT_RECORDING; break;
        case OP_RECORD_PREPARE: text = UI_TEXT_TO_HOME; break;
        case OP_SAVING: text = UI_TEXT_SAVING; break;
        case OP_EXECUTING: case OP_WAIT_FORCE: text = UI_TEXT_EXECUTING; break;
        case OP_DONE: text = UI_TEXT_DONE; break;
        case OP_ERROR:
            text = o->result == ARM_NOT_CONFIGURED ? UI_TEXT_NOT_CONFIGURED :
                   (!s->mounted ? UI_TEXT_STORAGE_MISSING : UI_TEXT_ERROR);
            break;
        default: break;
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
        else if (o->state == OP_EXECUTING)
        {
            Ui_Row(m, 1, o->factory_index >= 0 && o->factory_index < 3
                              ? UiStrings_Get((UiText)(UI_TEXT_SMALL + o->factory_index)) : o->name);
            (void)Text_Format(m->rows[2], 64, "%lu%%", (unsigned long)o->progress_percent);
        }
        if (LocalUi_MotionActive(startup, robot, o))
            Ui_Row(m, 3, UiStrings_Get(UI_TEXT_ENCODER_STOP));
        else if (o->state == OP_DONE || o->state == OP_ERROR || robot == ROBOT_SOFT_STOPPED)
            Ui_Row(m, 3, UiStrings_Get(UI_TEXT_BACK_HINT));
        return;
    }
    if (u->page == UI_PAGE_DELETE_CONFIRM)
    {
        /* All 31 allowed ASCII filename bytes are visible before deletion. */
        size_t bytes = strlen(u->custom_name);
        size_t first = bytes < 16 ? bytes : 16;
        memcpy(m->rows[1], u->custom_name, first);
        if (bytes > first)
            Ui_Row(m, 2, u->custom_name + first);
        Ui_Row(m, 3, u->selected ? "[否] 是" : "否 [是]");
        return;
    }
    if (u->page == UI_PAGE_DEVICE)
    {
        if (!u->device.can_ready)
            Ui_Row(m, 1, UiStrings_Get(UI_TEXT_CAN_ERROR));
        else if (!u->device.motors_configured)
            Ui_Row(m, 1, UiStrings_Get(UI_TEXT_MOTOR_CONFIG));
        else
            (void)Text_Format(m->rows[1], 64, "电机 %u/6", u->device.online_motors);
        Ui_Row(m, 2, UiStrings_Get(!u->device.force_online ? UI_TEXT_FORCE_OFFLINE :
                                  (!u->device.force_calibrated ? UI_TEXT_FORCE_CONFIG :
                                   (u->device.force_valid ? UI_TEXT_FORCE_READY : UI_TEXT_FORCE_INVALID))));
        Ui_Row(m, 3, u->selected ? "系统信息 [返回]" : "[系统信息] 返回");
        return;
    }
    if (u->page == UI_PAGE_SYSTEM)
    {
        (void)Text_Format(m->rows[1], 64, "固件 %s", u->device.version);
        Ui_Row(m, 2, UiStrings_Get(u->device.output_armed ? UI_TEXT_OUTPUT_ON : UI_TEXT_OUTPUT_OFF));
        Ui_Row(m, 3, UiStrings_Get(UI_TEXT_RETURN));
        m->has_selection = true;
        m->selected = 3;
        return;
    }
    if (u->page == UI_PAGE_TF_STATUS)
    {
        Ui_Row(m, 1, UiStrings_Get(s->mounted ? UI_TEXT_TF_READY : UI_TEXT_STORAGE_MISSING));
        (void)Text_Format(m->rows[2], 64, "个人%u/32 码%u", s->count, (unsigned)s->last_error);
        Ui_Row(m, 3, UiStrings_Get(UI_TEXT_RETURN));
        m->has_selection = true;
        m->selected = 3;
        return;
    }
    unsigned count = Ui_Count(u, s), base = (u->selected / UI_CONTENT_ROWS) * UI_CONTENT_ROWS;
    if (count > UI_CONTENT_ROWS)
        (void)Text_Format(m->rows[0], 64, "%s %u/%u", UiStrings_Get(Ui_Title(u, startup)),
                          base / UI_CONTENT_ROWS + 1, (count + UI_CONTENT_ROWS - 1) / UI_CONTENT_ROWS);
    if (u->page == UI_PAGE_FILES && !s->count)
    {
        Ui_Row(m, 1, UiStrings_Get(s->mounted ? UI_TEXT_EMPTY : UI_TEXT_STORAGE_MISSING));
        Ui_Row(m, 3, UiStrings_Get(UI_TEXT_RETURN));
        m->has_selection = true;
        m->selected = 3;
        return;
    }
    m->has_selection = count != 0;
    m->selected = u->selected - base + 1;
    for (unsigned row = 1; row <= UI_CONTENT_ROWS && base + row - 1 < count; ++row)
    {
        unsigned i = base + row - 1;
        UiText t = UI_TEXT_RETURN;
        switch (u->page)
        {
        case UI_PAGE_MAIN:
        {
            const UiText main[] = {UI_TEXT_EXECUTE, UI_TEXT_RECORD, UI_TEXT_TF, UI_TEXT_DEVICE};
            t = main[i];
            break;
        }
        case UI_PAGE_ACTIONS:
            if (i == 4 + s->count) { t = UI_TEXT_RETURN; break; }
            if (i < 4) { t = (UiText)(UI_TEXT_HOME + i); break; }
            /* fall through */
        case UI_PAGE_FILES:
        {
            if (u->page == UI_PAGE_FILES && i == s->count) { t = UI_TEXT_RETURN; break; }
            unsigned index = u->page == UI_PAGE_FILES ? i : i - 4;
            char stem[STORAGE_NAME_BYTES];
            strcpy(stem, s->names[index]);
            char *dot = strrchr(stem, '.');
            if (dot) *dot = 0;
            Ui_Row(m, row, stem);
            if (row == m->selected && strlen(stem) > 16)
            {
                m->scroll_selected = true;
                uint32_t elapsed = u->device.now_ms - u->selected_since_ms;
                unsigned cycle = (unsigned)strlen(stem) - 16 + 6;
                unsigned phase = (elapsed / 300) % cycle;
                m->scroll_offset = phase < 3 ? 0 : (phase - 3 > strlen(stem) - 16 ?
                                     (unsigned)strlen(stem) - 16 : phase - 3);
            }
            continue;
        }
        case UI_PAGE_FACTORY: t = i ? UI_TEXT_RETURN : UI_TEXT_START; break;
        case UI_PAGE_CUSTOM: t = i == 0 ? UI_TEXT_START : (i == 1 ? UI_TEXT_DELETE : UI_TEXT_RETURN); break;
        case UI_PAGE_DELETE_CONFIRM: t = i ? UI_TEXT_NO : UI_TEXT_YES; break;
        case UI_PAGE_RECORD: t = i ? UI_TEXT_RETURN : UI_TEXT_START; break;
        case UI_PAGE_START_CONFIRM: t = i ? UI_TEXT_RETURN : UI_TEXT_START; break;
        case UI_PAGE_TF:
        {
            const UiText tf[] = {UI_TEXT_FILES, UI_TEXT_TF_STATUS, UI_TEXT_REFRESH, UI_TEXT_RETURN};
            t = tf[i];
            break;
        }
        default: break;
        }
        Ui_Row(m, row, UiStrings_Get(t));
        /* Visible disabled marker; pressing still checks readiness before any command is submitted. */
        bool motion = (u->page == UI_PAGE_ACTIONS && i == 0) ||
                      ((u->page == UI_PAGE_CUSTOM || u->page == UI_PAGE_FACTORY || u->page == UI_PAGE_RECORD || u->page == UI_PAGE_START_CONFIRM) && i == 0);
        if (motion && (startup != STARTUP_READY || robot != ROBOT_READY))
            (void)Text_Format(m->rows[row], 64, "%s [X]", UiStrings_Get(t));
    }
    if (u->page == UI_PAGE_START_CONFIRM)
        Ui_Row(m, 3, u->start_command == CMD_GO_HOME ? UiStrings_Get(UI_TEXT_HOME) : u->custom_name);

}
