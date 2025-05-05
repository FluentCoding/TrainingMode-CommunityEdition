    # To be inserted at 8022e5e4
    .include "../../../../Globals.s"
    .include "../../../../m-ex/Header.s"

    # Original Codeline
    stw r3, -0x4AE8(r13)

    # GET START OF DAT
    lwz r5, 0x0(r3)         # get dat length
    subi r3, r3, 0x31       # get to the end of the dat in memory
    sub r5, r3, r5          # get start of dat

    # Zero
    li r3, 0

    # Make Background Texture Transparent
    load r4, 0x1e754
    add r4, r4, r5
    stw r3, 0xC(r4)
    # Make Background Texture (Lang:JP) Transparent
    load r4, 0x1e755
    add r4, r4, r5
    stw r3, 0xC(r4)

    # Make Subject Table Texture Transparent
    load r4, 0x1ec88
    add r4, r4, r5
    stw r3, 0xC(r4)
    # Make Subject Table Texture (Lang:JP) Transparent
    load r4, 0x1ec89
    add r4, r4, r5
    stw r3, 0xC(r4)

exit:
