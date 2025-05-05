    # To be inserted at 801bab04
    .include "../../../Globals.s"
    .include "../../../m-ex/Header.s"

    backup

    mr r31, r3

    # Loop through selected characters and load .ssm files
    mr r27, r31
    li r28, 0
    mulli r0, r28, 36
    li r29, 0
    li r30, 0

Loop:
    lbz r3, 0x70(r27)
    extsb r3, r3
    branchl r12, 0x80026e84
    addi r28, r28, 1
    cmpwi r28, 6
    or r29, r29, r4
    or r30, r30, r3
    addi r27, r27, 36
    blt Loop

    # Clear Audio Cache?
    li r3, 20
    branchl r12, 0x80026f2c
    # Request All ssm Files
    li r3, 4
    mr r5, r30
    mr r6, r29
    branchl r12, 0x8002702c
    # Load Preload Cache
    branchl r12, 0x80027168

exit:
    mr r3, r31
    restore
    addi r4, r31, 2
