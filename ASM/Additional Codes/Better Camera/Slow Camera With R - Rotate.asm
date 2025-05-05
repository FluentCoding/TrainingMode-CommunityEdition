    # to be inserted at 80227ac4

    .include "../../Globals.s"
    .include "../../m-ex/Header.s"

    backupall

    li r3, 0

    load r4, HSD_InputStructStart
    li r5, 4
CheckForL:
    lwz r6, 0x0(r4)
    or r3, r3, r6
    addi r4, r4, 0x44
    addi r5, r5, -1
    cmpwi r5, 0
    bne CheckForL
EndCheckForL:
    li r4, PAD_TRIGGER_R
    and r3, r3, r4

    bl NormalSpeed
    mflr r4
    cmpwi r3, 0
    beq EndGetSpeed
    bl SlowSpeed
    mflr r4
EndGetSpeed:
    lfs f0, 0x0(r4)
    stfs f0, -0x3C88(rtoc) # need to store, this value is read for both x & y rotation
    b Exit

SlowSpeed:
    blrl
    .float 0.001745
NormalSpeed:
    blrl
    .float 0.01745

Exit:
    restoreall
