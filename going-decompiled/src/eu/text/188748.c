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
extern SubtitleState g_subtitleState;          /* EU 0x254E78 */

/* The voice/cinematic-clip control block lives at g_saveImageArea + 0x1000
 * (EU 0x1A6428). +0x24 is this area's current clip id; +0x68 a "voice busy"
 * gate. EU twin of the USA AreaClipState. */
typedef struct AreaClipState {
    u8  _pad00[0x24];
    s32 currentClip;   /* +0x24 */
    u8  _pad28[0x40];
    s32 voiceBusy;     /* +0x68 */
} AreaClipState;
extern u8  g_saveImageArea[];                  /* EU 0x1A5428 */

/* Per-line subtitle/voice timing table at g_health+0x66C (EU 0x18C9D8, stride
 * 0xC, indexed by voice handle; +0x0 u16 duration, 0xFFFF = "no line"). g_health
 * is the EU region anchor the displacement folds onto. */
extern s32 g_health;                           /* EU 0x18C36C */
extern s32 g_nGameState;                        /* EU 0x1A8C60 */
extern s32 g_gameTime;                          /* global frame counter (EU gp anchor g_nLevelExitDestination+0x8, 0x1B1688) */

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", FindTextTableEntry);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", GetLocalizedString);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_00289948);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", BeginSubtitleDisplay);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", UpdateSubtitleStateMachine);

void func_0028A468(void) {
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028A470);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/188748", func_0028A990);

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
extern s32 D_1A7C8C;             /* EU area-LRU list length (0x1A7C8C) */
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

/* One weapon-select wheel record (stride 0x1C); +0x18 holds the resolved item
 * id. EU twin of the USA WheelRecord. */
typedef struct WheelRecord {
    s32 nameStringId;   /* +0x00 */
    u8  _pad04[0x14];
    s32 itemId;         /* +0x18 */
} WheelRecord;                                /* stride 0x1C */

/* The wheel-record array base pointer is the word at g_pActiveTextTable+0x94
 * (EU 0x1B18D4; the USA twin reads g_hudMobySpawnStart+0x2C). */
extern void *g_pActiveTextTable;              /* EU 0x1B1840 (used here as +0x94 anchor) */
/* g_equippedItemSlots[8]: EU 0x1A7438, addressed off the unnamed region anchor
 * D_001A7308 (+0x130) — anchor stays its D_ name per the region-anchor rule. */
extern u8 D_001A7308[];                       /* EU region data anchor */

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
