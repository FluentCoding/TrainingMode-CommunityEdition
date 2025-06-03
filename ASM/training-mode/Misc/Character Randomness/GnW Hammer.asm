# To be inserted at 8014c780

.include "Common.s"

# r3 = 0-index number output
# r31 contains Fighter

CharRng_FetchSetting r6, CharRng_Setting_GnW_Hammer, SetHammer

cmpwi r6, 0 # 0 = default setting
beq+ SetHammer
# srwi r6, r6, 12 # shift 12 bits to the right, 0x00001000 > 0x00000001
subi r30, r6, 1 # 1-9 gotta be 0-indexed

SetHammer:
  # original line
  stw r30, 0x222c(r31)
