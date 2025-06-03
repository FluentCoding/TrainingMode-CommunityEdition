# To be inserted at 8011d0a4

.include "Common.s"

.set PEACH_TURNIP_MENUID, 0x1
.set PEACH_MRSATURN_MENUID, 0x5
.set PEACH_BOBOMB_MENUID, 0x6
.set PEACH_BEAMSWORD_MENUID, 0x7

.set PEACH_TURNIP_ITEMID, 0x63
.set PEACH_MRSATURN_ITEMID, 0x07
.set PEACH_BOBOMB_ITEMID, 0x06
.set PEACH_BEAMSWORD_ITEMID, 0x0C

# r6 = ItemKind to be passed to it_802BD4AC
# r30 contains Fighter

CharRng_FetchSetting r6, CharRng_Setting_Peach_Item, SetResult

cmpwi r6, 0 # 0 = default setting
beq+ SetResult
cmpwi r6, PEACH_BEAMSWORD_MENUID
beq PullBeamsword
cmpwi r6, PEACH_BOBOMB_MENUID
beq PullBobomb
cmpwi r6, PEACH_MRSATURN_MENUID
beq PullMrSaturn
# r6 != 0 && r6 < mrsaturn && r6 >= turnip

PullTurnip:
  li r31, PEACH_TURNIP_ITEMID
  b SetResult

PullMrSaturn:
  li r31, PEACH_MRSATURN_ITEMID
  b SetResult

PullBobomb:
  li r31, PEACH_BOBOMB_ITEMID
  b SetResult

PullBeamsword:
  li r31, PEACH_BEAMSWORD_ITEMID

SetResult:
  # original line
  mr r6, r31
