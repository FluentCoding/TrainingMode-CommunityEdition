    # To be inserted at 8008faf0
    .include "../../../Globals.s"
    .include "../../../m-ex/Header.s"

    # Branch to Interrupt Check With Interrupt Bool in r3 and player in r4
    mr r4, r30
    branchl r12, 0x80005504

    branch r12, 0x8008fb00
