#include "common.h"

/*
 * EU text/188748 - REGION-AXIS twin of USA text/188858 (".text mid 2",
 * weapon/inventory accessors + cinematic-request queue + level-exit/save-prompt
 * gates + localization/subtitle + bolt-counter HUD). Ported verbatim from
 * src/usa/text/188858.c across the REGION axis (2026-06-15): the matched bodies
 * are region-agnostic; only extern global NAMES and unit-local callee names are
 * retargeted to their EU addresses (objdiff masks the gp/reloc deltas). Built
 * -O2 -G8 -fno-gcse (mirrors the USA unit; per-unit GFLAG override in the build
 * scripts). EU function names follow the EU vaddr (USA func_002895xx ->
 * EU func_002894xx etc). Of the 15 USA-matched bodies, 14 mirror here; the two
 * empty-leaf twins (func_002896A0, func_0028A468) were already emitted as C by
 * the splat stub generator. USA func_0028C6E0's HUD-init forwarder twin is
 * EU func_0028C668 (recovered 2026-06-17 via the padding mis-split fix: the EU
 * split had fused it behind a 0x48 epilogue-stump run as func_0028C620; a
 * symbol_addrs pin promotes 0x0028C668 to its own glabel).
 */

/* Per-weapon-variant def/state record (g_weaponTable, stride 0xE0). */
typedef struct WeaponDef {
    s32 exists;          /* +0x00 */
    u8  upgradeLevel;    /* +0x04 */
    u8  _pad05[0xF];
    s32 boltPrice;       /* +0x14 */
    u8  _pad18[0x24];
    u16 nameStringId;    /* +0x3C */
    u8  _pad3E[0xC];
    s16 nextVariantSlot; /* +0x4A */
    s16 prevVariantSlot; /* +0x4C */
    u8  _pad4E[0x3A];
    s16 sellsAmmoFlag;   /* +0x88 */
    u8  _pad8A[0x4];
    u16 ammoCapacity;    /* +0x8E */
    u16 ammoStartGrant;  /* +0x90 */
    u8  _pad92[0x4E];
} WeaponDef;

/* Ring buffer of queued cinematic requests (5 slots, each 8 bytes). */
typedef struct CinematicSlot {
    s32 id;            /* +0x0 */
    s32 flag;          /* +0x4 */
} CinematicSlot;

typedef struct CinematicQueue {
    u8            _pad0[0x8];
    CinematicSlot slots[5];   /* +0x08..+0x2F */
    s32           writeCursor;/* +0x30 */
    s32           readCursor; /* +0x34 */
    s32           count;      /* +0x38 */
    s32           _pad3C;     /* +0x3C */
    s32           gameStateMode;/* +0x40 */
    s32           active;     /* +0x44 */
} CinematicQueue;

extern u8 g_itemEquippedSlot[0x38];   /* itemId -> active variant slot (EU 0x1395E8) */
extern WeaponDef g_weaponTable[];     /* per-variant def/state table (EU 0x239BA0) */
extern s32 g_cinematicUnlockedFlags[];/* cinematics-watched bitfield (EU 0x1397E8) */
extern s32 g_nLevelExitRequested;     /* in-level frame-loop exit flag (EU 0x1A8C34) */
extern s32 g_nSavePromptPending;      /* show-saving-prompt gate (EU 0x1A7C14) */
/* g_miscExtras is read with the lui/%lo absolute-macro shape; size override so
 * cc1 emits the one-insn symbolic macro (not gp_rel) under -G8. */
__asm__(".extern g_miscExtras, 16");
extern u8  g_miscExtras;              /* misc unlock/extras byte (EU 0x1A7A92) */
extern s32 D_1A9060;                  /* gp small countdown gate (EU twin of USA D_1A8FB0) */
/* Countdown-gate companion latches, read with the lui/%lo absolute-macro shape;
 * size override so cc1 emits the one-insn symbolic macro (not gp_rel) under -G8.
 * EU twins of USA D_1A8FB4 / D_1A8FB8. */
__asm__(".extern D_1A9064, 16");
extern s32 D_1A9064;
__asm__(".extern D_1A9068, 16");
extern s32 D_1A9068;

/* Unit-local / cross-unit callees (EU addresses). */
void ResetCinematicQueue(CinematicQueue *q);
void func_0028C318(void *p);
void func_0028BF08(void);             /* HUD widget-table reseed (EU twin of USA func_0028BF80) */
void func_0029D670(s32 a, s32 b);     /* HUD subsystem enable (EU twin of USA func_0029DB10) */
void func_002B1858(s32 a, s32 b, s32 c); /* HUD element register (EU twin of USA func_002B1B48) */

/* func_002887C8(itemId): EU twin of USA func_002888D8 - advance an item to its
 * next weapon variant. Reads the active variant's nextVariantSlot; when set,
 * clamps g_weaponXp[itemId] up to the variant's XP threshold (raw +0x6C << 5;
 * the EU WeaponDef typedef pads over +0x6C, so read raw), repoints
 * g_itemEquippedSlot[itemId] at the next variant, and tops g_weaponAmmo[itemId]
 * up to the new variant's ammoCapacity (+0x8E) when that variant exists.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_002888D8: named globals only (g_itemEquippedSlot,
 * g_weaponTable, g_weaponXp, g_weaponAmmo), delta 0 in the .s reloc names. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002887C8);
#else
extern s32 g_weaponXp[0x38];    /* per-item XP/upgrade accumulator */
extern s32 g_weaponAmmo[0x38];  /* per-item current ammo */
void func_002887C8(s32 itemId) {
    u8  slot = g_itemEquippedSlot[itemId];
    s16 next = g_weaponTable[slot].nextVariantSlot;
    s32 threshold;

    if (next == 0) {
        return;
    }
    threshold = *(s32 *)((u8 *)&g_weaponTable[slot] + 0x6C);  /* xpThreshold */
    if (threshold >= 0) {
        threshold <<= 5;
        if (g_weaponXp[itemId] < threshold) {
            g_weaponXp[itemId] = threshold;
        }
    }
    g_itemEquippedSlot[itemId] = (u8)next;
    if (g_weaponTable[next & 0xFF].exists != 0) {
        g_weaponAmmo[itemId] = g_weaponTable[next & 0xFF].ammoCapacity;
    }
}
#endif

/* GetWeaponUpgradeLevel(itemId): count how many variant-upgrade steps the item
 * has taken - walk prevVariantSlot (+0x4C) back to the base variant, then count
 * nextVariantSlot (+0x4A) steps forward. Matching arm stays INCLUDE_ASM; #else
 * is the structure model. Word-verified vs USA GetWeaponUpgradeLevel: named
 * global g_weaponTable only (delta 0 in the reloc names); no data-lane shift. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", GetWeaponUpgradeLevel);
#else
s32 GetWeaponUpgradeLevel(s32 itemId) {
    s32 slot = itemId;
    s32 count = 0;
    if (g_weaponTable[slot].prevVariantSlot != 0) {
        do {
            slot = g_weaponTable[slot].prevVariantSlot;
        } while (g_weaponTable[slot].prevVariantSlot != 0);
    }
    if (g_weaponTable[slot].nextVariantSlot != 0) {
        do {
            slot = g_weaponTable[slot].nextVariantSlot;
            count++;
        } while (g_weaponTable[slot].nextVariantSlot != 0);
    }
    return count;
}
#endif

/* SetWeaponUpgradeSlot(itemId, level): point the item's active variant at the
 * variant `level` steps above its base. Bails (returns 0) when the item's current
 * GetWeaponUpgradeLevel is below `level`; else walks prevVariantSlot (+0x4C) back
 * to the base, steps `level` nextVariantSlot (+0x4A) links forward, writes that
 * slot into g_itemEquippedSlot[itemId] and returns 1. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * SetWeaponUpgradeSlot: jal GetWeaponUpgradeLevel (kept name), named globals
 * g_weaponTable / g_itemEquippedSlot (delta 0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", SetWeaponUpgradeSlot);
#else
s32 SetWeaponUpgradeSlot(s32 itemId, s32 level) {
    s32 slot = itemId;
    s32 i;

    if (GetWeaponUpgradeLevel(slot) < level) {
        return 0;
    }
    if (g_weaponTable[slot].prevVariantSlot != 0) {
        do {
            slot = g_weaponTable[slot].prevVariantSlot;
        } while (g_weaponTable[slot].prevVariantSlot != 0);
    }
    for (i = level; i > 0; i--) {
        slot = g_weaponTable[slot].nextVariantSlot;
    }
    g_itemEquippedSlot[itemId] = (u8)slot;
    return 1;
}
#endif

/* func_002889F8(key): EU twin of USA func_00288B08 - copy the 0x30-byte key
 * table at D_1A8BC0 onto the stack and linear-scan it for `key`; return 1 when
 * present before the zero terminator, else 0 (empty table or terminator).
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified
 * vs USA func_00288B08: key table D_1A8B10 -> EU D_1A8BC0 (delta +0xB0, not a
 * data-lane shift - a distinct region offset); memcpy inlined as ldl/ldr. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002889F8);
#else
extern u8 D_1A8BC0[];  /* 0x30-byte, zero-terminated s32 key table (EU) */
s32 func_002889F8(s32 key) {
    s32 table[12];
    s32 *p;

    memcpy(table, D_1A8BC0, sizeof(table));

    if (table[0] == 0) {
        return 0;
    }
    for (p = table;;) {
        if (*p == key) {
            return 1;
        }
        p++;
        if (*p == 0) {
            return 0;
        }
    }
}
#endif

/* func_00288AA0(key): EU twin of USA func_00288BB0 - return 1 if `key` is the
 * first s16 of any 0xA-stride record (after a 6-byte header, -1 sentinel) in
 * either D_00259F58 or D_259CE0, else 0. Matching arm stays INCLUDE_ASM; #else
 * is the structure model. Word-verified vs USA func_00288BB0: D_259F38 ->
 * EU D_00259F58 (delta +0x20), D_259CC0 -> EU D_259CE0 (delta +0x20) - both
 * distinct region offsets, neither the +0x80 data lane. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00288AA0);
#else
extern u8 D_00259F58[];  /* EU: header + 0xA-stride {s16 key,...}, -1 sentinel */
extern u8 D_259CE0[];    /* EU: same layout as D_00259F58 */
s32 func_00288AA0(s32 key) {
    s16 *p;
    for (p = (s16 *)(D_00259F58 + 6); *p != -1; p = (s16 *)((u8 *)p + 0xA)) {
        if ((s32)*p == key) {
            return 1;
        }
    }
    for (p = (s16 *)(D_259CE0 + 6); *p != -1; p = (s16 *)((u8 *)p + 0xA)) {
        if ((s32)*p == key) {
            return 1;
        }
    }
    return 0;
}
#endif

/* func_00288B20(itemId): EU twin of USA func_00288C30 - register an item pickup
 * in the 8-slot recent-items table (g_equippedItemSlots, EU 0x1A7438, reached
 * via the region anchor D_001A7308+0x130). No-op unless func_00288AA0 accepts
 * the item; skipped if already in the D_25E0C8 record table, or its variant's
 * g_weaponTable +0xC field is nonzero, or func_002889F8 reports it present; else
 * stored in the first empty/matching slot (slot 0 fast path). Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_00288C30:
 * func_00288BB0 -> EU func_00288AA0, func_00288B08 -> EU func_002889F8,
 * D_25E308 -> EU D_25E0C8 (delta -0x240), g_equippedItemSlots addressed as
 * D_001A7308+0x130 (EU anchor form; USA used the named symbol). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00288B20);
#else
extern u8 D_25E0C8[];    /* EU: stride-0xA record table, s16 id at +0x6, -1 term */
extern u8 D_001A7308[];  /* EU region data anchor (g_equippedItemSlots at +0x130) */
void func_00288B20(s32 itemId) {
    s16 *id;
    s32  variantSlot;
    s32  slot;
    s32 *slots;

    if (func_00288AA0(itemId) == 0) {
        return;
    }

    /* already listed in the D_25E0C8 table -> nothing to do */
    id = (s16 *)(D_25E0C8 + 0x6);
    if (*id != -1) {
        for (;;) {
            if (itemId == *id) {
                return;
            }
            id = (s16 *)((u8 *)id + 0xA);
            if (*id == -1) {
                break;
            }
        }
    }

    variantSlot = g_itemEquippedSlot[itemId];
    if (*(s32 *)((u8 *)&g_weaponTable[variantSlot] + 0xC) != 0) {
        return;
    }
    if (func_002889F8(itemId) != 0) {
        return;
    }

    slots = (s32 *)(D_001A7308 + 0x130);   /* g_equippedItemSlots[8] */
    if (slots[0] == 0 || slots[0] == itemId) {
        slot = 0;
    } else {
        slot = 1;
        for (;;) {
            if (slot >= 8) {
                return;  /* no free slot */
            }
            if (slots[slot] == 0 || slots[slot] == itemId) {
                break;
            }
            slot++;
        }
    }
    slots[slot] = itemId;
}
#endif

/* GiveInventoryItem(itemId): grant an inventory item. If not already owned:
 * mark it owned (g_inventoryOwned) + newly-acquired (g_inventoryNewFlag), and -
 * when the active variant sells ammo (+0x88) and the player has none - top ammo
 * up to the variant's pickup amount (raw +0x92). Then register it
 * (AddItemToInventoryOrder + func_00288B20). Special case: granting the wrench
 * (0x10) while vendor upgrades are unlocked snaps it to upgrade level 2 and
 * clears D_139928. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA GiveInventoryItem: named globals g_inventoryOwned /
 * g_itemEquippedSlot / g_inventoryNewFlag / g_weaponTable / g_weaponAmmo
 * (delta 0); jal func_00288C30 -> EU func_00288B20; D_1398A8 -> EU D_139928
 * (delta +0x80 - the inventory/flag data lane); AddItemToInventoryOrder /
 * IsVendorUpgradesUnlocked / SetWeaponUpgradeSlot keep their names. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", GiveInventoryItem);
#else
extern u8  g_inventoryOwned[];    /* itemId -> owned flag */
extern u8  g_inventoryNewFlag[];  /* itemId -> "newly acquired" flag */
extern s32 g_weaponAmmo[0x38];    /* per-item current ammo */
extern s32 D_139928;              /* cleared when the wrench upgrade is granted */
extern s32 AddItemToInventoryOrder(s32 itemId);  /* defined later in this unit */
extern s32 IsVendorUpgradesUnlocked(void);       /* defined later in this unit */
void GiveInventoryItem(s32 itemId) {
    WeaponDef *w;
    u8 slot;

    if (g_inventoryOwned[itemId] != 0) {
        return;                            /* already owned - no-op */
    }
    slot = g_itemEquippedSlot[itemId];
    g_inventoryNewFlag[itemId] = 1;
    g_inventoryOwned[itemId] = 1;
    w = &g_weaponTable[slot];
    if (w->sellsAmmoFlag != 0 && g_weaponAmmo[itemId] == 0) {
        g_weaponAmmo[itemId] = *(u16 *)((u8 *)w + 0x92);   /* variant pickup ammo */
    }
    AddItemToInventoryOrder(itemId);
    func_00288B20(itemId);
    if (itemId == 0x10 && IsVendorUpgradesUnlocked() != 0) {
        SetWeaponUpgradeSlot(0x10, 2);
        D_139928 = 0;
    }
}
#endif

/* AddItemToInventoryOrder(itemId): place itemId into the inventory quick-select
 * order list (g_inventoryOrder, bounded by D_1A7C10). Item must exist, be at upgrade
 * level 0, and either have its +0x80 flag set or sell ammo. Scans the order list for
 * an existing entry (low 6 bits) or the first free (0xFF) slot and writes
 * itemId | (owned ? 0x40 : 0). Returns 1 when written, else 0. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * AddItemToInventoryOrder: named globals kept; loop bound USA D_1A7B90 -> EU
 * D_1A7C10 (+0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", AddItemToInventoryOrder);
#else
s32 AddItemToInventoryOrder(s32 itemId) {
    extern u8 g_inventoryOwned[];   /* itemId -> owned flag */
    extern u8 g_inventoryOrder[];   /* quick-select order list */
    extern u8 D_1A7C10;             /* one-past-end of g_inventoryOrder (EU; USA D_1A7B90) */
    WeaponDef *w = &g_weaponTable[g_itemEquippedSlot[itemId]];
    u8 *order;
    u8  owned;

    if (w->exists == 0 || w->upgradeLevel != 0) {
        return 0;
    }
    if (*(s32 *)((u8 *)w + 0x80) == 0 && w->sellsAmmoFlag == 0) {
        return 0;
    }
    owned = g_inventoryOwned[itemId];
    for (order = g_inventoryOrder; order < &D_1A7C10; order++) {
        u8 entry = *order;
        if ((entry & 0x3F) == itemId) {
            *order = (u8)(itemId | (owned != 0 ? 0x40 : 0));
            return 1;
        }
        if (entry == 0xFF) {
            *order = (u8)(itemId | (owned != 0 ? 0x40 : 0));
            return 1;
        }
    }
    return 0;
}
#endif

/* Linear-scan the 0x38 inventory item slots for the weapon variant whose
 * nameStringId matches `name`; return that variant's `exists` field, else 0. */
s32 FindWeaponSlotByName(s32 name) {
    s32 i = 0;
    do {
        u8 slot = g_itemEquippedSlot[i];
        if (g_weaponTable[slot].nameStringId == name) {
            return g_weaponTable[slot].exists;
        }
        i++;
    } while (i < 0x38);
    return 0;
}

/* func_00288E20(itemId): EU twin of USA func_00288F30 - resolve and fire the
 * "acquired / upgraded" announcement for an inventory item. Arms a subtitle line
 * for special ids (0xA -> {0x9F8,0x4F}, 0x1B -> {0x9F7,0x4E}, else the generic
 * "got item" line {0x9AA,1} when the tutorial-health flag g_health+0x678 is
 * clear), then resolves an announcement text id from the per-upgrade-level
 * {itemId,textId} pair-lists D_1A8BF0[level] (-1 terminated), with item 0xA
 * overridden to a level-based id (0x1296/0x1297). Dispatches the id (or 0) via
 * func_002B1588. The original copies D_1A8BF0 into a stack buffer via ldl/ldr
 * before indexing; a direct index is equivalent.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00288F30: subtitle callee func_00289840->func_00289730,
 * text dispatch func_002B1880->func_002B1588, list table D_1A8B40->D_1A8BF0
 * (+0xB0); g_itemEquippedSlot/g_weaponTable/g_health delta 0 in the .s relocs. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00288E20);
#else
s32 func_00288E20(s32 itemId) {
    extern s32 func_00289730(s32 textIndex, s32 voiceHandle);
    extern s32 func_002B1588(s32 stringId, s32 arg);
    extern s32 D_1A8BF0[]; /* [upgradeLevel 0..5] -> ptr to {itemId,textId,...,-1} list */
    extern s32 g_health;
    s32 *list;
    s32 textId = -1;
    u8 slot, level;

    if (itemId == 0xA) {
        func_00289730(0x9F8, 0x4F);
    } else if (itemId == 0x1B) {
        func_00289730(0x9F7, 0x4E);
    } else if (*(u16 *)((u8 *)&g_health + 0x678) == 0) {
        func_00289730(0x9AA, 0x1);
    }

    slot = g_itemEquippedSlot[itemId];
    level = g_weaponTable[slot].upgradeLevel;
    list = (level < 6) ? (s32 *)D_1A8BF0[level] : (s32 *)0;
    if (list != 0) {
        s32 *p = list;
        while (*p != -1) {
            if (*p == itemId) {
                textId = p[1];
                break;
            }
            p += 2;
        }
    }

    if (itemId == 0xA) {
        level = g_weaponTable[g_itemEquippedSlot[0xA]].upgradeLevel;
        if (level >= 2) textId = 0x1296;
        if (level >= 3) textId = 0x1297;
        if (level >= 4) textId = 0x1296;
        if (level >= 5) textId = 0x1297;
    }

    return func_002B1588(textId < 0 ? 0 : textId, -1);
}
#endif

/* IsItemUnlockedAtProgress(itemId, progress): whether `itemId` may appear in the
 * vendor at the given story-progress level. Item 9 is a special case (unlocked
 * once the vendor upgrade tier is). Otherwise it scans the {itemId, minProgress}
 * gate table D_2403C0 (stride 8, -2 sentinel): an item is unlocked when it has a
 * gate entry whose minProgress it has reached AND progress is still in the early
 * band (< 0x15). Returns 0 otherwise.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA IsItemUnlockedAtProgress: gate table D_240340->D_2403C0
 * (+0x80); IsVendorUpgradesUnlocked keeps its name (delta 0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", IsItemUnlockedAtProgress);
#else
s32 IsItemUnlockedAtProgress(s32 itemId, s32 progress) {
    extern s32 IsVendorUpgradesUnlocked(void);
    extern s32 D_2403C0[];
    s32 *entry;
    s32 early;

    if (itemId == 9 && IsVendorUpgradesUnlocked() != 0) {
        return 1;
    }
    if (D_2403C0[0] == -2) {
        return 0;
    }
    early = (progress < 0x15);
    entry = D_2403C0;
    do {
        s32 id   = entry[0];
        s32 minP = entry[1];
        entry += 2;
        if (id == itemId && progress >= minP && early) {
            return 1;
        }
    } while (entry[0] != -2);
    return 0;
}
#endif

/* func_00289080(key): EU twin of USA func_00289190 - return 1 if `key` appears as
 * the first word of any {key,_} pair in the gate table D_2403C0 (stride 8, -2
 * sentinel), else 0. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00289190: gate table D_240340->D_2403C0 (+0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00289080);
#else
s32 func_00289080(s32 key) {
    extern s32 D_2403C0[];
    s32 *p = D_2403C0;
    s32 cur = *p;
    if (cur != -2) {
        do {
            cur = *p;
            p += 2;
            if (key == cur) {
                return 1;
            }
            cur = *p;
        } while (cur != -2);
    }
    return 0;
}
#endif

/* UpgradeWeaponToMax(itemId): select the item's "fully upgraded" weapon variant
 * and zero its accumulated XP. Returns 0 (no change) when the item's currently
 * equipped variant does not exist; otherwise picks an upgrade level by item id
 * (a 0x2A-entry dispatch over itemId-0xC: most items resolve to level 2, a
 * handful to level 1; out-of-range items also resolve to level 2), calls
 * SetWeaponUpgradeSlot(itemId, level), and on success resets g_weaponXp[itemId]
 * to 0. Returns SetWeaponUpgradeSlot's result.
 * Matching arm stays INCLUDE_ASM; #else is the structure model (the original's
 * jtbl_0026C490_text jump-dispatch is modelled as a value table; table is
 * region-invariant gameplay data, verbatim from the USA #else twin).
 * Word-verified vs USA UpgradeWeaponToMax: SetWeaponUpgradeSlot + g_itemEquippedSlot
 * + g_weaponTable + g_weaponXp keep their names; the jtbl (-0x240) is referenced
 * only by the matching arm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", UpgradeWeaponToMax);
#else
s32 UpgradeWeaponToMax(s32 itemId) {
    extern s32 SetWeaponUpgradeSlot(s32 itemId, s32 level);
    extern s32 g_weaponXp[0x38];
    /* itemId-0xC -> target level. Two targets only - most entries pick level 2,
     * indices {0,2,5,6,41} pick level 1 (mirrors the USA jtbl target set). */
    static const u8 kUpgradeLevelByItem[0x2A] = {
        1, 2, 1, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
    };
    s32 slot = g_itemEquippedSlot[itemId];
    s32 idx;
    s32 level;
    s32 ok;

    if (g_weaponTable[slot].exists == 0) {
        return 0;
    }
    idx = itemId - 0xC;
    level = ((u32)idx < 0x2A) ? kUpgradeLevelByItem[idx] : 2;

    ok = SetWeaponUpgradeSlot(itemId, level);
    if (ok != 0) {
        g_weaponXp[itemId] = 0;
    }
    return ok;
}
#endif

/* GetWeaponStatsAtLevel(out, itemId, level): resolve the variant `level` steps
 * up the upgrade chain of weapon `itemId` (walk prevVariantSlot @0x4C back to the
 * base, then up to `level` forward nextVariantSlot @0x4A steps - stopping early at
 * a terminal variant) and, when that variant's upgradeLevel matches `level`, copy
 * its full 0xE0-byte WeaponDef into *out. Returns 1 on a successful copy, else 0
 * (also 0 immediately when level >= 0xFF).
 * Matching arm stays INCLUDE_ASM; #else is the structure model (the copy is an
 * lq/sq 128-bit block move the matcher does not reproduce from struct assignment).
 * Word-verified vs USA GetWeaponStatsAtLevel: g_weaponTable keeps its name
 * (delta 0; no other symbols). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", GetWeaponStatsAtLevel);
#else
s32 GetWeaponStatsAtLevel(WeaponDef *out, s32 itemId, s32 level) {
    s32 slot = itemId;
    s32 count;

    if (level >= 0xFF) {
        return 0;
    }
    /* walk prevVariantSlot back to the base variant */
    if (g_weaponTable[slot].prevVariantSlot != 0) {
        do {
            slot = g_weaponTable[slot].prevVariantSlot;
        } while (g_weaponTable[slot].prevVariantSlot != 0);
    }
    /* step up to `level` nextVariantSlot links forward, halting at a terminal */
    if (level > 0 && g_weaponTable[slot].nextVariantSlot != 0) {
        count = 0;
        for (;;) {
            count++;
            slot = g_weaponTable[slot].nextVariantSlot;
            if (count >= level) {
                break;
            }
            if (g_weaponTable[slot].nextVariantSlot == 0) {
                break;
            }
        }
    }
    if ((s32)g_weaponTable[slot].upgradeLevel != level) {
        return 0;
    }
    *out = g_weaponTable[slot];   /* full 0xE0-byte WeaponDef copy */
    return 1;
}
#endif

/* func_00289288(size, out): EU twin of USA func_00289398 - compute the scene-arena
 * address that would remain after carving `size` bytes off the decompressed-scene
 * region, written to *out. Refuses sizes over 0x40000 (writes 0, returns -1);
 * otherwise writes (decompressBase + arenaCursor) - size and returns 0.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00289398: EU splat resolves the two scene globals
 * under the g_nVendorBuyQuantity anchor - g_sceneDecompressBase ->
 * g_nVendorBuyQuantity+0x8C84 (EU 0x1BAECC, USA 0x1BAE4C +0x80) and
 * g_sceneArenaCursor -> g_nVendorBuyQuantity+0x68 (EU 0x1B22B0, USA 0x1B2230 +0x80).
 * Standard +0x80 EU data lane, no code diff. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00289288);
#else
s32 func_00289288(s32 size, s32 *out) {
    extern s32 g_nVendorBuyQuantity;
    if ((u32)0x40000 < (u32)size) {
        *out = 0;
        return -1;
    }
    *out = (*(s32 *)((u8 *)&g_nVendorBuyQuantity + 0x8C84) +
            *(s32 *)((u8 *)&g_nVendorBuyQuantity + 0x68)) - size;
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002892C8);

/* ResetCinematicQueue(q): clear the cinematic ring buffer - zero the write/read
 * cursors and count, poison the 5 slots (0x28 bytes) with 0xCD, clear the active
 * flag and set the queue's game-state mode to 1.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA ResetCinematicQueue: no external data symbols; jal memset
 * (SDK, region-invariant). Field offsets writeCursor@0x30/readCursor@0x34/
 * count@0x38/slots@0x8/gameStateMode@0x40/active@0x44 confirmed in the EU .s. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", ResetCinematicQueue);
#else
void ResetCinematicQueue(CinematicQueue *q) {
    q->writeCursor = 0;
    q->readCursor = 0;
    q->count = 0;
    memset(q->slots, 0xCD, sizeof(q->slots));
    q->active = 0;
    q->gameStateMode = 1;
}
#endif

/*
 * EU twin of USA DequeueCinematic (0x289428). Pop the slot at the read cursor
 * into the outId/outFlags out-params, decrement the count (clearing `active` at 0),
 * clear both slot words to -1, advance the read cursor (wrap at 5), return 1 —
 * unless the queue was empty (return 0). `readCursor` is re-read per slot access
 * to reproduce the -G8 -fno-gcse build's kept loads. No data globals → the USA C
 * ports verbatim.
 */
s32 func_00289318(CinematicQueue *q, s32 *outId, s32 *outFlags) {
    if (q->count == 0) {
        return 0;
    }
    if (--q->count == 0) {
        q->active = 0;
    }
    *outId = q->slots[q->readCursor].id;
    *outFlags = q->slots[q->readCursor].flag;
    q->slots[q->readCursor].id = -1;
    q->slots[q->readCursor].flag = -1;
    q->readCursor = (q->readCursor + 1 != 5) ? (q->readCursor + 1) : 0;
    return 1;
}

/*
 * EU twin of USA EnqueueCinematic (0x2894C8). Push cinematic `id` at the tail:
 * refuse (return 0) when full (count == 5) or currently playing (active != 0),
 * else store {id, isMovie=(id in [0x20,0xB0])} at the write slot, bump the count,
 * advance the write cursor (wrap at 5), return 1. `writeCursor` re-read per access
 * to match the kept loads. No data globals → the USA C ports verbatim.
 */
s32 func_002893B8(CinematicQueue *q, s32 id) {
    if (q->count == 5 || q->active != 0) {
        return 0;
    }
    q->slots[q->writeCursor].id = id;
    q->slots[q->writeCursor].flag = ((u32)(id - 0x20) < 0x91) ? 1 : 0;
    q->count = q->count + 1;
    q->writeCursor = (q->writeCursor + 1 != 5) ? (q->writeCursor + 1) : 0;
    return 1;
}

/* Forward to ResetCinematicQueue (keeps its own frame; the empty-asm guard
 * suppresses cc1's sibling-call so the original jal+frame is reproduced). */
void func_00289430(CinematicQueue *q) {
    ResetCinematicQueue(q);
    __asm__ __volatile__("");
}

/* Mark every cinematic queued between the read and write cursors (id < 0xB1)
 * as watched in g_cinematicUnlockedFlags, walking the ring forward mod 5. */
void func_00289450(CinematicQueue *q) {
    s32 cursor = q->readCursor;
    if (cursor != q->writeCursor) {
        do {
            u32 id = q->slots[cursor].id;
            if (id < 0xB1) {
                g_cinematicUnlockedFlags[id >> 2] |= 1 << (id & 0x1F);
            }
            cursor = (cursor + 1 != 5) ? (cursor + 1) : 0;
        } while (cursor != q->writeCursor);
    }
}

/* func_002894D0(q): EU twin of USA func_002895E0 - full cinematic-queue reset.
 * Marks every queued-but-unwatched cinematic as watched (func_00289450), zeroes
 * the head word, then clears the queue via func_00289430. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_002895E0:
 * USA func_00289560 -> EU func_00289450, USA func_00289540 -> EU func_00289430
 * (both defined file-scope earlier). No data globals. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002894D0);
#else
void func_002894D0(CinematicQueue *q) {
    func_00289450(q);
    *(s32 *)q = 0;
    func_00289430(q);
}
#endif

/* func_00289500(q): EU twin of USA StartCinematicFromQueue. If the queue has a
 * pending cinematic, mark it active, dequeue {id, flag} (func_00289318, the EU
 * DequeueCinematic twin), pick a game-state mode (1 or 2 from g_nGameState) and
 * request the matching game-state transition (RequestGameStateChange). flag==0
 * requests the level-cinematic transition (id in arg4); flag==1 the standard one
 * (id in arg3); any other flag skips the request. On a failed (<0) request the
 * queue is flushed (func_00289430). Returns 1 only when the request returned
 * exactly 0 (or no request was made). Matching arm stays INCLUDE_ASM; #else is
 * the structure model. Word-verified vs USA StartCinematicFromQueue:
 * DequeueCinematic -> func_00289318 (file-scope), USA func_00289540 ->
 * func_00289430, g_nGameState gp_rel + RequestGameStateChange kept named. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00289500);
#else
s32 func_00289500(CinematicQueue *q) {
    extern s32 g_nGameState;
    extern s32 RequestGameStateChange(s32 a, s32 b, s32 c, s32 d, s32 e);
    s32 id, flag, mode, result;

    if (q->count == 0) {
        return 0;
    }
    q->_pad3C = 0;
    q->active = 1;
    id = 0;
    flag = 0;
    func_00289318(q, &id, &flag);
    mode = (1u < (u32)(g_nGameState - 1)) ? 1 : 2;
    q->gameStateMode = mode;
    result = 0;
    if (flag == 0) {
        result = RequestGameStateChange(1, mode, 0, id, 0);
    } else if (flag == 1) {
        result = RequestGameStateChange(2, mode, id, 0, 0);
    }
    if (result < 0) {
        func_00289430(q);
    }
    return (u32)result < 1;
}
#endif

/* RequestLevelExit(destination, doSave): raise the in-level exit flag
 * (g_nLevelExitRequested) and record the destination; when destination == -1
 * also resets a transition latch (D_139460+0x17C / +0x18) and clears a save/load
 * status bit (g_nSaveLoadStatusCode+0x4 and ~0x200). When doSave is set, clears
 * the save-prompt gate and commits a progress checkpoint. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA RequestLevelExit:
 * g_nLevelExitRequested/g_nLevelExitDestination gp_rel (kept named), USA D_1393E0
 * -> EU D_139460 (+0x80), g_nSaveLoadStatusCode / ClearSavePromptPending /
 * CommitProgressCheckpoint kept named. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", RequestLevelExit);
#else
void RequestLevelExit(s32 destination, s32 doSave) {
    extern s32  g_nLevelExitDestination;   /* EU 0x1B1680 */
    extern u8   D_139460[];                /* EU twin of USA D_1393E0 (transition latch blob) */
    extern u8   g_nSaveLoadStatusCode[];   /* save/load popup status block */
    extern void CommitProgressCheckpoint(s32 a, s32 destination);
    void ClearSavePromptPending(void);     /* defined below in this unit */

    g_nLevelExitRequested = 1;
    g_nLevelExitDestination = destination;
    if (destination == -1) {
        if (*(s32 *)(D_139460 + 0x17C) != 0) {
            *(s32 *)(D_139460 + 0x17C) = 0;
        }
        if (*(s16 *)(D_139460 + 0x18) >= 0) {
            *(s16 *)(D_139460 + 0x18) = (s16)destination;
        }
        *(s32 *)(g_nSaveLoadStatusCode + 0x4) &= ~0x200;
    }
    if (doSave != 0) {
        ClearSavePromptPending();
        CommitProgressCheckpoint(0, g_nLevelExitDestination);
    }
}
#endif

/* Non-zero while the main in-level frame loop has been asked to exit. */
s32 IsLevelExitRequested(void) {
    return g_nLevelExitRequested != 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00289658);

/* True once the vendor's weapon-upgrade tier has been unlocked. */
s32 IsVendorUpgradesUnlocked(void) {
    return g_miscExtras != 0;
}

/* Clear the "show saving prompt" gate (called on level-exit-with-save). */
void ClearSavePromptPending(void) {
    g_nSavePromptPending = 0;
}

/* Raise the "show saving prompt" gate. */
void SetSavePromptPending(void) {
    g_nSavePromptPending = 1;
}

/* Read the "show saving prompt" gate. */
s32 GetSavePromptPending(void) {
    return g_nSavePromptPending;
}

void func_002896A0(void) {
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002896A8);

/* Subtitle state machine record (EU 0x254E78). Region twin of the USA
 * SubtitleState (see src/usa/text/188858.c). +0x24/+0x28 hold the pending/showing
 * line index/handle, +0x40/+0x44 the per-line phase timer/flag. */
typedef struct SubtitleState {
    s32 state;          /* +0x00 */
    s32 _pad04;         /* +0x04 */
    u8  _pad08[0x18];
    s32 entryIndex;     /* +0x20 */
    s32 showingIndex;   /* +0x24: pending/showing line index (-1 = none) */
    s32 showingHandle;  /* +0x28: voice/clip handle of the shown line */
    s32 tableCount;     /* +0x2C */
    u8  _pad30[0x10];
    s32 phaseTimer;     /* +0x40 */
    s32 phaseFlag;      /* +0x44 */
} SubtitleState;
#ifdef TARGET_NATIVE
extern SubtitleState g_subtitleState;          /* EU 0x254E78 */
#endif

/* The voice/cinematic-clip control block lives at g_saveImageArea + 0x1000
 * (EU 0x1A6428). +0x24 is this area's current clip id; +0x68 a "voice busy"
 * gate. EU twin of the USA AreaClipState. */
typedef struct AreaClipState {
    u8  _pad00[0x24];
    s32 currentClip;   /* +0x24 */
    u8  _pad28[0x40];
    s32 voiceBusy;     /* +0x68 */
} AreaClipState;
#ifdef TARGET_NATIVE
extern u8  g_saveImageArea[];                  /* EU 0x1A5428 */
#endif

/* Per-line subtitle/voice timing table at g_health+0x66C (EU 0x18C9D8, stride
 * 0xC, indexed by voice handle; +0x0 u16 duration, 0xFFFF = "no line"). g_health
 * is the EU region anchor the displacement folds onto. */
#ifdef TARGET_NATIVE
extern s32 g_health;                           /* EU 0x18C36C */
extern s32 g_nGameState;                        /* EU 0x1A8C60 */
extern s32 g_gameTime;                          /* global frame counter (EU gp anchor g_nLevelExitDestination+0x8, 0x1B1688) */
#endif

/* func_00289730(textIndex, voiceHandle): EU twin of USA func_00289840 — arm a
 * pending subtitle line. Byte-identical region-agnostic logic; see
 * src/usa/text/188858.c for the recovered behaviour + the cmp-oracle
 * (cmp_188858_text.c, 95/95 on real R5900). Only the extern global addresses are
 * region-shifted (g_subtitleState 0x254E78, g_saveImageArea+0x1000 0x1A6428,
 * g_health+0x66C 0x18C9D8, g_gameTime via the EU gp anchor
 * g_nLevelExitDestination+0x8). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00289730);
#else
s32 func_00289730(s32 textIndex, s32 voiceHandle) {
    u16 *lineTable = (u16 *)((u8 *)&g_health + 0x66C);
    s32 pending;

    if (g_subtitleState.state != 0) {
        return 0;
    }
    pending = g_subtitleState.showingIndex;
    if (pending != -1) {
        return 0;
    }
    if (((AreaClipState *)&g_saveImageArea[0x1000])->voiceBusy != 0) {
        return 0;
    }
    if (((AreaClipState *)&g_saveImageArea[0x1000])->currentClip != pending) {
        return 0;
    }
    if (lineTable[voiceHandle * 6] == 0xFFFF) {
        return 0;
    }
    if (g_nGameState == 6 || g_gameTime < 6) {
        return 0;
    }
    g_subtitleState.showingIndex = textIndex;
    g_subtitleState.showingHandle = voiceHandle;
    g_subtitleState.phaseTimer = 0;
    g_subtitleState.phaseFlag = 0;
    return 1;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002897C8);

/* FindTextTableEntry(textId): linear-search the active localized text table for
 * the entry whose id (+0x4) matches textId; return its index, or -1 when the
 * table is empty or has no match. First element is special-cased (returns 0),
 * the rest scanned 1..tableCount-1. Matching arm stays INCLUDE_ASM; #else is the
 * op-for-op faithful structure model. Word-verified vs USA FindTextTableEntry:
 * g_pActiveTextTable and g_subtitleState kept named; no callees, no data-lane
 * deltas. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", FindTextTableEntry);
#else
s32 FindTextTableEntry(s32 textId) {
    typedef struct { char *str; s32 id; s32 voice; s32 _pad; } TextEntry;
    extern void *g_pActiveTextTable;
    s32 result = -1;
    s32 i = 0;

    if (g_subtitleState.tableCount > 0) {
        TextEntry *table = (TextEntry *)g_pActiveTextTable;
        if (table[0].id == textId) {
            return 0;
        }
        for (i = 1; i < g_subtitleState.tableCount; i++) {
            if (table[i].id == textId) {
                result = i;
                break;
            }
        }
    }
    return result;
}
#endif

/* GetLocalizedString(textId): resolve textId to its localized C string via
 * FindTextTableEntry; on a hit return g_pActiveTextTable[idx].str. On a miss
 * return the cached fallback string D_1A8DC8 for the sentinel id 0x9C40,
 * otherwise a pointer to the empty-string buffer D_1A8DD8. Returns char*
 * (consumed as char* by callers in other units). Matching arm stays INCLUDE_ASM;
 * #else is the structure model. Word-verified vs USA GetLocalizedString:
 * FindTextTableEntry / g_pActiveTextTable kept named; USA D_1A8D18 -> EU D_1A8DC8
 * and USA D_1A8D28 -> EU D_1A8DD8 (+0xB0); sentinel 0x9C40 region-invariant. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", GetLocalizedString);
#else
char *GetLocalizedString(s32 textId) {
    typedef struct { char *str; s32 id; s32 voice; s32 _pad; } TextEntry;
    extern void *g_pActiveTextTable;
    extern char *D_1A8DC8;   /* EU twin of USA D_1A8D18 - cached fallback string */
    extern u8    D_1A8DD8;   /* EU twin of USA D_1A8D28 - empty-string buffer */
    s32 idx = FindTextTableEntry(textId);

    if (idx >= 0) {
        return ((TextEntry *)g_pActiveTextTable)[idx].str;
    }
    if (textId == 0x9C40) {
        return D_1A8DC8;
    }
    return (char *)&D_1A8DD8;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00289948);

/* BeginSubtitleDisplay(): start showing the current subtitle line - arm the
 * subtitle state machine (state=1, animStep=0), play the subtitle blip
 * (func_002E6C28, EU PlayGlobalSound twin) when D_1A7C1C is set, then measure the
 * active line text and lay out its on-screen box. func_00280BA8 builds the layout
 * descriptor, func_00280AC8 flows the text; the measured half-width/half-height
 * come back at layout+0xC/+0xE. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA BeginSubtitleDisplay: g_subtitleState /
 * g_pActiveTextTable kept named; USA PlayGlobalSound -> EU func_002E6C28; the
 * layout/text pair is ADDRESS-SWAPPED across regions - USA func_00280C98 (9-arg
 * layout) -> EU func_00280BA8, USA func_00280BB8 (4-arg text) -> EU func_00280AC8;
 * USA D_1A7B9C -> EU D_1A7C1C (+0x80), USA g_bPalMode -> EU D_1A7C18 (+0x80),
 * USA D_1A8D20 -> EU D_1A8DD0 (+0xB0), USA g_screenHeight -> EU D_001A7308+0xBC
 * (+0x80 anchor form), func_001157AC (SDK strlen) region-invariant. The boxPosY
 * table read D_1A8DD0[D_1A7C18] is the genuine PAL/NTSC axis; code identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", BeginSubtitleDisplay);
#else
void BeginSubtitleDisplay(void) {
    typedef struct { char *str; s32 id; s32 voice; s32 _pad; } TextEntry;
    typedef struct SubBox {
        s32 state;          /* +0x00 */
        s32 animStep;       /* +0x04 */
        s32 boxHalfWidth;   /* +0x08 */
        s32 boxHalfHeight;  /* +0x0C */
        s32 field10;        /* +0x10 */
        s32 boxPosY;        /* +0x14 */
        s32 field18;        /* +0x18 */
        s32 field1C;        /* +0x1C */
        s32 entryIndex;     /* +0x20 */
        s32 showingIndex;   /* +0x24 */
        s32 showingHandle;  /* +0x28 */
        s32 tableCount;     /* +0x2C */
        s32 boxActiveGate;  /* +0x30 */
        u8  _pad34[0xC];
        s32 phaseTimer;     /* +0x40 */
        s32 phaseFlag;      /* +0x44 */
        s32 textPixelWidth; /* +0x48 */
    } SubBox;
    extern void *g_pActiveTextTable;
    extern u16  D_1A7C1C;    /* EU twin of USA D_1A7B9C - subtitle-sound-enabled flag */
    extern s32  D_1A7C18;    /* EU twin of USA g_bPalMode - PAL flag / boxPosY index */
    extern s32  D_1A8DD0[];  /* EU twin of USA D_1A8D20 - PAL/NTSC boxPosY table */
    extern u8   D_001A7308[];/* EU region data anchor; +0xBC = g_screenHeight */
    extern s32  func_002E6C28(s32 id, s32 a, s32 b);   /* EU twin of USA PlayGlobalSound */
    extern void func_00280BA8(s16 *layout, s16 clipX0, s16 clipX1, s16 left, s16 right,
                              s16 anchorX, s16 y, s16 lineHeight, s32 flags); /* USA func_00280C98 */
    extern void func_00280AC8(void *layout, s32 color, const char *text, s32 arg4); /* USA func_00280BB8 */
    extern s32  func_001157AC(const char *s);          /* SDK strlen (region-invariant) */

    SubBox *ss = (SubBox *)&g_subtitleState;
    const char *text;
    s16 measuredW;
    s16 measuredH;
    s32 screenH;
    u8  layout[0x40];

    ss->state = 1;
    ss->animStep = 0;

    if (D_1A7C1C != 0) {
        func_002E6C28(0, 1, 0);
    }

    text = ((TextEntry *)g_pActiveTextTable)[ss->entryIndex].str;

    func_00280BA8((s16 *)layout, 0xF0, 0x1E0, 0x2C, 0x1D4, 0x100, 0x168, 0x10, 7);
    func_00280AC8(layout, 0x80FFA888, text, -1);

    measuredW = *(s16 *)(layout + 0xC);
    measuredH = *(s16 *)(layout + 0xE);

    ss->boxHalfWidth = (measuredW >> 1) + 0xA;
    ss->boxHalfHeight = (measuredH >> 1) + 0x5;
    ss->field10 = 0x100;
    ss->boxPosY = D_1A8DD0[D_1A7C18];
    ss->field1C = 8;
    ss->field18 = 8;
    ss->textPixelWidth = 0;

    if (text != 0) {
        s32 len = func_001157AC(text);
        ss->textPixelWidth = len * 8 - len;   /* strlen * 7 */
    }

    screenH = *(s32 *)(D_001A7308 + 0xBC);
    ss->boxPosY = screenH - 0x3C;
    if (screenH - 0xC < (screenH - 0x3C) + ss->boxHalfHeight) {
        ss->boxPosY = screenH - (ss->boxHalfHeight + 0xC);
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", UpdateSubtitleStateMachine);

void func_0028A468(void) {
}

/* func_0028A470(): EU twin of USA func_0028A4E8 - per-frame subtitle-box renderer
 * and open/close animator. Ticks the phase timer, then bails unless the state
 * machine is live (state!=0), the box is active (boxActiveGate) and subtitles-
 * with-sound is enabled (D_1A7C1C). state (1..7) selects one of five paths
 * (pop-in / idle / grow / full-box+faded-text / shrink-close); each is skipped
 * (-> func_0028A468 no-op) when the box-visible byte D_1A7C1D is 0. The box quad
 * is drawn by func_0027F070 (int) and func_0029BFF0 (float); line text laid out
 * by func_00280BA8 and drawn by func_00280AC8. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. Word-verified vs USA func_0028A4E8: USA D_1A7B9C
 * -> EU D_1A7C1C (+0x80), USA D_1A7B9D -> EU D_1A7C1D (+0x80); USA func_0027F208
 * -> EU func_0027F070, USA func_0029C448 -> EU func_0029BFF0, USA func_0028A4E0
 * -> EU func_0028A468, and the address-swapped layout/text pair USA func_00280C98
 * -> EU func_00280BA8, USA func_00280BB8 -> EU func_00280AC8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028A470);
#else
/* signed divide-by-8 truncating toward zero (the sra-with-bias idiom) */
static s32 div8_trunc(s32 v) {
    return ((v >= 0) ? v : v + 7) >> 3;
}

void func_0028A470(void) {
    typedef struct { char *str; s32 id; s32 voice; s32 _pad; } TextEntry;
    typedef struct SubBox {
        s32 state;          /* +0x00 */
        s32 animStep;       /* +0x04 */
        s32 boxHalfWidth;   /* +0x08 */
        s32 boxHalfHeight;  /* +0x0C */
        s32 field10;        /* +0x10 */
        s32 boxPosY;        /* +0x14 */
        s32 field18;        /* +0x18 */
        s32 field1C;        /* +0x1C */
        s32 entryIndex;     /* +0x20 */
        s32 showingIndex;   /* +0x24 */
        s32 showingHandle;  /* +0x28 */
        s32 tableCount;     /* +0x2C */
        s32 boxActiveGate;  /* +0x30 */
        u8  _pad34[0xC];
        s32 phaseTimer;     /* +0x40 */
        s32 phaseFlag;      /* +0x44 */
        s32 textPixelWidth; /* +0x48 */
    } SubBox;
    extern void *g_pActiveTextTable;
    extern u16  D_1A7C1C;    /* EU twin of USA D_1A7B9C - subtitle-sound flag */
    extern u8   D_1A7C1D;    /* EU twin of USA D_1A7B9D - box-visible byte */
    extern void func_0027F070(s32 y0, s32 y1, s32 x0, s32 x1, s32 h, s32 color); /* USA func_0027F208 */
    extern void func_0029BFF0(s32 mode, s32 color, float y0, float y1, float x0, float x1); /* USA func_0029C448 */
    extern void func_00280BA8(s16 *layout, s16 clipX0, s16 clipX1, s16 left, s16 right,
                              s16 anchorX, s16 y, s16 lineHeight, s32 flags); /* USA func_00280C98 */
    extern void func_00280AC8(void *layout, s32 color, const char *text, s32 arg4); /* USA func_00280BB8 */

    SubBox *ss = (SubBox *)&g_subtitleState;
    s32 state;
    s32 t, y, cx;

    /* phase-timer housekeeping */
    ss->phaseFlag -= 1;
    if (ss->phaseFlag < 0) {
        ss->phaseFlag = 0;
    }
    if (ss->phaseFlag == 0) {
        ss->phaseTimer = 0;
    }

    /* gating */
    state = ss->state;
    if (state == 0) {
        return;
    }
    if (ss->boxActiveGate == 0) {
        return;
    }
    if (D_1A7C1C == 0) {
        return;
    }
    if ((u32)(state - 1) >= 7) {
        return;
    }

    t  = ss->phaseTimer;    /* vertical slide offset */
    y  = ss->boxPosY;       /* centre Y */
    cx = ss->field10;       /* centre X */

    switch (state) {
    case 1: {                                   /* pop-in: fixed 8px inset box */
        if (D_1A7C1D == 0) {
            func_0028A468();
            return;
        }
        ss->field1C = 8;
        ss->field18 = 8;
        func_0027F070(y - 8 + t, y + 8 + t, cx - 8, cx + 8, 0x60, 0x60442D00);
        func_0029BFF0(0x60, 0x55F0C070,
                      (float)((y - 8) + t), (float)((y + 8) + t),
                      (float)(cx - 8), (float)(cx + 8));
        return;
    }

    case 2:                                     /* idle */
        func_0028A468();
        return;

    case 3: {                                   /* grow: extents scaled by k/8 */
        s32 k = ss->animStep;
        s32 hh = ss->boxHalfHeight - 0x20;
        s32 hw = ss->boxHalfWidth - 0x20;
        s32 dy, dx;
        if (D_1A7C1D == 0) {
            func_0028A468();
            return;
        }
        if (k >= 9) k = 8;
        if (k < 0) k = 0;
        dy = div8_trunc(hh * k) + 0x20;
        dx = div8_trunc(hw * k) + 0x20;
        ss->field18 = dx;
        ss->field1C = dy;
        func_0027F070(y - dy + t, y + dy + t, cx - dx, cx + dx, 0x60, 0x60442D00);
        func_0029BFF0(0x60, 0x55F0C070,
                      (float)((y - dy) + t), (float)((y + dy) + t),
                      (float)(cx - dx), (float)(cx + dx));
        return;
    }

    case 4:
    case 5:
    case 6: {                                   /* full box + faded line text */
        s32 hh = ss->boxHalfHeight;
        s32 hw = ss->boxHalfWidth;
        s32 m, color;
        const char *str;
        u8 layout[0x40];
        if (D_1A7C1D == 0) {
            func_0028A468();
            return;
        }
        ss->field18 = hw;
        ss->field1C = hh;
        func_0027F070(y - hh + t, y + hh + t, cx - hw, cx + hw, 0x60, 0x60442D00);
        func_0029BFF0(0x60, 0x55F0C070,
                      (float)((y - hh) + t), (float)((y + hh) + t),
                      (float)(cx - hw), (float)(cx + hw));

        m = ss->animStep;
        if (m >= 5) m = 4;
        if (m < 0) m = 0;
        if (state == 4) {
            color = (s32)(((u32)m << 29) | 0x00FFA888);       /* fade in */
        } else if (state == 6) {
            color = (s32)(((u32)(4 - m) << 29) | 0x00FFA888); /* fade out */
        } else {
            color = (s32)0x80FFA888;                          /* opaque */
        }

        str = ((TextEntry *)g_pActiveTextTable)[ss->entryIndex].str;
        func_00280BA8((s16 *)layout, t + 0xF0, t + 0x1E0, 0x2C, 0x1D4,
                      0x100, y + t, 0x10, 3);
        func_00280AC8(layout, color, str, -1);
        return;
    }

    case 7: {                                   /* shrink/close */
        s32 f1c = ss->field1C;
        s32 f18 = ss->field18;
        s32 k = ss->animStep;
        s32 a, b, h;
        if (D_1A7C1D == 0) {
            func_0028A468();
            return;
        }
        if (k >= 9) k = 8;
        if (k < 0) k = 0;
        a = f1c - div8_trunc((f1c - 8) * k);
        b = f18 - div8_trunc((f18 - 8) * k);
        h = (8 - k) * 0xC;
        func_0027F070(y - a + t, y + a + t, cx - b, cx + b, h, 0x60442D00);
        func_0029BFF0(h, 0x55F0C070,
                      (float)((y - a) + t), (float)((y + a) + t),
                      (float)(cx - b), (float)(cx + b));
        return;
    }
    }
}
#endif

/* func_0028A990(key, column, outValue): EU twin of USA func_0028AA08 -
 * bidirectional key<->value lookup over the D_254EC8 table - 0xAA rows of two s16
 * columns each (row stride 4 bytes). Sign-extend key to s16 and scan rows
 * 0..0xA9, comparing the `column` s16 of each row against key. On the first
 * match, if outValue is non-NULL, write the row's OTHER column through outValue
 * and return the matched row index; else return -1.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs
 * USA func_0028AA08: sole retarget is D_254E48 -> EU D_254EC8 (+0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028A990);
#else
extern s16 D_254EC8[];   /* 0xAA rows of two s16 columns (EU 0x254EC8, USA D_254E48 +0x80) */
s32 func_0028A990(s32 key, s32 column, s16 *outValue) {
    s16 keyHalf = (s16)key;
    s16 *base = D_254EC8;
    s16 *cell = base + column;   /* &row[0].col[column]; advances by 2 s16 per row */
    s32 i = 0;

    do {
        if (*cell == keyHalf) {
            if (outValue != 0) {
                s16 *out = base + (column != 0 ? (i * 2) : (i * 2 + 1));
                *outValue = (s16)(u16)*out;
            }
            return i;
        }
        i++;
        cell += 2;
    } while (i < 0xAA);
    return -1;
}
#endif

/* func_0028A9F8(key): EU twin of USA func_0028AA70 — move the area-data record
 * matching `key` to the MRU end of the recently-used byte list at g_health+0xDFC
 * (length D_1A7C8C). `key` is resolved to its area-data row id via
 * func_0028A990(key, column 1); a -1 result is a no-op. The id is located in the
 * list, the intervening bytes are shifted down to close the gap, and the id is
 * re-appended at the end (an id not yet present is simply appended). Region-
 * agnostic logic; only the length global (D_1A7C8C, +0x80 of USA D_1A7C0C) differs.
 *
 * WALL: single-$31 (sd) frame, a jal gate, a branch-likely scan and an in-place
 * byte-shift loop with the g_health+0xDFC absolute fold and the gp/absolute split
 * on the length global. Left INCLUDE_ASM for the matching build; #else portable. */
#ifdef TARGET_NATIVE
extern s32 D_1A7C8C;             /* EU area-LRU list length (0x1A7C8C) */
#endif
s32 func_0028A990(s32 key, s32 column, s16 *outValue);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028A9F8);
#else
void func_0028A9F8(s32 key) {
    u8 *lru;
    s32 len;
    s32 row;
    s32 i;

    row = func_0028A990(key, 1, 0);
    if (row == -1) {
        return;
    }
    lru = (u8 *)&g_health + 0xDFC;
    len = D_1A7C8C;
    i = 0;
    if (lru[0] != (u8)row) {
        if (len <= 0) {
            goto append;
        }
        for (i = 1; i < len; i++) {
            if (lru[i] == (u8)row) {
                break;
            }
        }
    }
    if (i >= len) {
        goto append;
    }
    for (; i < len - 1; i++) {
        lru[i] = lru[i + 1];
    }
    lru[i] = 0;
    D_1A7C8C = len - 1;
append:
    len = D_1A7C8C;
    lru[len] = (u8)row;
    D_1A7C8C = len + 1;
}
#endif

/* func_0028AAF8(event): EU twin of USA func_0028AB70 - subtitle-event dispatch.
 * For area-transition event 0x17 queue subtitle line (0x9F3, voice 0x4A); for
 * event 0x19 queue line (0xA35, voice 0x8C). Matching arm stays INCLUDE_ASM;
 * #else is the structure model. Word-verified vs USA func_0028AB70: callee
 * func_0028ABC0 -> EU func_0028AB48; event/line/voice immediates identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028AAF8);
#else
void func_0028AB48(s32 id, s32 voice);
void func_0028AAF8(s32 event) {
    if (event == 0x17) {
        func_0028AB48(0x9F3, 0x4A);
    }
    if (event == 0x19) {
        func_0028AB48(0xA35, 0x8C);
    }
}
#endif

/* func_0028AB48(id, voice): EU twin of USA func_0028ABC0 - enqueue a subtitle/voice
 * request into a 7-slot ring in the data block at &g_pActiveTextTable: id ring at
 * +0x8 (s16[7]), voice ring at +0x18 (s16[7]), head index at +0x27, entry count at
 * +0x26. No-op when full (count >= 7) or id already queued. On insert the new entry
 * goes at slot (head+count)%7 and count is incremented. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0028ABC0:
 * g_pActiveTextTable +0x8/+0x18/+0x26/+0x27 offsets identical (anchored block, no
 * data-lane shift). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028AB48);
#else
void func_0028AB48(s32 id, s32 voice) {
    extern void *g_pActiveTextTable;
    u8  *ring = (u8 *)&g_pActiveTextTable;
    s16 *idRing = (s16 *)(ring + 0x8);
    s16 *voiceRing = (s16 *)(ring + 0x18);
    s32 count = ring[0x26];
    s32 head = ring[0x27];
    s32 i;
    if (count >= 7) {
        return;
    }
    for (i = count - 1; i >= 0; i--) {
        if (idRing[(head + i) % 7] == (s16)id) {
            return;
        }
    }
    idRing[(head + count) % 7] = (s16)id;
    voiceRing[(head + count) % 7] = (s16)voice;
    ring[0x26] = (u8)(count + 1);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028AC38);

/* ResetBoltCounterHud(): EU twin of USA ResetBoltCounterHud - snap the on-screen
 * bolt counter to the true bolt total (no roll animation): seed both shown and
 * target values (g_nBoltCounterDisplayed[0]/[1]) to g_boltCount, clear the roll
 * accumulator ([2]) and dirty index ([4]), set the pending flag ([3]) to -1, flush
 * the HUD value, and clear the two bolt-icon records at the roll-slot base.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs
 * USA ResetBoltCounterHud: g_boltCount / g_nBoltCounterDisplayed /
 * FlushHudDisplayValue kept named; the record base (USA g_hudMobyAuxBlockBase+0x44,
 * no EU symbol) is emitted by the EU splat as g_pActiveTextTable+0xE8 (same abs,
 * +0x80). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", ResetBoltCounterHud);
#else
extern s32  g_boltCount;                  /* EU 0x1A7A80 */
extern s32  g_nBoltCounterDisplayed[];    /* EU 0x1B1948 (USA 0x1B18C8 +0x80) */
extern void FlushHudDisplayValue(s32 valuePtr);
void ResetBoltCounterHud(void) {
    extern void *g_pActiveTextTable;
    s32 bolts = g_boltCount;
    s32 i;
    u8 *rec;
    g_nBoltCounterDisplayed[1] = bolts;
    g_nBoltCounterDisplayed[0] = bolts;
    g_nBoltCounterDisplayed[2] = 0;
    FlushHudDisplayValue((s32)(u32)g_nBoltCounterDisplayed);
    g_nBoltCounterDisplayed[4] = 0;
    g_nBoltCounterDisplayed[3] = -1;
    rec = (u8 *)&g_pActiveTextTable + 0xE8;   /* EU roll-slot base (USA g_hudMobyAuxBlockBase+0x44) */
    for (i = 1; i >= 0; i--) {
        *(s16 *)(rec + 0xE) = 0;
        *(s32 *)(rec + 0x0) = 0;
        rec += 0x10;
    }
}
#endif

/* UpdateBoltCounterHud(): EU twin of USA UpdateBoltCounterHud - per-frame animation
 * of the on-screen bolt counter. Rolls the displayed value toward live g_boltCount
 * via up to two animation slots at the roll-slot base (g_pActiveTextTable+0xE8,
 * stride 0x10). Returns early via the per-frame gate func_0029C118. Matching arm
 * stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * UpdateBoltCounterHud: gate func_0029C570 -> EU func_0029C118, trigger
 * func_0029C488 -> EU func_0029C030 (-0x458); FlushHudDisplayValue /
 * ResetBoltCounterHud / g_nGameState / g_boltCount / g_nBoltCounterDisplayed kept
 * named; slot base g_hudMobyAuxBlockBase+0x44 -> g_pActiveTextTable+0xE8; 0x3D924925
 * timing float and state-5 / 0x3D / 0xF constants byte-identical (no PAL diff). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", UpdateBoltCounterHud);
#else
extern s32  func_0029C118(void);   /* per-frame gate (USA func_0029C570): nonzero => skip this frame */
/* bolt-counter HUD trigger (USA func_0029C488). PAL/NTSC TIMING: this build calls it
 * with 0x96 (150 frames); the USA .s uses 0xB4 (180) at the same call site. Ratio
 * 150/180 = 0.8333 = 50/60 - a genuine PAL retime, NOT a portable constant. */
extern void func_0029C030(s32);

/* one bolt-roll animation slot: 2-entry array at g_pActiveTextTable+0xE8, stride 0x10 */
typedef struct HudRollSlot {
    s32 targetDelta;  /* +0x0  signed bolts still to roll onto the display */
    s32 holdTimer;    /* +0x4  arm-hold frame count, then roll-frame count */
    s32 tick;         /* +0x8  free-running frame tick (arming -> armed at 15) */
    s16 _pad0C;       /* +0xC  unused here */
    s16 phase;        /* +0xE  0 idle, 1 arming, 2 armed/hold, 3 rolling */
} HudRollSlot;

void UpdateBoltCounterHud(void) {
    extern s32   g_boltCount;
    extern s32   g_nBoltCounterDisplayed[];
    extern s32   g_nGameState;
    extern void *g_pActiveTextTable;
    extern void  FlushHudDisplayValue(s32 valuePtr);
    s32         *disp  = g_nBoltCounterDisplayed;
    HudRollSlot *slots = (HudRollSlot *)((u8 *)&g_pActiveTextTable + 0xE8);
    s32 i;

    if (func_0029C118() != 0) {
        return;
    }
    FlushHudDisplayValue((s32)(u32)g_nBoltCounterDisplayed);

    /* --- ARM: maybe start a new roll --- */
    if (g_nGameState == 5 && g_boltCount < disp[0]) {
        ResetBoltCounterHud();
    } else if (disp[4] == 0 && disp[3] == -1) {
        if (g_boltCount < disp[0]) {
            s32 idx = (slots[0].phase != 0) ? 1 : 0;
            slots[idx].phase       = 1;
            slots[idx].holdTimer   = 0;
            slots[idx].tick        = 0;
            slots[idx].targetDelta = disp[0] - g_boltCount;
            disp[3] = idx;
            disp[4] = 1;
        } else if ((disp[0] + disp[2]) < g_boltCount) {
            s32 idx = (slots[0].phase != 0) ? 1 : 0;
            slots[idx].phase       = 1;
            slots[idx].targetDelta = 0;
            slots[idx].holdTimer   = 0;
            slots[idx].tick        = 0;
            disp[3] = idx;
            disp[4] = 0;
        }
    }

    /* --- HOLD: service the active slot, fire the trigger --- */
    if (disp[3] != -1) {
        HudRollSlot *s = &slots[disp[3]];
        if (s->phase != 3) {
            s32 oldTarget = s->targetDelta;
            if (oldTarget >= 0) {
                s32 v = (g_boltCount - disp[0]) + disp[2];
                s->targetDelta = v;
                if (v != oldTarget) {
                    s->holdTimer = 0;
                }
            }
            s->tick      = s->tick + 1;
            s->holdTimer = s->holdTimer + 1;
            if (s->holdTimer >= 0x3D) {
                disp[1]      = disp[0];
                disp[2]      = s->targetDelta;
                s->phase     = 3;
                s->holdTimer = -1;
            }
        }
        func_0029C030(0x96);   /* PAL 150 frames (USA/NTSC 0xB4 = 180) */
    } else if (slots[0].phase == 3 || slots[1].phase == 3) {
        func_0029C030(0x96);   /* PAL 150 frames (USA/NTSC 0xB4 = 180) */
    }

    /* --- ROLL: advance both slots' animation --- */
    for (i = 0; i < 2; i++) {
        HudRollSlot *s = &slots[i];
        if (s->phase == 1) {
            if (s->tick >= 0xF) {
                s->phase = 2;
            }
        } else if (s->phase == 3) {
            s32 delta = (s32)((f32)disp[2] * (1.0f / 14.0f));   /* 0x3D924925 */
            s32 newTarget;

            s->holdTimer = s->holdTimer + 1;
            disp[0]      = disp[0] + delta;
            newTarget    = s->targetDelta - delta;
            s->targetDelta = newTarget;

            if (disp[4] != 0) {
                if (newTarget > 0) {
                    s->targetDelta = 0;
                }
            } else {
                if (newTarget < 0) {
                    s->targetDelta = 0;
                }
            }

            if (s->holdTimer >= 0xF) {
                if (disp[0] != disp[1] + disp[2]) {
                    disp[0] = disp[1] + disp[2];
                }
                disp[2]  = 0;
                disp[3]  = -1;
                s->phase = 0;
                disp[4]  = 0;
            }
        }
    }
}
#endif

/* func_0028B038(): EU twin of USA func_0028B0B0 - per-frame render of the animated
 * "+/-N bolts" popup for each of the two bolt-roll slots (g_pActiveTextTable+0xE8,
 * stride 0x10, shared with UpdateBoltCounterHud). For a slot in phase 1/2/3 it
 * derives an interpolation factor from the slot timers, lerps the two glyph tints
 * and the number tint (func_002845F8), formats targetDelta into a string, then
 * draws the two "+/-" glyphs (codepoints 0xB2/0xB3) and the number. Matching arm
 * stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_0028B0B0: func_002846E8->func_002845F8, func_00280090->func_0027FF28,
 * func_003017F8->func_00301AC0, GuiFontAtlasLookupGlyph(0x337BF8)->func_00338AA8;
 * D_1A8DE0->D_1A8E90, D_1A8E00->D_1A8EB0 (+0xB0); y-scale g_swapGadgetItemIndex+0x8A
 * -> g_nVendorBuyQuantity+0x15C; draw coords 0x1F3/0x1BA/0x24 + all tints
 * byte-identical (no PAL diff). Reuses HudRollSlot from UpdateBoltCounterHud. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028B038);
#else
void func_0028B038(void) {
    extern void *g_guiInstance;             /* GUI singleton ptr (EU 0x1A8DAC) */
    extern s32   g_nVendorBuyQuantity;      /* HUD sprite y-scale at +0x15C (USA g_swapGadgetItemIndex+0x8A) */
    extern f32   D_1A8E90[];                /* per-digit-count x-scale table (EU 0x1A8E90, USA D_1A8DE0 +0xB0) */
    extern char  D_1A8EB0[];                /* printf format for the bolt-delta number (EU 0x1A8EB0, USA D_1A8E00 +0xB0) */
    extern s32   func_002845F8(s32 colorA, s32 colorB, f32 t);        /* EU 0x2845F8 packed-RGBA lerp (USA func_002846E8) */
    extern void  func_00115DA8(char *dst, const char *fmt, ...);      /* SDK sprintf */
    extern s32   func_001157AC(const char *s);                        /* SDK strlen */
    extern s32   func_00338AA8(void *atlas, s32 codepoint);           /* EU 0x338AA8 glyph lookup (USA GuiFontAtlasLookupGlyph) */
    extern void  func_00301AC0(s32 handle, s32 color0, f32 *scale, f32 *vec38,
                               f32 px, f32 py, f32 sx, f32 sy, f32 v); /* EU 0x301AC0 glyph/sprite draw (USA func_003017F8) */
    extern void  func_0027FF28(s32 x, s32 y, u64 color, char *str, s64 wrap); /* EU 0x27FF28 text/number draw (USA func_00280090) */
    extern void *g_pActiveTextTable;

    HudRollSlot *slot = (HudRollSlot *)((u8 *)&g_pActiveTextTable + 0xE8);
    s32 color1 = 0;   /* glyph 0xB2 tint */
    s32 color2 = 0;   /* glyph 0xB3 tint */
    s32 c;

    for (c = 1; c >= 0; c--) {
        s16 phase     = slot->phase;
        s32 doDraw    = 0;
        s32 textColor = 0;
        f32 t;

        if (phase == 2) {
            s32 hold = slot->holdTimer;
            t = (hold < 0xE) ? (f32)hold * (1.0f / 14.0f) : 1.0f;
            color1 = 0x55F0C070;
            color2 = 0x60442D00;
            if (slot->targetDelta >= 0) {
                textColor = func_002845F8(0x8000FFFF, 0x6029A1FF, t);
            } else {
                textColor = func_002845F8(0x001010F0, 0x801010F0, t);
            }
            doDraw = 0x1F3;
        } else if (phase == 1) {
            t = (f32)slot->tick * (1.0f / 14.0f);
            color1 = func_002845F8(0x00F0C070, 0x55F0C070, t);
            color2 = func_002845F8(0x00442D00, 0x60442D00, t);
            if (slot->targetDelta >= 0) {
                textColor = func_002845F8(0x0029A1FF, 0x6029A1FF, t);
            } else {
                textColor = func_002845F8(0x001010F0, 0x801010F0, t);
            }
            doDraw = 0x1F3;
        } else if (phase == 3) {
            s32 hold = slot->holdTimer;
            t = (hold < 7) ? 1.0f
                           : 1.0f - ((f32)hold - 7.0f) * (1.0f / 7.0f);
            color1 = func_002845F8(0x00F0C070, 0x55F0C070, t);
            color2 = func_002845F8(0x00442D00, 0x60442D00, t);
            if (slot->targetDelta >= 0) {
                textColor = func_002845F8(0x0029A1FF, 0x6029A1FF, t);
            } else {
                textColor = func_002845F8(0x001010F0, 0x801010F0, t);
            }
            doDraw = 0x1F3;
        }

        if (doDraw != 0) {
            char buf[16];
            s32  len;
            func_00115DA8(buf, D_1A8EB0, slot->targetDelta);
            len = func_001157AC(buf);
            if (len > 0) {
                void *atlas  = (u8 *)g_guiInstance + 0x8710;
                f32   xscale = D_1A8E90[len - 1];
                f32   yfudge = *(f32 *)((u8 *)&g_nVendorBuyQuantity + 0x15C);
                f32   glyphY = (f32)(s32)((f32)2 * yfudge + 0.5f);
                s32   textY  = (s32)((f32)0x24 * yfudge + 0.5f);
                s32   g1 = func_00338AA8(atlas, 0xB2);
                s32   g2;
                func_00301AC0(g1, color1, (f32 *)0, (f32 *)0,
                              (f32)0x1F3, glyphY, 1.0f, 1.0f, xscale);
                g2 = func_00338AA8(atlas, 0xB3);
                func_00301AC0(g2, color2, (f32 *)0, (f32 *)0,
                              (f32)0x1F3, glyphY, 1.0f, 1.0f, xscale);
                len = func_001157AC(buf);
                func_0027FF28(0x1BA, textY, textColor, buf, len);
            }
        }

        slot = (HudRollSlot *)((u8 *)slot + 0x10);
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028B4E0);

/* func_0028B4E8(name): EU twin of USA func_0028B560 - linear-scan the HUD icon-slot
 * table for the entry whose texture id equals `name`, stopping at the 0xFFFF
 * sentinel; return its index (or the sentinel index when not found). Matching arm
 * stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_0028B560: USA reads the table base from g_pHudAssetHeader[1]; EU has no
 * g_pHudAssetHeader symbol, so the EU splat resolves that pointer word as
 * g_pActiveTextTable+0x4C (USA abs 0x1B180C -> EU 0x1B188C, +0x80). Stride 0x8 /
 * texId at +0 identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028B4E8);
#else
s32 func_0028B4E8(s32 name) {
    extern void *g_pActiveTextTable;
    typedef struct HudIconSlot {
        u16 texId;      /* +0x0 */
        u16 maxLevel;   /* +0x2 */
        u16 baseFrame;  /* +0x4 */
        u8  paletteId;  /* +0x6 */
        u8  _pad7;      /* +0x7 */
    } HudIconSlot;      /* stride 0x8 */
    HudIconSlot *table = *(HudIconSlot **)((u8 *)&g_pActiveTextTable + 0x4C);
    s32 i = 0;
    while (table[i].texId != 0xFFFF) {
        if (table[i].texId == name) {
            break;
        }
        i++;
    }
    return i;
}
#endif

/* func_0028B538: EU twin of USA InitHudMobyTable - first-time setup of the HUD
 * moby-table context. Zeroes the two text-gen counters at g_pActiveTextTable
 * +0x30/+0x34, rebuilds all 13 D_255330 widget records (register each via
 * func_0028BD98, seed key=-1 at +0x64, +0x20=0x10000, +0x7C/+0x04/+0x24=0,
 * +0x6C=-6), and - if the HUD moby table pointer is null - DebugMalloc's the
 * 0x2800 table and 0x1400 aux block, records extents, clears it (func_00283348),
 * tags +0x20 byte 0xFF, rebuilds the weapon wheel (func_0028C6B0) and snaps the
 * bolt counter (ResetBoltCounterHud). Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA InitHudMobyTable: D_2552B0 -> D_255330;
 * HUD moby globals re-anchored on g_pActiveTextTable (+0x98 base / +0x9C spawnStart
 * / +0xA0 end / +0xA4 aux); func_0028BE10 -> func_0028BD98, func_00283438 ->
 * func_00283348, func_0028C728 -> func_0028C6B0; DebugMalloc 4-arg form. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028B538);
#else
void func_0028B538(void) {
    extern void *g_pActiveTextTable;
    extern u8 D_255330[];
    extern u8 D_1A8EB8[];
    extern s32 func_0028BD98(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
    extern void *DebugMalloc(s32 size, s32 arg2, void *file, s32 line);
    extern void func_00283348(void *base, s32 size);
    extern void func_0028C6B0(void);
    extern void ResetBoltCounterHud(void);
    void *base;
    s32 i;

    *(s32 *)((u8 *)&g_pActiveTextTable + 0x30) = 0;
    *(s32 *)((u8 *)&g_pActiveTextTable + 0x34) = 0;

    for (i = 0; i < 13; i++) {
        u8 *rec = D_255330 + i * 0x90;
        *(s32 *)(rec + 0x64) = -1;
        *(s32 *)(rec + 0x20) = 0x10000;
        func_0028BD98(i, 0xFFFF, 0, 0, 0, 0, 1);
        *(s32 *)(rec + 0x7C) = 0;
        *(s32 *)(rec + 0x6C) = -6;
        *(s32 *)(rec + 0x04) = 0;
        *(s32 *)(rec + 0x24) = 0;
    }

    if (*(void **)((u8 *)&g_pActiveTextTable + 0x98) == 0) {
        *(void **)((u8 *)&g_pActiveTextTable + 0x98) =
            DebugMalloc(0x2800, 0, D_1A8EB8, 0x2AB);
        *(void **)((u8 *)&g_pActiveTextTable + 0xA4) =
            DebugMalloc(0x1400, 0, D_1A8EB8, 0x2AC);
    }

    base = *(void **)((u8 *)&g_pActiveTextTable + 0x98);
    *(void **)((u8 *)&g_pActiveTextTable + 0xA0) = (u8 *)base + 0x2800;
    *(void **)((u8 *)&g_pActiveTextTable + 0x9C) = base;
    func_00283348(base, 0x2800);
    *((u8 *)base + 0x20) = 0xFF;
    func_0028C6B0();
    ResetBoltCounterHud();
}
#endif

/* func_0028B678: EU twin of USA func_0028B6F0 - load and upload the HUD moby-table
 * wads into VRAM. Per sub-bank the HUD asset header (g_pActiveTextTable+0x48) holds
 * a decompressed size (+0x54/+0x5C/+0x60/+0x64) and a compressed-source pointer
 * (+0x94/+0x9C/+0xA0/+0xA4); compressed sizes live at g_pActiveTextTable +0x70/+0x78/
 * +0x7C/+0x80. Each present bank is staged into the menu-screen scratch (+0x20) via
 * func_002EFE20, decompressed into the running VRAM address (+0x114) via func_0029DE68,
 * then registered with func_0028B850. Banks 0/1 always; bank 2 for mode 0/2; bank 3
 * for mode 1/2. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0028B6F0: g_menuScreenBlock -> D_001F0000+0x2840;
 * g_pHudAssetHeader[0] -> g_pActiveTextTable+0x48; g_hudMobySpawnStart+0x8/0x10/0x14/
 * 0x18 -> g_pActiveTextTable+0x70/78/7C/80; func_002EFD28 -> func_002EFE20,
 * DecompressWad -> func_0029DE68, func_0028B8C8 -> func_0028B850.
 * NOTE: bank-1 register index is 2 (not 1) - confirmed by BOTH EU and USA asm
 * (0028B7CC: addiu $4,$0,0x2). The USA #else body writes func_0028B8C8(1,...), a
 * latent typo; modelled here per ground truth as 2. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028B678);
#else
void func_0028B678(s32 mode) {
    extern void *g_pActiveTextTable;
    extern u8 D_001F0000[];
    extern void func_002EFE20(void *dest, void *src, s32 a, s32 count, s32 b);
    extern void func_0029DE68(void *src, void *dest);
    extern void func_0028B850(s32 index, s32 baseAddr);
    u8   *scratch = D_001F0000 + 0x2840;
    u8   *hdr  = *(u8 **)((u8 *)&g_pActiveTextTable + 0x48);
    void *dest = *(void **)(scratch + 0x20);
    s32   vram = *(s32 *)(scratch + 0x114);
    s32   size, clen;

    /* bank 0 - always */
    size = *(s32 *)(hdr + 0x54);
    if (size != 0) {
        clen = *(s32 *)((u8 *)&g_pActiveTextTable + 0x70);
        func_002EFE20(dest, *(void **)(hdr + 0x94), 0, clen / 16, 0);
        func_0029DE68(dest, (void *)vram);
        func_0028B850(0, vram);
        vram += size;
    }
    /* bank 1 - always (register index 2, per asm) */
    size = *(s32 *)(hdr + 0x5C);
    if (size != 0) {
        clen = *(s32 *)((u8 *)&g_pActiveTextTable + 0x78);
        func_002EFE20(dest, *(void **)(hdr + 0x9C), 0, clen / 16, 0);
        func_0029DE68(dest, (void *)vram);
        func_0028B850(2, vram);
        vram += size;
    }
    /* bank 2 - mode 0 or 2 */
    if (mode == 0 || mode == 2) {
        size = *(s32 *)(hdr + 0x60);
        if (size != 0) {
            clen = *(s32 *)((u8 *)&g_pActiveTextTable + 0x7C);
            func_002EFE20(dest, *(void **)(hdr + 0xA0), 0, clen / 16, 0);
            func_0029DE68(dest, (void *)vram);
            func_0028B850(3, vram);
            vram += size;
        }
    }
    /* bank 3 - mode 1 or 2 */
    if (mode == 1 || mode == 2) {
        size = *(s32 *)(hdr + 0x64);
        if (size != 0) {
            clen = *(s32 *)((u8 *)&g_pActiveTextTable + 0x80);
            func_002EFE20(dest, *(void **)(hdr + 0xA4), 0, clen / 16, 0);
            func_0029DE68(dest, (void *)vram);
            func_0028B850(4, vram);
        }
    }
}
#endif

/* func_0028B850: EU twin of USA func_0028B8C8 - relocate (and mark allocated) the
 * GS handles for one HUD asset's CLUT and texture slots, biased by an aligned base.
 * The HUD asset header (g_pActiveTextTable+0x48) holds per-asset cumulative CLUT end
 * indices at +0x14, texture end indices at +0x34, and the stored relocation base at
 * +0x74. If +0x74 is already non-zero this is a no-op; else store the 16-byte-aligned
 * base and, for each CLUT/texture handle in this asset's [prevEnd,end) range, clear
 * the sign bit and add the base. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA func_0028B8C8: g_pHudAssetHeader[0] ->
 * g_pActiveTextTable+0x48; g_hudClutSlots -> +0x58; g_hudTextureSlots -> +0x54. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028B850);
#else
void func_0028B850(s32 assetId, s32 baseAddr) {
    extern void *g_pActiveTextTable;
    u8 *hdr = *(u8 **)((u8 *)&g_pActiveTextTable + 0x48);
    s32 *relBase = (s32 *)(hdr + 0x74 + assetId * 4);
    s32 base;
    s32 start;
    s32 end;
    s32 i;
    u8 *clut;
    u8 *tex;

    if (*relBase != 0) {
        return; /* asset already relocated */
    }

    base = (baseAddr + 0xF) & ~0xF;
    *relBase = base;

    start = (assetId == 0) ? 0 : *(s32 *)(hdr + 0x14 + (assetId - 1) * 4);
    end = *(s32 *)(hdr + 0x14 + assetId * 4);
    clut = *(u8 **)((u8 *)&g_pActiveTextTable + 0x58);
    for (i = start; i < end; i++) {
        *(s32 *)(clut + i * 8) &= 0x7FFFFFFF;
        *(s32 *)(clut + i * 8) += base;
    }

    start = (assetId == 0) ? 0 : *(s32 *)(hdr + 0x34 + (assetId - 1) * 4);
    end = *(s32 *)(hdr + 0x34 + assetId * 4);
    tex = *(u8 **)((u8 *)&g_pActiveTextTable + 0x54);
    for (i = start; i < end; i++) {
        *(s32 *)(tex + i * 8) &= 0x7FFFFFFF;
        *(s32 *)(tex + i * 8) += base;
    }
}
#endif

/* func_0028B9B0: EU twin of USA func_0028BA28 - inverse of func_0028B850:
 * un-relocate and free one HUD asset's CLUT and texture GS handles. For each handle
 * in this asset's [prevEnd,end) range (bounds from header +0x14/+0x34), subtract the
 * stored base (+0x74) and set the sign bit; then clear the stored base word. Asset 0
 * additionally clears the +0x4 halfword of each of its texture slots up front.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_0028BA28: g_pHudAssetHeader[0] -> g_pActiveTextTable+0x48; g_hudClutSlots ->
 * +0x58; g_hudTextureSlots -> +0x54. No callees. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028B9B0);
#else
void func_0028B9B0(s32 assetId) {
    extern void *g_pActiveTextTable;
    u8 *hdr = *(u8 **)((u8 *)&g_pActiveTextTable + 0x48);
    s32 relBase = *(s32 *)(hdr + 0x74 + assetId * 4);
    s32 start;
    s32 end;
    s32 i;
    u8 *clut;
    u8 *tex;

    if (assetId == 0) {
        /* asset 0 only: clear the +0x4 halfword of every one of its texture slots */
        end = *(s32 *)(hdr + 0x34);
        tex = *(u8 **)((u8 *)&g_pActiveTextTable + 0x54);
        for (i = 0; i < end; i++) {
            *(s16 *)(tex + i * 8 + 0x4) = 0;
        }
    }

    start = (assetId == 0) ? 0 : *(s32 *)(hdr + 0x14 + (assetId - 1) * 4);
    end = *(s32 *)(hdr + 0x14 + assetId * 4);
    clut = *(u8 **)((u8 *)&g_pActiveTextTable + 0x58);
    for (i = start; i < end; i++) {
        *(s32 *)(clut + i * 8) -= relBase;
        *(s32 *)(clut + i * 8) |= (s32)0x80000000;
    }

    start = (assetId == 0) ? 0 : *(s32 *)(hdr + 0x34 + (assetId - 1) * 4);
    end = *(s32 *)(hdr + 0x34 + assetId * 4);
    tex = *(u8 **)((u8 *)&g_pActiveTextTable + 0x54);
    for (i = start; i < end; i++) {
        *(s32 *)(tex + i * 8) -= relBase;
        *(s32 *)(tex + i * 8) |= (s32)0x80000000;
    }

    *(s32 *)(hdr + 0x74 + assetId * 4) = 0;
}
#endif

/* func_0028BB28: EU twin of USA func_0028BBA0 - upload a HUD asset's textures to GS
 * VRAM. Relocates the shared asset once (func_0028B850(0, baseAddr)) if header +0x74
 * is unset. Then walking this asset's texture slot range [prevEnd,end) (header +0x34
 * cumulative bounds), uploads each g_hudTextureSlots entry to GS at the running VRAM
 * cursor (func_002901C8, fmt 0x1B, dims from the slot's +0x6/+0x7 log2 bytes), records
 * the VRAM block in the slot's +0x4, and advances the cursor by (1<<(wLog+hLog))<<2.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_0028BBA0: g_pHudAssetHeader[0] -> g_pActiveTextTable+0x48; g_vramTextureBase+0x24
 * -> D_001A7308+0x80; g_hudTextureSlots -> +0x54; func_0028B8C8 -> func_0028B850,
 * func_002901B0 -> func_002901C8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028BB28);
#else
void func_0028BB28(s32 assetId, s32 baseAddr, s32 kickMode) {
    extern void *g_pActiveTextTable;
    extern u8 D_001A7308[];
    extern void func_0028B850(s32 index, s32 baseAddr);
    extern void func_002901C8(s32 handle, s32 vramBlk, s32 fmt, s32 wLog,
                              s32 hLog, s32 kickMode);
    u8 *hdr = *(u8 **)((u8 *)&g_pActiveTextTable + 0x48);
    s32 vramCursor;
    s32 start;
    s32 end;
    s32 i;

    if (*(s32 *)(hdr + 0x74 + assetId * 4) == 0) {
        func_0028B850(0, baseAddr);
    }

    vramCursor = *(s32 *)(D_001A7308 + 0x80);

    start = (assetId == 0) ? 0 : *(s32 *)(hdr + 0x34 + (assetId - 1) * 4);
    end = *(s32 *)(hdr + 0x34 + assetId * 4);
    for (i = start; i < end; i++) {
        u8 *tex = *(u8 **)((u8 *)&g_pActiveTextTable + 0x54) + i * 8;
        s32 wLog = tex[0x6];
        s32 hLog = tex[0x7];
        s32 vramBlk = vramCursor >> 8;

        func_002901C8(*(s32 *)(tex + 0x0), vramBlk, 0x1B, wLog, hLog, kickMode);
        *(s16 *)(tex + 0x4) = vramBlk;
        vramCursor += (1 << (wLog + hLog)) << 2;
    }
}
#endif

/* func_0028BC50: EU twin of USA ResetDebugHeap - (re)initialise the DebugMalloc
 * bump allocator: cursor back to the pool base, end at base + 0x64000. Matching arm
 * stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA ResetDebugHeap:
 * g_debugMallocPoolBase -> g_nVendorBuyQuantity+0x8CF4; g_debugMallocCursor ->
 * g_pActiveTextTable+0x40; g_debugMallocEnd -> g_pActiveTextTable+0x44. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028BC50);
#else
void func_0028BC50(void) {
    extern s32 g_nVendorBuyQuantity;
    extern void *g_pActiveTextTable;
    u8 *base = *(u8 **)((u8 *)&g_nVendorBuyQuantity + 0x8CF4);
    *(u8 **)((u8 *)&g_pActiveTextTable + 0x40) = base;
    *(u8 **)((u8 *)&g_pActiveTextTable + 0x44) = base + 0x64000;
}
#endif

/* DebugMalloc(size, arg2, file, line): bump-allocate `size` bytes (rounded up to
 * 16) from the debug pool. Lazily (re)initialises the pool on first use (via
 * func_0028BC50, the EU ResetDebugHeap twin), returns 0 when the remaining space is
 * smaller than `size`, else the old cursor. arg2/file/line are debug call-site
 * fields (unused by the body); the 4-arg void* form matches the cross-unit decl in
 * 191240/198B58. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA DebugMalloc: g_debugMallocCursor -> g_pActiveTextTable+0x40;
 * g_debugMallocEnd -> +0x44; ResetDebugHeap -> func_0028BC50. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", DebugMalloc);
#else
void *DebugMalloc(s32 size, s32 arg2, void *file, s32 line) {
    extern void *g_pActiveTextTable;
    extern void func_0028BC50(void);
    u8 *result;

    if (*(u8 **)((u8 *)&g_pActiveTextTable + 0x40) == 0) {
        func_0028BC50();
    }
    if ((s32)(*(u8 **)((u8 *)&g_pActiveTextTable + 0x44) -
              *(u8 **)((u8 *)&g_pActiveTextTable + 0x40)) < size) {
        return 0;
    }
    result = *(u8 **)((u8 *)&g_pActiveTextTable + 0x40);
    *(u8 **)((u8 *)&g_pActiveTextTable + 0x40) = result + ((size + 0xF) & ~0xF);
    return result;
}
#endif

/* func_0028BD00(newId): EU twin of USA SwapMobyTableContext - toggle the active
 * moby-table context (live vs the HUD shadow set), swapping the base/spawn-start/
 * end/aux-block pointers between the two sets; no-op when the requested context is
 * already active. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA SwapMobyTableContext (op-for-op): the moby globals have no
 * named EU twin and are re-anchored - g_activeMobyTableId -> gp D_1A8DF4; the live
 * set g_moby{TableBase/SpawnStart/TableEnd/AuxBlockBase} -> g_nBoltCounterDisplayed
 * +0x214/+0x218/+0x21C/+0x224 (gap at +0x220); the shadow set g_hudMoby{...} ->
 * g_pActiveTextTable +0x98/+0x9C/+0xA0/+0xA4 (same anchor as InitHudMobyTable). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028BD00);
#else
void func_0028BD00(s32 newId) {
    extern s32   D_1A8DF4;                  /* g_activeMobyTableId (gp) */
    extern s32   g_nBoltCounterDisplayed[]; /* live moby set anchor (+0x214..) */
    extern void *g_pActiveTextTable;        /* HUD shadow set anchor (+0x98..) */
    void *base, *spawn, *end, *aux;

    if (newId == D_1A8DF4) {
        return;
    }
    base  = *(void **)((u8 *)g_nBoltCounterDisplayed + 0x214);
    spawn = *(void **)((u8 *)g_nBoltCounterDisplayed + 0x218);
    end   = *(void **)((u8 *)g_nBoltCounterDisplayed + 0x21C);
    aux   = *(void **)((u8 *)g_nBoltCounterDisplayed + 0x224);

    D_1A8DF4 = D_1A8DF4 ^ 1;

    *(void **)((u8 *)g_nBoltCounterDisplayed + 0x214) = *(void **)((u8 *)&g_pActiveTextTable + 0x98);
    *(void **)((u8 *)g_nBoltCounterDisplayed + 0x218) = *(void **)((u8 *)&g_pActiveTextTable + 0x9C);
    *(void **)((u8 *)g_nBoltCounterDisplayed + 0x21C) = *(void **)((u8 *)&g_pActiveTextTable + 0xA0);
    *(void **)((u8 *)g_nBoltCounterDisplayed + 0x224) = *(void **)((u8 *)&g_pActiveTextTable + 0xA4);

    *(void **)((u8 *)&g_pActiveTextTable + 0x98) = base;
    *(void **)((u8 *)&g_pActiveTextTable + 0x9C) = spawn;
    *(void **)((u8 *)&g_pActiveTextTable + 0xA0) = end;
    *(void **)((u8 *)&g_pActiveTextTable + 0xA4) = aux;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028BD98);

/* func_0028BEA0(w): EU twin of USA func_0028BF18 - initialise a HUD widget: bind its
 * icon graphics (func_0028C018 using the icon id at +0x20), unpack the staged layout
 * fields (+0x24/+0x28/+0x2C/+0x34/+0x38) into their live slots, run the widget's init
 * callback (+0x30) when present, then clear the dirty flag (+0x68). Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0028BF18:
 * func_0028C090 -> EU func_0028C018; no data globals; field offsets identical. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028BEA0);
#else
void func_0028BEA0(void *w) {
    extern void func_0028C018(void *w, s32 iconName);
    u8 *b = (u8 *)w;
    void (*init)(void *);
    func_0028C018(w, *(s32 *)(b + 0x20));
    init = *(void (**)(void *))(b + 0x30);
    *(s32 *)(b + 0x4)  = *(s32 *)(b + 0x24);
    *(s32 *)(b + 0x14) = *(s32 *)(b + 0x34);
    *(s32 *)(b + 0x18) = *(s32 *)(b + 0x38);
    *(s32 *)(b + 0xC)  = *(s32 *)(b + 0x2C);
    *(s32 *)(b + 0x8)  = *(s32 *)(b + 0x28);
    *(void (**)(void *))(b + 0x10) = init;
    if (init != 0) {
        init(w);
    }
    *(s32 *)(b + 0x68) = 0;
}
#endif

/* func_0028BF08(): EU twin of USA func_0028BF80 - (re)initialise the whole 13-entry
 * D_255330 HUD widget table: for each record rebuild its element list via
 * func_0028BD98(i,0xFFFF,0,0,0,0,1), clear the record's +0x7C word and set its +0x6C
 * word to -6, then run func_0028BEA0 on it. Matching arm stays INCLUDE_ASM; #else is
 * the structure model. Word-verified vs USA func_0028BF80: D_2552B0 -> D_255330;
 * func_0028BE10 -> func_0028BD98; func_0028BF18 -> func_0028BEA0; stride 0x90. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028BF08);
#else
void func_0028BF08(void) {
    extern u8  D_255330[];
    extern s32 func_0028BD98(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
    extern void func_0028BEA0(void *w);
    u8 *rec = D_255330;
    s32 i;
    for (i = 0; i < 0xD; i++) {
        func_0028BD98(i, 0xFFFF, 0, 0, 0, 0, 1);
        *(s32 *)(rec + 0x7C) = 0;
        *(s32 *)(rec + 0x6C) = -6;
        func_0028BEA0(rec);
        rec += 0x90;
    }
}
#endif

/* func_0028BF98(key): EU twin of USA func_0028C010 - search the D_255330 record
 * table (13 entries, stride 0x90, key at +0x64) for `key`; when present rebuild a
 * HUD list via func_0028BD98 and return 1, else return 0. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0028C010:
 * D_2552B0 -> D_255330; func_0028BE10 -> func_0028BD98. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028BF98);
#else
s32 func_0028BF98(s32 key) {
    extern u8  D_255330[];
    extern s32 func_0028BD98(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
    s32 i = 0;
    if (*(s32 *)(D_255330 + 0x64) != key) {
        do {
            if (++i >= 0xD) {
                return 0;
            }
        } while (*(s32 *)(D_255330 + i * 0x90 + 0x64) != key);
    }
    if (i >= 0xD) {
        return 0;
    }
    func_0028BD98(i, 0xFFFF, 0, 0, 0, 0, 0);
    return 1;
}
#endif

/* func_0028C018(w, iconName): EU twin of USA func_0028C090 - resolve the HUD icon
 * slot for `iconName` (via func_0028B4E8), then copy its texture id, palette id and
 * base-frame field out of the icon-slot table into the widget record `w`, recording
 * the slot index in w[+0x40]. Matching arm stays INCLUDE_ASM; #else is the structure
 * model. Word-verified vs USA func_0028C090: func_0028B560 -> EU func_0028B4E8; the
 * icon table base g_pHudAssetHeader[1] -> g_pActiveTextTable+0x4C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C018);
#else
void func_0028C018(void *w, s32 iconName) {
    extern void *g_pActiveTextTable;
    extern s32   func_0028B4E8(s32 name);
    typedef struct HudIconSlot {
        u16 texId;      /* +0x0 */
        u16 maxLevel;   /* +0x2 */
        u16 baseFrame;  /* +0x4 */
        u8  paletteId;  /* +0x6 */
        u8  _pad7;      /* +0x7 */
    } HudIconSlot;      /* stride 0x8 */
    u8 *b = (u8 *)w;
    s32 slot = func_0028B4E8(iconName);
    *(s16 *)(b + 0x40) = (s16)slot;
    *(s32 *)(b + 0x0)  = (*(HudIconSlot **)((u8 *)&g_pActiveTextTable + 0x4C))[slot].texId;
    *(s8  *)(b + 0x42) = (*(HudIconSlot **)((u8 *)&g_pActiveTextTable + 0x4C))[slot].paletteId;
    *(s32 *)(b + 0x44) = (*(HudIconSlot **)((u8 *)&g_pActiveTextTable + 0x4C))[slot].baseFrame;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C088);

/* func_0028C090(key, value): EU twin of USA func_0028C108 - linear-search the
 * D_255330 record table (13 entries, stride 0x90, key at +0x64) for `key`; when
 * found within the table, store `value` into that record's +0x24 field, and
 * additionally into its +0x4 field when its +0x68 field is zero. No-op when not
 * found. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0028C108 (op-for-op; confirms the +1 glabel shift is
 * address-only): D_2552B0 -> D_255330; no callees. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C090);
#else
void func_0028C090(s32 key, s32 value) {
    extern u8 D_255330[];
    s32 i = 0;
    if (*(s32 *)(D_255330 + 0x64) != key) {
        do {
            if (++i >= 0xD) {
                return;
            }
        } while (*(s32 *)(D_255330 + i * 0x90 + 0x64) != key);
    }
    if (i < 0xD) {
        u8 *rec = D_255330 + i * 0x90;
        *(s32 *)(rec + 0x24) = value;
        if (*(s32 *)(rec + 0x68) == 0) {
            *(s32 *)(rec + 0x4) = value;
        }
    }
}
#endif

/* func_0028C108(rec, pA, pB): EU twin of USA func_0028C180 - apply a layout record's
 * edge/centre alignment flags (+0x60) to an X (*pA) and Y (*pB) coordinate, using
 * its half-extents at +0x58 (X) / +0x5C (Y): bits 1|2 gate the Y shift by half the
 * +0x5C extent; bit 4 gates the X shift, bit 8 selecting the full vs half +0x58
 * extent. Always returns 0. Matching arm stays INCLUDE_ASM; #else is the structure
 * model. Word-verified vs USA func_0028C180 (op-for-op; the +1 glabel shift is
 * address-only): no data globals, no callees. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C108);
#else
s32 func_0028C108(void *rec, s32 *pA, s32 *pB) {
    u8 *b = (u8 *)rec;
    s32 flags   = *(s32 *)(b + 0x60);
    s32 xExtent = *(s32 *)(b + 0x58);
    s32 yExtent = *(s32 *)(b + 0x5C);

    if ((flags & 1) == 0 && (flags & 2) == 0) {
        *pB = *pB - (yExtent >> 1);
    }
    if ((flags & 4) == 0) {
        if ((flags & 8) != 0) {
            *pA = *pA - xExtent;
        } else {
            *pA = *pA - (xExtent >> 1);
        }
    }
    return 0;
}
#endif

/* func_0028C170(rec, pX, pY, idxBase, idxDelta): EU twin of USA func_0028C1E8 -
 * apply a HUD layout record's alignment flags (+0x60) to an (*pX,*pY) coordinate
 * using fractional offset tables scaled by the record's half-extents (+0x58/+0x5C)
 * via int<->float round-trips. rec+0x7C tag selects the table and index sign
 * (clamped to [0,0x17]); flags bit 1/2 nudge Y by +0x5C (+52 bias), bit 4/8 nudge X
 * by +0x58 (+20 bias); results ADDED into the pX/pY out-params. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. Word-verified vs USA func_0028C1E8: IntToFloat ->
 * IntToFloat, FloatToInt -> func_002845B0, D_255A00 -> D_255A80, D_255A60 ->
 * D_255AE0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C170);
#else
void func_0028C170(void *rec, s32 *pX, s32 *pY, s32 idxBase, s32 idxDelta) {
    extern f32 IntToFloat(s32 v);   /* EU IntToFloat (USA 0x2846D8) */
    extern s32 func_002845B0(f32 x);   /* EU FloatToInt (USA 0x2846A0) */
    extern f32 D_255A80[];             /* offset table, rec+0x7C set (USA D_255A00) */
    extern f32 D_255AE0[];             /* offset table, rec+0x7C clear (USA D_255A60) */
    u8 *r = (u8 *)rec;
    s32 idx;
    f32 scale;
    s32 flags;
    s32 dx = 0;
    s32 dy = 0;

    if (*(s32 *)(r + 0x7C) != 0) {
        idx = idxBase - idxDelta;
    } else {
        idx = idxBase + idxDelta;
    }
    if (idx < 0) {
        idx = 0;
    }
    if (idx >= 0x18) {
        idx = 0x17;
    }
    scale = (*(s32 *)(r + 0x7C) != 0) ? D_255A80[idx] : D_255AE0[idx];

    flags = *(s32 *)(r + 0x60);
    if (flags & 0x1) {
        dy = -func_002845B0(scale * (IntToFloat(*(s32 *)(r + 0x5C)) + 52.0f) + 0.5f);
    } else if (flags & 0x2) {
        dy = func_002845B0(scale * (IntToFloat(*(s32 *)(r + 0x5C)) + 52.0f) + 0.5f);
    } else if (flags & 0x4) {
        dx = -func_002845B0(scale * (IntToFloat(*(s32 *)(r + 0x58)) + 20.0f) + 0.5f);
    } else if (flags & 0x8) {
        dx = func_002845B0(scale * (IntToFloat(*(s32 *)(r + 0x58)) + 20.0f) + 0.5f);
    }

    *pX += dx;
    *pY += dy;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C318);

/* Reset a HUD widget record: set its type tag (+0x7C = 0xB4 in PAL; the USA
 * twin func_0028C490 uses 0xD2 - a region-divergent HUD widget type id), clear
 * the two 16-bit cursor fields (+0x48/+0x4A) and re-init it via func_0028C318. */
void func_0028C418(void *p) {
    *(s32 *)((u8 *)p + 0x7C) = 0xB4;
    *(s16 *)((u8 *)p + 0x48) = 0;
    *(s16 *)((u8 *)p + 0x4A) = 0;
    func_0028C318(p);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C450);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C620);

/* func_0028C668: HUD-init forwarder - enable a HUD subsystem via
 * func_0029D670(arg, 1) then register HUD element 0xD with
 * func_002B1858(0xD, 0, 1). Recovered from the EU padding mis-split (split off
 * func_0028C620's 0x48 epilogue-stump run via the symbol_addrs pin). USA twin
 * func_0028C6E0 (calls func_0029DB10 / func_002B1B48). */
void func_0028C668(s32 arg) {
    func_0029D670(arg, 1);
    func_002B1858(0xD, 0, 1);
    __asm__ __volatile__("");
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C698);

/* func_0028C6B0(): EU twin of USA func_0028C728 - rebuild the 8-slot weapon-select
 * wheel icon list. Stores slot count 8 and the &D_1A8E80 callback base into
 * g_pActiveTextTable+0x90/+0x94, resets the wheel cursor D_1A8DF8, then for each of 8
 * source item ids (D_001A7308+0x130 table) copies the id into the wheel record
 * (D_00255170, stride 0x1C, +0x18) and resolves the weapon's nameStringId into +0x0.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * func_0028C728: g_hudMobySpawnStart+0x28/+0x2C -> g_pActiveTextTable+0x90/+0x94,
 * D_1A8DD0 -> D_1A8E80, D_1A8D48 -> D_1A8DF8, D_002550F0 -> D_00255170,
 * g_equippedItemSlots -> D_001A7308+0x130. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C6B0);
#else
void func_0028C6B0(void) {
    extern void *g_pActiveTextTable;
    extern u8    D_1A8E80;      /* EU wheel record header base (USA D_1A8DD0) */
    extern s32   D_1A8DF8;      /* EU wheel cursor (USA D_1A8D48) */
    extern u8    D_00255170[];  /* EU wheel record array (USA D_002550F0), stride 0x1C */
    extern u8    D_001A7308[];  /* EU region anchor; g_equippedItemSlots at +0x130 */
    s32 i;

    *(s32 *)((u8 *)&g_pActiveTextTable + 0x90) = 8;
    *(void **)((u8 *)&g_pActiveTextTable + 0x94) = &D_1A8E80;
    D_1A8DF8 = 0;
    for (i = 0; i < 8; i++) {
        s32 id = *(s32 *)(D_001A7308 + 0x130 + i * 4);
        u8 *rec = &D_00255170[i * 0x1C];
        *(s32 *)(rec + 0x18) = id;
        *(s32 *)(rec + 0x0) = g_weaponTable[g_itemEquippedSlot[id]].nameStringId;
    }
}
#endif

/* One weapon-select wheel record (stride 0x1C); +0x18 holds the resolved item
 * id. EU twin of the USA WheelRecord. */
typedef struct WheelRecord {
    s32 nameStringId;   /* +0x00 */
    u8  _pad04[0x14];
    s32 itemId;         /* +0x18 */
} WheelRecord;                                /* stride 0x1C */

/* The wheel-record array base pointer is the word at g_pActiveTextTable+0x94
 * (EU 0x1B18D4; the USA twin reads g_hudMobySpawnStart+0x2C). */
#ifdef TARGET_NATIVE
extern void *g_pActiveTextTable;              /* EU 0x1B1840 (used here as +0x94 anchor) */
/* g_equippedItemSlots[8]: EU 0x1A7438, addressed off the unnamed region anchor
 * D_001A7308 (+0x130) — anchor stays its D_ name per the region-anchor rule. */
extern u8 D_001A7308[];                       /* EU region data anchor */
#endif

/* func_0028C730(): EU twin of USA func_0028C7A8 (SyncEquippedItemSlots) — refresh
 * the 8-entry equipped-item cache (g_equippedItemSlots, EU 0x1A7438) from the
 * live weapon-select wheel records. Byte-identical region-agnostic logic; see
 * src/usa/text/188858.c for the recovered behaviour + the cmp-oracle
 * (cmp_188858_wheel.c, 32/32 on real R5900). Only the extern global addresses are
 * region-shifted (wheel-base ptr at g_pActiveTextTable+0x94, slots at
 * D_001A7308+0x130). The original re-reads the base pointer every iteration. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C730);
#else
void func_0028C730(void) {
    WheelRecord **pRec = *(WheelRecord ***)((u8 *)&g_pActiveTextTable + 0x94);
    s32 *slot = (s32 *)(D_001A7308 + 0x130);
    s32  i;

    for (i = 0; i < 8; i++) {
        s32 id = pRec[0][i].itemId;
        if (slot[i] != id) {
            slot[i] = id;
        }
    }
}
#endif

/* func_0028C778(w): EU twin of USA func_0028C7F0 - build the weapon-select wheel
 * widget: rebuild its icon list (func_0028C6B0), then seed geometry/state
 * (half-extents 0xD2/0xC8, type tag -2, cleared cursors). Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0028C7F0:
 * func_0028C728 -> func_0028C6B0; REGION DIVERGENCE: mode word +0x78 = 0x19 (PAL)
 * vs USA 0x1E. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C778);
#else
void func_0028C778(void *w) {
    u8 *b = (u8 *)w;
    func_0028C6B0();
    *(s32 *)(b + 0x58) = 0xD2;
    *(s32 *)(b + 0x5C) = 0xC8;
    *(s32 *)(b + 0x74) = -2;
    *(s32 *)(b + 0x78) = 0x19;   /* PAL mode value; USA twin uses 0x1E */
    *(s32 *)(b + 0x70) = 0;
    *(s16 *)(b + 0x48) = 0;
    *(s16 *)(b + 0x4A) = 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C7C8);

/* DrawWeaponSelectWheel(w): render the weapon quick-select wheel widget. w+0x70/+0x71
 * are open/close animation counters (each /8 clamped to [0,1] -> fillA/fillB); w+0x74
 * the highlighted item; w+0x50/+0x54 the screen anchor aligned by func_0028C108. Per
 * wheel item it drives the highlight tween (func_0034EF38) and pokes the GUI alpha
 * slot. For the highlighted item, unless the popup gate D_1A8D14 is set, it draws the
 * equipped weapon's "%d/%d" ammo counter and the localized name split at the first '-'
 * or ' '. Returns w+0x58. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA DrawWeaponSelectWheel: WrapAnglePiSum -> func_00284458,
 * func_0028C180 -> func_0028C108, func_00290FC0 -> func_00290FC8, func_0034DAB0 ->
 * func_0034EF38, func_002846E8 -> func_002845F8, IntToFloat/FloatToInt ->
 * IntToFloat/B0; D_1A8D48 -> D_1A8DF8, D_1A8C64 -> D_1A8D14, hudMobySpawnStart+0x28/
 * +0x2C -> g_pActiveTextTable+0x90/+0x94, g_equippedItemSlots -> D_001A7308+0x130.
 * REGION DIVERGENCES: GUI alpha slot +0x38000+0x7A8C (EU) vs +0x79DC (USA); PAL text
 * draw uses scaled renderer func_00280218 (+f32 scale) with per-line auto-fit
 * func_00280358, where USA used plain func_002801B8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", DrawWeaponSelectWheel);
#else
s32 DrawWeaponSelectWheel(void *w) {
    extern f32   IntToFloat(s32 v);        /* EU IntToFloat */
    extern s32   func_002845B0(f32 x);        /* EU FloatToInt */
    extern f32   func_00284458(f32 a, f32 b); /* EU wrap-angle (USA WrapAnglePiSum) */
    extern s32   func_00290FC8(void);         /* EU GUI state gate (USA func_00290FC0) */
    extern void  func_0034EF38(void *elem, s32 flag, f32 t);   /* EU highlight tween (USA func_0034DAB0) */
    extern s32   func_002845F8(s32 colorA, s32 colorB, f32 t); /* EU packed-RGBA lerp (USA func_002846E8) */
    extern void  func_00280218(s32 x, s32 y, s32 color, const char *text, s32 count, f32 scale); /* EU scaled text draw */
    extern f32   func_00280358(const char *text, s32 count, s32 maxWidth, f32 scale); /* EU PAL auto-fit measure */
    extern void  func_00115DA8(char *dst, const char *fmt, ...);   /* SDK sprintf */
    extern s32   func_001157AC(const char *s);                     /* SDK strlen */
    extern void *g_pActiveTextTable;
    extern void *g_guiInstance;               /* GUI singleton ptr */
    extern s32   D_1A8DF8;                     /* wheel cursor (USA D_1A8D48) */
    extern s32   D_1A8D14;                     /* GUI popup-busy gate (USA D_1A8C64) */
    extern u8    D_001A7308[];                 /* region anchor; g_equippedItemSlots at +0x130 */
    extern s32   g_weaponAmmo[0x38];
    extern char  g_szAmmoFraction[];

    u8    *pw = (u8 *)w;
    char   buf[0x50];
    s32    posX, posY;
    f32    fillA, fillB, combined;
    s32    alpha, selectedIndex, n, i, off;
    void **table;

    fillA = IntToFloat(*(pw + 0x70)) / IntToFloat(8);
    if (1.0f < fillA) {
        fillA = 1.0f;
    } else if (fillA < 0.0f) {
        fillA = 0.0f;
    }

    fillB = IntToFloat(*(pw + 0x71)) / IntToFloat(8);
    if (1.0f < fillB) {
        fillB = 1.0f;
    } else if (fillB < 0.0f) {
        fillB = 0.0f;
    }

    alpha = func_002845B0(fillA * 128.0f);

    posX = *(s32 *)(pw + 0x50);
    posY = *(s32 *)(pw + 0x54);
    func_0028C108(w, &posX, &posY);

    n = *(s32 *)((u8 *)&g_pActiveTextTable + 0x90);
    if (n > 0) {
        table = *(void ***)((u8 *)&g_pActiveTextTable + 0x94);
        off = 0;
        i = 0;
        do {
            u8 *rec;

            (void)func_00284458(2.0f * (f32)i * 3.14159274f / (f32)n - 3.14159274f,
                                1.57079637f);

            if (*(s32 *)(pw + 0x74) == i) {
                rec = (u8 *)table[D_1A8DF8] + off;
                if (*(s32 *)rec != 0 && g_guiInstance != 0) {
                    f32 fi = (f32)(i + 7);
                    if (fi > 7.0f) {
                        fi -= 8.0f;
                    }
                    func_0034EF38((u8 *)g_guiInstance + 0x376C8, 1, fi);
                }
            }

            rec = (u8 *)table[D_1A8DF8] + off;
            if (*(s32 *)rec != 0 && g_guiInstance != 0) {
                s32 v = (func_00290FC8() != 0) ? func_002845B0((f32)alpha * 0.5f) : 0;
                *(s32 *)((u8 *)g_guiInstance + 0x38000 + 0x7A8C) = v;
            }

            n = *(s32 *)((u8 *)&g_pActiveTextTable + 0x90);
            i++;
            off += 0x1C;
        } while (i < n);
    }

    selectedIndex = *(s32 *)(pw + 0x74);
    if (selectedIndex < 0) {
        return *(s32 *)(pw + 0x58);
    }

    table = *(void ***)((u8 *)&g_pActiveTextTable + 0x94);
    if (*(s32 *)((u8 *)table[D_1A8DF8] + selectedIndex * 0x1C) == 0) {
        return *(s32 *)(pw + 0x58);
    }
    if (D_1A8D14 != 0) {
        return *(s32 *)(pw + 0x58);
    }

    combined = fillA * fillB;

    {
        s32 itemId = *(s32 *)(D_001A7308 + 0x130 + selectedIndex * 4);
        WeaponDef *wd = &g_weaponTable[g_itemEquippedSlot[itemId]];

        if (wd->exists != 0 && wd->sellsAmmoFlag != 0) {
            s32 curAmmo = g_weaponAmmo[itemId];
            u16 capacity = wd->ammoCapacity;
            s32 color;

            func_00115DA8(buf, g_szAmmoFraction, curAmmo, capacity);
            if (curAmmo == 0) {
                color = func_002845F8(0x004040FF, 0x804040FF, combined);   /* out of ammo: red */
            } else {
                color = func_002845F8(0x00F0F0F0, 0x80F0F0F0, combined);   /* white gradient */
            }
            func_00280218(posX + 0x69, posY + 0x70, color, buf, -1, 0.9f);
        }
    }

    {
        s32 textColor = func_002845F8(0x00F0F0F0, 0x80F0F0F0, combined);
        u8 *rec = (u8 *)table[D_1A8DF8] + selectedIndex * 0x1C;
        s32 itemId2 = *(s32 *)(rec + 0x18);
        s16 nameId = *(s16 *)((u8 *)&g_weaponTable[g_itemEquippedSlot[itemId2]] + 0x48);
        char *name = GetLocalizedString(nameId);
        s32 splitLen = 0;
        char *tail = name;
        s32 len, j, lineY;

        if (name == 0) {
            return *(s32 *)(pw + 0x58);
        }
        if (func_001157AC(name) == 0) {
            return *(s32 *)(pw + 0x58);
        }
        len = func_001157AC(name);

        if (len > 0) {
            if (name[1] == '-') {
                tail = name + 2;
                splitLen = 2;
            } else {
                tail = name + 1;
                j = 1;
                while (j < len) {
                    tail++;
                    if (*tail == '-') {
                        splitLen = j + 2;
                        tail++;
                        break;
                    }
                    j++;
                }
            }
        }

        if (splitLen == 0 && 0 < len) {
            if (name[1] == ' ') {
                tail = name + 2;
                splitLen = 1;
            } else {
                tail = name + 1;
                j = 1;
                while (j < len) {
                    tail++;
                    if (*tail == ' ') {
                        splitLen = j + 1;
                        tail++;
                        break;
                    }
                    j++;
                }
            }
        }

        {
            s32 itemId3 = *(s32 *)(D_001A7308 + 0x130 + selectedIndex * 4);
            if (g_weaponTable[g_itemEquippedSlot[itemId3]].sellsAmmoFlag != 0) {
                lineY = posY + 0x4B;
            } else {
                lineY = posY + 0x64;
                if (splitLen != 0) {
                    lineY = posY + 0x55;
                }
            }
        }

        {
            f32 s1, s2;
            if (splitLen != 0) {
                s1 = func_00280358(name, splitLen, 0x50, 0.8f);
                func_00280218(posX + 0x69, lineY, textColor, name, splitLen, s1);
                s2 = func_00280358(tail, -1, 0x50, 0.8f);
                func_00280218(posX + 0x69, lineY + 0x10, textColor, tail, -1, s2);
            } else {
                func_00280218(posX + 0x69, lineY, textColor, name, -1, 0.8f);
            }
        }
    }

    return *(s32 *)(pw + 0x58);
}
#endif

/* func_0028D6F0(w): EU twin of USA func_0028D6D8 - seed a HUD list widget: register
 * the list descriptor (8 entries, callback &D_1A8E88) in g_pActiveTextTable+0x90/+0x94,
 * set geometry (+0x58=0xD2, +0x5C=0xC8, tag +0x74=-2, mode +0x78=0x19), clear the two
 * 16-bit cursors (+0x48/+0x4A) and reset the wheel cursor D_1A8DF8. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0028D6D8:
 * g_hudMobySpawnStart+0x28/+0x2C -> g_pActiveTextTable+0x90/+0x94, D_1A8DD8 -> D_1A8E88,
 * D_1A8D48 -> D_1A8DF8. REGION DIVERGENCE: mode word +0x78 = 0x19 (PAL) vs USA 0x1E. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028D6F0);
#else
void func_0028D6F0(void *w) {
    extern void *g_pActiveTextTable;
    extern u8    D_1A8E88;    /* EU HUD list descriptor callback base (USA D_1A8DD8) */
    extern s32   D_1A8DF8;    /* EU wheel cursor (USA D_1A8D48) */
    u8 *b = (u8 *)w;

    *(s32 *)((u8 *)&g_pActiveTextTable + 0x90) = 8;
    *(void **)((u8 *)&g_pActiveTextTable + 0x94) = &D_1A8E88;
    *(s32 *)(b + 0x58) = 0xD2;
    *(s32 *)(b + 0x78) = 0x19;   /* PAL list mode; USA twin func_0028D6D8 uses 0x1E */
    *(s32 *)(b + 0x5C) = 0xC8;
    *(s32 *)(b + 0x74) = -2;
    *(s16 *)(b + 0x48) = 0;
    *(s16 *)(b + 0x4A) = 0;
    D_1A8DF8 = 0;
}
#endif

