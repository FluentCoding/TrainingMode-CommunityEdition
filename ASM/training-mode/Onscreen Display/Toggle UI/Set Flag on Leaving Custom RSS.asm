    # To be inserted at 80235fcc
    .include "../../../Globals.s"
    .include "../../../m-ex/Header.s"

    # CHECK FLAG IN RULES STRUCT
    lbz r0, 0x0011(r31)
    cmpwi r0, 0x2
    bge exit

original:
    stb r30, 0x0011(r31)

exit:
