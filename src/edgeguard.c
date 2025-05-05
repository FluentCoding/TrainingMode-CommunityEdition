#include "../MexTK/mex.h"
#include "events.h"

#define ANGLE_MIN 45.f
#define ANGLE_MAX 60.f
#define MAG_MIN 106.f
#define MAG_MAX 120.f
#define DMG_MIN 50
#define DMG_MAX 100

#define RESET_DELAY 30

#define FIREFOX_CHANCE 8
#define FIREFOX_DISTANCE 85
#define DISTANCE_FROM_LEDGE 15

#define DEGREES_TO_RADIANS 0.01745329252

static int reset_timer = -1;
static Vec2 ledge_positions[2];

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

static void GetLedgePositions(Vec2 coords_out[2]) {
    static char ledge_ids[34][2] = {
        { 0xFF, 0xFF }, { 0xFF, 0xFF }, { 0x03, 0x07 }, { 0x33, 0x36 },
        { 0x03, 0x0D }, { 0x29, 0x45 }, { 0x05, 0x11 }, { 0x09, 0x1A },
        { 0x02, 0x06 }, { 0x15, 0x17 }, { 0x00, 0x00 }, { 0x43, 0x4C },
        { 0x00, 0x00 }, { 0x00, 0x00 }, { 0x0E, 0x0D }, { 0x00, 0x00 },
        { 0x00, 0x05 }, { 0x1E, 0x2E }, { 0x0C, 0x0E }, { 0x02, 0x04 },
        { 0x03, 0x05 }, { 0x00, 0x00 }, { 0x06, 0x12 }, { 0x00, 0x00 },
        { 0xD7, 0xE2 }, { 0x00, 0x00 }, { 0x00, 0x00 }, { 0x00, 0x00 },
        { 0x03, 0x05 }, { 0x03, 0x0B }, { 0x06, 0x10 }, { 0x00, 0x05 },
        { 0x00, 0x02 }, { 0x01, 0x01 },
    };

    int stage_id = Stage_GetExternalID();
    char left_id = ledge_ids[stage_id][0];
    char right_id = ledge_ids[stage_id][1];

    Vec3 pos;
    Stage_GetLeftOfLineCoordinates(left_id, &pos);
    coords_out[0] = (Vec2) { pos.X, pos.Y };
    Stage_GetRightOfLineCoordinates(right_id, &pos);
    coords_out[1] = (Vec2) { pos.X, pos.Y };
}

static void UpdateCameraBox(GOBJ *fighter) {
    Fighter_UpdateCameraBox(fighter);

    FighterData *data = fighter->userdata;
    CmSubject *subject = data->camera_subject;
    subject->boundleft_curr = subject->boundleft_proj;
    subject->boundright_curr = subject->boundright_proj;

    Match_CorrectCamera();
}

static float Vec2_Distance(Vec2 *a, Vec2 *b) {
    float dx = a->X - b->X;
    float dy = a->Y - b->Y;
    return sqrtf(dx*dx + dy*dy);
}

static float Vec2_Length(Vec2 *a) {
    float x = a->X;
    float y = a->Y;
    return sqrtf(x*x + y*y);
}

static int in_hitstun_anim(int state) {
    return ASID_DAMAGEHI1 <= state && state <= ASID_DAMAGEFLYROLL;
}

static int hitstun_ended(GOBJ *fighter) {
    FighterData *data = fighter->userdata;
    float hitstun = *((float*)&data->state_var.state_var1);
    return hitstun == 0.0;
}

static bool air_actionable(GOBJ *fighter) {
    FighterData *data = fighter->userdata;

    // ensure airborne
    if (data->phys.air_state == 0)
        return false;

    int state = data->state_id;

    if (in_hitstun_anim(state) && hitstun_ended(fighter))
        return true;

    return (ASID_JUMPF <= state && state <= ASID_FALLAERIALB)
        || state == ASID_DAMAGEFALL;
}

