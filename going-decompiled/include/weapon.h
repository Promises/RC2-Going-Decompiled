#ifndef WEAPON_H
#define WEAPON_H

#include "common.h"

/* One g_weaponTable entry (USA 0x239B20): the per-weapon-VARIANT definition
 * table, stride 0xE0, indexed by g_itemEquippedSlot[itemId]. The table is also
 * written as whole records: RestorePlayerProgressState copies every entry out
 * to the stack and back with lq/sq (0x2DF930, 0x2DFC48).
 *
 * This is the layout symbol_addrs points at. 188858, 1A8180 and 1CA080 used to
 * type it three different ways (WeaponDef, WeaponVariant, WeaponVariantFields)
 * and now include this one (task #1737). A FOURTH typing remains:
 * GuiWeaponTableEntry in 235FE8.c, a partial view (+0x06, +0x08, +0x42) that
 * 235FE8 casts g_weaponTable to. It stays where it is: 235FE8 is matched code
 * and does not include this header. It agrees on every offset and width but
 * not on two names: its "nameStringId" is unk06 here and its "descStringId"
 * is captionTextId here (see those fields).
 *
 * Evidence, field by field. A field comment names a USA ROM reader by vaddr
 * and says what the reader does with the value. "Meaning not established"
 * means the reader is known and the meaning is not. A field whose name is not
 * borne out by its readers says UNCONFIRMED. Readers were found by a register
 * taint walk over the 52 USA nonmatchings .s files that name g_weaponTable
 * (task #1746, FACT in the store). The walk ignores control flow and loses the
 * pointer through stack copies and callee arguments, so it can miss readers.
 *
 * The header defines the TYPE only. Each unit still declares g_weaponTable
 * itself, because the declared type is load-bearing:
 * 188858 and 1A8180 declare `WeaponDef g_weaponTable[]`. 191238, 1A00F0, 1CA080,
 * 1EFFC0 and 235FE8 declare `u8 g_weaponTable[]` and do byte arithmetic on it
 * (`g_weaponTable + slot * 0xE0 + off`). Retyping those changes what that
 * arithmetic means.
 *
 * Offsets with no named field are not all unread. The ROM also reads +0x98,
 * +0x9C and +0xA0 (lw; func_002D02A0 0x2D03C8, func_002F8228 0x2F846C); those
 * were not traced. Older text here said the ROM reads +0x38/+0x3A ("the pickup
 * sounds") and writes +0x64..+0x74. Neither the taint walk nor a file-wide
 * scan of those 52 files found such an access through a g_weaponTable pointer.
 * Every +0x38/+0x64..+0x74 hit checked is through another struct
 * (UpdateVendorMenuInput's +0x6C is g_vendorUi; func_0028C840's +0x64..+0x74
 * is its GUI state in $17; func_002B1DF0's +0x38 is the struct in $18). Those
 * two claims are UNVERIFIED and have no known source. */
typedef struct WeaponDef {
    s32 exists;           /* +0x00 lw: nonzero when this variant is defined.
                             func_002888D8 0x288960 tests it before refilling
                             ammo; AddItemToInventoryOrder 0x288E38 requires it */
    u8  upgradeLevel;     /* +0x04 lbu: GetWeaponStatsAtLevel 0x289360 compares it
                             with its level argument after stepping that many
                             nextVariantSlot links; AddItemToInventoryOrder
                             0x288E44 requires 0 */
    u8  _pad05;
    s16 unk06;            /* +0x06 lh: func_002D4568 0x2D4620 stores it in a widget
                             (+0x58); GuiWeaponGridTick 0x347FBC and four other
                             235FE8 readers store it in D_1ADAF0 (0x347FCC), which
                             func_00337D98 returns to the menu ticks at 0x2CE230 and
                             0x2CE498. No reader found passes it to
                             GetLocalizedString. 235FE8 names it "nameStringId";
                             that name is UNCONFIRMED. Meaning not established */
    s32 captionTextId;    /* +0x08 lw: a localized text id. GuiWeaponGridTick
                             0x347FC4 passes it to GetLocalizedString (0x347FC8) and
                             prints the result into its name buffer; func_00344808
                             0x344888 sets the result as the selected weapon's
                             caption. 235FE8 names it "descStringId" */
    s32 equipMode;        /* +0x0C lw (func_002AE6C8 0x2AE70C): 0 = gadget (load-gated),
                             1..3 = activeGadgetItem slot */
    u8  _pad10[0x4];
    s32 mobyClass;        /* +0x14 lw: the moby class this variant spawns and answers
                             to. func_002AE7E8 compares it with Moby +0xAA (0x2AE9A4);
                             UpdateQuickSelectWheelInput passes it to
                             IsGadgetClassResident 0x294EE0 (0x28CEAC, FACT #5685).
                             Older notes call it "boltPrice". The price is +0x80. */
    u8  _pad18[0x24];
    u16 iconId;           /* +0x3C lhu: the variant's HUD icon. func_00348628 passes
                             it to GuiSpriteSetTexture (0x348828); 0xEA7E is the
                             empty-icon sentinel (func_00341708 0x341784,
                             func_00348628 0x3488A0); func_002D9D60 resolves it
                             through the HUD icon map with func_0028EDF0 (0x2DA198);
                             RefreshAmmoHudElement hands it to RegisterHudElement as
                             arg 2 (0x28EB6C), the icon-bind path. Never passed to
                             GetLocalizedString (FACT #5685). FindWeaponSlotByName
                             matches its argument against this field. Formerly
                             "nameStringId" (task #1746) */
    u8  _pad3E[0x4];
    s16 unk42;            /* +0x42 lh (func_002D4568 0x2D45F0); meaning not established */
    u8  _pad44[0x4];
    s16 localizedNameId;  /* +0x48 lh: the text id DrawWeaponSelectWheel passes to
                             GetLocalizedString (0x28D518/0x28D51C, FACT #5685) */
    s16 nextVariantSlot;  /* +0x4A lh: the slot of the next upgrade, 0 = none.
                             func_002888D8 0x288908 stops when it is 0;
                             GetWeaponStatsAtLevel 0x28930C walks it */
    s16 prevVariantSlot;  /* +0x4C lh: the slot of the previous upgrade, 0 = base.
                             GetWeaponUpgradeLevel 0x2889A0 walks it back */
    u8  _pad4E[0x6];
    f32 unk54;            /* +0x54 lwc1 (func_002B1DF0 0x2B1F88); meaning not established */
    u8  _pad58[0x14];
    s32 xpThreshold;      /* +0x6C lw: the XP needed for the next variant, in units
                             of 32 XP; 0 or negative = no next level.
                             func_002888D8 0x288914 raises g_weaponXp[item] to it
                             << 5 (0x288928) on upgrade when it is >= 0.
                             func_00348628 makes it the max of the XP bar
                             (GuiListSetItemCount, 0x348904) with g_weaponXp >> 5 as
                             the fill (0x348928), and shows a full bar when it is
                             <= 0 (0x3489EC). "List item count" in 235FE8's comments
                             is that bar's max. symbol_addrs used to call it
                             "upgradeCountGate" */
    u8  _pad70[0x10];
    s32 price;            /* +0x80 lw: bolt price. GetVendorItemPrice reads it from
                             the entry (0x2F7474) and from a GetWeaponStatsAtLevel
                             copy (0x2F742C) */
    u8  _pad84[0x4];
    u16 sellsAmmoFlag;    /* +0x88 lhu: nonzero for a weapon that keeps an ammo
                             count. GiveInventoryItem 0x288D94 grants ammo only when
                             it is set; AddItemToInventoryOrder 0x288E5C accepts it
                             in place of a price; func_00348628 0x348A54 tints an
                             empty weapon's backdrop only when it is set. "Sells" is
                             UNCONFIRMED: no reader found ties it to the vendor */
    u8  _pad8A[0x4];
    u16 ammoCapacity;     /* +0x8E lhu: maximum ammo. func_002888D8 0x288974 refills
                             g_weaponAmmo[item] to it on upgrade;
                             RefreshAmmoHudElement 0x28EB98 passes it to
                             RegisterHudElement beside &g_weaponAmmo[item] */
    u8  _pad90[0x2];
    u16 ammoStartGrant;   /* +0x92 lhu: GiveInventoryItem stores it into
                             g_weaponAmmo[itemId] (0x288DB8/0x288DBC) */
    u8  _pad94[0x4C];
} WeaponDef;              /* stride 0xE0 */

#endif /* WEAPON_H */
