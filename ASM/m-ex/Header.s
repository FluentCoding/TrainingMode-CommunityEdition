    # Custom Functions
    .set Reloc, 0x803d7074

    # Fighter Data Sizes
    .set FighterDataOrigSize, 0x23ec
    .set MEX_FighterDataSize, 0xC
    .set TM_FighterDataSize, 0x138
    .set FighterDataTotalSize, FighterDataOrigSize + MEX_FighterDataSize + TM_FighterDataSize

    # Fighter Data Start
    .set FighterDataStart, 0x0
    .set MEX_FighterDataStart, FighterDataStart + FighterDataOrigSize
    .set TM_FighterDataStart, MEX_FighterDataStart + MEX_FighterDataSize

    # Fighter Data Variables
    # MEX
    .set MEX_AnimOwner, MEX_FighterDataStart + 0x0                  # 4 bytes
    .set MEX_UCFCurrX, MEX_AnimOwner + 0x4                          # 1 byte
    .set MEX_UCFPrevX, MEX_UCFCurrX + 0x1                           # 1 byte
    .set MEX_UCF2fX, MEX_UCFPrevX + 0x1                             # 1 byte
    .set MEX_align, MEX_UCF2fX + 0x1                                # 1 byte

    # TM
    ## Dedicated ##
    .set TM_FramesinCurrentAS, TM_FighterDataStart + 0x0            # half
    .set TM_ShieldFrames, TM_FramesinCurrentAS + 0x2                # half
    .set TM_PrevASSlots, 6
    .set TM_PrevASStart, TM_ShieldFrames + 0x2                      # halves
    .set TM_OneASAgo, TM_PrevASStart+0x0
    .set TM_TwoASAgo, TM_OneASAgo+0x2
    .set TM_ThreeASAgo, TM_TwoASAgo+0x2
    .set TM_FourASAgo, TM_ThreeASAgo+0x2
    .set TM_FiveASAgo, TM_FourASAgo+0x2
    .set TM_SixASAgo, TM_FiveASAgo+0x2
    .set TM_FramesInPrevASStart, TM_SixASAgo + 0x2                  # halves
    .set TM_FramesinOneASAgo, TM_FramesInPrevASStart+0x0
    .set TM_FramesinTwoASAgo, TM_FramesinOneASAgo+0x2
    .set TM_FramesinThreeASAgo, TM_FramesinTwoASAgo+0x2
    .set TM_FramesinFourASAgo, TM_FramesinThreeASAgo+0x2
    .set TM_FramesinFiveASAgo, TM_FramesinFourASAgo+0x2
    .set TM_FramesinSixASAgo, TM_FramesinFiveASAgo+0x2
    .set TM_PreviousMoveInstanceHitBy, TM_FramesinSixASAgo + 0x2    # half
    .set TM_TangibleFrameCount, TM_PreviousMoveInstanceHitBy + 0x2  # half
    .set TM_CanFastfallFrameCount, TM_TangibleFrameCount + 0x2      # half
    .set TM_IasaFrames, TM_CanFastfallFrameCount + 0x2              # half
    .set TM_PostHitstunFrameCount, TM_IasaFrames + 0x2              # word
    .set TM_PlayerWhoShieldedAttack, TM_PostHitstunFrameCount + 0x4 # word
    .set TM_AnimCallback, TM_PlayerWhoShieldedAttack + 0x4
    # union
    .set TM_UnionStart, TM_AnimCallback + 0x4
    .set TM_AirdodgeAngle, TM_UnionStart
    .set TM_ShortOrFullHop, TM_AirdodgeAngle + 0x4
    .set TM_SuccessfulSDIInputs, TM_UnionStart
    .set TM_TotalSDIInputs, TM_SuccessfulSDIInputs + 0x2
    # end union
    .set TM_Inputs, TM_UnionStart + 0x8
