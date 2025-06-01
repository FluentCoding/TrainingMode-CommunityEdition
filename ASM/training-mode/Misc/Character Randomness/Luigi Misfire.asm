# To be inserted at 80142ae0

.include "../../../Globals.s"

# r3 = output, 0 is misfire, 1 is no misfire
# r31 contains Fighter

# TODO make macro for all randomness ASMs to easily retrieve and compare hmn/cpu rng setting

load r6, EventVars_Ptr
lwz r6, 0x0(r6)
lwz r6, EventVars_EventGobj(r6) # eventgobj
lwz r6, 0x2c(r6) # userdata

lhz r6, 0x2a(r6) # setting, format: 0x00001200 where 1 is hmn and 2 is cpu
lbz r7, 0xc(r31) # player_id
cmpwi r7, 0
bne LoadCpuSetting

LoadHmnSetting:
  rlwinm r6, r6, 0, 16, 19
  cmpwi r6, 0x1000
  b BranchSettingCheck

LoadCpuSetting:
  rlwinm r6, r6, 0, 20, 23
  cmpwi r6, 0x0100

BranchSettingCheck:
  blt+ RandomMisfire
  beq AlwaysMisfire

NeverMisfire:
  li r3, 1
  b Exit

RandomMisfire:
  # original line
  branchl r12, HSD_Randi
  b Exit

AlwaysMisfire:
  li r3, 0

Exit: