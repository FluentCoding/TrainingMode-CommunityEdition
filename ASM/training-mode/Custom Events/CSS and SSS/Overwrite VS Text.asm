    # To be inserted at 80266314
    .include "../../../Globals.s"
    .include "../../../m-ex/Header.s"

    # Overwrites a call to CSS_UpdateRulesText
    # And sets the menu text to null if in event mode.

    # Ensure this is event mode
    load r3, SceneController
    lbz r3, Scene.CurrentMajor(r3)
    cmpwi r3, Scene.EventMode
    bne Original

    li r3, 0
    li r4, 0x4a
    li r5, 0
    branchl r12, 0x803a6530

    li r3, 0
    li r4, 0x4a
    li r5, 0
    branchl r12, 0x803a660c
    b Exit

Original:
    branchl r12, 0x8025bd30
Exit:
