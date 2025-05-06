#include "../MexTK/mex.h"
#include "events.h"

#define DISTANCE_FROM_LEDGE 15
#define RESET_DELAY 30

// 1 in n chance every frame
#define FIREFOX_CHANCE 8 
#define DOUBLEJUMP_CHANCE_BELOW_LEDGE 5
#define DOUBLEJUMP_CHANCE_ABOVE_LEDGE 30
#define ILLUSION_CHANCE 10
#define FASTFALL_CHANCE 8

#define MAX_ILLUSION_HEIGHT 30.f
#define MIN_ILLUSION_HEIGHT -15.f
#define ILLUSION_DISTANCE 75
#define FIREFOX_DISTANCE 80
#define DEGREES_TO_RADIANS 0.01745329252

static int reset_timer = -1;
static Vec2 ledge_positions[2];

void Exit(int value);

static const char *OffOn[2] = {"Off", "On"};

enum options {
    OPT_HITSTRENGTH,
    OPT_FF_LOW,
    OPT_FF_MID,
    OPT_FF_HIGH,
    OPT_JUMP,
    OPT_ILLUSION,
    OPT_FASTFALL,
    OPT_EXIT,
};

typedef struct KBValues {
    float mag_min, mag_max;
    float ang_min, ang_max;
    float dmg_min, dmg_max;
} KBValues;

static const char *Values_HitStrength[] = {"Weak", "Normal", "Hard"};
static KBValues HitStrength_KBRange[] = {
    {
         80.f, 100.f, // mag
         45.f,  60.f, // ang
         10.f,  40.f, // dmg
    },
    {
        106.f, 120.f, // mag
         45.f,  60.f, // ang
         50.f,  80.f, // dmg
    },
    {
        140.f, 200.f, // mag
         65.f,  70.f, // ang
         80.f, 110.f, // dmg
    },
};

static EventOption Options_Main[] = {
    {
        .option_kind = OPTKIND_STRING,
        .option_name = "Hit Strength",
        .desc = "How far Fox will be knocked back.",
        .option_values = Values_HitStrength,
        .value_num = countof(Values_HitStrength),
        .option_val = 1
    },
    {
        .option_kind = OPTKIND_STRING,
        .option_name = "Firefox Low",
        .desc = "Allow Fox to aim his up special to the ledge.",
        .option_values = OffOn,
        .value_num = 2,
        .option_val = 1,
    },
    {
        .option_kind = OPTKIND_STRING,
        .option_name = "Firefox Mid",
        .desc = "Allow Fox to aim his up special to the stage.",
        .option_values = OffOn,
        .value_num = 2,
    },
    {
        .option_kind = OPTKIND_STRING,
        .option_name = "Firefox High",
        .desc = "Allow Fox to aim his up special high.",
        .option_values = OffOn,
        .value_num = 2,
    },
    {
        .option_kind = OPTKIND_STRING,
        .option_name = "Double Jump",
        .desc = "Allow Fox to double jump.",
        .option_values = OffOn,
        .value_num = 2,
    },
    {
        .option_kind = OPTKIND_STRING,
        .option_name = "Illusion",
        .desc = "Allow Fox to side special.",
        .option_values = OffOn,
        .value_num = 2,
    },
    {
        .option_kind = OPTKIND_STRING,
        .option_name = "Fast Fall",
        .desc = "Allow Fox to fast fall.",
        .option_values = OffOn,
        .value_num = 2,
    },
    {
        .option_kind = OPTKIND_FUNC,
        .option_name = "Exit",
        .desc = "Return to the Event Select Screen.",
        .onOptionSelect = Exit,
    },
};
static EventMenu Menu_Main = {
    .name = "Fox Edgeguard",
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

static bool enabled(int opt_idx) {
    return Options_Main[opt_idx].option_val;
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
    
    cpu_data->jump.jumps_used = 1;
    hmn_data->jump.jumps_used = 1;

    // set hmn action state
    Fighter_EnterAerial(hmn, ASID_ATTACKAIRB);
    Fighter_ApplyAnimation(hmn, 7, 1, 0);
    hmn_data->state.frame = 7;
    hmn_data->script.script_event_timer = 0;
    Fighter_SubactionFastForward(hmn);
    Fighter_UpdateStateFrameInfo(hmn);
    Fighter_HitboxDisableAll(hmn);
    hmn_data->script.script_current = 0;

    KBValues vals = HitStrength_KBRange[Options_Main[OPT_HITSTRENGTH].option_val];
    
    float mag = vals.mag_min + (vals.mag_max - vals.mag_min) * HSD_Randf();
    
    int state = mag > 130.f ? ASID_DAMAGEFLYHI : ASID_DAMAGEFLYN;

    // set cpu action state
    Fighter_EnterFall(cpu);
    ActionStateChange(0, 1, 0, cpu, state, 0x40, 0);
    Fighter_UpdateStateFrameInfo(cpu);

    // fix camera
    UpdateCameraBox(hmn);
    UpdateCameraBox(cpu);
    
    // give cpu knockback
    float angle_deg = vals.ang_min + (vals.ang_max - vals.ang_min) * HSD_Randf();
    float angle_rad = angle_deg * DEGREES_TO_RADIANS;

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
    int dmg = vals.dmg_min + HSD_Randi(vals.dmg_max - vals.dmg_min);

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
    
    Vec2 target_ledge = ledge_positions[cpu_data->phys.pos.X > 0.f];
    
    // ledge sweetspot is slightly further away and down if above the stage
    if (cpu_data->phys.pos.Y > 20.f) {
        target_ledge.X -= 16 * dir;
        target_ledge.Y -= 4;
    }
    
    // ensure the player L-cancels the initial bair.
    hmn_data->input.timer_trigger_any_ignore_hitlag = 0;
    
    if (
        reset_timer == -1
        && (
            cpu_data->flags.dead
            || hmn_data->flags.dead
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
        float distance_to_ledge = Vec2_Distance(&cpu_data->phys.pos, &target_ledge);
        
        int dj_chance = cpu_data->phys.pos.Y > target_ledge.Y ?
            DOUBLEJUMP_CHANCE_ABOVE_LEDGE : DOUBLEJUMP_CHANCE_BELOW_LEDGE;
        // JUMP
        if (
            enabled(OPT_JUMP)
            && cpu_data->jump.jumps_used < 2
            && (
                // force jump if at end of range
                distance_to_ledge > FIREFOX_DISTANCE
                
                // otherwise, random chance to jump
                || HSD_Randi(dj_chance) == 0
            )
        ) {
            cpu_data->cpu.held |= PAD_BUTTON_Y;
            cpu_data->cpu.lstickX = 127 * dir;
            
        // ILLUSION
        } else if (
            enabled(OPT_ILLUSION)
            && cpu_data->phys.pos.Y > MIN_ILLUSION_HEIGHT
            && cpu_data->phys.pos.Y < MAX_ILLUSION_HEIGHT
            && fabs(target_ledge.X - cpu_data->phys.pos.X) < ILLUSION_DISTANCE
            && HSD_Randi(ILLUSION_CHANCE) == 0
        ) {
            cpu_data->cpu.lstickX = 127 * dir;
            cpu_data->cpu.held |= PAD_BUTTON_B;
            
        // FIREFOX
        } else if (
            cpu_data->phys.self_vel.Y <= 1.5f
            && (
                enabled(OPT_FF_LOW) || enabled(OPT_FF_MID) || enabled(OPT_FF_HIGH)
            ) && (
                // force upb if at end of range
                (cpu_data->phys.pos.Y < 0.f && distance_to_ledge > FIREFOX_DISTANCE)
    
                // otherwise, random chance to upb
                || (distance_to_ledge < FIREFOX_DISTANCE && HSD_Randi(FIREFOX_CHANCE) == 0)
            )
        ) {
            cpu_data->cpu.lstickY = 127;
            cpu_data->cpu.held |= PAD_BUTTON_B;
            
        // FASTFALL
        } else if (
            enabled(OPT_FASTFALL)
            && !cpu_data->flags.is_fastfall
            && cpu_data->phys.self_vel.Y < 0.f
            && HSD_Randi(FASTFALL_CHANCE) == 0
        ) {
            cpu_data->cpu.lstickY = -127;
            
        // WAIT
        } else {
            // drift towards stage
            cpu_data->cpu.lstickX = 127 * dir;
        }
        
        // if in firefox starting states
    } else if (0x161 <= cpu_state && cpu_state <= 0x162) {
        // compute firefox angle
        
        int low = enabled(OPT_FF_LOW);
        int mid = enabled(OPT_FF_MID);
        int high = enabled(OPT_FF_HIGH);
        int option_count = low + mid + high;
        int choice = HSD_Randi(option_count);
        
        float x_to_ledge = fabs(target_ledge.X - cpu_data->phys.pos.X);
        float high_y = cpu_data->phys.pos.Y + sqrtf(
            FIREFOX_DISTANCE*FIREFOX_DISTANCE
            - x_to_ledge*x_to_ledge
        );
        
        Vec2 target = { .X = target_ledge.X };
        if (low && choice-- == 0) {
            target.Y = target_ledge.Y;
        } else if (mid && choice-- == 0) {
            target.Y = (target_ledge.Y + high_y) / 2.f;
        } else if (high && choice-- == 0) {
            target.Y = high_y;
        }
        
        Vec2 vec_to_target = {
            .X = target.X - cpu_data->phys.pos.X,
            .Y = target.Y - cpu_data->phys.pos.Y,
        };
        Vec2_Normalize(&vec_to_target);
        
        cpu_data->cpu.lstickX = (s8)(vec_to_target.X * 127.f);
        cpu_data->cpu.lstickY = (s8)(vec_to_target.Y * 127.f);
    
    } else if (
        // if in firefox ending states
        (0x162 <= cpu_state && cpu_state <= 0x167)
         
        // or special fall
        || (ASID_FALLSPECIAL <= cpu_state && cpu_state <= ASID_FALLSPECIALB)
    ) {
        // drift towards stage
        cpu_data->cpu.lstickX = 127 * dir;
    }
}
