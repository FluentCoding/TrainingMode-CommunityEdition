    # To be inserted at 80236bbc
    .include "../../../Globals.s"
    .include "../../../m-ex/Header.s"

    # CHECK FLAG IN RULES STRUCT
    load r3, 0x804a04f0
    lbz r0, 0x0011(r3)
    cmpwi r0, 0x2
    blt original

    # REMOVE CUSTOM TEXT
    lwz r3, 0x40(r31)       # GET TEXT POINTER
    branchl r12, Text_RemoveText # REMOVE TEXT

    lwz r3, 0x44(r31)       # GET TEXT POINTER
    branchl r12, Text_RemoveText # REMOVE TEXT

    branch r12, 0x80236be0  # EXIT

original:
    li r25, 0

exit:
