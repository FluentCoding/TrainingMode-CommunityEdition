# To be inserted at 802bd3ec

.include "Fetch Setting.s"

.set PEACH_REGULAR_TURNIP_MENUID, 0x1000
.set PEACH_WINKY_TURNIP_MENUID, 0x2000
.set PEACH_DOT_EYES_TURNIP_MENUID, 0x3000
.set PEACH_STITCH_FACE_TURNIP_MENUID, 0x4000

.set PEACH_REGULAR_TURNIP_RNG_VALUE, 0
.set PEACH_WINKY_TURNIP_RNG_VALUE, 52
.set PEACH_DOT_EYES_TURNIP_RNG_VALUE, 56
.set PEACH_STITCH_FACE_TURNIP_RNG_VALUE, 57

# r3 = RNG Return Value
# r31 contains Fighter

CharRng_FetchSetting r9, RandomTurnip
lbz r10, 0xc(r31) # player_id
cmpwi r10, 0
bne LoadCpuSetting

LoadHmnSetting:
  CharRng_ExtractSetting r9, hmn
  b DetermineTurnip

LoadCpuSetting:
  CharRng_ExtractSetting r9, cpu

DetermineTurnip:
  cmpwi r9, 0 # 0 = default setting
  beq+ RandomTurnip
  cmpwi r9, PEACH_REGULAR_TURNIP_MENUID
  beq PullRegularTurnip
  cmpwi r9, PEACH_WINKY_TURNIP_MENUID
  beq PullWinkyTurnip
  cmpwi r9, PEACH_DOT_EYES_TURNIP_MENUID
  beq PullDotEyesTurnip
  # r == PEACH_STITCH_FACE_TURNIP_MENUID

PullStitchFaceTurnip:
  li r3, PEACH_STITCH_FACE_TURNIP_RNG_VALUE
  b Exit

PullRegularTurnip:
  li r3, PEACH_REGULAR_TURNIP_RNG_VALUE
  b Exit

PullWinkyTurnip:
  li r3, PEACH_WINKY_TURNIP_RNG_VALUE
  b Exit

PullDotEyesTurnip:
  li r3, PEACH_DOT_EYES_TURNIP_RNG_VALUE
  b Exit

RandomTurnip:
  # original line
  branchl r12, HSD_Randi

Exit:
