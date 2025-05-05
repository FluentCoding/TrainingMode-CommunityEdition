    # To be inserted at 801af6f4
    .include "../../Globals.s"

    .set MEMCARD_HASSAVE, 0x0
    .set MEMCARD_NOSAVE, 0x4
    .set MEMCARD_NONE, 0xF
    .set MEMCARD_NONE2, 0xD

    # Skip if we have save
    cmpwi r29, MEMCARD_HASSAVE
    beq Original

InitSave:
    backup
    lwz r4, MemcardData(r13)
    # Set Max OSD on No Memcard
    li r3, 1
    stb r3, OSDMaxWindows(r4)
    # Set Initial Page Number
    li r3, 1
    stb r3, CurrentEventPage(r4)
    # Enable Recommended OSDs
    li r3, 0
    stb r3, OSDRecommended(r4)
    # Turn off OSDs by default
    li r3, 0
    stw r3, OSDBitfield(r4)

    restore

    cmpwi r29, MEMCARD_NOSAVE
    beq Original

NoMemcard:
    # No memcard available
    # Exit memcard think and disable saving
    branch r12, 0x801b01ac

Original:
    cmpwi r29, 0
