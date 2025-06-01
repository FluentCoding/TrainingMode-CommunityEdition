.include "../../../Globals.s"

.macro CharRng_FetchSetting reg, default_branch
load \reg, EventVars_Ptr
lwz \reg, 0x0(\reg)
lwz \reg, EventVars_EventGobj(\reg) # eventgobj
cmpwi \reg, 0 # if eventgobj is null, we default to normal rng behavior
beq \default_branch
lwz \reg, 0x2c(\reg) # userdata
lhz \reg, 0x2a(\reg) # setting, format: 0x00001200 where 1 is hmn and 2 is cpu
.endm

.macro CharRng_ExtractSetting reg, type
.if \type == hmn
rlwinm \reg, \reg, 0, 16, 19
.elseif \type == cpu
rlwinm \reg, \reg, 4, 16, 19
.else
"'hmn' or 'rng' expected as second argument"
.endif
.endm

.macro CharRng_LoadPlayerIdOfFighter reg, fighter_reg
lbz \reg, 0xc(\fighter_reg)
.endm
