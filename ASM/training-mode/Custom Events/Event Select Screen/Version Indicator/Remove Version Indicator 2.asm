    # To be inserted at 8024e3b0
    .include "../../../../Globals.s"
    .include "../../../../m-ex/Header.s"

    .set MainTextOffset, -0x4EB4
    .set LeftArrowTextOffset, -0x4EB0
    .set RightArrowTextOffset, -0x4EAC

    # Remove All Text
    lwz r3, MainTextOffset (r13)
    branchl r12, Text_RemoveText
    lwz r3, LeftArrowTextOffset (r13)
    branchl r12, Text_RemoveText
    lwz r3, RightArrowTextOffset (r13)
    branchl r12, Text_RemoveText

    # Original Line
    lwz r3, 0x0074(r31)
