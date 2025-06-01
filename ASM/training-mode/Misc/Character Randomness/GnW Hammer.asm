# To be inserted at 8014c780

.include "Fetch Setting.s"

# r3 = 0-index number output
# r31 contains Fighter

CharRng_FetchSetting r6, RandomHammer
lbz r7, 0xc(r31) # player_id
cmpwi r7, 0
bne LoadCpuSetting

LoadHmnSetting:
  CharRng_ExtractSetting r6, hmn
  b DetermineHammer

LoadCpuSetting:
  CharRng_ExtractSetting r6, cpu

DetermineHammer:
  cmpwi r6, 0 # 0 = default
  beq+ SetHammer
  srwi r6, r6, 12 # shift 12 bits to the right, 0x00001000 > 0x00000001
  subi r30, r6, 1 # 1-9 gotta be 0-indexed

SetHammer:
  # original line
  stw r30, 0x222c(r31)

Exit:
