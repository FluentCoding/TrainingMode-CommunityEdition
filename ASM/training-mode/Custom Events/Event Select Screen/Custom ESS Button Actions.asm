    # To be inserted at 8024d92c
    .include "../../../Globals.s"
    .include "../../../m-ex/Header.s"

    backup

    # Check For L
    li r3, 4
    branchl r12, Inputs_GetPlayerInstantInputs
    
    rlwinm. r0, r4, 0, 25, 25   # CHECK FOR L HARD PRESS
    bne OpenFDD
    
    load r3, 0x804c20bc # stc_css_pad
    lbz r3, 0x1C(r3) # triggerLeft
    cmpwi r3, 20
    bge OpenFDD

    /*
    rlwinm. r0, r4, 0, 27, 27   # CHECK FOR Z
    bne OpenOptions

    # Check for Tutotial (R)
    # Check For Training Mode ISO Game ID First
    lis r5, 0x8000
    lwz r5, 0x0(r5)
    load r6, 0x47544d45         # GTME
    cmpw r5, r6
    bne CheckToSwitchPage
    # Check for R
    rlwinm. r0, r4, 0, 26, 26   # CHECK FOR R
    bne PlayMovie
    */

CheckToSwitchPage:
    li r3, 4
    branchl r12, Inputs_GetPlayerRapidInputs
    # Check For Left
    li r5, -1
    rlwinm. r0, r3, 0, 25, 25
    bne SwitchPage
    # Check For Right
    li r5, 1
    rlwinm. r0, r3, 0, 24, 24
    bne SwitchPage
    b exit

OpenFDD:
    # PLAY SFX
    li r3, 1
    branchl r4, SFX_MenuCommonSound

    # SET FLAG IN RULES STRUCT
    li r0, 3                    # 3 = frame data from event toggle
    load r3, 0x804a04f0
    stb r0, 0x0011(r3)

    # SET SOMETHING
    li r0, 5
    sth r0, -0x4AD8(r13)

    # BACKUP CURRENT EVENT ID
    lwz r3, -0x4A40(r13)
    lwz r5, 0x002C(r3)
    lbz r3, 0x0(r5)
    lwz r4, 0x4(r5)
    add r3, r3, r4
    lwz r4, MemcardData(r13)
    stb r3, 0x0535(r4)

    # LOAD RSS
    branchl r3, 0x80237410

    # REMOVE EVENT THINK FUNCTION
    lwz r3, -0x3E84(r13)
    branchl r12, GObj_Destroy

    b exit

SwitchPage:
    # Get number of pages
    rtocbl r12, TM_GetPageNum
    mr r6, r3
    # Change page
    lwz r4, MemcardData(r13)
    lbz r3, CurrentEventPage(r4)
    add r3, r3, r5
    stb r3, CurrentEventPage(r4)

# Check if within page bounds
SwitchPage_CheckHigh:
    cmpw r3, r6
    ble SwitchPage_CheckLow
    # Stay on current page
    subi r3, r3, 1
    stb r3, CurrentEventPage(r4)
    b exit

SwitchPage_CheckLow:
    cmpwi r3, 0
    bge SwitchPage_ChangePage
    # Stay on current page
    li r3, 0
    stb r3, CurrentEventPage(r4)
    b exit

SwitchPage_ChangePage:
    # Get Page Name string
    lwz r3, MemcardData(r13)
    lbz r3, CurrentEventPage(r3)
    rtocbl r12, TM_GetPageName
    # Update Page Name
    mr r5, r3
    lwz r3, -0x4EB4(r13)
    li r4, 0
    branchl r12, Text_UpdateSubtextContents

    # Reset cursor to 0, 0
    lwz r5, -0x4A40(r13)
    lwz r5, 0x002C(r5)
    li r3, 0
    stw r3, 0x0004(r5)          # Selection Number
    stb r3, 0(r5)               # Page Number

# Redraw Event Text
SwitchPage_DrawEventTextInit:
    li r29, 0                   # loop count
    lwz r3, 0x0004(r5)          # Selection Number
    lbz r4, 0(r5)               # Page Number
    add r28, r3, r4

SwitchPage_DrawEventTextLoop:
    mr r3, r29
    add r4, r29, r28
    branchl r12, 0x8024d15c
    addi r29, r29, 1
    cmpwi r29, 9
    blt SwitchPage_DrawEventTextLoop

    # Redraw Event Description
    lwz r3, -0x4A40(r13)
    mr r4, r28
    branchl r12, 0x8024d7e0

    # Update High Score
    lwz r3, -0x4A40(r13)
    li r4, 0
    branchl r12, 0x8024d5b0

    # Update cursor position
    # Get Texture Data
    lwz r3, -0x4A40(r13)
    lwz r3, 0x0028(r3)
    addi r4, sp, 0x40
    li r5, 11
    li r6, -1
    crclr 6
    branchl r12, 0x80011e24
    lwz r3, 0x40(sp)
    # Change Y offset?
    li r0, 0
    stw r0, 0x3C(r3)
    # DirtySub
    branchl r12, HSD_JObjSetMtxDirtySub

    # Play SFX
    li r3, 2
    branchl r12, SFX_MenuCommonSound

#######################################

exit:
    restore
    li r0, 16
