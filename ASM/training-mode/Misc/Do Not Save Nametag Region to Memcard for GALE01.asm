    # To be inserted at 8001ae8c
    .include "../../Globals.s"
    .include "../../m-ex/Header.s"

    # Check If Save Chunk #2 and #3
    cmpwi r25, 0x2
    beq SkipSave
    cmpwi r25, 0x3
    beq SkipSave
    b original

# Check For Game ID GALE01(Memcard Only)
SkipSave:
    lis r3, 0x8000
    lwz r3, 0x0(r3)
    load r4, 0x47414c45     # GALE
    cmpw r3, r4
    bne original
    li r0, 0
    b exit

original:
    lwz r0, 0x00F4(r31)

exit:
