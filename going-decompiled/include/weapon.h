#ifndef WEAPON_H
#define WEAPON_H

#include "common.h"

/* One g_weaponTable entry (USA 0x239B20): the per-weapon-VARIANT definition
 * table, stride 0xE0, indexed by g_itemEquippedSlot[itemId].
 *
 * This is the one layout for the table. 188858, 1A8180 and 1CA080 used to type
 * it three different ways (WeaponDef, WeaponVariant, WeaponVariantFields). All
 * three agreed on the stride and on every offset they shared. They disagreed on
 * one name. Each field below was checked against a USA ROM reader, named by
 * vaddr (task #1737).
 *
 * The header defines the TYPE only. Each unit still declares g_weaponTable
 * itself, because the declared type is load-bearing:
 * 188858 and 1A8180 declare `WeaponDef g_weaponTable[]`. 191238, 1A00F0, 1CA080
 * and 1EFFC0 declare `u8 g_weaponTable[]` and do byte arithmetic on it
 * (`g_weaponTable + slot * 0xE0 + off`). Retyping those changes what that
 * arithmetic means.
 *
 * Fields with no established meaning keep an offset name. Offsets with no named
 * field are not unread: the ROM also reads +0x08, +0x98, +0x9C and +0xA0 (lw),
 * +0x38/+0x3A (the pickup sounds), and writes +0x64..+0x74. Those were not
 * traced. */
typedef struct WeaponDef {
    s32 exists;           /* +0x00 lw: nonzero when this variant is defined */
    u8  upgradeLevel;     /* +0x04 lbu */
    u8  _pad05;
    s16 unk06;            /* +0x06 lh (func_002D4568 0x2D4620); meaning not established */
    u8  _pad08[0x4];
    s32 equipMode;        /* +0x0C lw (func_002AE6C8 0x2AE70C): 0 = gadget (load-gated),
                             1..3 = activeGadgetItem slot */
    u8  _pad10[0x4];
    s32 mobyClass;        /* +0x14 lw: the moby class this variant spawns and answers
                             to. func_002AE7E8 compares it with Moby +0xAA (0x2AE9A4);
                             UpdateQuickSelectWheelInput passes it to
                             IsGadgetClassResident 0x294EE0 (0x28CEAC, FACT #5685).
                             Older notes call it "boltPrice". The price is +0x80. */
    u8  _pad18[0x24];
    u16 nameStringId;     /* +0x3C lhu: handed to RegisterHudElement as arg 2
                             (0x28EB6C) and compared by FindWeaponSlotByName. Never
                             passed to GetLocalizedString (FACT #5685), so "name
                             string" is UNCONFIRMED. */
    u8  _pad3E[0x4];
    s16 unk42;            /* +0x42 lh (func_002D4568 0x2D45F0); meaning not established */
    u8  _pad44[0x4];
    s16 localizedNameId;  /* +0x48 lh: the text id DrawWeaponSelectWheel passes to
                             GetLocalizedString (0x28D518/0x28D51C, FACT #5685) */
    s16 nextVariantSlot;  /* +0x4A lh */
    s16 prevVariantSlot;  /* +0x4C lh */
    u8  _pad4E[0x6];
    f32 unk54;            /* +0x54 lwc1 (func_002B1DF0 0x2B1F88); meaning not established */
    u8  _pad58[0x14];
    s32 xpThreshold;      /* +0x6C lw: variant XP threshold (<<5); negative = no clamp */
    u8  _pad70[0x10];
    s32 price;            /* +0x80 lw: bolt price. GetVendorItemPrice reads it from
                             the entry (0x2F7474) and from a GetWeaponStatsAtLevel
                             copy (0x2F742C) */
    u8  _pad84[0x4];
    u16 sellsAmmoFlag;    /* +0x88: every ROM reader uses lhu */
    u8  _pad8A[0x4];
    u16 ammoCapacity;     /* +0x8E lhu */
    u8  _pad90[0x2];
    u16 ammoStartGrant;   /* +0x92 lhu: GiveInventoryItem stores it into
                             g_weaponAmmo[itemId] (0x288DB8/0x288DBC) */
    u8  _pad94[0x4C];
} WeaponDef;              /* stride 0xE0 */

#endif /* WEAPON_H */