void Exit(int value) {
    Match *match = MATCH;
    match->state = 3;
    Match_EndVS();
}

void Reset(void) {
    for (int ply = 0; ply < 2; ++ply) {
        MatchHUDElement *hud = &stc_matchhud->element_data[ply];
        if (hud->is_removed == 1)
            Match_CreateHUD(ply);
    }

    GOBJ *hmn = Fighter_GetGObj(0);
    GOBJ *cpu = Fighter_GetGObj(1);
    FighterData *hmn_data = hmn->userdata;
    FighterData *cpu_data = cpu->userdata;

    cpu_data->cpu.ai = 15;

    int side_idx = HSD_Randi(2);
    int side = side_idx * 2 - 1;

    hmn_data->facing_direction = -side; 
    cpu_data->facing_direction = -side;

    float ledge_x = ledge_positions[side_idx].X - DISTANCE_FROM_LEDGE * side;

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

    cpu_data->dmg.percent = dmg;
    Fighter_SetHUDDamage(cpu_data->ply, dmg);

    hmn_data->dmg.percent = 0;
    Fighter_SetHUDDamage(hmn_data->ply, 0);
}

void Event_Init(GOBJ *gobj) {
    GetLedgePositions(&ledge_positions);
    Reset();
}

void Event_Think(GOBJ *menu) {
    GOBJ *hmn = Fighter_GetGObj(0);
    GOBJ *cpu = Fighter_GetGObj(1);
    FighterData *hmn_data = hmn->userdata;
    FighterData *cpu_data = cpu->userdata;

    if (reset_timer > 0) 
        reset_timer--;

    if (reset_timer == 0) {
        reset_timer = -1;
        Reset();
    }
    
    int cpu_state = cpu_data->state_id;
    int dir = cpu_data->phys.pos.X > 0.f ? -1 : 1;
    Vec2 *target_ledge = &ledge_positions[cpu_data->phys.pos.X > 0.f];
    
    // ensure the player L-cancels the initial bair.
    hmn_data->input.timer_trigger_any_ignore_hitlag = 0;
    
    if (
        reset_timer == -1
        && (
            cpu_data->flags.dead || hmn_data->flags.dead
            || cpu_data->phys.air_state == 0
            || cpu_state == ASID_CLIFFCATCH
        )
    ) {
        reset_timer = RESET_DELAY;
    }

    if (hmn_data->input.down & PAD_BUTTON_DPAD_LEFT)
        reset_timer = 0;

    if (cpu_data->flags.hitstun) {
        // DI inwards
        cpu_data->cpu.lstickX = 90 * dir;
        cpu_data->cpu.lstickY = 90;
    } else if (air_actionable(cpu)) {
        float distance_to_ledge = Vec2_Distance(&cpu_data->phys.pos, target_ledge);
        if (
            // force upb if at end of range
            FIREFOX_DISTANCE - 5.f < distance_to_ledge

            // otherwise, random chance to upb
            || (distance_to_ledge < FIREFOX_DISTANCE && HSD_Randi(FIREFOX_CHANCE) == 0)
        ) {
            cpu_data->cpu.lstickY = 127;
            cpu_data->cpu.held |= PAD_BUTTON_B;
        } else {
            // drift towards stage if out of range
            cpu_data->cpu.lstickX = 127 * dir;
        }
    } else if (0x161 <= cpu_state && cpu_state <= 0x167) {
        // compute firefox angle
        Vec3 vec_to_ledge = {
            .X = target_ledge->X - cpu_data->phys.pos.X,
            .Y = target_ledge->Y - cpu_data->phys.pos.Y,
        };
        Vec3_Normalize(&vec_to_ledge);

        cpu_data->cpu.lstickX = (s8)(vec_to_ledge.X * 127.f);
        cpu_data->cpu.lstickY = (s8)(vec_to_ledge.Y * 127.f);
    }  
}
