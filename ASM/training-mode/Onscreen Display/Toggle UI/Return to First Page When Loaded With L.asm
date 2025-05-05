    # To be inserted at 80235fe4
    .include "../../../Globals.s"
    .include "../../../m-ex/Header.s"

    # CHECK FLAG IN RULES STRUCT
    load r3, 0x804a04f0
    lbz r0, 0x0011(r3)
    cmpwi r0, 0x2
    blt original

    # DETERMINE WHERE TO RETURN
    cmpwi r0, 0x2
    beq LoadRulesFirstPage
    cmpwi r0, 0x3
    beq LoadEvents

LoadRulesFirstPage:
    branchl r12, 0x8023164c
    b exit

LoadEvents:
    lwz r3, MemcardData(r13)
    lbz r3, 0x0535(r3)      # Get Last Event Match
    li r4, 1                # No Delay On Text Loading
    branchl r12, 0x8024e838
    b exit

original:
    branchl r12, 0x802339fc

exit:
