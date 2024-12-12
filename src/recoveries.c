#ifndef _RECOVERIES_C
#define _RECOVERIES_C

#include "../MexTK/mex.h"

// PUBLIC ######################################################

typedef enum Recover_Ret {
    RECOVER_IN_PROGRESS,
    RECOVER_UNIMPLEMENTED,
    RECOVER_FINISHED,
} Recover_Ret;

Recover_Ret Recover_Think(GOBJ *cpu, GOBJ *hmn);

// PRIVATE ######################################################

static Vec2 Vec2_add(Vec2 a, Vec2 b) { return (Vec2){a.X+b.X, a.Y+b.Y}; }
static Vec2 Vec2_sub(Vec2 a, Vec2 b) { return (Vec2){a.X-b.X, a.Y-b.Y}; }
static float Vec2_Length(Vec2 v) { return sqrtf(v.X*v.X + v.Y*v.Y); }

static GOBJ *recover_debug_gobjs[32] = { 0 };

void Recover_DebugPointThink(GOBJ *gobj) {
    int lifetime = (int)gobj->userdata;
    if (lifetime == 0) {
        GObj_Destroy(gobj);
        return;
    }

    gobj->userdata = lifetime - 1;
}

GOBJ *Recover_DebugPointSet(int id, Vec2 point, int lifetime) {
    if (id >= 32) assert("Recover_DebugPointSet: id too large");

    GOBJ *prev_gobj = recover_debug_gobjs[id];
    if (prev_gobj != 0) GObj_Destroy(prev_gobj);

    GOBJ *gobj = GObj_Create(0, 0, 0);
    recover_debug_gobjs[id] = gobj;

    gobj->userdata = lifetime;

    JOBJ *jobj = JOBJ_LoadJoint(event_vars->menu_assets->arrow);
    jobj->trans = (Vec3){ point.X, point.Y, 0.0 };
    jobj->scale = (Vec3){ 4.0, 4.0, 1.0 };
    GObj_AddObject(gobj, 3, jobj);
    JOBJ *jobj2 = JOBJ_LoadJoint(event_vars->menu_assets->arrow);
    jobj2->trans = (Vec3){ 0.0, 0.0, 0.0 };
    jobj2->scale = (Vec3){ -1.0, 1.0, 1.0 };
    JOBJ_AddChild(jobj, jobj2);
    GObj_AddGXLink(gobj, GXLink_Common, 0, 0);
    //GObj_AddProc(gobj, Recover_DebugPointThink, 4);

    return gobj;
}

// extra computed data, not contained in FighterData, needed for recovery decision making
typedef struct Recover_Data {
    Vec2 ledge;
    int direction; // zero if between ledges
    int jumps;
} Recover_Data;

// decompiled from GetLedgeCoords in Custom Even Code
static void Recover_LedgeCoords(Vec2 coords_out[2]) {
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

    // we add/subtract a bit from the ledge, because we can snap from quite a bit further than the actual ledge
    Vec3 pos;
    Stage_GetLeftOfLineCoordinates(left_id, &pos);
    coords_out[0] = (Vec2) { pos.X, pos.Y };
    Stage_GetRightOfLineCoordinates(right_id, &pos);
    coords_out[1] = (Vec2) { pos.X, pos.Y };
}

bool Recover_IsAirActionable(GOBJ *cpu) {
    FighterData *data = cpu->userdata;
    int state = data->state_id;

    if (ASID_DAMAGEHI1 <= state && state <= ASID_DAMAGEFLYROLL) {
        float hitstun = *((float*)&data->state_var.state_var1);
        if (hitstun != 0.0) return false;
    }

    if (ASID_JUMPF <= state && state <= ASID_FALLAERIALB) return true;

    // TODO iasa

    return false;
}

bool Recover_CanDrift(GOBJ *cpu) {
    FighterData *data = cpu->userdata;
    int state = data->state_id;
    int frame = (int)(data->state.frame / data->state.rate);

    if (ASID_DAMAGEHI1 <= state && state <= ASID_DAMAGEFLYROLL) {
        float hitstun = *((float*)&data->state_var.state_var1);
        return hitstun == 0.0;
    }

    if (state == ASID_ESCAPEAIR)
        return frame >= 35;

    switch (data->kind) {
    case FTKIND_FOX:
        if (341 <= state && state <= 346) return true; // laser
        if (357 <= state && state <= 358) return true; // firefox end
        return false;
    }

    return false;
}

// main tree:
//  side b
//   to ledge
//   just above ledge
//    shorten to ledge
//    mid shorten
//    full
//   just above plat
//    shorten to ledge
//    edge cancel
//    full
//  upb
//   to ledge
//   straight
//   to side plat
//   to top plat
//   to ground
//   very high
//    full drift to stage
//    start drift to stage then to ledge
//    drift to side platform land
//    drift to side platform fall through
//    drift in then to ledge
//
// pre:
//  double jump
//  shine stall
//  fall
//  fast fall

typedef struct Platform {
    float x_1;
    float x_2;
    float y;
} Platform;

// returns true if has a side platform
int Recover_PlatformSide(Platform *plat, float direction) {
    switch (Stage_GetExternalID()) {
    case GRKINDEXT_PSTAD:
        *plat = (Platform) {
            .x_1 = -55.0 * direction,
            .x_2 = -25.0 * direction,
            .y = 25.0
        };
        return true;
    case GRKINDEXT_BATTLE:
        *plat = (Platform) {
            .x_1 = -57.6 * direction,
            .x_2 = -20.0 * direction,
            .y = 27.2
        };
        return true;
    case GRKINDEXT_STORY:
        *plat = (Platform) {
            .x_1 = -59.5 * direction,
            .x_2 = -28.0 * direction,
            .y = 23.45
        };
        return true;
    case GRKINDEXT_OLDPU: // dreamland
        *plat = (Platform) {
            .x_1 = -61.393 * direction,
            .x_2 = -31.725 * direction,
            .y = 30.142
        };
        return true;
    case GRKINDEXT_IZUMI:
        // TODO: FOD
        return false;
    }

    return false;
}

int Recover_PlatformTop(Platform *plat) {
    switch (Stage_GetExternalID()) {
    case GRKINDEXT_BATTLE:
        *plat = (Platform) {
            .x_1 = -18.8,
            .x_2 = 18.8,
            .y = 54.5
        };
        return true;
    case GRKINDEXT_STORY:
        *plat = (Platform) {
            .x_1 = -15.75,
            .x_2 = 15.75,
            .y = 42.0
        };
        return true;
    case GRKINDEXT_OLDPU: // dreamland
        *plat = (Platform) {
            .x_1 = -19.018,
            .x_2 = 19.018,
            .y = 51.425,
        };
        return true;
    case GRKINDEXT_IZUMI:
        *plat = (Platform) {
            .x_1 = -14.25,
            .x_2 = 14.25,
            .y = 42.75
        };
        return true;
    }

    return false;
}

int InFirefoxDeadzone(s8 x, s8 y) {
    return (-23 < x && x < 23) || (-23 < y && y < 23);
}

