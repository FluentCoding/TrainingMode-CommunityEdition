    # To be inserted at 8016d1f8
    # In SceneThink_VSMove, just before the branch to skip the lras check

    .include "../../Globals.s"
    .include "../../m-ex/Header.s"
    
    load r3, 0x804c1fac # master pad array
    li r6, 0
        
LRAS_Loop:
    lwz r4, 0x0(r3) # held
    
    CheckStart:
        andi. r0, r4, PAD_BUTTON_START
        beq Continue
        
    CheckA:
        andi. r0, r4, PAD_BUTTON_A
        beq Continue
        
    CheckL:
        mr r20, r3
        mr r3, r20
        lbz r5, 0x1C(r3) # triggerLeft
        cmpwi r5, 20
        bge CheckLEnd
        andi. r0, r4, PAD_TRIGGER_L
        bne CheckLEnd
        b Continue
    CheckLEnd:
        
    CheckR:
        lbz r5, 0x1D(r3) # triggerRight
        cmpwi r5, 20
        bge CheckREnd
        andi. r0, r4, PAD_TRIGGER_R
        bne CheckREnd
        b Continue
    CheckREnd:
        li r3, 1
        b Exit
        
    Continue:
        addi r3, r3, 0x44 # inc pad
        addi r6, r6, 1 # inc ply
        cmpwi r6, 6
        bne LRAS_Loop
        li r3, 0
    
Exit:
    cmpwi r3, 1
