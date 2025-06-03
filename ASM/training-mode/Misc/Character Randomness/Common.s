.include "../../../Globals.s"

.set CharRng_Setting_Peach_Item, 0x0
.set CharRng_Setting_Luigi_Misfire, 0x1
.set CharRng_Setting_GnW_Hammer, 0x2
.set CharRng_Setting_Nana_Throw, 0x3

.macro CharRng_FetchSetting reg, setting, default_branch
load \reg, EventVars_Ptr
lwz \reg, 0x0(\reg)
lwz \reg, EventVars_PersistentData(\reg) # persistent data
cmpwi \reg, 0 # if persistent data is null, we default to normal rng behavior
beq \default_branch
lbz \reg, \setting(\reg)
.endm
