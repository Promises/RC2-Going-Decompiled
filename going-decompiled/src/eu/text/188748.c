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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002887C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", GetWeaponUpgradeLevel);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", SetWeaponUpgradeSlot);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002889F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00288AA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00288B20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", GiveInventoryItem);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", AddItemToInventoryOrder);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00288E20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", IsItemUnlockedAtProgress);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00289080);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", UpgradeWeaponToMax);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", GetWeaponStatsAtLevel);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00289288);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002892C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", ResetCinematicQueue);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00289318);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002893B8);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002894D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00289500);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", RequestLevelExit);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00289730);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002897C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", FindTextTableEntry);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", GetLocalizedString);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00289948);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", BeginSubtitleDisplay);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", UpdateSubtitleStateMachine);

void func_0028A468(void) {
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028A470);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028A990);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028A9F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028AAF8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028AB48);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028AC38);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", ResetBoltCounterHud);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", UpdateBoltCounterHud);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028B038);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028B4E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028B4E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028B538);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028B678);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028B850);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028B9B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028BB28);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028BC50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", DebugMalloc);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028BD00);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028BD98);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028BEA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028BF08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028BF98);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C018);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C088);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C090);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C108);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C170);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C6B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C730);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C778);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028C7C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", DrawWeaponSelectWheel);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028D6F0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028D738);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028DC40);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028E800);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028E9B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028EAE0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028EB28);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028ECD8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028EE08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028EEC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028F0E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028F2D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028F558);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028F700);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028F8F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028FAF8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028FC88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028FC90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00290008);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002901C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00290338);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002904C8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00290658);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_002907D0);
