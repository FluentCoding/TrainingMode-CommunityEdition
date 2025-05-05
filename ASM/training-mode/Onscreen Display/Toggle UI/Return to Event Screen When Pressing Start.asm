    # To be inserted at 80236128
    .include "../../../Globals.s"
    .include "../../../m-ex/Header.s"

    # CHECK FLAG IN RULES STRUCT
    lbz r3, 0x0011(r31)
    cmpwi r3, 0x3
    bne original

LoadEvents:
    mr r3, r27
    branchl r12, GObj_Destroy
    lwz r3, MemcardData(r13)
    lbz r3, 0x0535(r3)      # Get Last Event Match
    li r4, 1                # No Delay On Text Loading
    branchl r12, 0x8024e838
    branch r12, 0x80236164

original:
    li r3, 2
