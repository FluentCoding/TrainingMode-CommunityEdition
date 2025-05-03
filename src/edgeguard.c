#include "../MexTK/mex.h"
#include "events.h"

#define ANGLE_MIN 45.f
#define ANGLE_MAX 60.f
#define MAG_MIN 106.f
#define MAG_MAX 120.f

#define DEGREES_TO_RADIANS 0.01745329252

static Savestate *savestate;

// 1 for right, -1 for left
static int side;

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

static void UpdatePosition(GOBJ *fighter) {
    FighterData *data = fighter->userdata;
    
    Vec3 pos = data->phys.pos;
    data->coll_data.topN_Curr = pos;
    data->coll_data.topN_CurrCorrect = pos;
    data->coll_data.topN_Prev = pos;
    data->coll_data.topN_Proj = pos;
    data->coll_data.coll_test = R13_INT(COLL_TEST);
}

static void UpdateCameraBox(GOBJ *fighter) {
    Fighter_UpdateCameraBox(fighter);
    
    FighterData *data = fighter->userdata;
    CmSubject *subject = data->camera_subject;
    subject->boundleft_curr = subject->boundleft_proj;
    subject->boundright_curr = subject->boundright_proj;
    
    Match_CorrectCamera();
}

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
    
    side = HSD_Randi(2) * 2 - 1;
    
    hmn_data->facing_direction = -side; 
    cpu_data->facing_direction = -side;
    
    // TEMP
    float ledge_x = 45 * side;
    
    // set pos
    hmn_data->phys.pos.X = ledge_x;
    cpu_data->phys.pos.X = ledge_x;
    UpdatePosition(hmn);
    UpdatePosition(cpu);
    
    // set hmn action state
    Fighter_EnterAerial(hmn, ASID_ATTACKAIRB);
    Fighter_ApplyAnimation(hmn, 7, 1, 0);
    hmn_data->state.frame = 7;
    hmn_data->script.script_event_timer = 0;
    Fighter_SubactionFastForward(hmn);
    Fighter_UpdateStateFrameInfo(hmn);
    Fighter_HitboxDisableAll(hmn);
    hmn_data->script.script_current = 0;
    
    // set cpu action state
    ActionStateChange(0, 1, 0, cpu, ASID_DAMAGEFLYN, 0x40, 0);
    Fighter_UpdateStateFrameInfo(cpu);
    cpu_data->jump.jumps_used = cpu_data->attr.max_jumps;
    
    // fix camera
    UpdateCameraBox(hmn);
    UpdateCameraBox(cpu);
    
    // give cpu knockback
    float angle_deg = ANGLE_MIN + (ANGLE_MAX - ANGLE_MIN) * HSD_Randf();
    float angle_rad = angle_deg * DEGREES_TO_RADIANS;
    float mag = MAG_MIN + (MAG_MAX - MAG_MIN) * HSD_Randf();
    float mag_x = cos(angle_rad) * mag * (float)side;
    float mag_y = sin(angle_rad) * mag;
    cpu_data->phys.kb_vel.X = mag_x;
    cpu_data->phys.kb_vel.Y = mag_y;
    
    ftCommon->x154 * mag
    
    
    // give hitlag
    hmn_data->hitlag_frames = 7;
    cpu_data->hitlag_frames = 7;
    
    hmn_data->flags.hitlag = 1;
    hmn_data->flags.hitlag_unk = 1;
    cpu_data->flags.hitlag = 1;
    cpu_data->flags.hitlag_unk = 1;
}

void Event_Init(GOBJ *gobj) {
    savestate = HSD_MemAlloc(sizeof(*savestate));
    (stc_event_vars.Savestate_Save)(savestate);
    Reset();
}

void Event_Think(GOBJ *menu) {
}
