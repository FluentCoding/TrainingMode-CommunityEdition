    # To be inserted at 801a49a8
    .include "../../../Globals.s"

    lwz r0, DEBUGLV(r13)
    cmpwi r0, 3
    bge isDebug

NoDebug:
    # Run just HSDUpdate
    branch r12, 0x801a4a78

isDebug:
# Run as normal
