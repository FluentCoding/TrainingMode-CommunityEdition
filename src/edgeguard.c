#include "../MexTK/mex.h"
#include "events.h"

#define ANGLE_MIN 45.f
#define ANGLE_MAX 60.f
#define MAG_MIN 106.f
#define MAG_MAX 120.f
#define DMG_MIN 50
#define DMG_MAX 100

#define RESET_QUICK 30
#define RESET_SLOW 80

#define FIREFOX_CHANCE 8
#define FIREFOX_DISTANCE 80

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

static float Vec3_Distance(Vec3 *a, Vec3 *b) {
    float dx = a->X - b->X;
    float dy = a->Y - b->Y;
    float dz = a->Z - b->Z;
    return sqrtf(dx*dx + dy*dy + dz*dz);
}

static float Vec3_Length(Vec3 *a) {
    float x = a->X;
    float y = a->Y;
    float z = a->Z;
    return sqrtf(x*x + y*y + z*z);
}

void Exit(int value) {
    Match *match = MATCH;
    match->state = 3;
    Match_EndVS();
}

void Reset(void) {
    GOBJ *hmn = Fighter_GetGObj(0);
    GOBJ *cpu = Fighter_GetGObj(1);
    FighterData *hmn_data = hmn->userdata;
    FighterData *cpu_data = cpu->userdata;
    
    //cpu_data->cpu.ai = 15;
    
    side = HSD_Randi(2) * 2 - 1;
    
    hmn_data->facing_direction = -side; 
    cpu_data->facing_direction = -side;
    
    // TEMP
    float ledge_x = 45 * side;
    
    // set phys
    cpu_data->phys.kb_vel.X = 0.f;
    cpu_data->phys.kb_vel.Y = 0.f;
    cpu_data->phys.self_vel.X = 0.f;
    cpu_data->phys.self_vel.Y = 0.f;
    hmn_data->phys.kb_vel.X = 0.f;
    hmn_data->phys.kb_vel.Y = 0.f;
    hmn_data->phys.self_vel.X = 0.f;
    hmn_data->phys.self_vel.Y = 0.f;
    
    hmn_data->phys.pos.X = ledge_x;
    hmn_data->phys.pos.Y = 0.f;
    cpu_data->phys.pos.X = ledge_x;
    cpu_data->phys.pos.Y = 0.f;
    
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
    Fighter_EnterFall(cpu);
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
    
    float vel = mag * (*stc_ftcommon)->force_applied_to_kb_mag_multiplier;
    float vel_x = cos(angle_rad) * vel * (float)side;
    float vel_y = sin(angle_rad) * vel;
    cpu_data->phys.kb_vel.X = vel_x;
    cpu_data->phys.kb_vel.Y = vel_y;
    
    float kb_frames = (float)(int)((*stc_ftcommon)->x154 * mag);
    *(float*)&cpu_data->state_var.state_var1 = kb_frames;
    cpu_data->flags.hitstun = 1;
    Fighter_EnableCollUpdate(cpu);
    
    // give hitlag
    hmn_data->dmg.hitlag_frames = 7;
    cpu_data->dmg.hitlag_frames = 7;
    
    hmn_data->flags.hitlag = 1;
    hmn_data->flags.hitlag_unk = 1;
    cpu_data->flags.hitlag = 1;
    cpu_data->flags.hitlag_unk = 1;
    
    // random percent
    int dmg = DMG_MIN + HSD_Randi(DMG_MAX - DMG_MIN);
    Fighter_SetHUDDamage(cpu_data->ply, dmg);
}

void Event_Init(GOBJ *gobj) {
    Reset();
    /*savestate = HSD_MemAlloc(sizeof(*savestate));
    (stc_event_vars.Savestate_Save)(savestate);*/
}

static int reset_timer = -1;

void Event_Think(GOBJ *menu) {
    if (reset_timer > 0) 
        reset_timer--;
        
    if (reset_timer == 0) {
        reset_timer = -1;
        Reset();
    }
    
    GOBJ *hmn = Fighter_GetGObj(0);
    GOBJ *cpu = Fighter_GetGObj(1);
    FighterData *hmn_data = hmn->userdata;
    FighterData *cpu_data = cpu->userdata;
    
    hmn_data->input.timer_trigger_any_ignore_hitlag = 0; // always l-cancel
    
    int cpu_state = cpu_data->state_id;
    
    if (
        reset_timer == -1
        && (
            cpu_data->flags.dead || hmn_data->flags.dead
            || hmn_data->input.held & PAD_BUTTON_DPAD_LEFT
            || cpu_data->phys.air_state == 0
            || cpu_state == ASID_CLIFFCATCH
        )
    ) {
        reset_timer = RESET_QUICK; 
    }
    
    if (cpu_data->flags.hitstun) {
        // pass until hitstun over
        
    } else if (cpu_state == ASID_DAMAGEFALL) {
        float distance_to_ledge = Vec3_Distance(&cpu_data->phys.pos, &cpu_data->cpu.nearest_ledge);
        OSReport("checking %f %f, %f\n", cpu_data->cpu.nearest_ledge.X, cpu_data->cpu.nearest_ledge.Y, distance_to_ledge);
        if (
            distance_to_ledge < FIREFOX_DISTANCE
            && HSD_Randi(FIREFOX_CHANCE) == 0
        ) {
            cpu_data->cpu.lstickY = 127;
            cpu_data->cpu.held |= PAD_BUTTON_B;
        }
        
    } else if (0x161 <= cpu_state && cpu_state <= 0x167) {
        // compute firefox angle
        Vec3 vec_to_ledge = {
            .X = cpu_data->cpu.nearest_ledge.X - cpu_data->phys.pos.X,
            .Y = cpu_data->cpu.nearest_ledge.Y - cpu_data->phys.pos.Y,
            .Z = cpu_data->cpu.nearest_ledge.Z - cpu_data->phys.pos.Z,
        };
        Vec3_Normalize(&vec_to_ledge);
        
        cpu_data->cpu.lstickX = (s8)(vec_to_ledge.X * 127.f);
        cpu_data->cpu.lstickY = (s8)(vec_to_ledge.Y * 127.f);
    }  
}