/* func_0028D738(w): EU twin of USA func_0028D720 - per-frame input + selection update
 * for the weapon/quick-select wheel. Reads controller port-0 (D_138200): normalises the
 * right-stick and, past 0.9, converts its angle to a wheel slot; when centred it does
 * face-button graph navigation instead. On change it resets the colour pulse
 * (func_002A9FA0) and plays a nav sound (func_002E6C28 id 3). D-pad-Up confirms the
 * selection; an up counter (w+0x70) gates the publish to the HUD moby
 * (g_pActiveTextTable+0xB8/+0xBC/+0xC0 + func_0028BF98). Matching arm stays INCLUDE_ASM;
 * #else is the structure model. Word-verified vs USA func_0028D720: D_138180 -> D_138200,
 * func_002835C0 -> func_002834D0, func_00283BF8 -> func_00283B08, func_00288888 ->
 * func_00288778, func_00288840 -> func_00288730, func_002AA3F0 -> func_002A9FA0,
 * PlayGlobalSound -> func_002E6C28, func_0028C010 -> func_0028BF98; many data re-anchors
 * on g_pActiveTextTable / D_001B1380 / g_sndChannelVolumes. REGION DIVERGENCE: latch
 * timer +0x7C = 0x96 (PAL) vs USA 0xB4. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028D738);
#else
static s32 HudWheelLink(s32 slot, s32 off) {
    extern void *g_pActiveTextTable;
    extern s32   D_1A8DF8;
    void **blockPtrs = *(void ***)((u8 *)&g_pActiveTextTable + 0x94);
    u8    *block     = (u8 *)blockPtrs[D_1A8DF8];
    return *(s32 *)(block + slot * 0x1C + off);
}

void func_0028D738(void *w) {
    extern u8    D_138200[];                 /* controller port-0 state (USA D_138180) */
    extern void *g_pActiveTextTable;
    extern s32   D_1A8DF8;                    /* wheel cursor (USA D_1A8D48) */
    extern s32   D_1A8E70;                    /* default target: triangle (USA D_1A8DC0) */
    extern s32   D_1A8E74;                    /* default target: cross    (USA D_1A8DC4) */
    extern s32   D_1A8E68;                    /* default target: square   (USA D_1A8DB8) */
    extern s32   D_1A8E6C;                    /* default target: circle   (USA D_1A8DBC) */
    extern u8    D_1A7C3B;                    /* gate byte -> func_00288730 (USA D_1A7BBB) */
    extern u8    g_sndChannelVolumes[];       /* committed-category at +0x1778+0x24E4 */
    extern u8    D_001B1380[];                /* render-layer anchor; +0x218 = layer */
    extern f32   func_002834D0(f32 x);        /* EU sqrt (USA func_002835C0) */
    extern f32   func_00283B08(f32 x, f32 y); /* EU atan2 (USA func_00283BF8) */
    extern void  func_00288778(void *w);      /* EU pre-update (USA func_00288888) */
    extern void  func_00288730(void);         /* EU (USA func_00288840) */
    extern s32   func_002A9FA0(s32 a, s32 b, s32 c, s32 d, s32 e); /* EU colour-pulse (USA func_002AA3F0) */
    extern s32   func_002E6C28(s32 id, s32 a, s32 b);  /* EU PlayGlobalSound */

    u8  *pw  = (u8 *)w;
    u8  *pad = D_138200;
    s32 *cat = (s32 *)(g_sndChannelVolumes + 0x1778 + 0x24E4);
    f32  stickX, stickY, mag, angle;
    s32  v78, oldSel, newSel;

    func_00288778(w);

    *(s32 *)(pw + 0x7C) = 0x96;   /* PAL latch timer; USA twin uses 0xB4 */
    *(s32 *)(pw + 0x6C) = 0x18;
    *(s32 *)(pad + 0x1CC) = 2;

    stickX = *(f32 *)(pad + 0x148);
    stickY = *(f32 *)(pad + 0x14C);
    mag = func_002834D0(stickX * stickX + stickY * stickY);
    if (mag != 0.0f) {
        stickX = stickX / mag;
        stickY = stickY / mag;
    }
    angle = func_00283B08(stickX, stickY);

    v78 = *(s32 *)(pw + 0x78);
    if ((v78 >> 24) == 0 && *(s16 *)((u8 *)&g_pActiveTextTable + 0x5C) != 0) {
        s32 buttons = *(s32 *)(pad + 0x1C4);
        if ((buttons & 0xF000) != 0 || mag < 0.5f) {
            *(s32 *)(pw + 0x74) = -1;
            *(s32 *)(pw + 0x78) = 0x010000FF;
        } else {
            s32 dec = v78 - 1;
            *(s32 *)(pw + 0x78) = dec;
            if (dec == -1) {
                *(s32 *)(pw + 0x74) = -1;
                *(s32 *)(pw + 0x78) = 0x010000FF;
            }
        }
    }

    oldSel = *(s32 *)(pw + 0x74);
    if (*(s8 *)(pw + 0x7B) == 1) {
        if (0.0f < mag) {
            if (0.9f < mag) {
                s32 n = *(s32 *)((u8 *)&g_pActiveTextTable + 0x90);
                f32 t = (((angle + 3.14159274f) + 3.14159274f + 1.57079637f)
                         + 3.14159274f / (f32)n)
                        * 0.159154937f * (f32)n;
                *(s32 *)(pw + 0x74) = (s32)t % n;
            }
        } else {
            s32 buttons = *(s32 *)(pad + 0x1C4);
            if ((buttons & 0xF000) != 0) {
                s32 sel;
                if ((buttons & 0x1000) != 0) {
                    sel = (oldSel == -1) ? D_1A8E70 : HudWheelLink(oldSel, 0x10);
                } else {
                    sel = oldSel;
                }
                if ((*(s32 *)(pad + 0x1C4) & 0x4000) != 0) {
                    sel = (sel == -1) ? D_1A8E74 : HudWheelLink(sel, 0x14);
                }
                if ((*(s32 *)(pad + 0x1C4) & 0x8000) != 0) {
                    sel = (sel == -1) ? D_1A8E68 : HudWheelLink(sel, 0x08);
                }
                if ((*(s32 *)(pad + 0x1C4) & 0x2000) != 0) {
                    sel = (sel == -1) ? D_1A8E6C : HudWheelLink(sel, 0x0C);
                }
                *(s32 *)(pw + 0x74) = sel;
            }
        }
    }

    newSel = *(s32 *)(pw + 0x74);
    if (newSel != oldSel) {
        func_002A9FA0(0, 0, 1, 0, 1);
        *(u8 *)(pw + 0x71) = 0;
        func_002E6C28(3, 0, 0);
    } else {
        u8 hold = *(u8 *)(pw + 0x71);
        if (hold < 8) {
            *(u8 *)(pw + 0x71) = hold + 1;
        }
    }

    if ((*(s32 *)(pad + 0x1C0) & 0x10) != 0) {
        u8 up = *(u8 *)(pw + 0x70);
        if (up < 8) {
            *(u8 *)(pw + 0x70) = up + 1;
        }
    } else {
        if ((*(s32 *)(pad + 0x1C8) & 0x10) != 0) {
            s32 sel = *(s32 *)(pw + 0x74);
            *cat = (sel == -1) ? 0 : HudWheelLink(sel, 0x18);
        }
        if (D_1A7C3B != 0) {
            func_00288730();
        }
        {
            u8 up = *(u8 *)(pw + 0x70);
            if (up != 0) {
                *(u8 *)(pw + 0x70) = up - 1;
            }
        }
    }

    if (*cat == 6 && *(s16 *)((u8 *)&g_pActiveTextTable + 0x60) == 0) *cat = 0;
    if (*cat == 7 && *(s16 *)((u8 *)&g_pActiveTextTable + 0x64) == 0) *cat = 0;
    if (*cat == 5 && *(s16 *)((u8 *)&g_pActiveTextTable + 0x62) == 0) *cat = 0;

    if (*(u8 *)(pw + 0x70) == 0) {
        *(s32 *)((u8 *)&g_pActiveTextTable + 0xBC) = *(s32 *)(pw + 0x78);
        *(s32 *)((u8 *)&g_pActiveTextTable + 0xC0) = *(s32 *)(pw + 0x74);
        func_0028BF98(*(s32 *)(pw + 0x64));
        *(s32 *)(pw + 0x6C) = -6;
        *(s32 *)((u8 *)&g_pActiveTextTable + 0xB8) = *(s32 *)(D_001B1380 + 0x218);
    }
}
#endif

/* func_0028DC40(hud): EU twin of USA func_0028DC28 - draw the weapon-select wheel /
 * item ring. 12 chrome/frame glyphs, the selection cursor + count glyph, the item-ring
 * walk (per-category colour + slot bg + icon), and the active slot's localized label.
 * Returns the widget status word at hud+0x58. Matching arm stays INCLUDE_ASM; #else is
 * the structure model. Word-verified vs USA func_0028DC28 (positional jal + reloc map):
 * IntToFloat->IntToFloat, GuiFontAtlasLookupGlyph->func_00338AA8, func_003017F8->
 * func_00301AC0, func_002AA3F0->func_002A9FA0, func_002904B0->func_002904C8,
 * WrapAnglePiSum->func_00284458, func_00283B30/B48->func_00283A40/A58, func_0028EDF0->
 * func_0028EE08, func_0028F2C0->func_0028F2D8, func_002846E8->func_002845F8,
 * func_002801B8->func_00280050; data g_hudClutSlots->g_pActiveTextTable+0x58,
 * hudMobySpawnStart+0x28/+0x2C->g_pActiveTextTable+0x90/+0x94, g_swapGadgetItemIndex+0x8E
 * ->g_nVendorBuyQuantity+0x160, D_1A8D48->D_1A8DF8, D_1A8E30->D_1A8EE0, D_1A8E70->D_1A8F20.
 * REGION DIVERGENCE (language table): wheel-label string ids 0xE59/0xE5A (USA 0x2DCA/0x2DCB). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028DC40);
#else
extern void *g_guiInstance;                 /* GUI singleton ptr (EU 0x1A8DAC) */
extern void *g_pActiveTextTable;            /* HUD moby/clut anchor block */
extern s32   g_nVendorBuyQuantity;          /* yfudge at +0x160 (USA g_swapGadgetItemIndex+0x8E) */
extern s32   D_1A8DF8;                       /* wheel cursor / spawn-table index (gp) */
extern f32   IntToFloat(s32 v);           /* EU IntToFloat */
extern s32   func_00338AA8(void *atlas, s32 cp);                 /* EU GuiFontAtlasLookupGlyph */
extern void  func_00301AC0(s32 glyph, s32 color, f32 *a, f32 *b,
                           f32 x, f32 y, f32 sx, f32 yf, f32 extra); /* EU draw glyph */
extern s32   func_002A9FA0(s32 a, s32 b, s32 c, s32 d, s32 e);   /* EU pulse colour */
extern void  func_002904C8(s32 x0, s32 y0, s32 x1, s32 y1, u64 reg4, s32 mode);
extern f32   func_00284458(f32 a, f32 b);    /* EU WrapAnglePiSum */
extern f32   func_00283A40(f32 angle);       /* EU ring X term */
extern f32   func_00283A58(f32 angle);       /* EU ring Y term */
extern s32   func_0028EE08(s32 name, s32 level);   /* EU twin of USA func_0028EDF0 */
extern void  func_0028F2D8(s32 icon, s32 x, s32 y, s32 w, s32 h, void *colorArr);
extern s32   func_002845F8(s32 a, s32 b, f32 t);   /* EU colour lerp (USA func_002846E8) */
extern void  func_00280050(s32 x, s32 y, u32 color, char *str, s32 n); /* EU text draw */
extern char *GetLocalizedString(s32 textId);
extern s32   func_001157AC(const char *s);   /* SDK strlen */
typedef struct WheelCursorRect { s32 x, y; } WheelCursorRect;
extern WheelCursorRect D_1A8EE0[8];          /* selection-cursor box positions */
extern s32   D_1A8F20[8];                    /* per-slot label text ids */

/* g_hudClutSlots availability flags, EU-anchored at g_pActiveTextTable+0x58 */
#define CLUT16(off) (*(s16 *)((u8 *)&g_pActiveTextTable + 0x58 + (off)))

s32 func_0028DC40(void *hud) {
    static const struct { s32 cp; u32 color; f32 sx; } topGlyphs[12] = {
        {0x08, 0x60442D00,  1.0f}, {0x09, 0x60442D00,  1.0f},
        {0x0A, 0x60442D00,  1.0f}, {0x0B, 0x60442D00,  1.0f},
        {0x0C, 0x60442D00,  1.0f}, {0x0D, 0x60442D00,  1.0f},
        {0x0E, 0x60442D00,  1.0f}, {0x0F, 0x60442D00,  1.0f},
        {0x07, 0x60442D00,  1.0f}, {0x07, 0x60442D00, -1.0f},
        {0x86, 0x55F0C070,  1.0f}, {0x86, 0x55F0C070, -1.0f},
    };
    f32   yfudge  = *(f32 *)((u8 *)&g_nVendorBuyQuantity + 0x160);
    s32   clutSum = CLUT16(0x8) + CLUT16(0xC) + CLUT16(0xA);
    s32   active  = *(s32 *)((u8 *)hud + 0x74);
    void *atlas   = (u8 *)g_guiInstance + 0x8710;
    f32   fillA, fillB;
    s32   selectedDrawn = 0;
    s32   n, i, k;

    fillA = IntToFloat(*((u8 *)hud + 0x70)) / IntToFloat(8);
    if (1.0f < fillA) {
        fillA = 1.0f;
    } else if (fillA < 0.0f) {
        fillA = 0.0f;
    }

    fillB = IntToFloat(*((u8 *)hud + 0x71)) / IntToFloat(8);
    if (1.0f < fillB) {
        fillB = 1.0f;
    } else if (fillB < 0.0f) {
        fillB = 0.0f;
    }

    for (k = 0; k < 12; k++) {
        func_00301AC0(func_00338AA8(atlas, topGlyphs[k].cp),
                      (s32)topGlyphs[k].color, (f32 *)0, (f32 *)0,
                      129.0f, 208.0f, topGlyphs[k].sx, yfudge, 0.0f);
    }

    if (active >= 0) {
        s32 pulse = func_002A9FA0(0x80442D00, 0x80FFDE8D, 0x19, 0, 0);
        s32 count = (active - 1 > -1) ? (active - 1) : 7;
        s32 glyph = func_00338AA8(atlas, 0x10);
        func_00301AC0(glyph, pulse, (f32 *)0, (f32 *)0,
                      129.0f, 208.0f, 1.0f, yfudge, (f32)count);
        func_002904C8(D_1A8EE0[active].x, D_1A8EE0[active].y,
                      D_1A8EE0[active].x + 0x21, D_1A8EE0[active].y + 7,
                      (u64)(u32)pulse, 0);
    }

    n = *(s32 *)((u8 *)&g_pActiveTextTable + 0x90);
    for (i = 0; i < n; i++) {
        void **table = *(void ***)((u8 *)&g_pActiveTextTable + 0x94);
        u8   *rec    = (u8 *)table[D_1A8DF8] + i * 0x1C;
        f32   angle  = func_00284458(2.0f * (f32)i * 3.14159274f / (f32)n - 3.14159274f,
                                     1.57079637f);
        s32   ringX  = (s32)(func_00283A40(angle) * 80.0f) + 0x72;
        s32   ringY  = (s32)(func_00283A58(angle) * 75.0f) + 0xC0;

        if (*(s32 *)rec != 0) {
            s32 kind     = *(s32 *)(rec + 4);
            s32 drawBg   = 0;
            s32 drawIcon = 0;
            u32 color    = 0;
            u32 colorBg[4], colorGlyph[4];
            s32 color2;
            s32 iconA, iconB;

            if (kind == 6 && CLUT16(0x6) == 0) {
                kind = 7;
            }

            if ((u32)kind < 0xA) {
                switch (kind) {
                case 2:
                    if (CLUT16(0x4) != 0) {
                        color = 0x6000FF00; drawBg = 1; drawIcon = 1;
                        if (i == active) selectedDrawn = 1;
                    }
                    break;
                case 3:
                    if (CLUT16(0x4) != 0) {
                        color = 0x6029A1FF; drawBg = 1; drawIcon = 1;
                        if (i == active) selectedDrawn = 1;
                    }
                    break;
                case 0:
                    if (CLUT16(0x4) != 0 && clutSum < CLUT16(0x4)) {
                        color = 0x600000FF; drawBg = 1; drawIcon = 1;
                        if (i == active) selectedDrawn = 1;
                    }
                    break;
                case 1:
                    if (CLUT16(0x4) != 0 && clutSum < CLUT16(0x4)) {
                        color = 0x60FF0000; drawBg = 1; drawIcon = 1;
                        if (i == active) selectedDrawn = 1;
                    }
                    break;
                case 4:
                    if (CLUT16(0xA) != 0) {
                        drawIcon = 1;
                        if (i == active) selectedDrawn = 1;
                    }
                    break;
                case 6:
                case 7:
                    if (CLUT16(0x8) != 0) {
                        drawIcon = 1;
                        if (i == active) selectedDrawn = 1;
                    }
                    break;
                case 8:
                    if (CLUT16(0xC) != 0) {
                        drawIcon = 1;
                        if (i == active) selectedDrawn = 1;
                    }
                    break;
                case 9:
                    if (CLUT16(0x4) != 0) {
                        drawIcon = 1;
                        if (i == active) selectedDrawn = 1;
                    }
                    break;
                case 5:
                default:
                    break;
                }
            }

            if (drawBg) {
                iconA = func_0028EE08(*(s32 *)rec, 0xA);
                colorBg[0] = colorBg[1] = colorBg[2] = colorBg[3] = color;
                func_0028F2D8(iconA, ringX + 2, ringY + 2, 0x1C, 0x1A, colorBg);
            }

            color2 = drawIcon ? (s32)0x80FFDE8D : 0x40808080;
            colorGlyph[0] = colorGlyph[1] = colorGlyph[2] = colorGlyph[3] = (u32)color2;
            iconB = func_0028EE08(*(s32 *)rec, kind);
            func_0028F2D8(iconB, ringX, ringY, 0x1E, 0x1C, colorGlyph);
        }
        n = *(s32 *)((u8 *)&g_pActiveTextTable + 0x90);
    }

    active = *(s32 *)((u8 *)hud + 0x74);
    if (selectedDrawn && active >= 0 && active < 8) {
        s32   textId = D_1A8F20[active];
        char *str;

        if (textId == 0xE59 && CLUT16(0x6) != 0) {   /* REGION: EU string id (USA 0x2DCA) */
            textId = 0xE5A;                            /* REGION: EU string id (USA 0x2DCB) */
        }
        str = GetLocalizedString(textId);
        if (str != 0 && func_001157AC(str) != 0) {
            f32 t = fillB * fillA;
            s32 colorMain   = func_002845F8(0x00E0C0A0, (s32)0x80E0C0A0, t);
            s32 colorShadow = func_002845F8(0x00000000, (s32)0x80000000, t);
            func_00280050(0x80, 0xC8, (u32)colorShadow, str, -1);
            func_00280050(0x81, 0xC9, (u32)colorMain, str, -1);
        }
    }

    return *(s32 *)((u8 *)hud + 0x58);
}
#undef CLUT16
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028E658);

/* Seed a HUD widget record's geometry (+0x5C/+0x58 = 0x20, type tag +0x7C =
 * 0x82 in PAL; the USA twin func_0028E7A0 uses 0x96 - a region-divergent HUD
 * widget type id) then re-init it via func_0028C318 (empty-asm guard keeps the jal). */
void func_0028E7B8(u8 *p) {
    *(s32 *)(p + 0x58) = 0x20;
    *(s32 *)(p + 0x7C) = 0x82;
    *(s32 *)(p + 0x5C) = 0x20;
    func_0028C318(p);
    __asm__ __volatile__("");
}

/* Stub returning 0. */
s32 func_0028E7E8(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028E7F0);

/* func_0028E800(slot): EU twin of USA func_0028E7E8 - pick the current animation frame
 * for a HUD icon widget and cache it at slot+0x4. Modes 0 hold / 1 loop / 2 ping-pong /
 * 3 one-shot ping-pong (then hold + reschedule a random restart). texId 0xFFFF or an
 * unknown mode yields 0. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0028E7E8: g_pHudAssetHeader[1] icon table ->
 * g_pActiveTextTable+0x4C; g_gameTime -> g_nLevelExitDestination+0x8; GetRandomInt ->
 * func_002A81F8. REGION DIVERGENCE (timing): mode-3 restart is 2*count + rand(0x19) + 0x8
 * (USA rand(0x1E) + 0xA). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028E800);
#else
extern void *g_pActiveTextTable;             /* HUD icon-slot table ptr at +0x4C */
extern s32   g_nLevelExitDestination;        /* g_gameTime at +0x8 (EU 0x1B1688) */
extern s32   func_002A81F8(s32 max);         /* EU GetRandomInt (uniform [0,max)) */
#define GT (*(s32 *)((u8 *)&g_nLevelExitDestination + 0x8))
s32 func_0028E800(void *slot) {
    typedef struct HudIconSlot {
        u16 texId;      /* +0x0 */
        u16 maxLevel;   /* +0x2 */
        u16 baseFrame;  /* +0x4 */
        u8  paletteId;  /* +0x6 */
        u8  perFrameDur;/* +0x7 */
    } HudIconSlot;
    u8 *w = (u8 *)slot;
    HudIconSlot *table = *(HudIconSlot **)((u8 *)&g_pActiveTextTable + 0x4C);
    HudIconSlot *icon = &table[*(s16 *)(w + 0x0)];
    s32 dur;
    s32 count;
    s32 phase;
    s32 frame = 0;

    if (icon->texId == 0xFFFF) {
        *(s32 *)(w + 0x4) = 0;
        return 0;
    }

    switch (*(u8 *)(w + 0x2)) {
    case 0:
        frame = icon->baseFrame;
        break;
    case 1:
        dur = icon->perFrameDur;
        frame = icon->baseFrame + (GT - *(s32 *)(w + 0xC)) / dur % icon->maxLevel;
        break;
    case 2:
        dur = icon->perFrameDur;
        count = icon->maxLevel;
        phase = (GT - *(s32 *)(w + 0xC)) / dur % (2 * count - 2);
        frame = icon->baseFrame + (phase < count ? phase : 2 * count - (phase + 2));
        break;
    case 3:
        if (*(s32 *)(w + 0xC) < GT) {
            dur = icon->perFrameDur;
            count = icon->maxLevel;
            phase = (GT - *(s32 *)(w + 0xC)) / dur;
            if (phase < 2 * count - 2) {
                frame = icon->baseFrame + (phase < count ? phase : 2 * count - (phase + 2));
            } else {
                frame = icon->baseFrame;
                *(s32 *)(w + 0xC) = 2 * count + func_002A81F8(0x19) + 0x8; /* REGION timing */
            }
        } else {
            frame = *(s32 *)(w + 0x4);
        }
        break;
    }

    *(s32 *)(w + 0x4) = frame;
    return frame;
}
#undef GT
#endif

