# To be inserted at 800dd368

.include "Common.s"

# r4 = selected throw
# r31 contains Fighter

# original line
addi r4, r30, 0
lwz r30, 0x4(r31) # internal char id
cmpwi r30, 0xb
bne- Exit # isn't Nana

CharRng_FetchSetting r5, CharRng_Setting_Nana_Throw, Exit
cmpwi r5, 0 # 0 = default setting
beq+ Exit
# srwi r5, r5, 12 # shift 12 bits to the right, 0x00001000 > 0x00000001
li r4, 0xda # 1 - fthrow
add r4, r4, r5

Exit:
