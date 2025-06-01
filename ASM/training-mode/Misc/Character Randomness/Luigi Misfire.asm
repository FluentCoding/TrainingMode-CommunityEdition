# To be inserted at 80142ae0

.include "Fetch Setting.s"

# r3 = output, 0 is misfire, 1 is no misfire
# r31 contains Fighter

CharRng_FetchSetting r6, RandomMisfire
lbz r7, 0xc(r31) # player_id
cmpwi r7, 0
bne LoadCpuSetting

LoadHmnSetting:
  CharRng_ExtractSetting r6, hmn
  b SettingCheck

LoadCpuSetting:
  CharRng_ExtractSetting r6, cpu

SettingCheck:
  cmpwi r6, 0x1000
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