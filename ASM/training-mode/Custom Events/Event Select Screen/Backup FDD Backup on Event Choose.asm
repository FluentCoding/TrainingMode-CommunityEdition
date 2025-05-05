    # To be inserted at 8024d95c
    .include "../../../Globals.s"
    .include "../../../m-ex/Header.s"

    # Backup
    lwz r6, MemcardData(r13)
    lwz r5, 0x1F24(r6)
    stw r5, -0xDA8(rtoc)

Exit:
    lwz r3, 0x0004(r28)
