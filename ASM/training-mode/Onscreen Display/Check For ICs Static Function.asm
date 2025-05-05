    # To be inserted at 80005510

    .include "../../Globals.s"

    .set playerdata, 31
    .set player, 30

    backup

    mr r31, r3

    # Get Byte
    lbz r3, 0x221F(playerdata)

    # Check If Subchar
    rlwinm. r0, r3, 29, 31, 31
    beq NoFollower

    # Check If Follower
    lbz r3, 0xC(playerdata) # get slot
    branchl r12, PlayerBlock_LoadExternalCharID # get external character ID
    load r4, 0x803bcde0     # pdLoadCommonData table
    mulli r0, r3, 3         # struct length
    add r3, r4, r0          # get characters entry
    lbz r0, 0x2(r3)         # get subchar functionality
    cmpwi r0, 0x0           # if not a follower, exit
    bne NoFollower

Follower:
    li r3, 0x1
    b 0x8

NoFollower:
    li r3, 0x0

Exit:
    restore
    blr
