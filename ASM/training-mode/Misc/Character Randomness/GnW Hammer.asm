# To be inserted at 8014c780

.include "Common.s"

# r3 = 0-index number output
# r31 contains Fighter

CharRng_FetchSetting r6, CharRng_Setting_GnW_Hammer, SetHammer

cmpwi r6, 0 # 0 = default setting
beq+ SetHammer
subi r30, r6, 1 # 1-9 gotta be 0-indexed

SetHammer:
  # original line
  stw r30, 0x222c(r31)
