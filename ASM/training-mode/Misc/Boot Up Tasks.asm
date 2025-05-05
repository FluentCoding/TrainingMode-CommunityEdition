    # To be inserted at 801b02ec
    .include "../../Globals.s"
    .include "../../m-ex/Header.s"

    # Set First Boot Flag (used for OSD backup/restore)
    li r3, 0x1
    stb r3, FirstBootFlag(rtoc)
    # Set CPU Info
    li r3, 0x21
    stb r3, EventCPUBackup_CharID(rtoc)                 # Set CPU Character ID

Original:
    lwz r0, 0x001C(sp)
