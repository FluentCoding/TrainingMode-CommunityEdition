#include "../MexTK/mex.h"
#include "events.h"

static Savestat *savestate;

void Exit(int value);

static EventOption Options_Main[] = {
    {
        .option_kind = OPTKIND_FUNC,
        .option_name = "Exit",
        .desc = "Return to the Event Select Screen.",
        .onOptionSelect = Exit,
    },
};
static EventMenu Menu_Main = {
    .name = "Firefox Edgeguard",
    .option_num = sizeof(Options_Main) / sizeof(EventOption),
    .options = &Options_Main,
};
EventMenu *Event_Menu = &Menu_Main;

void Exit(int value) {
    Match *match = MATCH;
    match->state = 3;
    Match_EndVS();
}

void Reset(void) {
    GOBJ *hmn = Fighter_GetGObj(0);
    FighterData *hmn_data = hmn->userdata;
    GOBJ *cpu = Fighter_GetGObj(1);
    FighterData *cpu_data = cpu->userdata;
}

void Event_Init(GOBJ *gobj) {
    savestate = HSD_MemAlloc(sizeof(*savestate));
    (*stc_event_vars)->Savestate_Save(savestate);
    Reset();
}

void Event_Think(GOBJ *menu) {
}
