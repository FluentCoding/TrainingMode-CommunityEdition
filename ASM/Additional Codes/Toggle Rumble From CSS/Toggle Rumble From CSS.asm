    # To be inserted at 802608D8
    .include "../../Globals.s"

# CREDIT: Dan Salvato
# https://smashboards.com/threads/d-pad-up-down-on-css-to-toggle-rumble-with-cursor-shake-effect.417382/
# https://web.archive.org/web/20151121044233/https://www.20xx.me/blog/toggling-rumble-the-te-way

    # If shake bit is on, skip d-pad checks (we're still shaking)
    lbz r3, 7(r31)
    cmpwi r3, 0x0
    bne- Shake
    lbz r4, 4(r31)
    # Check if d-pad up or down is pressed
    mr r23, r4
    lwz r0, MemcardData(r13)
    add r3, r0, r4
    lbz r5, 7360(r3)
    rlwinm. r0, r28, 0, 28, 28
    bne- RumbleOn
    rlwinm. r0, r28, 0, 29, 29
    bne- RumbleOff
    b Exit

RumbleOn:
    cmplwi r5, 1
    beq- Exit
    mr r3, r23
    li r4, 0x0
    li r5, 0xE
    li r6, 0x0
    subi r7, r13, 0x66B0
    branchl r12, 0x80378430
    # set r4 to 1 indicating we want to enable rumble
    li r4, 1
    b ToggleRumble

RumbleOff:
    cmplwi r5, 0
    beq- Exit
    # set r4 to 0 indicating we want to disable rumble
    li r4, 0

ToggleRumble:
    mr r3, r23
    branchl r12, Rumble_StoreRumbleFlag
    li r4, 0x1
    stb r4, 7(r31)
    lis r4, 0xC040
    stw r4, 20(r31)

Shake:
    # Get x position, amplitude, and decay
    lfs f1, 0x14(r31)
    lfs f2, -0x71F4(r2)
    lfs f0, 0xC(r31)
    fadds f0, f1, f0
    stfs f0, 12(r31)
    # Flip amplitude
    fneg f3, f1
    fcmpo cr0, f3, f1
    bgt- SkipDecay
    # If amplitude went from positive to negative, then multiply by decay
    fmuls f3, f3, f2

SkipDecay:
    # Store new amplitude value
    stfs f3, 0x14(r31)
    blt- Exit

    # Check if amplitude is less than .001 (approx)
    lfs f4, -0x7DA8(r2)
    fcmpo cr0, f3, f4
    bgt- Exit

    # If so, turn off the shake bit and set amplitude to 0
    li r4, 0x0
    stw r4, 0x14(r31)
    stb r4, 7(r31)

Exit:
    lbz r4, 4(r31) # Replaced code line