void Recover_ThinkFox(const Recover_Data * const rec_data, GOBJ *cpu, GOBJ *hmn) {
    FighterData *cpu_data = cpu->userdata;
    FighterData *hmn_data = hmn->userdata;

    #define FOX_UPB_DISTANCE 88.0
    #define FOX_SIDEB_DISTANCE_1_4 27.0
    #define FOX_SIDEB_DISTANCE_2_4 46.0
    #define FOX_SIDEB_DISTANCE_3_4 65.0
    #define FOX_SIDEB_DISTANCE_4_4 83.0

    Vec2 pos = { cpu_data->phys.pos.X, cpu_data->phys.pos.Y };
    int state = cpu_data->state_id;
    int frame = (int)(cpu_data->state.frame / cpu_data->state.rate);

    Vec2 ledge_grab_offset = { -10.0 * rec_data->direction, -7.0 };
    Vec2 ledge_grab_point = Vec2_add(rec_data->ledge, ledge_grab_offset);
    //Recover_DebugPointSet(0, ledge_grab_point, 1);
    Vec2 vec_to_ledge_grab = Vec2_sub(ledge_grab_point, pos);
    float dist_to_ledge = Vec2_Length(vec_to_ledge_grab);

    Vec2 vec_to_stage = {
        .X = rec_data->ledge.X + 5.0 * rec_data->direction - pos.X,
        .Y = 1.0 - pos.Y,
    };
    float dist_to_stage = Vec2_Length(vec_to_stage);

    bool double_jump_rising = (state == ASID_JUMPAERIALF || state == ASID_JUMPAERIALB) && cpu_data->phys.self_vel.Y > 0.0;

    int in_fastfall = cpu_data->flags.is_fastfall;
    float drift_speed = cpu_data->attr.aerial_drift_max;
    float fall_speed = in_fastfall ? cpu_data->attr.fastfall_velocity : cpu_data->attr.terminal_velocity;
    float fall_slope = fall_speed / drift_speed;

    float ledge_slope = fabs(vec_to_ledge_grab.Y) / fabs(vec_to_ledge_grab.X);
    float stage_slope = fabs(vec_to_stage.Y) / fabs(vec_to_stage.X);
    int can_fall_to_ledge = ledge_slope > fall_slope;
    int can_fall_to_stage = stage_slope > fall_slope;

    Platform side_plat;
    if (!Recover_PlatformSide(&side_plat, rec_data->direction)) goto PICK_FIREFOX_ANGLE;
    side_plat.y += 2.0; // make sure we get on top of the platform

    // Because the cpu is often set to double jump out of hitstun, 
    // we hardcode it to not upb immediately because we want the counter-action to "complete".
    if (Recover_IsAirActionable(cpu) && !double_jump_rising && rec_data->direction != 0) {
        int n = HSD_Randi(100);

        if (rec_data->jumps > 0) {
            n -= 1;
            if (n < 0 || vec_to_ledge_grab.Y > 0.0) {
                cpu_data->cpu.held |= HSD_BUTTON_Y;
                cpu_data->cpu.lstickX = 127 * rec_data->direction;
                return;
            }
        }

        if (dist_to_ledge < FOX_UPB_DISTANCE) {
            n -= 3;
            if (n < 0 || (vec_to_ledge_grab.Y > 0.0 && dist_to_ledge > 70.0)) {
                // upb
                cpu_data->cpu.held |= HSD_BUTTON_B;
                cpu_data->cpu.lstickY = 127;
                return;
            }
        }

        if (fabs(vec_to_ledge_grab.X) < FOX_SIDEB_DISTANCE_4_4 && vec_to_ledge_grab.Y <= 0.0) {
            if (pos.Y > side_plat.y + 5.0)
                n -= 1;
            else
                n -= 6;
            if (n < 0) {
                // side b
                cpu_data->cpu.held |= HSD_BUTTON_B;
                cpu_data->cpu.lstickX = 127 * rec_data->direction;
                return;
            }
        }

        if (!in_fastfall) {
            n -= 2;
            if (n < 0) {
                cpu_data->cpu.lstickY = -127;
                return;
            }
        }

        cpu_data->cpu.lstickX = 127 * rec_data->direction;
        return;
    } else if (Recover_CanDrift(cpu)) { // special fall / in aerial / past ledge
        if (vec_to_ledge_grab.Y > 0.0) {
            cpu_data->cpu.lstickY = -127;
            return;
        }

        // if ambiguous if cpu can drift to ledge. then hold in regardless
        if (!can_fall_to_stage && vec_to_ledge_grab.X * rec_data->direction < 0.0) {
            cpu_data->cpu.lstickX = 127 * rec_data->direction;
        // if can drift to ledge
        } else if (can_fall_to_ledge && vec_to_ledge_grab.X * rec_data->direction < 0.0) {
            if (!in_fastfall && pos.Y > 0.0 && HSD_Randi(15) == 0) {
                cpu_data->cpu.lstickY = -127; // fastfall
            } else {
                cpu_data->cpu.lstickX = -127 * rec_data->direction;
            }
        } else {
            if (!in_fastfall && pos.Y > 0.0) {
                cpu_data->cpu.lstickY = -127;
            } else {
                // hold away from hmn
                float direction_to_hmn = pos.X < hmn_data->phys.pos.X ? 1.0 : -1.0;
                cpu_data->cpu.lstickX = 127 * direction_to_hmn;
            }
        }
    } else if (state == 354 && frame >= 41) {
        PICK_FIREFOX_ANGLE:
        switch (HSD_Randi(5)) {
        case 0: { // to ledge
            cpu_data->cpu.lstickX = (s8)(vec_to_ledge_grab.X * 127.0 / dist_to_ledge);
            cpu_data->cpu.lstickY = (s8)(vec_to_ledge_grab.Y * 127.0 / dist_to_ledge);

            break;
        }
        case 1: { // straight
            if (pos.Y < 5.0) goto PICK_FIREFOX_ANGLE;

            cpu_data->cpu.lstickX = 127 * rec_data->direction;
            break;
        }
        case 2: { // up
            // aim to y point above ledge

            float x = vec_to_ledge_grab.X;
            float y = sqrtf(FOX_UPB_DISTANCE*FOX_UPB_DISTANCE - x*x);

            float mul = 127.0 / FOX_UPB_DISTANCE;

            cpu_data->cpu.lstickX = (s8)(x * mul) * rec_data->direction;
            cpu_data->cpu.lstickY = (s8)(y * mul) * rec_data->direction;

            break;
        }
        case 3: { // to plat
            Vec2 to_point_1 = { .X = side_plat.x_1 - pos.X, .Y = side_plat.y - pos.Y };
            Vec2 to_point_2 = { .X = side_plat.x_2 - pos.X, .Y = side_plat.y - pos.Y };
            float to_point_1_length = Vec2_Length(to_point_1);
            float to_point_2_length = Vec2_Length(to_point_2);

            float y = to_point_1.Y;
            float x = sqrtf(FOX_UPB_DISTANCE*FOX_UPB_DISTANCE - y*y);
            float mul = 127.0 / FOX_UPB_DISTANCE;

            cpu_data->cpu.lstickX = (s8)(x * mul) * rec_data->direction;
            cpu_data->cpu.lstickY = (s8)(y * mul) * rec_data->direction;
            break;
        }
        case 4: { // just above stage
            if (FOX_UPB_DISTANCE < dist_to_stage) goto PICK_FIREFOX_ANGLE;

            float y = vec_to_stage.Y;
            float x = sqrtf(FOX_UPB_DISTANCE*FOX_UPB_DISTANCE - y*y);
            float mul = 127.0 / FOX_UPB_DISTANCE;

            cpu_data->cpu.lstickX = (s8)(x * mul) * rec_data->direction;
            cpu_data->cpu.lstickY = (s8)(y * mul) * rec_data->direction;

            if (InFirefoxDeadzone(cpu_data->cpu.lstickX, cpu_data->cpu.lstickY)) 
                goto PICK_FIREFOX_ANGLE;

            break;
        }
        }
    } else {
        // survival DI
        cpu_data->cpu.lstickY = 90;
        cpu_data->cpu.lstickX = 90 * rec_data->direction;
    }
}

Recover_Ret Recover_Think(GOBJ *cpu, GOBJ *hmn) {
    FighterData *cpu_data = cpu->userdata;
    FighterData *hmn_data = hmn->userdata;

    Recover_Data data;
    Vec2 ledges[2];
    Recover_LedgeCoords(ledges);

    float xpos = cpu_data->phys.pos.X;
    data.ledge = ledges[xpos > 0.0];
    if (xpos < ledges[0].X)
        data.direction = 1;
    else if (xpos <= ledges[1].X)
        data.direction = 0;
    else
        data.direction = -1;
    data.jumps = cpu_data->attr.max_jumps - cpu_data->jump.jumps_used;

    // Normal getup from ledge
    if (cpu_data->state_id == ASID_CLIFFWAIT && cpu_data->state.frame == 1) {
        cpu_data->cpu.lstickX = 127 * data.direction;
        return RECOVER_IN_PROGRESS;
    } else if (ASID_CLIFFCATCH <= cpu_data->state_id && cpu_data->state_id <= ASID_CLIFFJUMPQUICK2) {
        return RECOVER_IN_PROGRESS;
    }

    // Respawn platform
    if (cpu_data->state_id == ASID_REBIRTHWAIT) {
        if (cpu_data->TM.state_frame & 1)
            cpu_data->cpu.lstickY = -127;
        return RECOVER_IN_PROGRESS;
    }

    // Finish recovery if grounded
    if (cpu_data->phys.air_state == 0)
        return RECOVER_FINISHED;

    switch (cpu_data->kind) {
        case FTKIND_FOX:
            Recover_ThinkFox(&data, cpu, hmn);
            break;
        default:
            return RECOVER_UNIMPLEMENTED;
    }

    return RECOVER_IN_PROGRESS;
}

#endif