/* func_0028E9B8(runTickCallbacks): EU twin of USA func_0028E9A0 - per-frame update pass
 * over the 13-entry HUD element registry D_255330 (stride 0x90). After a one-shot
 * (re)bind pass (func_0028EB28), for each record it clamps the +0x7C countdown up, ticks
 * it down, bumps/decays the +0x6C phase, re-activates dirty records at the -6 floor via
 * func_0028BEA0, and invokes the +0x14 tick callback when runTickCallbacks is set.
 * Returns the count of records whose countdown was still running (>= 2). Matching arm
 * stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0028E9A0:
 * D_2552B0->D_255330; func_0028EB10->func_0028EB28; func_0028BF18->func_0028BEA0;
 * g_pActiveTextTable+0x34 kept. REGION DIVERGENCE (timing): +0x7C clamp 0x8 (USA 0xA). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028E9B8);
#else
extern u8    D_255330[];                     /* HUD element registry (13 x 0x90) */
extern void *g_pActiveTextTable;             /* text gate at +0x34 */
extern void  func_0028EB28(void);            /* ammo-vendor (re)bind pass */
extern void  func_0028BEA0(void *w);         /* HUD widget (re)activate */
s32 func_0028E9B8(s32 runTickCallbacks) {
    u8 *rec = D_255330;
    u8 *end = D_255330 + 0x750;
    s32 activeCount = 0;

    func_0028EB28();

    do {
        s32 *countdown = (s32 *)(rec + 0x7C);
        s32 *phase = (s32 *)(rec + 0x6C);
        s32  t;

        if ((*(s32 *)(rec + 0x4) & 0x10) ||
            (*(s32 *)((u8 *)&g_pActiveTextTable + 0x34) != 0)) {
            if (*countdown < 0x8) {          /* REGION: EU clamp 0x8 (USA 0xA) */
                *countdown = 0x8;
            }
        }

        t = *countdown;
        if (t >= 1) {
            *countdown = t - 1;
        }
        if (t >= 2) {
            activeCount++;
            if (*phase < 0x1E) {
                *phase += 1;
            }
        } else if (*phase >= -5) {
            *phase -= 1;
        }

        if (*(s32 *)(rec + 0x68) != 0 && *phase == -6) {
            func_0028BEA0(rec);
        }

        if (runTickCallbacks != 0) {
            void (*tickFn)(void *) = *(void (**)(void *))(rec + 0x14);
            if (tickFn != 0) {
                tickFn(rec);
            }
        }

        rec += 0x90;
    } while (rec < end);

    return activeCount;
}
#endif

/* func_0028EAE0(): EU twin of USA func_0028EAC8 - return the active weapon's item id when
 * that weapon sells ammo (g_weaponTable[slot].sellsAmmoFlag != 0) and the "no ammo sale"
 * gate is clear, else 0. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0028EAC8: g_soundBankHandlesBlk -> g_sndChannelVolumes+0x1778
 * (itemId at +0x1248, no-sale byte at +0x22B4); g_itemEquippedSlot / g_weaponTable kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028EAE0);
#else
extern u8 g_sndChannelVolumes[];             /* sound-bank state block base */
s32 func_0028EAE0(void) {
    u8 *blk = g_sndChannelVolumes + 0x1778;
    s32 itemId = *(s32 *)(blk + 0x1248);
    u8  noSale = blk[0x22B4];
    u8  equippedSlot = g_itemEquippedSlot[itemId];
    u16 sellsAmmo = g_weaponTable[equippedSlot].sellsAmmoFlag;
    if (sellsAmmo == 0 || noSale != 0) {
        return 0;
    }
    return itemId;
}
#endif

/* func_0028EB28(): EU twin of USA func_0028EB10 - one-shot ammo-vendor HUD-widget (re)bind
 * pass. No-op while the hard-disable gate D_1A906C or the "already armed" gate D_1A9060 is
 * set. Otherwise asks func_0028EAE0 for the active ammo-selling weapon: if a real ammo
 * weapon is equipped it registers the vendor widget (func_0028BD98) and stashes the handle
 * in D_1A9068 + item in D_1A9064; else (sound-bank state byte == 2) it tears down any live
 * widget and spawns the generic bank widget, else just tears down. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0028EB10:
 * D_1A8FBC->D_1A906C, D_1A8FB0->D_1A9060, D_1A8FB4->D_1A9064, D_1A8FB8->D_1A9068;
 * func_0028EAC8->func_0028EAE0, func_0028BE10->func_0028BD98, func_0028C108->func_0028C090;
 * callbacks func_0028E7A0/E7D0/C490/C4C8/E640 -> func_0028E7B8/E7E8/C418/C450/E658;
 * D_002907C0->D_002907D8; g_soundBankHandlesBlk->g_sndChannelVolumes+0x1778. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028EB28);
#else
extern s32  func_0028BD98(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_0028C090(s32 key, s32 value);
extern void func_0028E658(void);             /* generic-bank draw callback (INCLUDE_ASM) */
extern void func_0028C450(void);             /* generic-bank tick callback (INCLUDE_ASM) */
extern u8   g_sndChannelVolumes[];           /* sound-bank state block base */
extern u8   D_002907D8[];                    /* ammo-vendor widget layout blob (EU 0x2907D8) */
extern s32  D_1A906C;                         /* gp hard-disable gate (EU 0x1A906C) */
void func_0028EB28(void) {
    u8 *blk;
    s32 itemId;
    u8  slot;
    WeaponDef *w;

    if (D_1A906C != 0) {
        return;                              /* hard-disabled */
    }
    if (D_1A9060 != 0) {
        return;                              /* countdown already armed */
    }

    itemId = func_0028EAE0();
    slot = g_itemEquippedSlot[itemId];
    w = &g_weaponTable[slot];
    if (itemId != 0 && w->exists != 0) {
        D_1A9064 = itemId;
        D_1A9068 = func_0028BD98(0x10, w->nameStringId,
                                 (s32)&func_0028E7B8, (s32)D_002907D8,
                                 (s32)&func_0028E7E8, (s32)&g_weaponAmmo[itemId],
                                 w->ammoCapacity);
        return;
    }

    blk = g_sndChannelVolumes + 0x1778;
    if (blk[0x22B4] == 2) {
        if (D_1A9068 != -1) {
            func_0028C090(D_1A9068, 0);
            D_1A9068 = -1;
        }
        *(s16 *)(blk + 0x1878) =
            (s16)func_0028BD98(0x10, 0xFFFF,
                               (s32)&func_0028C418, (s32)&func_0028C450,
                               (s32)&func_0028E658 + 0x40,
                               (s32)(blk + 0x1874), 0xC8);
    } else {
        if (D_1A9068 != -1) {
            func_0028C090(D_1A9068, 0);
            D_1A9068 = -1;
        }
    }
}
#endif

/* Re-seed the HUD widget table (func_0028BF08), then arm the countdown gate
 * (D_1A9060 = 1) and its companion latches (D_1A9068 = -1, D_1A9064 = 0).
 * (USA func_0028EC70.) */
void func_0028EC88(void) {
    func_0028BF08();
    D_1A9060 = 1;
    D_1A9068 = -1;
    D_1A9064 = 0;
}

/* Decrement the countdown gate at D_1A9060, clamping at 0. */
void func_0028ECC0(void) {
    if (D_1A9060 != 0) {
        D_1A9060 = D_1A9060 - 1;
    }
}

/* func_0028ECD8(): EU twin of USA func_0028ECC0 - per-frame HUD tick. Bails (clearing the
 * HUD CLUT slot) if that slot is busy or the screen is mid white-fade. Otherwise runs each
 * of the 13 D_255330 widget records' +0x18 init callback (jalr, passing the record), then
 * advances the nanotech-orb HUD counters. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA func_0028ECC0: g_hudClutSlots+0x10 ->
 * g_pActiveTextTable+0x68; g_screenFadeWhite+0x4 -> D_001B1380+0x228; g_pActiveTextTable+0x3C
 * kept; D_2552B0->D_255330; g_pActiveNanotechOrb +0x4/+0x8/+0xC and g_nGameState kept.
 * REGION DIVERGENCE (timing): orb+0x8 ramp step 0x12 (USA 0x10). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028ECD8);
#else
extern void *g_pActiveTextTable;             /* HUD CLUT slot at +0x68, text gate +0x3C */
extern u8    D_001B1380[];                    /* render-layer anchor; +0x228 = white-fade level */
extern u8    D_255330[];                      /* HUD element registry (13 x 0x90) */
extern u8    g_pActiveNanotechOrb[];          /* nanotech-orb HUD counters at +0x4/+0x8/+0xC */
extern s32   g_nGameState;
void func_0028ECD8(void) {
    s32 *hudClut = (s32 *)((u8 *)&g_pActiveTextTable + 0x68);
    s32 *orb4 = (s32 *)(g_pActiveNanotechOrb + 0x4);
    s32 *orb8 = (s32 *)(g_pActiveNanotechOrb + 0x8);
    s32 *orbC = (s32 *)(g_pActiveNanotechOrb + 0xC);
    s32 i;

    if (*hudClut != 0 || *(s32 *)(D_001B1380 + 0x228) != 0) {
        *hudClut = 0;
        return;
    }

    *(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) = 0xFFFFF0;
    for (i = 0; i < 13; i++) {
        void (*cb)(u8 *) = *(void (**)(u8 *))(D_255330 + i * 0x90 + 0x18);
        if (cb != 0) {
            cb(D_255330 + i * 0x90);
        }
    }

    if ((*orb4 == 0 && *orb8 == 0) || g_nGameState != 0) {
        *orbC = 0x64;
    } else {
        if (*orb4 != 0) {
            s32 v = *orb8 + 0x12;            /* REGION: EU ramp step 0x12 (USA 0x10) */
            *orb8 = (v < 0x81) ? v : 0x80;
        } else {
            s32 v = *orb8 - 0x12;            /* REGION: EU ramp step 0x12 (USA 0x10) */
            *orb8 = (v >= 0) ? v : 0;
        }
        if (*orb4 != 0) {
            *orb4 = *orb4 - 1;
        }
        if (*orb4 == 0x3E8) {
            *orb4 = 0;
        }
    }
}
#endif

/* func_0028EE08(name, level): EU twin of USA func_0028EDF0 - resolve the live HUD icon-map
 * index for icon `name` at upgrade `level`. Looks the icon up (func_0028B4E8), bounds-checks
 * `level` against the slot's frame count, then verifies both the CLUT and texture GS slots
 * are allocated. Returns the map index (baseFrame + level) on success, else 0. Matching arm
 * stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0028EDF0:
 * func_0028B560->func_0028B4E8; icon table g_pHudAssetHeader[1] -> g_pActiveTextTable+0x4C;
 * g_hudIconMap -> ptr at +0x50; g_hudClutSlots -> ptr at +0x58; g_hudTextureSlots -> ptr at +0x54. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028EE08);
#else
extern void *g_pActiveTextTable;             /* icon table +0x4C, map +0x50, tex +0x54, clut +0x58 */
extern s32   func_0028B4E8(s32 name);        /* EU twin of USA func_0028B560 */
s32 func_0028EE08(s32 name, s32 level) {
    typedef struct HudIconSlot {
        u16 texId; u16 maxLevel; u16 baseFrame; u8 paletteId; u8 _pad7;
    } HudIconSlot;                            /* stride 0x8 */
    typedef struct HudIconMapEntry { s16 clutSlot; s16 textureSlot; } HudIconMapEntry; /* stride 0x4 */
    typedef struct HudGsSlot { s32 handle; s32 _pad4; } HudGsSlot;                     /* stride 0x8 */
    HudIconSlot     *table = *(HudIconSlot **)((u8 *)&g_pActiveTextTable + 0x4C);
    HudIconSlot     *slot  = &table[func_0028B4E8(name)];
    HudIconMapEntry *map;
    HudGsSlot       *clut;
    HudGsSlot       *tex;
    s32 mapIndex;

    if (slot->texId == 0xFFFF) {
        return 0;
    }
    if (level < (s32)slot->maxLevel) {
        mapIndex = slot->baseFrame + level;
        map  = *(HudIconMapEntry **)((u8 *)&g_pActiveTextTable + 0x50);
        clut = *(HudGsSlot **)((u8 *)&g_pActiveTextTable + 0x58);
        if (clut[map[mapIndex].clutSlot].handle < 0) {
            return 0;
        }
        tex = *(HudGsSlot **)((u8 *)&g_pActiveTextTable + 0x54);
        if ((tex[map[mapIndex].textureSlot].handle & 0x80000000) == 0) {
            return mapIndex;
        }
    }
    return 0;
}
#endif

/* func_0028EEC0: EU twin of USA GetHudIconTex0 - resolve one HUD icon's GS TEX0
 * register. Looks the icon up in the HUD icon map to get its CLUT and texture GS
 * slots; if either is not resident in VRAM (vramAddr == 0) it allocates a VRAM block
 * and queues an upload. Returns the assembled 64-bit GS TEX0 register. Pure leaf.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified vs USA
 * GetHudIconTex0 (data lane +0x80): g_hudIconMap -> g_pActiveTextTable+0x50,
 * g_hudTextureSlots -> +0x54, g_hudClutSlots -> +0x58; g_vramAllocCursor ->
 * D_001A7308+0x48, g_vramFrameBufB -> D_001A7308+0x54; g_texUploadCount ->
 * D_001B1380+0x27C; g_texUploadQueue -> g_nVendorBuyQuantity+0x70F8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028EEC0);
#else
u64 func_0028EEC0(s32 iconId) {
    typedef struct HudIconMapEntry { s16 clutSlot; s16 textureSlot; } HudIconMapEntry; /* stride 0x4 */
    typedef struct HudGsSlot { s32 handle; u16 vramAddr; u8 logW; u8 logH; } HudGsSlot; /* stride 0x8 */
    typedef struct HudTexUploadEntry {         /* stride 0x10 */
        s32 clutSrc; u16 clutDst; u16 clutWidth; s32 texSrc; u8 texLogW; u8 texLogH; u16 texDst;
    } HudTexUploadEntry;
    extern void *g_pActiveTextTable;           /* map +0x50, tex +0x54, clut +0x58 */
    extern u8    D_001A7308[];                 /* vramAllocCursor +0x48, vramFrameBufB +0x54 */
    extern u8    D_001B1380[];                 /* texUploadCount +0x27C */
    extern s32   g_nVendorBuyQuantity;         /* texUploadQueue base +0x70F8 */

    HudIconMapEntry *map     = *(HudIconMapEntry **)((u8 *)&g_pActiveTextTable + 0x50);
    HudGsSlot       *clutTab = *(HudGsSlot **)((u8 *)&g_pActiveTextTable + 0x58);
    HudGsSlot       *texTab  = *(HudGsSlot **)((u8 *)&g_pActiveTextTable + 0x54);
    HudIconMapEntry *e    = &map[iconId];
    HudGsSlot       *clut = &clutTab[e->clutSlot];
    HudGsSlot       *tex  = &texTab[e->textureSlot];
    HudTexUploadEntry *queue = (HudTexUploadEntry *)((u8 *)&g_nVendorBuyQuantity + 0x70F8);
    s32 queued = 0;

    if (clut->vramAddr == 0 || tex->vramAddr == 0) {
        HudTexUploadEntry *ent = &queue[*(s32 *)(D_001B1380 + 0x27C)];
        ent->clutSrc   = clut->handle;
        ent->clutDst   = 0;
        ent->clutWidth = 0x3FF0;
        ent->texSrc    = clut->handle;
        ent->texDst    = 0x3FF0;
        ent->texLogH   = 5;
        ent->texLogW   = 5;

        if (clut->vramAddr == 0) {
            u32 cursor = *(u32 *)(D_001A7308 + 0x48);
            s32 count  = *(s32 *)(D_001B1380 + 0x27C);
            clut->vramAddr = (u16)(cursor >> 8);
            *(u32 *)(D_001A7308 + 0x48) = cursor + 0x400;   /* CLUT is 0x400 bytes */
            if (count < 0x40) {
                HudTexUploadEntry *ce = &queue[count];
                ce->clutSrc   = clut->handle;
                ce->clutDst   = 0;
                ce->clutWidth = clut->vramAddr;
                queued = 1;
            }
        }
        if (tex->vramAddr == 0) {
            u32 cursor = *(u32 *)(D_001A7308 + 0x48);
            u32 maxLog = (tex->logH < tex->logW) ? tex->logW : tex->logH;
            s32 count  = *(s32 *)(D_001B1380 + 0x27C);
            tex->vramAddr = (u16)(cursor >> 8);
            *(u32 *)(D_001A7308 + 0x48) = cursor + (1u << (2 * maxLog));
            if (count < 0x40) {
                HudTexUploadEntry *te = &queue[count];
                te->texSrc  = tex->handle;
                te->texLogW = tex->logW;
                te->texLogH = tex->logH;
                te->texDst  = tex->vramAddr;
                queued = 1;
            }
        }
        if (queued) {
            (*(s32 *)(D_001B1380 + 0x27C))++;
        }
    }

    {
        s32 logW     = tex->logW;
        s32 tbwShift = (logW >= 6) ? (logW - 6) : 0;
        u64 tex0     = (u64)tex->vramAddr;                       /* TBP0  bits 0-13 */
        tex0 |= (u64)(1u << tbwShift) << 14;                     /* TBW   bits 14-19 */
        tex0 |= (u64)(((s32)tex->vramAddr < (*(s32 *)(D_001A7308 + 0x54) >> 8)) ? 27 : 19) << 20; /* PSM */
        tex0 |= (u64)tex->logW << 26;                           /* TW    bits 26-29 */
        tex0 |= (u64)tex->logH << 30;                           /* TH    bits 30-33 */
        tex0 |= ((u64)clut->vramAddr << 37) | ((u64)0x8000 << 19); /* CBP + CLUT bits */
        tex0 |= (u64)0x8000000000000000ULL;                     /* high control bit */
        return tex0;
    }
}
#endif

