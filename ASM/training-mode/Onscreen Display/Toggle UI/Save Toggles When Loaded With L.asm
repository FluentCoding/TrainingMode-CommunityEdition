    # To be inserted at 80235994
    .include "../../../Globals.s"
    .include "../../../m-ex/Header.s"

    # CHECK FLAG IN RULES STRUCT
    load r5, 0x804a04f0
    lbz r0, 0x0011(r5)
    cmpwi r0, 0x2
    blt original

    # CUSTOM CODE
    rlwinm r0, r3, 0, 24, 31    # put menu selection in r0
    # lwz r5, -0xdbc(rtoc) #get frame data toggle bits
    lwz r6, MemcardData(r13)
    lwz r5, 0x1F24(r6)
    rlwinm. r4, r4, 0, 24, 31   # check if turning on or off

    beq turnOff

turnOn:
    li r4, 1
    slw r0, r4, r0
    or r0, r5, r0
    stw r0, 0x1F24(r6)
    # stw r0, -0xdbc(rtoc) #store frame data toggle bits
    b exit

turnOff:
    li r4, 1
    slw r0, r4, r0
    andc r0, r5, r0
    stw r0, 0x1F24(r6)
    # stw r0, -0xdbc(rtoc) #store frame data toggle bits
    b exit

original:
    branchl r5, 0x801641e4

exit:
