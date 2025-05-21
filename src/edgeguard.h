#include "../MexTK/mex.h"
#include "events.h"

static void Exit(int value);
static void Reset();

static void Think_Spacies(void);
static void Think_Sheik(void);

// Common values -------------------------------------------------

// hit strength MUST be the first option
#define OPT_HITSTRENGTH 0

static const char *OffOn[2] = {"Off", "On"};

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

#define ExitOption {\
    .kind = OPTKIND_FUNC,\
    .name = "Exit",\
    .desc = "Return to the Event Select Screen.",\
    .OnSelect = Exit,\
}

// must be the first option
#define HitStrengthOption {\
    .kind = OPTKIND_STRING,\
    .name = "Hit Strength",\
    .desc = "How far the CPU will be knocked back.",\
    .values = Values_HitStrength,\
    .value_num = countof(Values_HitStrength),\
    .val = 1,\
}

// Fox -------------------------------------------------

enum options_spacies {
    OPT_SPACIES_HITSTRENGTH,
    OPT_SPACIES_FF_LOW,
    OPT_SPACIES_FF_MID,
    OPT_SPACIES_FF_HIGH,
    OPT_SPACIES_JUMP,
    OPT_SPACIES_ILLUSION,
    OPT_SPACIES_FASTFALL,
    OPT_SPACIES_EXIT,
    
    OPT_SPACIES_COUNT
};

static EventOption Options_Main_Fox[] = {
    HitStrengthOption,
    {
        .kind = OPTKIND_STRING,
        .name = "Firefox Low",
        .desc = "Allow Fox to aim his up special to the ledge.",
        .values = OffOn,
        .value_num = 2,
        .val = 1,
    },
    {
        .kind = OPTKIND_STRING,
        .name = "Firefox Mid",
        .desc = "Allow Fox to aim his up special to the stage.",
        .values = OffOn,
        .value_num = 2,
    },
    {
        .kind = OPTKIND_STRING,
        .name = "Firefox High",
        .desc = "Allow Fox to aim his up special high.",
        .values = OffOn,
        .value_num = 2,
    },
    {
        .kind = OPTKIND_STRING,
        .name = "Double Jump",
        .desc = "Allow Fox to double jump.",
        .values = OffOn,
        .value_num = 2,
    },
    {
        .kind = OPTKIND_STRING,
        .name = "Illusion",
        .desc = "Allow Fox to side special.",
        .values = OffOn,
        .value_num = 2,
    },
    {
        .kind = OPTKIND_STRING,
        .name = "Fast Fall",
        .desc = "Allow Fox to fast fall.",
        .values = OffOn,
        .value_num = 2,
    },
    ExitOption,
};
static EventMenu Menu_Main_Fox = {
    .name = "Fox Edgeguard",
    .option_num = sizeof(Options_Main_Fox) / sizeof(EventOption),
    .options = &Options_Main_Fox,
};

// Falco -------------------------------------------------

static EventOption Options_Main_Falco[] = {
    HitStrengthOption,
    {
        .kind = OPTKIND_STRING,
        .name = "Firebird Low",
        .desc = "Allow Falco to aim his up special to the ledge.",
        .values = OffOn,
        .value_num = 2,
        .val = 1,
    },
    {
        .kind = OPTKIND_STRING,
        .name = "Firebird Mid",
        .desc = "Allow Falco to aim his up special to the stage.",
        .values = OffOn,
        .value_num = 2,
    },
    {
        .kind = OPTKIND_STRING,
        .name = "Firebird High",
        .desc = "Allow Falco to aim his up special high.",
        .values = OffOn,
        .value_num = 2,
    },
    {
        .kind = OPTKIND_STRING,
        .name = "Double Jump",
        .desc = "Allow Falco to double jump.",
        .values = OffOn,
        .value_num = 2,
    },
    {
        .kind = OPTKIND_STRING,
        .name = "Illusion",
        .desc = "Allow Falco to side special.",
        .values = OffOn,
        .value_num = 2,
    },
    {
        .kind = OPTKIND_STRING,
        .name = "Fast Fall",
        .desc = "Allow Falco to fast fall.",
        .values = OffOn,
        .value_num = 2,
    },
    ExitOption,
};
static EventMenu Menu_Main_Falco = {
    .name = "Falco Edgeguard",
    .option_num = sizeof(Options_Main_Falco) / sizeof(EventOption),
    .options = &Options_Main_Falco,
};

// Sheik -------------------------------------------------

enum options_sheik {
    OPT_SHEIK_HITSTRENTH,
    OPT_SHEIK_UPB_LEDGE,
    OPT_SHEIK_UPB_STAGE,
    OPT_SHEIK_UPB_HIGH,
    OPT_SHEIK_JUMP,
    OPT_SHEIK_FASTFALL,
    OPT_SHEIK_FAIR,
    OPT_SHEIK_AMSAH_TECH,
    OPT_SHIEK_EXIT,
    
    OPT_SHEIK_COUNT
};

static EventOption Options_Main_Sheik[] = {
    HitStrengthOption,
    {
        .kind = OPTKIND_STRING,
        .name = "Vanish to Ledge",
        .desc = "Allow Sheik to vanish to ledge and the\ntip of the stage.",
        .values = OffOn,
        .value_num = 2,
        .val = 1,
    },
    {
        .kind = OPTKIND_STRING,
        .name = "Vanish to Stage",
        .desc = "Allow Sheik to vanish deep into stage.",
        .values = OffOn,
        .value_num = 2,
        .val = 1,
    },
    {
        .kind = OPTKIND_STRING,
        .name = "Vanish High",
        .desc = "Allow Sheik to vanish above the ledge\nand to platforms.",
        .values = OffOn,
        .value_num = 2,
        .val = 1,
    },
    {
        .kind = OPTKIND_STRING,
        .name = "Double Jump",
        .desc = "Allow Sheik to double jump.",
        .values = OffOn,
        .value_num = 2,
    },
    {
        .kind = OPTKIND_STRING,
        .name = "Fast Fall",
        .desc = "Allow Sheik to fast fall.",
        .values = OffOn,
        .value_num = 2,
    },
    {
        .kind = OPTKIND_STRING,
        .name = "Fair",
        .desc = "Allow Sheik to fair the player.",
        .values = OffOn,
        .value_num = 2,
    },
    {
        .kind = OPTKIND_STRING,
        .name = "Amsah Tech",
        .desc = "Allow Sheik to Amsah Tech.",
        .values = OffOn,
        .value_num = 2,
    },
    ExitOption,
};

static EventMenu Menu_Main_Sheik = {
    .name = "Sheik Edgeguard",
    .option_num = sizeof(Options_Main_Sheik) / sizeof(EventOption),
    .options = &Options_Main_Sheik,
};

// Info lookup -----------------------------------------

typedef struct EdgeguardInfo {
    EventMenu *menu;
    void (*Think)(void);
} EdgeguardInfo;

static EdgeguardInfo InfoLookup[] = {
    {0}, // MARIO
    {
        .menu = &Menu_Main_Fox,
        .Think = Think_Spacies,
    },
    {0}, // FALCON
    {0}, // DK
    {0}, // KIRBY
    {0}, // BOWSER
    {0}, // LINK
    {
        .menu = &Menu_Main_Sheik,
        .Think = Think_Sheik,
    },
    {0}, // NESS
    {0}, // PEACH
    {0}, // POPO
    {0}, // NANA
    {0}, // PIKACHU
    {0}, // SAMUS
    {0}, // YOSHI
    {0}, // JIGGLYPUFF
    {0}, // MEWTWO
    {0}, // LUIGI
    {0}, // MARTH
    {0}, // ZELDA
    {0}, // YOUNGLINK
    {0}, // DRMARIO
    {
        .menu = &Menu_Main_Falco,
        .Think = Think_Spacies,
    },
    {0}, // PICHU
    {0}, // GAW,
    {0}, // GANONDORF
    {0}, // ROY
};