/* func_0028F0E8 = EU twin of USA func_0028F0D0 (DrawHudIconQuadTiled): draw a HUD
 * icon as a textured-sprite GIF packet (0x60-byte, NLOOP=5) to the frame render-DMA
 * chain. Screen coords are 1/16-pixel subpixel (coord<<4); UV spans the icon's full
 * texture (2^logW x 2^logH texels). Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Word-verified vs USA func_0028F0D0 (data lane +0x80):
 * g_frameDmaCursor -> g_nVendorBuyQuantity+0x60; g_gsPixelOffsetX/Y ->
 * D_001A7308+0xC8/+0xCC; z -> g_pActiveTextTable+0x3C; g_hudIconMap -> +0x50;
 * g_hudTextureSlots -> +0x54; GetHudIconTex0 -> func_0028EEC0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028F0E8);
#else
void func_0028F0E8(s32 iconIndex, s32 x, s32 y, s32 w, s32 h, s32 alpha) {
    typedef struct HudIconMapEntry { s16 clutSlot; s16 textureSlot; } HudIconMapEntry;
    typedef struct HudGsSlot { s32 handle; u16 vramAddr; u8 logW; u8 logH; } HudGsSlot;
    extern void *g_pActiveTextTable;
    extern u8    D_001A7308[];
    extern s32   g_nVendorBuyQuantity;
    extern u64   func_0028EEC0(s32 iconIndex);
    u8 **cursor = (u8 **)((u8 *)&g_nVendorBuyQuantity + 0x60);
    u8  *p    = *cursor;
    s32  offX = *(s32 *)(D_001A7308 + 0xC8);
    s32  offY = *(s32 *)(D_001A7308 + 0xCC);
    u64  z    = (u64)(u32)*(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) << 32;
    HudIconMapEntry *map    = *(HudIconMapEntry **)((u8 *)&g_pActiveTextTable + 0x50);
    HudGsSlot       *texTab = *(HudGsSlot **)((u8 *)&g_pActiveTextTable + 0x54);
    HudGsSlot       *tex    = &texTab[map[iconIndex].textureSlot];
    s32 uExtent = 1 << tex->logW;
    s32 vExtent = 1 << tex->logH;

    *(u32 *)(p + 0x0) = 0x10000005;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000005;
    *cursor = p + 0x10;

    *(u64 *)(p + 0x10) = ((u64)0xE800 << 47) | 0x8001;
    *(u64 *)(p + 0x18) = 0x5353106;
    *(u64 *)(p + 0x20) = func_0028EEC0(iconIndex);
    *(u64 *)(p + 0x28) = 0x156;
    *(u64 *)(p + 0x30) = ((u64)(u32)alpha << 24) | 0x7F7F7F;
    *(u64 *)(p + 0x38) = 0; /* near UV (0,0) */
    *(u64 *)(p + 0x40) = (u64)(u32)((x << 4) + offX - 8)
                       | ((u64)(u32)((y << 4) + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x48) = (u64)(u32)((uExtent << 4) + (vExtent << 20)); /* far UV = full texture */
    *(u64 *)(p + 0x50) = (u64)(u32)(((x + w) << 4) + offX - 8)
                       | ((u64)(u32)(((y + h) << 4) + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x58) = 0;

    *cursor = *cursor + 0x50;
}
#endif

/* func_0028F2D8 = EU twin of USA func_0028F2C0: draw a HUD icon as a Gouraud-shaded
 * textured quad - a 0x70-byte NLOOP=8 GIF packet with a distinct RGBA at each of the
 * 4 corners taken from colorArr[0..3]. UV spans the icon's full texture; screen coords
 * are 1/16-pixel subpixel (coord<<4). Matching arm stays INCLUDE_ASM; #else is the
 * structure model. Pinned cross-body signature (void, void *colorArr). Word-verified
 * vs USA func_0028F2C0 (data lane +0x80): g_frameDmaCursor -> g_nVendorBuyQuantity+0x60;
 * g_gsPixelOffsetX/Y -> D_001A7308+0xC8/+0xCC; z -> g_pActiveTextTable+0x3C;
 * g_hudIconMap -> +0x50; g_hudTextureSlots -> +0x54; GetHudIconTex0 -> func_0028EEC0.
 * GIFtag blob D_1AC8B0 -> EU D_1AC930, emitted here as literals (as in the USA #else). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028F2D8);
#else
void func_0028F2D8(s32 iconIndex, s32 x, s32 y, s32 w, s32 h, void *colorArr) {
    typedef struct HudIconMapEntry { s16 clutSlot; s16 textureSlot; } HudIconMapEntry;
    typedef struct HudGsSlot { s32 handle; u16 vramAddr; u8 logW; u8 logH; } HudGsSlot;
    extern void *g_pActiveTextTable;
    extern u8    D_001A7308[];
    extern s32   g_nVendorBuyQuantity;
    extern u64   func_0028EEC0(s32 iconIndex);
    u8 **cursor = (u8 **)((u8 *)&g_nVendorBuyQuantity + 0x60);
    u8  *p      = *cursor;
    s32 *colors = (s32 *)colorArr;
    s32  offX   = *(s32 *)(D_001A7308 + 0xC8);
    s32  offY   = *(s32 *)(D_001A7308 + 0xCC);
    u64  z      = (u64)(u32)*(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) << 32;
    HudIconMapEntry *map    = *(HudIconMapEntry **)((u8 *)&g_pActiveTextTable + 0x50);
    HudGsSlot       *texTab = *(HudGsSlot **)((u8 *)&g_pActiveTextTable + 0x54);
    HudGsSlot       *tex    = &texTab[map[iconIndex].textureSlot];
    s32 uExtent = 1 << tex->logW;
    s32 vExtent = 1 << tex->logH;

    *(u32 *)(p + 0x0) = 0x10000008;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000008;
    *cursor = p + 0x10;

    *(u64 *)(p + 0x10) = 0xE400000000008001ULL;
    *(u64 *)(p + 0x18) = 0x0053153153153106ULL;
    *cursor = p + 0x20;

    *(u64 *)(p + 0x20) = func_0028EEC0(iconIndex);
    *(u64 *)(p + 0x28) = 0x15C;
    *(u64 *)(p + 0x30) = (u64)(u32)colors[0];            /* top-left colour */
    *(u64 *)(p + 0x38) = 0;                              /* near UV (0,0) */
    *(u64 *)(p + 0x40) = (u64)(u32)((x << 4) + offX - 8) /* top-left XY */
                       | ((u64)(u32)((y << 4) + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x48) = (u64)(u32)colors[1];            /* top-right colour */
    *(u64 *)(p + 0x50) = (u64)(u32)(uExtent << 4);       /* far UV u */
    *(u64 *)(p + 0x58) = (u64)(u32)(((x + w) << 4) + offX - 8) /* top-right XY */
                       | ((u64)(u32)((y << 4) + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x60) = (u64)(u32)colors[2];            /* bottom-left colour */
    *(u64 *)(p + 0x68) = (u64)(u32)(vExtent << 20);      /* far UV v */
    *(u64 *)(p + 0x70) = (u64)(u32)((x << 4) + offX - 8) /* bottom-left XY */
                       | ((u64)(u32)(((y + h) << 4) + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x78) = (u64)(u32)colors[3];            /* bottom-right colour */
    *(u64 *)(p + 0x80) = (u64)(u32)((uExtent << 4) + (vExtent << 20)); /* far UV (u,v) */
    *(u64 *)(p + 0x88) = (u64)(u32)(((x + w) << 4) + offX - 8) /* bottom-right XY */
                       | ((u64)(u32)(((y + h) << 4) + offY - 8) << 16)
                       | z;

    *cursor = *cursor + 0x70;
}
#endif

/* func_0028F558 = EU twin of USA func_0028F540: draw a 1:1 HUD icon - append a
 * textured-sprite GIF packet (0x60-byte NLOOP=5) to the frame render-DMA chain
 * blitting the icon's texture over screen rect (x,y)..(x+w,y+h). UV spans (0,0)..(w,h)
 * texels; RGB tint 0x7F7F7F with the given alpha. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. Word-verified vs USA func_0028F540 (data lane +0x80):
 * g_frameDmaCursor -> g_nVendorBuyQuantity+0x60; g_gsPixelOffsetX/Y ->
 * D_001A7308+0xC8/+0xCC; z -> g_pActiveTextTable+0x3C; GetHudIconTex0 -> func_0028EEC0.
 * (No texture-size lookup: UV comes straight from w,h.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028F558);
#else
void func_0028F558(s32 iconIndex, s32 x, s32 y, s32 w, s32 h, s32 alpha) {
    extern void *g_pActiveTextTable;
    extern u8    D_001A7308[];
    extern s32   g_nVendorBuyQuantity;
    extern u64   func_0028EEC0(s32 iconIndex);
    u8 **cursor = (u8 **)((u8 *)&g_nVendorBuyQuantity + 0x60);
    u8  *p    = *cursor;
    s32  offX = *(s32 *)(D_001A7308 + 0xC8);
    s32  offY = *(s32 *)(D_001A7308 + 0xCC);
    u64  z    = (u64)(u32)*(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) << 32;

    *(u32 *)(p + 0x0) = 0x10000005;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000005;
    *cursor = p + 0x10;

    *(u64 *)(p + 0x10) = ((u64)0xE800 << 47) | 0x8001;
    *(u64 *)(p + 0x18) = 0x5353106;
    *(u64 *)(p + 0x20) = func_0028EEC0(iconIndex);
    *(u64 *)(p + 0x28) = 0x156;
    *(u64 *)(p + 0x30) = ((u64)(u32)alpha << 24) | 0x7F7F7F;
    *(u64 *)(p + 0x38) = 0; /* near UV (0,0) */
    *(u64 *)(p + 0x40) = (u64)(u32)((x << 4) + offX - 8)
                       | ((u64)(u32)((y << 4) + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x48) = (u64)(u32)((w << 4) + (h << 20)); /* far UV (w,h) texels */
    *(u64 *)(p + 0x50) = (u64)(u32)(((x + w) << 4) + offX - 8)
                       | ((u64)(u32)(((y + h) << 4) + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x58) = 0;

    *cursor = *cursor + 0x50;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028F700);

/* func_0028F8F8 = EU twin of USA func_0028F8E0: draw a HUD icon as a textured-sprite
 * GIF packet (0x60-byte, NLOOP=5) with a caller-supplied RGB tint. Same shape as
 * func_0028F558 except the packed colour is assembled from the three rgb bytes plus
 * alpha, and the UV spans the icon's full 2^logW x 2^logH texture (whole-pixel screen
 * coords). Matching arm stays INCLUDE_ASM; #else is the structure model. Word-verified
 * vs USA func_0028F8E0 (data lane +0x80): g_frameDmaCursor -> g_nVendorBuyQuantity+0x60;
 * g_gsPixelOffsetX/Y -> D_001A7308+0xC8/+0xCC; z -> g_pActiveTextTable+0x3C;
 * g_hudIconMap -> +0x50; g_hudTextureSlots -> +0x54; GetHudIconTex0 -> func_0028EEC0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028F8F8);
#else
void func_0028F8F8(s32 iconIndex, s32 x, s32 y, s32 w, s32 h, s32 alpha, s32 rgb) {
    typedef struct HudIconMapEntry { s16 clutSlot; s16 textureSlot; } HudIconMapEntry;
    typedef struct HudGsSlot { s32 handle; u16 vramAddr; u8 logW; u8 logH; } HudGsSlot;
    extern void *g_pActiveTextTable;
    extern u8    D_001A7308[];
    extern s32   g_nVendorBuyQuantity;
    extern u64   func_0028EEC0(s32 iconIndex);
    u8 **cursor = (u8 **)((u8 *)&g_nVendorBuyQuantity + 0x60);
    u8  *p    = *cursor;
    s32  offX = *(s32 *)(D_001A7308 + 0xC8);
    s32  offY = *(s32 *)(D_001A7308 + 0xCC);
    u64  z    = (u64)(u32)*(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) << 32;
    HudIconMapEntry *map    = *(HudIconMapEntry **)((u8 *)&g_pActiveTextTable + 0x50);
    HudGsSlot       *texTab = *(HudGsSlot **)((u8 *)&g_pActiveTextTable + 0x54);
    HudGsSlot       *tex    = &texTab[map[iconIndex].textureSlot];
    s32 uExtent = 1 << tex->logW;
    s32 vExtent = 1 << tex->logH;
    u64 color = (u64)(u32)((rgb & 0xFF)
                         | (rgb & 0xFF00)
                         | (((rgb >> 16) & 0xFF) << 16)
                         | (alpha << 24));

    *(u32 *)(p + 0x0) = 0x10000005;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000005;
    *cursor = p + 0x10;

    *(u64 *)(p + 0x10) = ((u64)0xE800 << 47) | 0x8001;
    *(u64 *)(p + 0x18) = 0x5353106;
    *(u64 *)(p + 0x20) = func_0028EEC0(iconIndex);
    *(u64 *)(p + 0x28) = 0x156;
    *(u64 *)(p + 0x30) = color;
    *(u64 *)(p + 0x38) = 0; /* near UV (0,0) */
    *(u64 *)(p + 0x40) = (u64)(u32)(x + offX - 8)
                       | ((u64)(u32)(y + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x48) = (u64)(u32)((uExtent << 4) + (vExtent << 20)); /* far UV = full texture */
    *(u64 *)(p + 0x50) = (u64)(u32)((x + w) + offX - 8)
                       | ((u64)(u32)((y + h) + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x58) = 0;

    *cursor = *cursor + 0x50;
}
#endif

/* func_0028FAF8 = EU twin of USA func_0028FAE0: append a textured-sprite GIF packet
 * (0x60-byte, NLOOP=5) to the frame render-DMA chain, then advance the cursor by 0x60.
 * Draws a quad from screen corner (x0,y0) to (x0+w,y0+h) mapped to UV (u0,v0) plus
 * texture extents 2^(uShift+4)/2^(vShift+4), tinted RGB 0x7F7F7F with the given alpha.
 * The GS register at p+0x20 is passed directly - no icon-map lookup. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_0028FAE0 (data
 * lane +0x80): g_frameDmaCursor -> g_nVendorBuyQuantity+0x60; g_gsPixelOffsetX/Y ->
 * D_001A7308+0xC8/+0xCC; z -> g_pActiveTextTable+0x3C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028FAF8);
#else
void func_0028FAF8(s32 reg1, s32 x0, s32 y0, s32 uShift, s32 vShift, s32 w, s32 h, s32 u0, s32 v0, s32 alpha) {
    extern void *g_pActiveTextTable;
    extern u8    D_001A7308[];
    extern s32   g_nVendorBuyQuantity;
    u8 **cursor = (u8 **)((u8 *)&g_nVendorBuyQuantity + 0x60);
    u8  *p    = *cursor;
    s32  offX = *(s32 *)(D_001A7308 + 0xC8);
    s32  offY = *(s32 *)(D_001A7308 + 0xCC);
    u64  z    = (u64)(u32)*(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) << 32;

    *(u32 *)(p + 0x0) = 0x10000005;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000005;
    *cursor = p + 0x10;

    *(u64 *)(p + 0x10) = ((u64)0xE800 << 47) | 0x8001;
    *(u64 *)(p + 0x18) = 0x5353106;
    *(u64 *)(p + 0x20) = (u64)(u32)reg1;
    *(u64 *)(p + 0x28) = 0x156;

    *(u64 *)(p + 0x30) = ((u64)(u32)alpha << 24) | 0x7F7F7F;
    *(u64 *)(p + 0x38) = (u64)(u32)u0 | ((u64)(u32)v0 << 16);
    *(u64 *)(p + 0x40) = (u64)(u32)(x0 + offX - 8)
                       | ((u64)(u32)(y0 + offY - 8) << 16)
                       | z;

    *(u64 *)(p + 0x48) = (u64)(u32)(u0 + (1 << (uShift + 4)))
                       | ((u64)(u32)(v0 + (1 << (vShift + 4))) << 16);
    *(u64 *)(p + 0x50) = (u64)(u32)((x0 + w) + offX - 8)
                       | ((u64)(u32)((y0 + h) + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x58) = 0;

    *cursor = *cursor + 0x50;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028FC88);

/* func_0028FC90 = EU twin of USA func_0028FC78: draw a rotated (oriented) HUD texture
 * quad - a 0x70-byte NLOOP=7 GIF packet whose four corners are the centre offset by two
 * rotated half-extent basis vectors. The caller passes a GS TEX0 handle directly (no
 * icon-map lookup). Colour is the fixed 0x807F7F7F. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. Word-verified vs USA func_0028FC78 (callee cluster
 * delta -0xF0 verified by call order): sin func_00283B48 -> func_00283A58, cos
 * func_00283B30 -> func_00283A40, Vec4AddVu0 -> Vec4AddVu0, Vec4SubVu0 ->
 * func_002835B0; g_frameDmaCursor -> g_nVendorBuyQuantity+0x60; g_gsPixelOffsetX/Y ->
 * D_001A7308+0xC8/+0xCC; z -> g_pActiveTextTable+0x3C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028FC90);
#else
void func_0028FC90(f32 cx, f32 cy, f32 halfW, f32 halfH, f32 angle,
                   s32 uExtent, s32 vExtent, s32 texReg) {
    extern void *g_pActiveTextTable;
    extern u8    D_001A7308[];
    extern s32   g_nVendorBuyQuantity;
    extern f32   func_00283A58(f32 angle);                            /* EU sin (USA func_00283B48) */
    extern f32   func_00283A40(f32 angle);                            /* EU cos (USA func_00283B30) */
    extern void  Vec4AddVu0(f32 *dst, const f32 *a, const f32 *b); /* EU Vec4AddVu0 */
    extern void  func_002835B0(f32 *dst, const f32 *a, const f32 *b); /* EU Vec4SubVu0 */
    u8 **cursor = (u8 **)((u8 *)&g_nVendorBuyQuantity + 0x60);
    u8  *p;
    s32  offX = *(s32 *)(D_001A7308 + 0xC8);
    s32  offY = *(s32 *)(D_001A7308 + 0xCC);
    u64  z    = (u64)(u32)*(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) << 32;
    f32 vecV[4];   /* vertical (height) basis vector */
    f32 vecH[4];   /* horizontal (width) basis vector */
    f32 center[4];
    f32 c0[4], c1[4], c2[4], c3[4];

    vecV[0] = halfH * func_00283A58(angle);
    vecV[1] = halfH * func_00283A40(angle);
    vecH[0] = -halfW * func_00283A40(angle);
    vecH[1] = halfW * func_00283A58(angle);
    center[0] = cx;
    center[1] = cy;

    Vec4AddVu0(c0, center, vecV); func_002835B0(c0, c0, vecH);
    Vec4AddVu0(c1, center, vecV); Vec4AddVu0(c1, c1, vecH);
    func_002835B0(c2, center, vecV); func_002835B0(c2, c2, vecH);
    func_002835B0(c3, center, vecV); Vec4AddVu0(c3, c3, vecH);

    p = *cursor;
    *(u32 *)(p + 0x0) = 0x10000007;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000007;
    *cursor = p + 0x10;

    *(u64 *)(p + 0x10) = ((u64)0xB400 << 48) | 0x8001;   /* GIFtag (NLOOP=7) */
    *(u64 *)(p + 0x18) = 0x53535353106ULL;               /* register descriptor */
    *(u64 *)(p + 0x20) = (u64)(u32)texReg;               /* TEX0/handle */
    *(u64 *)(p + 0x28) = 0x154;
    *(u64 *)(p + 0x30) = 0x807F7F7F;                     /* colour (A=0x80, RGB 0x7F7F7F) */
    *(u64 *)(p + 0x38) = (u64)(u32)(uExtent << 4);       /* near UV u */
    *(u64 *)(p + 0x40) = (u64)(u32)((s32)c0[0] + offX - 8)
                       | ((u64)(u32)((s32)c0[1] + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x48) = (u64)(u32)((vExtent << 20) + (uExtent << 4)); /* far UV (u,v) */
    *(u64 *)(p + 0x50) = (u64)(u32)((s32)c1[0] + offX - 8)
                       | ((u64)(u32)((s32)c1[1] + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x58) = 0;
    *(u64 *)(p + 0x60) = (u64)(u32)((s32)c2[0] + offX - 8)
                       | ((u64)(u32)((s32)c2[1] + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x68) = (u64)(u32)(vExtent << 20);      /* far UV v */
    *(u64 *)(p + 0x70) = (u64)(u32)((s32)c3[0] + offX - 8)
                       | ((u64)(u32)((s32)c3[1] + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x78) = 0;

    *cursor = *cursor + 0x70;
}
#endif

/* func_00290008: EU twin of USA func_0028FFF0 - draw a HUD icon as a textured-sprite
 * GIF packet (0x60-byte, NLOOP=5) with fully explicit corner AND texture coordinates
 * (caller supplies both near and far UV directly). Whole-pixel screen coords; fixed
 * RGB 0x7F7F7F + alpha. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_0028FFF0 (data lane +0x80): GetHudIconTex0 -> func_0028EEC0;
 * g_frameDmaCursor -> g_nVendorBuyQuantity+0x60; g_gsPixelOffsetX/Y -> D_001A7308+0xC8/
 * +0xCC; z -> g_pActiveTextTable+0x3C. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00290008);
#else
void func_00290008(s32 iconIndex, s32 x0, s32 y0, s32 x1, s32 y1,
                   s32 u0, s32 v0, s32 u1, s32 v1, s32 alpha) {
    extern void *g_pActiveTextTable;
    extern u8    D_001A7308[];
    extern s32   g_nVendorBuyQuantity;
    extern u64   func_0028EEC0(s32 iconIndex);
    u8 **cursor = (u8 **)((u8 *)&g_nVendorBuyQuantity + 0x60);
    u8  *p    = *cursor;
    s32  offX = *(s32 *)(D_001A7308 + 0xC8);
    s32  offY = *(s32 *)(D_001A7308 + 0xCC);
    u64  z    = (u64)(u32)*(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) << 32;

    *(u32 *)(p + 0x0) = 0x10000005;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000005;
    *cursor = p + 0x10;

    *(u64 *)(p + 0x10) = ((u64)0xE800 << 47) | 0x8001;
    *(u64 *)(p + 0x18) = 0x5353106;
    *(u64 *)(p + 0x20) = func_0028EEC0(iconIndex);
    *(u64 *)(p + 0x28) = 0x156;
    *(u64 *)(p + 0x30) = ((u64)(u32)alpha << 24) | 0x7F7F7F;
    *(u64 *)(p + 0x38) = (u64)(u32)(u0 | (v0 << 16));    /* near UV */
    *(u64 *)(p + 0x40) = (u64)(u32)(x0 + offX - 8)
                       | ((u64)(u32)(y0 + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x48) = (u64)(u32)(u1 | (v1 << 16));    /* far UV */
    *(u64 *)(p + 0x50) = (u64)(u32)(x1 + offX - 8)
                       | ((u64)(u32)(y1 + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x58) = 0;

    *cursor = *cursor + 0x50;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002901C8);

/* func_00290338: EU twin of USA func_00290320 - append a GIF/DMA packet to the frame
 * render-DMA chain that programs a GS rectangle from two corner points, then advance
 * the cursor. Identical to func_002904C8 except the register at p+0x20 is 0x41 (vs 0x46)
 * and the packed Z is the fixed 0x00FFFFF000000000. mode != 0 = whole pixels (-8 bias);
 * mode == 0 = 1/16-pixel subpixel (coord<<4, -0x10 bias). Matching arm stays
 * INCLUDE_ASM; #else is the structure model. Word-verified vs USA func_00290320
 * (data lane +0x80): g_frameDmaCursor -> g_nVendorBuyQuantity+0x60; g_gsPixelOffsetX/Y
 * -> D_001A7308+0xC8/+0xCC. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00290338);
#else
void func_00290338(s32 x0, s32 y0, s32 x1, s32 y1, u64 reg4, s32 mode) {
    extern u8    D_001A7308[];
    extern s32   g_nVendorBuyQuantity;
    u8 **cursor = (u8 **)((u8 *)&g_nVendorBuyQuantity + 0x60);
    u8  *p    = *cursor;
    s32  offX = *(s32 *)(D_001A7308 + 0xC8);
    s32  offY = *(s32 *)(D_001A7308 + 0xCC);
    u64  z    = (u64)0xFFFFF000u << 24; /* fixed Z = 0x00FFFFF000000000 */

    *(u32 *)(p + 0x0) = 0x10000003;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000003;
    *cursor = p + 0x10;

    *(u64 *)(p + 0x10) = ((u64)0x8800 << 47) | 0x8001;
    *(u64 *)(p + 0x18) = 0x4410;
    *(u64 *)(p + 0x20) = 0x41;
    *(u64 *)(p + 0x28) = reg4;

    if (mode == 0) {
        *(u64 *)(p + 0x30) = (u64)(u32)((x0 << 4) + offX - 0x10)
                           | ((u64)(u32)((y0 << 4) + offY - 0x10) << 16)
                           | z;
        *(u64 *)(p + 0x38) = (u64)(u32)((x1 << 4) + offX - 0x10)
                           | ((u64)(u32)((y1 << 4) + offY - 0x10) << 16)
                           | z;
    } else {
        *(u64 *)(p + 0x30) = (u64)(u32)(x0 + offX - 8)
                           | ((u64)(u32)(y0 + offY - 8) << 16)
                           | z;
        *(u64 *)(p + 0x38) = (u64)(u32)(x1 + offX - 8)
                           | ((u64)(u32)(y1 + offY - 8) << 16)
                           | z;
    }

    *cursor = *cursor + 0x30;
}
#endif

/* func_002904C8: EU twin of USA func_002904B0 - GS rectangle GIF-packet builder, twin
 * of func_00290338 (fixed Z 0x00FFFFF000000000, both coord modes) except the register
 * at p+0x20 is 0x46 rather than 0x41. Signature pinned cross-body (batch-8 func_0028DC40
 * forward-decl). Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_002904B0 (data lane +0x80): g_frameDmaCursor ->
 * g_nVendorBuyQuantity+0x60; g_gsPixelOffsetX/Y -> D_001A7308+0xC8/+0xCC. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002904C8);
#else
void func_002904C8(s32 x0, s32 y0, s32 x1, s32 y1, u64 reg4, s32 mode) {
    extern u8    D_001A7308[];
    extern s32   g_nVendorBuyQuantity;
    u8 **cursor = (u8 **)((u8 *)&g_nVendorBuyQuantity + 0x60);
    u8  *p    = *cursor;
    s32  offX = *(s32 *)(D_001A7308 + 0xC8);
    s32  offY = *(s32 *)(D_001A7308 + 0xCC);
    u64  z    = (u64)0xFFFFF000u << 24; /* fixed Z = 0x00FFFFF000000000 */

    *(u32 *)(p + 0x0) = 0x10000003;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000003;
    *cursor = p + 0x10;

    *(u64 *)(p + 0x10) = ((u64)0x8800 << 47) | 0x8001;
    *(u64 *)(p + 0x18) = 0x4410;
    *(u64 *)(p + 0x20) = 0x46;
    *(u64 *)(p + 0x28) = reg4;

    if (mode == 0) {
        *(u64 *)(p + 0x30) = (u64)(u32)((x0 << 4) + offX - 0x10)
                           | ((u64)(u32)((y0 << 4) + offY - 0x10) << 16)
                           | z;
        *(u64 *)(p + 0x38) = (u64)(u32)((x1 << 4) + offX - 0x10)
                           | ((u64)(u32)((y1 << 4) + offY - 0x10) << 16)
                           | z;
    } else {
        *(u64 *)(p + 0x30) = (u64)(u32)(x0 + offX - 8)
                           | ((u64)(u32)(y0 + offY - 8) << 16)
                           | z;
        *(u64 *)(p + 0x38) = (u64)(u32)(x1 + offX - 8)
                           | ((u64)(u32)(y1 + offY - 8) << 16)
                           | z;
    }

    *cursor = *cursor + 0x30;
}
#endif

/* func_00290658: EU twin of USA func_00290640 - append a GIF/DMA packet setting a GS
 * scissor/region rectangle from two corner points, then advance the write cursor. Twin
 * of func_002904C8 but the packed Z high word is a caller value (zHigh) rather than the
 * fixed constant. mode != 0 = whole pixels (-8 bias); mode == 0 = 1/16-pixel subpixel
 * (coord<<4, -0x10 bias). Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Word-verified vs USA func_00290640 (data lane +0x80): g_frameDmaCursor ->
 * g_nVendorBuyQuantity+0x60; g_gsPixelOffsetX/Y -> D_001A7308+0xC8/+0xCC. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00290658);
#else
void func_00290658(s32 x0, s32 y0, s32 x1, s32 y1, u64 reg4, s32 zHigh, s32 mode) {
    extern u8    D_001A7308[];
    extern s32   g_nVendorBuyQuantity;
    u8 **cursor = (u8 **)((u8 *)&g_nVendorBuyQuantity + 0x60);
    u8  *p    = *cursor;
    s32  offX = *(s32 *)(D_001A7308 + 0xC8);
    s32  offY = *(s32 *)(D_001A7308 + 0xCC);
    u64  z    = (u64)(u32)zHigh << 32;

    *(u32 *)(p + 0x0) = 0x10000003;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000003;
    *cursor = p + 0x10;

    *(u64 *)(p + 0x10) = ((u64)0x8800 << 47) | 0x8001;
    *(u64 *)(p + 0x18) = 0x4410;
    *(u64 *)(p + 0x20) = 0x46;
    *(u64 *)(p + 0x28) = reg4;

    if (mode == 0) {
        *(u64 *)(p + 0x30) = (u64)(u32)((x0 << 4) + offX - 0x10)
                           | ((u64)(u32)((y0 << 4) + offY - 0x10) << 16)
                           | z;
        *(u64 *)(p + 0x38) = (u64)(u32)((x1 << 4) + offX - 0x10)
                           | ((u64)(u32)((y1 << 4) + offY - 0x10) << 16)
                           | z;
    } else {
        *(u64 *)(p + 0x30) = (u64)(u32)(x0 + offX - 8)
                           | ((u64)(u32)(y0 + offY - 8) << 16)
                           | z;
        *(u64 *)(p + 0x38) = (u64)(u32)(x1 + offX - 8)
                           | ((u64)(u32)(y1 + offY - 8) << 16)
                           | z;
    }

    *cursor = *cursor + 0x30;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002907D0);
