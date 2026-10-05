#include "common.h"

/*
 * text/188858 — ".text mid 2" tile (vaddr 0x2888D8..0x29086F), Tier-1-A carve
 * (2026-06-14). An original separate gameplay/UI translation unit holding the
 * weapon/inventory accessors (g_weaponTable walkers), the cinematic-request
 * queue, level-exit + save-prompt gates, the localization/subtitle text
 * system, the on-screen bolt counter HUD, the weapon-select wheel and a debug
 * heap allocator. Built by a later SN cc1 (no load-PRE; 8-byte-packed callee
 * saves) so the matcher compiles it at -O2 -G8 -fno-gcse (per-unit GFLAG
 * override in tools/ee/objdiff_build.sh / diff.sh / build.sh); every other
 * text unit keeps its own flag model. The s136os splice compile drops
 * -fno-gcse (S136EXTRA="", RULING #9450).
 *
 * -G8 extern-sizing rules (same as text/1907F0 / text/1B4218):
 *   - a complete extern object of size <= 8 bytes lands in small data
 *     (gp-relative access, matching the original's %gp_rel form);
 *   - an object the original reads with the lui/%lo "assembler macro" shape
 *     is declared small PLUS a file-scope `.extern sym,16` override so cc1
 *     emits the one-insn symbolic macro that GNU as expands absolutely.
 */

/* Ring buffer of queued cinematic requests (5 slots, each 8 bytes). The read
 * cursor and write cursor wrap modulo 5. */
typedef struct CinematicSlot {
    s32 id;            /* +0x0 */
    s32 flag;          /* +0x4: set when id is a "standard cinematic" id */
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

/* Full, bindable layout — verified under ILP32. ResetCinematicQueue clears the
 * whole record (cursors +0x30/+0x34, count +0x38, 0x28-byte 0xCD poison of the
 * slots, active +0x44, gameStateMode +0x40=1); the highest field touched across
 * every accessor (Enqueue/Dequeue/Start/func_00289560/func_002895E0) is active
 * at +0x44, so the record is 0x48 bytes. ee-gcc 2.9 predates __SIZEOF_POINTER__
 * so the check is guarded (no-op on the matching cross-compile). */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(CinematicQueue) == 0x48, "CinematicQueue must be 0x48 under ILP32");
_Static_assert(__builtin_offsetof(CinematicQueue, slots)       == 0x08, "CinematicQueue.slots");
_Static_assert(__builtin_offsetof(CinematicQueue, writeCursor) == 0x30, "CinematicQueue.writeCursor");
_Static_assert(__builtin_offsetof(CinematicQueue, readCursor)  == 0x34, "CinematicQueue.readCursor");
_Static_assert(__builtin_offsetof(CinematicQueue, count)       == 0x38, "CinematicQueue.count");
_Static_assert(__builtin_offsetof(CinematicQueue, gameStateMode) == 0x40, "CinematicQueue.gameStateMode");
_Static_assert(__builtin_offsetof(CinematicQueue, active)      == 0x44, "CinematicQueue.active");
#endif

/* gp-relative small-data globals (read %gp_rel in the original). */
extern s32 g_nSavePromptPending;    /* show-saving-prompt gate (0x1A7B94) */
extern s32 g_nLevelExitRequested;   /* in-level frame-loop exit flag (0x1A8B84) */
extern s32 g_nGameState;            /* top-level game state id */
extern s32 g_gameTime;              /* global frame counter (0x1B1608) */

extern s32 D_1A8FB0;                /* gp small countdown gate (0x1A8FB0) */

/* %hi/%lo-addressed globals (assembler-absolute: small scalar + size override
 * so cc1 emits the one-insn symbolic macro that GNU as expands absolutely). */
__asm__(".extern g_miscExtras, 16");
extern u8 g_miscExtras;             /* misc unlock/extras byte array (0x1A7A12) */
__asm__(".extern g_sceneArenaCursor, 16");
extern s32 g_sceneArenaCursor;      /* scene arena allocation cursor (0x1B2230) */
__asm__(".extern g_sceneDecompressBase, 16");
extern s32 g_sceneDecompressBase;   /* base of decompressed scene region (0x1BAE4C) */
__asm__(".extern D_1A8FB4, 16");
extern s32 D_1A8FB4;
__asm__(".extern D_1A8FB8, 16");
extern s32 D_1A8FB8;

/* Larger %hi/%lo globals (arrays/structs — already non-small, no override).
 * g_pHudAssetHeader[1] (the word at +0x4) is a pointer to the HUD icon-slot
 * table (8-byte records). Modelled as an array so accesses fold the +4 into
 * the %lo relocation as the original does. */
extern void *g_pHudAssetHeader[];   /* DebugMalloc'd HUD asset header (0x1B1808) */

/* One HUD icon-slot record (8-byte stride) in g_pHudAssetHeader[1]'s table. */
typedef struct HudIconSlot {
    u16 texId;     /* +0x0 */
    u16 maxLevel;  /* +0x2: number of upgrade frames available */
    u16 baseFrame; /* +0x4: first map index for this icon */
    u8  paletteId; /* +0x6 */
    u8  _pad7;     /* +0x7 */
} HudIconSlot;                                /* stride 0x8 */

/* HUD icon-id -> {clut slot, texture slot} map; each entry indexes the
 * g_hudClutSlots / g_hudTextureSlots tables. */
typedef struct HudIconMapEntry {
    s16 clutSlot;    /* +0x0 */
    s16 textureSlot; /* +0x2 */
} HudIconMapEntry;                            /* stride 0x4 */
__asm__(".extern g_hudIconMap, 16");
extern HudIconMapEntry *g_hudIconMap;         /* 0x1B1810 */
/* CLUT / texture slot tables: 8-byte-stride records, the gs-handle word at +0. */
typedef struct HudGsSlot {
    s32 handle;   /* +0x0: negative = unallocated */
    u16 vramAddr; /* +0x4: resident VRAM block address (0 = not uploaded yet) */
    u8  logW;     /* +0x6: log2 texture width  (texture slots only) */
    u8  logH;     /* +0x7: log2 texture height (texture slots only) */
} HudGsSlot;                                  /* stride 0x8 */
__asm__(".extern g_hudClutSlots, 16");
extern HudGsSlot *g_hudClutSlots;             /* 0x1B1818 */
__asm__(".extern g_hudTextureSlots, 16");
extern HudGsSlot *g_hudTextureSlots;          /* 0x1B1814 */

/* DebugMalloc bump pool (all %hi/%lo scalars). */
__asm__(".extern g_debugMallocPoolBase, 16");
extern u8 *g_debugMallocPoolBase;   /* pool base (0x1BAEBC) */
__asm__(".extern g_debugMallocCursor, 16");
extern u8 *g_debugMallocCursor;     /* bump cursor (0x1B1800) */
__asm__(".extern g_debugMallocEnd, 16");
extern u8 *g_debugMallocEnd;        /* pool end = base + 0x64000 (0x1B1804) */

/* Moby-table context globals: the live set and the HUD shadow set, swapped by
 * SwapMobyTableContext. All %hi/%lo scalars except g_activeMobyTableId (gp). */
extern s32 g_activeMobyTableId;     /* gp small: which context is swapped in (0x1A8D44) */
__asm__(".extern g_mobyTableBase, 16");
extern void *g_mobyTableBase;       /* 0x1B1ADC */
__asm__(".extern g_mobySpawnStart, 16");
extern void *g_mobySpawnStart;      /* 0x1B1AE0 */
__asm__(".extern g_mobyTableEnd, 16");
extern void *g_mobyTableEnd;        /* 0x1B1AE4 */
__asm__(".extern g_mobyAuxBlockBase, 16");
extern void *g_mobyAuxBlockBase;    /* 0x1B1AEC */
__asm__(".extern g_hudMobyTableBase, 16");
extern void *g_hudMobyTableBase;    /* 0x1B182C */
__asm__(".extern g_hudMobyTableEnd, 16");
extern void *g_hudMobyTableEnd;     /* 0x1B1860 */
__asm__(".extern g_hudMobyAuxBlockBase, 16");
extern void *g_hudMobyAuxBlockBase; /* 0x1B1864 */
/* g_hudMobySpawnStart: a record whose +0x00 is the HUD spawn-start pointer
 * (swapped here) and whose +0x28/+0x2C are touched by func_0028D6D8. */
__asm__(".extern g_hudMobySpawnStart, 64");
extern void *g_hudMobySpawnStart;   /* 0x1B1830 */

/* Weapon/inventory backing store. */
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
    u8  _pad4E[0x1E];
    s32 xpThreshold;     /* +0x6C: variant XP threshold (<<5); negative = no clamp */
    u8  _pad70[0x18];
    u16 sellsAmmoFlag;   /* +0x88: every ROM reader uses lhu */
    u8  _pad8A[0x4];
    u16 ammoCapacity;    /* +0x8E */
    u16 ammoStartGrant;  /* +0x90 */
    u8  _pad92[0x4E];
} WeaponDef;                                  /* stride 0xE0 */

extern u8 g_itemEquippedSlot[0x38];           /* itemId -> active variant slot (0x139568) */
extern WeaponDef g_weaponTable[];             /* per-variant def/state table (0x239B20) */
extern s32 g_weaponXp[0x38];                  /* per-item XP/upgrade accumulator (0x139868) */
extern s32 g_weaponAmmo[0x38];                /* per-item current ammo (0x139688) */

/* g_soundBankHandles+0x20 (0x189E20), the block func_0028EAC8 reads:
 * +0x1248 holds the active weapon's item id; +0x22B4 is a "no ammo sale" gate. */
extern u8 g_soundBankHandlesBlk[];

/* Localized text table + subtitle state machine. */
typedef struct TextEntry {
    char *str;   /* +0x0 */
    s32   id;    /* +0x4 */
    s32   voice; /* +0x8 */
    s32   _pad;  /* +0xC */
} TextEntry;                                  /* stride 0x10 */

/* g_pActiveTextTable is a 4-byte pointer accessed %gp_rel by func_002898E0
 * (small data); declared complete so -G8 keeps it gp-relative. */
extern TextEntry *g_pActiveTextTable;         /* active localized text table (0x1B17C0) */

/* Subtitle state machine record (0x254DF8). +0x2C holds the active text table's
 * entry count; the +0x24/+0x28 fields hold the showing-subtitle index/handle. */
typedef struct SubtitleState {
    s32 state;          /* +0x00 */
    s32 animStep;       /* +0x04: per-state open/close animation step (clamped 0..8 / 0..4) */
    s32 boxHalfWidth;   /* +0x08: half the measured text width + 10 (BeginSubtitleDisplay) */
    s32 boxHalfHeight;  /* +0x0C: half the measured text height + 5 */
    s32 field10;        /* +0x10 */
    s32 boxPosY;        /* +0x14: on-screen box top Y (clamped to the screen) */
    s32 field18;        /* +0x18 */
    s32 field1C;        /* +0x1C */
    s32 entryIndex;     /* +0x20: index into g_pActiveTextTable of shown line */
    s32 showingIndex;   /* +0x24: pending/showing line index (-1 = none) */
    s32 showingHandle;  /* +0x28: voice/clip handle of the shown line */
    s32 tableCount;     /* +0x2C */
    s32 boxActiveGate;  /* +0x30: nonzero while the box is live/drawn */
    u8  _pad34[0xC];
    s32 phaseTimer;     /* +0x40: per-line phase timer (cleared on arm) */
    s32 phaseFlag;      /* +0x44: per-line phase flag  (cleared on arm) */
    s32 textPixelWidth; /* +0x48: strlen(text) * 7 (BeginSubtitleDisplay) */
} SubtitleState;
extern SubtitleState g_subtitleState;
extern s32 g_discToc[];                       /* master disc asset directory (0x14B540) */
extern u8  g_saveImageArea[];                 /* per-area save image RAM buffer (0x1A53A8) */

/* The voice/cinematic-clip control block lives at g_saveImageArea + 0x1000.
 * func_00289840 reads +0x24 (this area's current clip id) and +0x68 (a "voice
 * busy" gate); idle == both clear / matching the requested -1. */
typedef struct AreaClipState {
    u8  _pad00[0x24];
    s32 currentClip;   /* +0x24 */
    u8  _pad28[0x40];
    s32 voiceBusy;     /* +0x68 */
} AreaClipState;

/* Per-line subtitle/voice timing record table at g_health+0x66C (stride 0xC,
 * indexed by the voice handle). +0x0 is a u16 duration; 0xFFFF marks "no line"
 * (the subtitle cannot be armed). g_health is the small-data anchor the original
 * folds the +0x66C displacement onto, so the table is expressed relative to it.
 * g_health is read with the absolute lui/%lo macro shape (size override). */
__asm__(".extern g_health, 16");
extern s32 g_health;                          /* 0x18C2EC (region anchor) */

extern s32 g_cinematicUnlockedFlags[];        /* cinematics-watched bitfield (0x139768) */

/* Tables in other text/data segments. */
extern s32 D_240340[];   /* {key, _} pairs (stride 8), -2 sentinel (0x240340) */
extern s16 D_254E48[];   /* 0xAA rows of two s16 columns (bidirectional key<->value lookup, 0x254E48) */
extern u8  D_00259F38[]; /* 0xA-stride records, s16 key at +6, -1 sentinel (0x259F38; the
                            linker and the data label spell it D_00259F38) */
extern u8  D_259CC0[];   /* same layout as D_00259F38 */
typedef struct Rec2552B0 {
    s32 field00;         /* +0x00 */
    s32 field04;         /* +0x04 */
    u8  _pad08[0x1C];
    s32 field24;         /* +0x24 */
    u8  _pad28[0x3C];
    s32 key;             /* +0x64 */
    s32 field68;         /* +0x68 */
    u8  _pad6C[0x24];
} Rec2552B0;                                  /* stride 0x90 */
extern Rec2552B0 D_2552B0[];   /* 13-entry record table (0x2552B0) */

/* HudElement: one on-screen HUD element record — the same 0x90-stride record
 * type held by the D_2552B0[] registry (13 slots) that RegisterHudElement
 * (func_0028BE10) / ActivateHudElement (func_0028BF18) manage. The six widget
 * helpers below (init/reset/seed callbacks) all take a pointer to one of these.
 * Modelled as an opaque sized blob: the bodies reach fields by raw cast (mixed
 * gp/absolute addressing makes typed field access perturb the matching codegen),
 * so only the SIZE is bound. Size 0x90 is authoritative — it is the registry
 * stride (D_2552B0 stride 0x90, == sizeof Rec2552B0) and the max offset touched
 * across every accessor is the +0x7C type tag (one s32, ends at 0x80), well
 * inside 0x90.
 *
 * Field map (per-field confidence; gaps are real padding, not invented):
 *   +0x00 s32   live icon texId         CONFIRMED (func_0028C090 writes w+0x0)
 *   +0x04 s32   live iconId             CONFIRMED (ActivateHudElement: <-pending+0x24)
 *   +0x08 s32   live value              CONFIRMED (LayoutHudCounterDigits reads +0x8)
 *   +0x0C ptr   live valuePtr           CONFIRMED (LayoutHudCounterDigits reads +0xC)
 *   +0x10 fnptr live initFn             CONFIRMED (ActivateHudElement: <-pending+0x30)
 *   +0x14 fnptr live tickFn             CONFIRMED (ActivateHudElement: <-pending+0x34)
 *   +0x18 fnptr live drawFn             CONFIRMED (ActivateHudElement: <-pending+0x38)
 *   +0x20 s32   pending flags           CONFIRMED (RegisterHudElement +0x20)
 *   +0x24 s32   pending iconId          CONFIRMED (RegisterHudElement +0x24)
 *   +0x28 s32   pending value           CONFIRMED (RegisterHudElement +0x28)
 *   +0x2C ptr   pending valuePtr        CONFIRMED (RegisterHudElement +0x2C)
 *   +0x30 fnptr pending initFn          CONFIRMED (RegisterHudElement +0x30)
 *   +0x34 fnptr pending tickFn          CONFIRMED (RegisterHudElement +0x34)
 *   +0x38 fnptr pending drawFn          CONFIRMED (RegisterHudElement +0x38)
 *   +0x40 s16   icon slot index         CONFIRMED (func_0028C090 w+0x40)
 *   +0x42 s8    icon paletteId          CONFIRMED (func_0028C090 w+0x42)
 *   +0x44 s32   icon baseFrame          CONFIRMED (func_0028C090 w+0x44)
 *   +0x48 s16   cursor A                CONFIRMED (func_0028C490/C7F0/D6D8)
 *   +0x4A s16   cursor B                CONFIRMED (func_0028C490/C7F0/D6D8)
 *   +0x58 s32   width / X half-extent   CONFIRMED (LayoutHudCounterDigits, seed fns)
 *   +0x5C s32   height / Y half-extent  CONFIRMED (LayoutHudCounterDigits, seed fns)
 *   +0x60 u32   layout/orientation flags CONFIRMED (LayoutHudCounterDigits reads)
 *   +0x64 s32   element handle id (key) CONFIRMED (RegisterHudElement +0x64)
 *   +0x68 s32   dirty flag              CONFIRMED (RegisterHudElement=1 / Activate=0)
 *   +0x70 s32   (cleared on register)   PROBABLE (RegisterHudElement +0x70=0)
 *   +0x74 s32   smoothed display value  CONFIRMED (LayoutHudCounterDigits +0x74)
 *   +0x78 s32   clamped display / mode  CONFIRMED (LayoutHudCounterDigits +0x78)
 *   +0x7C s32   element type tag        CONFIRMED (0xD2 health / 0x96 ammo / -2 wheel)
 * (Fields +0x4C..+0x57, +0x6C, the +0x42..+0x47 sub-bytes etc. are not yet
 *  pinned and remain inside the blob.) */
typedef struct HudElement {
    u8 _bytes[0x90];
} HudElement;
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(HudElement) == 0x90, "HudElement must be 0x90 (registry stride)");
#endif

/* Forward declarations of unit-local callees. */
void ResetCinematicQueue(CinematicQueue *q);
void func_00289540(CinematicQueue *q);
void func_00289560(CinematicQueue *q);
s32 func_0028B560(s32 iconName);
void func_0028BF80(void);
void func_0028C728(void);
void func_0028ABC0(s32 a, s32 b);
void func_0028C090(HudElement *w, s32 iconName);
void func_0028C390(void *p);
s32 func_0028BE10(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g); /* returns a widget-slot handle in $2 */
void func_0029DB10(s32 a, s32 b);
void func_002B1B48(s32 a, s32 b, s32 c);
void ResetDebugHeap(void);
void *DebugMalloc(s32 size);
void func_0028BF18(HudElement *w);
s32 DequeueCinematic(CinematicQueue *q, s32 *outId, s32 *outFlags);
s32 RequestGameStateChange(s32 a, s32 b, s32 c, s32 d, s32 e);
s32 FindTextTableEntry(s32 textId);

extern char *D_1A8D18;   /* gp small: cached fallback localized string */
extern u8    D_1A8D28;   /* gp small: empty-string buffer */

extern u8    g_inventoryOwned[];   /* itemId -> owned flag (0x1A7B00) */
extern u8    g_inventoryOrder[];   /* quick-select order list (0x1A7B70) */
extern u8    D_1A7B90;             /* one-past-end of g_inventoryOrder */

extern u8    D_1A8DD8;             /* HUD list-widget item-row callback (0x1A8DD8) */
extern s32   D_1A8D48;             /* gp small: weapon-wheel cursor (0x1A8D48) */

extern s32   g_boltCount;          /* live bolt total (0x1A7A00) */
extern s32   g_nBoltCounterDisplayed[]; /* bolt-counter HUD roll state (0x1B18C8) */
s32 FlushHudDisplayValue(s32 displayState);

/* func_002888D8(itemId): advance an item to its next weapon variant. Reads the
 * active variant's next-variant slot (g_weaponTable[slot].nextVariantSlot); when
 * set, clamps the item's accumulated XP (g_weaponXp[itemId]) up to the variant's
 * XP threshold (field +0x6C << 5), repoints g_itemEquippedSlot[itemId] at the
 * next variant, and tops up g_weaponAmmo[itemId] to the new variant's
 * ammoCapacity when that variant exists.
 *
 * WALL: frameless leaf, but the XP clamp lowers as a branch-likely (bnel) store
 * and the per-variant index uses a `mult`-scaled stride that cc1's array
 * indexing does not reproduce here. Left INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002888D8);
#else
void func_002888D8(s32 itemId) {
    u8  slot = g_itemEquippedSlot[itemId];
    s16 next = g_weaponTable[slot].nextVariantSlot;
    s32 threshold;

    if (next == 0) {
        return;
    }
    threshold = g_weaponTable[slot].xpThreshold;
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

/* GetWeaponUpgradeLevel(itemId): return how many variant-upgrade steps the item
 * has taken — walks the prevVariantSlot chain (+0x4C) to find the base variant,
 * then counts the nextVariantSlot chain (+0x4A) from there.
 *
 * WALL: two chained do-while scans whose `mult`-scaled (stride 0xE0) variant
 * indexing and bnez loop colouring cc1 does not reproduce. Left INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", GetWeaponUpgradeLevel);
#else
s32 GetWeaponUpgradeLevel(s32 itemId) {
    s32 slot = itemId;
    s32 count = 0;
    /* walk prevVariantSlot back to the base variant */
    if (g_weaponTable[slot].prevVariantSlot != 0) {
        do {
            slot = g_weaponTable[slot].prevVariantSlot;
        } while (g_weaponTable[slot].prevVariantSlot != 0);
    }
    /* count nextVariantSlot steps forward from the base */
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
 * variant `level` upgrade-steps above its base. Bails (returns 0) when the
 * item's current GetWeaponUpgradeLevel is below `level` (cannot select a
 * not-yet-unlocked variant); otherwise walks the prevVariantSlot chain back to
 * the base variant, steps `level` nextVariantSlot links forward from there, and
 * writes that slot into g_itemEquippedSlot[itemId], returning 1.
 *
 * WALL: four callee-saves (0x30 frame), a jal to GetWeaponUpgradeLevel and the
 * `mult`-scaled (0xE0) chain walks with bnez loop colouring. Left INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", SetWeaponUpgradeSlot);
#else
s32 SetWeaponUpgradeSlot(s32 itemId, s32 level) {
    s32 slot = itemId;
    s32 i;

    /* cannot select a variant above what the item has unlocked */
    if (GetWeaponUpgradeLevel(slot) < level) {
        return 0;
    }
    /* walk prevVariantSlot back to the base variant */
    if (g_weaponTable[slot].prevVariantSlot != 0) {
        do {
            slot = g_weaponTable[slot].prevVariantSlot;
        } while (g_weaponTable[slot].prevVariantSlot != 0);
    }
    /* step `level` nextVariantSlot links forward from the base */
    for (i = level; i > 0; i--) {
        slot = g_weaponTable[slot].nextVariantSlot;
    }
    g_itemEquippedSlot[itemId] = (u8)slot;
    return 1;
}
#endif

extern u8 D_1A8B10[]; /* 0x30-byte, zero-terminated s32 key table */

/**
 * Copy the 0x30-byte key table at D_1A8B10 onto the stack and linear-scan it for
 * `key`, returning 1 when `key` is present (before the zero terminator) and 0
 * otherwise (empty table or terminator reached). The terminator itself is never
 * matched against `key`.
 *
 *   key  the s32 key to look for
 *   ->   1 if present before the terminator, 0 if absent or the table is empty
 *
 * The whole table is copied to the stack first (an `ldl`/`ldr` unaligned-pair
 * sequence) even though the scan stops at the first hit — the copy is
 * unconditional, not lazy.
 *
 * Non-obvious: the comparison is written `key == *p`, NOT `*p == key`. The two
 * are semantically identical, but cc1 evaluates the LEFT operand first, and only
 * the key-first order both loads the operands in the original's order and lets
 * the loop's exit branch keep its annulling `bnel` form. Reversing the two
 * operands is the single-instruction difference between this body and the ROM.
 */
s32 func_00288B08(s32 key) {
    s32 table[12];
    s32 *p;

    memcpy(table, D_1A8B10, sizeof(table));

    if (table[0] == 0) {
        return 0;
    }
    for (p = table;;) {
        if (key == *p) {
            return 1;
        }
        p++;
        if (*p == 0) {
            return 0;
        }
    }
}

/*
 * One record of the D_00259F38 / D_259CC0 tables: 0xA bytes with the s16 key
 * at +6. What the other 8 bytes hold is not established here.
 */
typedef struct ItemKeyRecord {
    u8 unk0[6];
    s16 key; /* +0x06, -1 terminates the table */
    u8 unk8[2];
} ItemKeyRecord;

/*
 * R5900_SHORT_LOOP_PAD_IN(v, p): a SCHEDULING DEVICE, not a statement about
 * the machine (RULING #8435; same construct as R5900_SHORT_LOOP_PAD1 below,
 * FACT #7937). It emits the one short-loop pad `nop` the ROM's assembler put
 * before a backward branch closing a loop shorter than 6 instructions, which
 * cc1 2.9 never emits. It ties by INPUT operands only: `v` is the value the
 * branch tests and `p` the loop pointer. The `"+r"` form of PAD1 makes cc1
 * reuse the tested value at the loop head and turn the branch into a `bnel`
 * (measured on func_00288BB0, 83.87%). A no-op on the native build.
 */
#ifndef TARGET_NATIVE
#define R5900_SHORT_LOOP_PAD_IN(v, p) \
    __asm__ __volatile__(".set noreorder\n\tnop\n\t.set reorder" : : "r"(v), "r"(p))
#else
#define R5900_SHORT_LOOP_PAD_IN(v, p) ((void)0)
#endif

/*
 * func_00288BB0(key): return 1 if `key` is the key of any record in
 * D_00259F38 or, failing that, in D_259CC0 (see ItemKeyRecord), else 0.
 *   key - item id to look for (compared against the sign-extended s16 key)
 *   returns 1 on a hit, 0 when neither table holds it
 *
 * Byte-exact on sdk29 (task #1009). The old WALL note here (69.84%, and
 * 89.00% in task #850) said the ROM materialises each table's address once and
 * reaches the keys by displacement (`lh 0x6(base)`, `addiu base,0x6`), where
 * cc1 folds the +6 into the %lo relocation. Writing the first test as
 * `t->key` and starting the scan at `&t->key` gives exactly that. A separate
 * pointer for each table gives the ROM's registers (base in $4, key moved to
 * $5, the second base in $2). The second loop is 5 instructions, so the ROM
 * carries one short-loop pad `nop` before its closing `bne`
 * (R5900_SHORT_LOOP_PAD_IN, the scheduling device above). The first loop is 7
 * instructions and has none.
 */
s32 func_00288BB0(s32 key) {
    ItemKeyRecord *table = (ItemKeyRecord *)D_00259F38;
    if (table->key != -1) {
        s16 *p = &table->key;
        do {
            if (key == *p) {
                return 1;
            }
            p += sizeof(ItemKeyRecord) / sizeof(s16);
        } while (*p != -1);
    }
    {
        ItemKeyRecord *table2 = (ItemKeyRecord *)D_259CC0;
        if (table2->key != -1) {
            s16 *p = &table2->key;
            s32 cur;
            do {
                if (key == *p) {
                    return 1;
                }
                p += sizeof(ItemKeyRecord) / sizeof(s16);
                cur = *p;
                R5900_SHORT_LOOP_PAD_IN(cur, p);
            } while (cur != -1);
        }
    }
    return 0;
}

extern s32 func_00288BB0(s32 id); /* item-validity check (D_00259F38 lookup) */
extern u8 D_25E308[];             /* stride-0xA record table, s16 id at +0x6, -1 terminated */
extern s32 g_equippedItemSlots[8]; /* currently-equipped item ids (0x1A73B8) */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00288C30);
#else
/**
 * Register an item pickup in the 8-slot recent-items table (g_equippedItemSlots).
 *
 * Does nothing unless func_00288BB0 accepts the item. Then, if the item is
 * already listed in the D_25E308 record table, or its variant's g_weaponTable
 * entry has a non-zero +0xC field, or func_00288B08 reports it present, it is
 * skipped. Otherwise the item id is stored in the first g_equippedItemSlots slot
 * that is empty or already holds it (slot 0 is a fast path); if all 8 slots are
 * taken by other items nothing is recorded.
 */
void func_00288C30(s32 itemId) {
    s16 *id;
    s32 variantSlot;
    s32 slot;

    if (func_00288BB0(itemId) == 0) {
        return;
    }

    /* already listed in the D_25E308 table -> nothing to do */
    id = (s16 *)(D_25E308 + 0x6);
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
    if (func_00288B08(itemId) != 0) {
        return;
    }

    if (g_equippedItemSlots[0] == 0 || g_equippedItemSlots[0] == itemId) {
        slot = 0;
    } else {
        slot = 1;
        for (;;) {
            if (slot >= 8) {
                return; /* no free slot */
            }
            if (g_equippedItemSlots[slot] == 0 || g_equippedItemSlots[slot] == itemId) {
                break;
            }
            slot++;
        }
    }
    g_equippedItemSlots[slot] = itemId;
}
#endif

/*
 * The inventory arrays' absolute accesses. The ROM forms these addresses with
 * the assembler's one-insn `la` macro (`lui; addiu` kept adjacent), so cc1 must
 * see them as small (an 8-byte extern under -G8) while gas sizes them 16 and
 * expands the macro absolutely: assembler aliases sized 16 (the #8036
 * construct; relocations name the real symbols). D_1A7B90 is a 1-byte symbol
 * that would otherwise go %gp_rel. AddItemToInventoryOrder uses the first
 * three; GiveInventoryItem uses g_inventoryOwnedAbs and g_inventoryNewFlagAbs.
 * Natively they are the arrays themselves.
 */
#ifndef TARGET_NATIVE
__asm__(".extern g_inventoryOwnedAbs, 16\n\tg_inventoryOwnedAbs = g_inventoryOwned");
extern u8 g_inventoryOwnedAbs[8];
__asm__(".extern g_inventoryOrderAbs, 16\n\tg_inventoryOrderAbs = g_inventoryOrder");
extern u8 g_inventoryOrderAbs[8];
__asm__(".extern g_inventoryOrderEndAbs, 16\n\tg_inventoryOrderEndAbs = D_1A7B90");
extern u8 g_inventoryOrderEndAbs[8];
__asm__(".extern g_inventoryNewFlagAbs, 16\n\tg_inventoryNewFlagAbs = g_inventoryNewFlag");
extern u8 g_inventoryNewFlagAbs[8];
#else
#define g_inventoryOwnedAbs g_inventoryOwned
#define g_inventoryOrderAbs g_inventoryOrder
#define g_inventoryOrderEndAbs (&D_1A7B90)
#define g_inventoryNewFlagAbs g_inventoryNewFlag
#endif

/*
 * D_1398A8: the ROM clears it with cc1's own split `lui $3` / `sw $0,%lo($3)`.
 * `section(".data")` on this extern DECLARATION (RULING #8620, an addressing-
 * model device: it moves no data and emits nothing) tells cc1 -G8 it is not
 * small data, so cc1 splits the address itself; a plain `extern s32` is
 * %gp_rel, and a `.extern ,16` override gives gas's `lui $1` expansion instead.
 */
#ifndef TARGET_NATIVE
extern s32 D_1398A8 __attribute__((section(".data")));
#else
extern s32 D_1398A8;
#endif

/* func_00288C30 is defined above only inside its own guarded #else, so without
 * this prototype GiveInventoryItem's solo s136os TU calls it as implicit int and
 * cc1 keeps $v0 live across the call (the 0x10 compare then lands in $3, not $2;
 * FACT #9282). */
extern void func_00288C30(s32 itemId);

/**
 * Grant an inventory item (0x288D30).
 *
 *   itemId  inventory item id (0..0x37)
 *
 * No-op if the item is already owned. Otherwise marks it owned
 * (g_inventoryOwned) and newly acquired (g_inventoryNewFlag), and, when the
 * item's active variant sells ammo and the player currently has none, tops its
 * ammo up to the variant's pickup amount (u16 at +0x92). Then registers it
 * (AddItemToInventoryOrder + func_00288C30). Granting the wrench (itemId 0x10)
 * while vendor upgrades are unlocked snaps it to upgrade level 2 and clears
 * D_1398A8.
 *
 * MATCHED on the s136os arm (task #1603). Each of these is needed (measured by
 * removing it alone, solo s136os vmu against the ROM):
 * - the func_00288C30 prototype above (without: 2/56 words differ);
 * - the two flag arrays read and written through their `...Abs` aliases, so
 *   cc1 emits each address as one `la` macro (without Owned's alias: 2/56;
 *   without NewFlag's: 13/56);
 * - D_1398A8's section(".data") declaration (without: built 54 words, ROM 55);
 * - the owned store written BEFORE the new-flag store: SN 1.36 issues the last
 *   store of the run first (FACT #9254), and the ROM stores the new flag first
 *   (in the other order: 8/56).
 *
 * GUARD (task #1269): on EE this C is the image's body, compiled alone by SN
 * 2.95.3 v1.36 -fopt-stack (tools/ee/s136os_functions.txt) and spliced over the
 * S136OS_SLOT line by tools/ee/s136os_splice.sh. On native it is plain C.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GiveInventoryItem)
S136OS_SLOT(GiveInventoryItem);
#else
extern u8  g_inventoryNewFlag[];   /* itemId -> "newly acquired" flag (set on grant) */
extern s32 AddItemToInventoryOrder(s32 itemId);  /* defined below in this unit */
extern s32 IsVendorUpgradesUnlocked(void);       /* defined below in this unit */

void GiveInventoryItem(s32 itemId)
{
    WeaponDef *w;
    u8 slot;

    if (g_inventoryOwnedAbs[itemId] != 0) {
        return;                            /* already owned — no-op */
    }
    slot = g_itemEquippedSlot[itemId];
    g_inventoryOwnedAbs[itemId] = 1;
    g_inventoryNewFlagAbs[itemId] = 1;
    w = &g_weaponTable[slot];
    if (w->sellsAmmoFlag != 0 && g_weaponAmmo[itemId] == 0) {
        g_weaponAmmo[itemId] = *(u16 *)((u8 *)w + 0x92);   /* variant pickup ammo */
    }
    AddItemToInventoryOrder(itemId);
    func_00288C30(itemId);
    if (itemId == 0x10 && IsVendorUpgradesUnlocked() != 0) {
        SetWeaponUpgradeSlot(0x10, 2);
        D_1398A8 = 0;
    }
}
#endif

/**
 * Place an item into the inventory quick-select order list (g_inventoryOrder,
 * ending at D_1A7B90).
 *
 * The item's active variant must exist, be at upgrade level 0, and have either
 * its +0x80 word or its sellsAmmoFlag set. The list is scanned for the first
 * entry that already names this item (low 6 bits) or is free (0xFF); that entry
 * is overwritten with `itemId | 0x40` when the item is owned, else `itemId`.
 *
 *   itemId  inventory item id (0..0x37)
 *   returns 1 when an entry was written, 0 otherwise (item not eligible, or
 *           the list is full of other items)
 *
 * Byte-exact on sdk29 (task #1076). Matching notes:
 *   - owned pointer, 0xFF, 0x40, list start and list end are named locals set
 *     in that order before the loop: that is the ROM's preheader order, which
 *     cc1 does not reach when it hoists them out of the loop itself;
 *   - the owned flag is selected with `flag = 0x40; if (!owned) flag = 0`
 *     around an empty tied asm (RULING #8483: a SCHEDULING/REGISTER DEVICE, it
 *     emits nothing). Without it cc1 ties the result to the zero arm and emits
 *     `movn`; the ROM ties it to the 0x40 copy and emits `movz $2,$0,$3`.
 */
s32 AddItemToInventoryOrder(s32 itemId) {
    WeaponDef *w = &g_weaponTable[g_itemEquippedSlot[itemId]];
    s32 added = 0;

    if (w->exists != 0 && w->upgradeLevel == 0
        && (*(s32 *)((u8 *)w + 0x80) != 0 || w->sellsAmmoFlag != 0)) {
        u8 *owned = &g_inventoryOwnedAbs[itemId];
        s32 freeEntry = 0xFF;
        s32 ownedBit = 0x40;
        u8 *entryPtr = g_inventoryOrderAbs;
        u8 *end = g_inventoryOrderEndAbs;
        do {
            u8 entry = *entryPtr;
            if ((entry & 0x3F) == itemId || entry == freeEntry) {
                u8 isOwned = *owned;
                s32 flag = ownedBit;
                added = 1;
                __asm__ __volatile__("" : "+r"(flag) : "r"(isOwned));
                if (isOwned == 0) {
                    flag = 0;
                }
                *entryPtr = itemId | flag;
                break;
            }
            entryPtr++;
        } while ((s32)entryPtr < (s32)end);
    }
    return added;
}

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

/* func_00288F30(itemId): resolve and fire the "acquired / upgraded" announcement
 * for an inventory item. First arms a subtitle line for a few special item ids
 * (0xA -> {0x9F8,0x4F}, 0x1B -> {0x9F7,0x4E}, otherwise the generic "got item"
 * line {0x9AA,1} when the tutorial-health flag g_health+0x678 is clear). Then it
 * resolves an announcement text id for the item: each upgrade level 0..5 has a
 * {itemId, textId} pair-list D_1A8B40[level] (-1 terminated); the item's textId is
 * found by linear search of that list. Item 0xA overrides the search result with a
 * level-based id (0x1296/0x1297, alternating for upgrade level >= 2). Finally it
 * dispatches the resolved text id (or 0 when none was found) via func_002B1880.
 *
 * The matching build keeps the asm (callee-save + jal gate with a 0xA-bounded scan
 * whose register colouring cc1 does not reproduce). The #else below is the
 * functionally-equivalent portable body (the original's unaligned ldl/ldr copy of
 * D_1A8B40 into a stack buffer before indexing is just a compiler artifact of
 * copy-then-index; a direct index is equivalent). */
extern s32 func_00289840(s32 textIndex, s32 voiceHandle); /* defined below, this unit */
extern s32 func_002B1880(s32 stringId, s32 arg);          /* text/1A8180 */
extern s32 D_1A8B40[]; /* [upgradeLevel 0..5] -> ptr to {itemId,textId,...,-1} list */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00288F30);
#else
s32 func_00288F30(s32 itemId) {
    s32 *list;
    s32 textId = -1;
    u8 slot, level;

    if (itemId == 0xA) {
        func_00289840(0x9F8, 0x4F);
    } else if (itemId == 0x1B) {
        func_00289840(0x9F7, 0x4E);
    } else if (*(u16 *)((u8 *)&g_health + 0x678) == 0) {
        func_00289840(0x9AA, 0x1);
    }

    slot = g_itemEquippedSlot[itemId];
    level = g_weaponTable[slot].upgradeLevel;
    list = (level < 6) ? (s32 *)D_1A8B40[level] : (s32 *)0;
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

    return func_002B1880(textId < 0 ? 0 : textId, -1);
}
#endif

s32 IsVendorUpgradesUnlocked(void); /* fwd: defined below in this unit */

/* IsItemUnlockedAtProgress(itemId, progress): whether `itemId` may appear in the
 * vendor at the given story-progress level. Item 9 is a special case (unlocked
 * once the vendor upgrade tier is). Otherwise it scans the {itemId, minProgress}
 * gate table D_240340 (stride 8, -2 sentinel): an item is unlocked when it has a
 * gate entry whose minProgress it has reached AND progress is still in the early
 * band (< 0x15). Returns 0 otherwise.
 *
 * The matching build keeps the asm (two callee-saves + a jal-gated scan whose
 * branch colouring cc1 does not reproduce). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", IsItemUnlockedAtProgress);
#else
s32 IsItemUnlockedAtProgress(s32 itemId, s32 progress) {
    s32 *entry;
    s32 early;

    if (itemId == 9 && IsVendorUpgradesUnlocked() != 0) {
        return 1;
    }
    if (D_240340[0] == -2) {
        return 0;
    }
    early = (progress < 0x15);
    entry = D_240340;
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

/*
 * func_00289190(key): return 1 if `key` appears as the first word of any
 * {key,_} pair in D_240340 (stride 8, -2 sentinel), else 0.
 *   key - item id to look for (NOTE #5658: D_240340 is the vendor-progress
 *         {itemId, minProgress} table IsItemUnlockedAtProgress also scans)
 *   returns 1 on a hit, 0 when the table is empty or the key is absent
 *
 * Byte-exact on sdk29 (task #1009), plain C, no device. The old WALL note here
 * (93.12%; 98.44% with register pins in task #978) called the ROM's register
 * choice unreachable from clean C: the ROM keeps the first-entry load AND the
 * scan pointer both in $3, and materialises -2 twice ($2 for the first-entry
 * compare, $5 for the loop test). Measured: writing the scan as an INDEX, not
 * a pointer, gives both. The likely mechanism (inferred from the output, not
 * traced in cc1): loop.c strength-reduces `D_240340[i * 2]` into a pointer
 * whose initialisation is born in the loop preheader, after the first-entry
 * value has died, so the two can share $3. Every pointer-form spelling tried
 * (#978, #1009) left the scan pointer in $6.
 */
s32 func_00289190(s32 key) {
    if (D_240340[0] != -2) {
        s32 i = 0;
        do {
            if (key == D_240340[i * 2]) {
                return 1;
            }
            i++;
        } while (D_240340[i * 2] != -2);
    }
    return 0;
}

/* Inter-function padding at 0x2891D0..0x2891D7: the two zero words retail
 * places between func_00289190 and UpgradeWeaponToMax. They exist only after
 * `endlabel func_00289190` in its nonmatchings .s, which is no longer included
 * now that this unit supplies the C body, so without this directive every later
 * function in the unit lands 8 bytes low (landing_gate cmp 681225 at task
 * #1009's first gate run; UpgradeWeaponToMax linked at 0x2891D0, retail
 * 0x2891D8). Not a codegen device: layout data. Same construct as
 * text/191238.c's padding after StartFrontendSegmentLoad. */
#ifndef TARGET_NATIVE
__asm__(".word 0\n\t.word 0");
#endif

/* UpgradeWeaponToMax(itemId): select the item's "fully upgraded" weapon variant
 * and zero its accumulated XP. Returns 0 (no change) when the item's currently
 * equipped variant does not exist; otherwise picks an upgrade level by item id
 * (a 0x2A-entry dispatch table over itemId-0xC: most items resolve to level 2,
 * a handful to level 1; out-of-range items also resolve to level 2), calls
 * SetWeaponUpgradeSlot(itemId, level), and on success resets g_weaponXp[itemId]
 * to 0. Returns SetWeaponUpgradeSlot's result.
 *
 * WALL: single callee-save + jal, a jtbl dispatch and the 0xE0-stride `mult`
 * chain walk with bnez loop colouring cc1 does not reproduce. Left INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", UpgradeWeaponToMax);
#else
s32 UpgradeWeaponToMax(s32 itemId) {
    /* jtbl_0026C6D0_text: itemId-0xC -> target level. Two targets only — most
     * entries pick level 2, indices {0,2,5,6,41} pick level 1. */
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
 * up the upgrade chain of weapon `itemId` (walk prevVariantSlot to the base,
 * then up to `level` forward nextVariantSlot steps — stopping early at a
 * terminal variant) and, when that variant's upgradeLevel matches `level`, copy
 * its full 0xE0-byte WeaponDef into *out. Returns 1 on a successful copy, else 0
 * (also returns 0 immediately when level >= 0xFF).
 *
 * WALL (sq/lq 128-bit): the struct copy is emitted as a 128-bit lq/sq block move
 * the matcher cannot reproduce from C struct assignment. Left INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", GetWeaponStatsAtLevel);
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

/*
 * g_sceneDecompressBaseSplit: a second C name for g_sceneDecompressBase (same
 * assembler symbol via the asm label). `section(".data")` tells cc1 -G8 this
 * spelling is NOT small data, so cc1 splits the address itself
 * (`lui $2` ... `lw $3,%lo($2)`) and can schedule the g_sceneArenaCursor load
 * between the two halves (the ResetDebugHeap lever, task #850). No new symbol
 * reaches the object: the relocation names g_sceneDecompressBase.
 */
#ifndef TARGET_NATIVE
extern s32 g_sceneDecompressBaseSplit __asm__("g_sceneDecompressBase") __attribute__((section(".data")));
#else
#define g_sceneDecompressBaseSplit g_sceneDecompressBase
#endif

/*
 * Compute the scene-arena address that would remain after carving `size`
 * bytes off the decompressed-scene region, written to *out. Refuses sizes over
 * 0x40000 (writes 0, returns -1); otherwise returns 0.
 *   size - bytes to carve (compared unsigned)
 *   out  - receives the resulting address, or 0 on refusal
 *
 * Byte-exact on sdk29 (task #978). The old WALL note here (74.69%) had the
 * branch-likely form matching, with only the load interleave left ("cc1
 * schedules each lui/lw pair together"). Reading the base through the split
 * name above gives the ROM's `lui base; lui cursor; lw cursor; lw base`. The refusal
 * path is written last, behind a `goto`, so it lands after the success path
 * and the `*out = 0` store fills the `bnel` slot as in the ROM. With the
 * refusal inline, cc1 puts a load in a plain `beq` slot instead (the FACT #8385
 * probe shape).
 */
s32 func_00289398(s32 size, s32 *out) {
    if ((u32)0x40000 < (u32)size) {
        goto refuse;
    }
    *out = (g_sceneDecompressBaseSplit + g_sceneArenaCursor) - size;
    return 0;
refuse:
    *out = 0;
    return -1;
}

/* Handwritten no-return fragment: `sh $0,0x1C($a0); nop` with NO jr $ra (it
 * falls through). Not expressible as a returning C function — INCLUDE_ASM. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/188858", func_002893D8);

/* ResetCinematicQueue(q): clear the cinematic ring buffer — zero the write/read
 * cursors and count, poison the 5 slots (0x28 bytes) with 0xCD, clear the
 * active flag and set the queue's game-state mode to 1.
 *
 * MATCHED on the s136os arm (FACT #8830). Record: under cc1 2.9 the memset
 * call put `q` in $16 and the trailing cursor/flag stores were scheduled apart
 * from the original's interleave with the jal; engine96 (task #469) reached
 * 88.28% with a residual SCHED (the ROM hoists `addiu $6,$0,0x28` into the
 * prologue).
 * GUARD (task #1269): on EE this C is the image's body, compiled alone by SN
 * 2.95.3 v1.36 -fopt-stack (tools/ee/s136os_functions.txt) and spliced over the
 * S136OS_SLOT line by tools/ee/s136os_splice.sh; the 2.9 compile sees only the
 * slot, so a build that skips the splice loses the function. On native it is
 * plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_ResetCinematicQueue)
S136OS_SLOT(ResetCinematicQueue);
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
 * DequeueCinematic(q, outId, outFlags) + EnqueueCinematic(q, id): the cinematic
 * ring-buffer pop/push pair. Both are BYTE-EXACT matches. The (x+1 != 5)?x+1:0
 * cursor wrap emits a movz that the original schedules immediately before jr in
 * its delay slot; the asm_unit.sh -G8 pendmov handler now reproduces exactly
 * that ordering (harness fix @8098872), so the earlier "jr-delay conditional-move
 * tail wall" no longer applies. Per-function docs below. (See func_00289560 for
 * the matched companion.)
 */
/*
 * Pop the head cinematic request off the queue `q`: return 0 if empty, else
 * decrement the count (clearing the active flag when it hits 0), copy the head
 * slot's {id, flag} to *outId / *outFlags, mark that slot free (-1/-1), advance the
 * read cursor with a wrap at 5, and return 1. `readCursor` is re-read for every
 * slot access (matching the -G8 -fno-gcse build, which keeps each load rather than
 * caching the cursor in a register).
 */
s32 DequeueCinematic(CinematicQueue *q, s32 *outId, s32 *outFlags) {
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
 * Push cinematic `id` onto the tail of queue `q`: refuse (return 0) when the
 * queue is full (count == 5) or currently playing one (active != 0), else store
 * {id, isMovie=(id in [0x20,0xB0])} at the write slot, bump the count, advance the
 * write cursor (wrap at 5), and return 1. `writeCursor` is re-read for each slot
 * access + the wrap (matching the -G8 -fno-gcse build's kept loads).
 */
s32 EnqueueCinematic(CinematicQueue *q, s32 id) {
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
void func_00289540(CinematicQueue *q) {
    ResetCinematicQueue(q);
    __asm__ __volatile__("");
}

/* Mark every cinematic queued between the read and write cursors (id < 0xB1)
 * as watched in g_cinematicUnlockedFlags, walking the ring forward mod 5. */
void func_00289560(CinematicQueue *q) {
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

/* Flush the cinematic queue: mark all queued cinematics watched
 * (func_00289560), clear the queue's leading word, then reset it
 * (func_00289540).
 *
 * BYTE-EXACT under the ENGINE compiler (#259): ee-gcc 2.96 + the engine
 * pipeline reproduces all 12 ROM words and both relocations, once the trailing
 * `__asm__ __volatile__("")` suppresses the sibling call. Build it with
 * `tools/ee/diff96.sh usa text/188858 func_002895E0 <this file>`, which defines
 * MATCH_func_002895E0.
 *
 * ee-gcc 2.9 packs the two callee-saves into 16-byte slots in a 0x20 frame
 * where the original uses 8-byte slots in 0x10, so its output differs from the
 * ROM in exactly 5 prologue/epilogue words (98.75% at the unit objdiff gate).
 * The image now takes this body from the s136os arm, which packs the frame
 * like the ROM (task #1309, below); the MATCH_ route above is historical. */
/* GUARD (task #1309): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; census FACT #8830; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before.
 * Its MATCH_func_002895E0 engine96 guard is retired with this promotion: the function is
 * image-resident, no longer arm-scored (RULING #8118). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002895E0)
S136OS_SLOT(func_002895E0);
#else
void func_002895E0(CinematicQueue *q) {
    func_00289560(q);
    *(s32 *)q = 0;
    func_00289540(q);
    __asm__ __volatile__("");
}
#endif

/* StartCinematicFromQueue(q): if the queue has a pending cinematic, mark it
 * active, dequeue the next {id, flag}, pick a game-state mode (1 or 2 based on
 * g_nGameState) and request the matching game-state transition
 * (RequestGameStateChange, with the computed mode as arg b). flag==0 requests
 * the level-cinematic transition (arg id in slot 4); flag==1 requests the
 * standard-cinematic transition (arg id in slot 3); any other flag value skips
 * the request. On a failed (<0) request the queue is
 * flushed via func_00289540. Returns 1 only when the request returned exactly 0
 * (or no request was made); a non-zero request result (positive OR negative)
 * and the empty-queue early-out both return 0.
 *
 * WALL: multiple callee-saves (0x30 frame), a movz mode-select and the two-way
 * argument shuffle / branch colouring around RequestGameStateChange that cc1
 * does not reproduce. Kept as the portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", StartCinematicFromQueue);
#else
s32 StartCinematicFromQueue(CinematicQueue *q) {
    s32 id, flag, mode, result;
    if (q->count == 0) {
        return 0;
    }
    q->_pad3C = 0;
    q->active = 1;
    id = 0;
    flag = 0;
    DequeueCinematic(q, &id, &flag);
    mode = (1u < (u32)(g_nGameState - 1)) ? 1 : 2;
    q->gameStateMode = mode;
    result = 0;
    if (flag == 0) {
        result = RequestGameStateChange(1, mode, 0, id, 0);
    } else if (flag == 1) {
        result = RequestGameStateChange(2, mode, id, 0, 0);
    }
    if (result < 0) {
        func_00289540(q);
    }
    /* sltiu result,1 : an UNSIGNED "< 1", i.e. exactly result == 0 (a negative
     * result is a large unsigned and returns 0, same as a positive one). */
    return (u32)result < 1;
}
#endif

/* Targets touched by RequestLevelExit (all arena-placed / native). */
extern s32  g_nLevelExitDestination;   /* 0x1B1600 next level/scene id */
extern u8   D_1393E0[];                /* 0x1393E0 level-transition latch blob */
extern u8   g_nSaveLoadStatusCode[];   /* 0x1A7420 save/load popup status block */
extern void CommitProgressCheckpoint(s32 a, s32 destination);
void ClearSavePromptPending(void);     /* defined below in this unit */

/*
 * R5900 SHORT-LOOP PAD (the same construct as text/1A8180.c, FACT #7937). The
 * ROM's assembler padded every backward branch closing a loop shorter than 6
 * instructions with `nop`s up to 6 - the R5900 short-loop erratum workaround.
 * cc1 never emits the pad and neither assembler we run inserts it, so it is
 * written directly before the branch it pads:
 *   - `.set noreorder` stops the assembler swapping the last pad `nop` into
 *     the branch delay slot;
 *   - the "+r" operand is the value the branch tests, which pins the pad
 *     between that value's load and the branch;
 *   - PAD1's `next` is the register the ROM updates in the delay slot: reading
 *     it keeps that update after the pad, where reorg moves it into the slot.
 * A no-op on the native build.
 *
 * EE_REG(r) binds a local register variable to EE GPR `r`
 * (`register u32 x EE_REG("$3");`, as in text/1DFF80.c) where cc1 2.9's
 * allocator colours a value differently from the ROM; natively a plain local.
 */
#ifndef TARGET_NATIVE
#define EE_REG(r) __asm__(r)
#define R5900_SHORT_LOOP_PAD1(v, next) \
    __asm__(".set noreorder\n\tnop\n\t.set reorder" : "+r"(v) : "r"(next))
#define R5900_SHORT_LOOP_PAD2(v) \
    __asm__(".set noreorder\n\tnop\n\tnop\n\t.set reorder" : "+r"(v))
#else
#define EE_REG(r)
#define R5900_SHORT_LOOP_PAD1(v, next) ((void)0)
#define R5900_SHORT_LOOP_PAD2(v) ((void)0)
#endif

/*
 * RequestLevelExit's absolute accesses. The ROM re-reads g_nLevelExitDestination
 * (stored %gp_rel just before) and read-modify-writes g_gameStateFlags
 * (0x1A7424, the word at g_nSaveLoadStatusCode+0x4) with the assembler's
 * absolute macro pairs (`lui rX; lw rX,%lo(rX)`, `lui $at; sw`), so both go
 * through assembler aliases sized 16 (the #8036 construct; relocations name the
 * real symbols). g_areaTable is the ROM's name for D_1393E0 (symbol_addrs).
 */
#ifndef TARGET_NATIVE
__asm__(".extern g_gameStateFlagsAbs, 16\n\tg_gameStateFlagsAbs = g_gameStateFlags");
extern s32 g_gameStateFlagsAbs;
__asm__(".extern g_nLevelExitDestinationAbs, 16\n\tg_nLevelExitDestinationAbs = g_nLevelExitDestination");
extern s32 g_nLevelExitDestinationAbs;
extern u8 g_areaTable[];
#else
#define g_gameStateFlagsAbs (*(s32 *)(g_nSaveLoadStatusCode + 0x4))
#define g_nLevelExitDestinationAbs g_nLevelExitDestination
#define g_areaTable D_1393E0
#endif

/**
 * Raise the in-level exit flag and record the exit destination. For the "no
 * explicit destination" sentinel (-1) also clear the area record's exit latch
 * (g_areaTable+0x17C), store -1 into its status half-word (+0x18) unless that
 * is already negative, and clear bit 0x200 of g_gameStateFlags. With `doSave`
 * set, clear the save-prompt gate and commit a progress checkpoint for the
 * destination.
 *
 *   destination  next level/scene id, or -1
 *   doSave       non-zero to clear the save prompt and commit a checkpoint
 *
 * Matching notes. The area base goes through an empty tied asm so cc1 keeps
 * &g_areaTable in a register and addresses +0x17C/+0x18 by displacement
 * (otherwise it folds the offsets into %lo); it and the latch are EE_REG-bound
 * to the ROM's $6/$3. The mask is a signed ~0x200 so it loads as one addiu.
 * The checkpoint's destination read is volatile: it must stay after
 * ClearSavePromptPending, not be hoisted into that call's delay slot, where
 * the two-word absolute macro cannot go. The empty volatile asm after the last
 * call stops cc1 turning it into a tail jump.
 */
void RequestLevelExit(s32 destination, s32 doSave) {
    g_nLevelExitRequested = 1;
    g_nLevelExitDestination = destination;
    if (destination == -1) {
        u8 *areaBase;
        register u8 *area EE_REG("$6");
        register s32 latch EE_REG("$3");

        __asm__("" : "=r"(areaBase) : "0"(g_areaTable));
        area = areaBase;
        latch = *(s32 *)(area + 0x17C);
        if (latch != 0) {
            *(s32 *)(area + 0x17C) = 0;
        }
        if (*(s16 *)(area + 0x18) >= 0) {
            *(s16 *)(area + 0x18) = (s16)destination;
        }
        g_gameStateFlagsAbs &= ~0x200;
    }
    if (doSave != 0) {
        ClearSavePromptPending();
        CommitProgressCheckpoint(0, *(volatile s32 *)&g_nLevelExitDestinationAbs);
        __asm__ __volatile__("");
    }
}

/* Non-zero while the main in-level frame loop has been asked to exit. */
s32 IsLevelExitRequested(void) {
    return g_nLevelExitRequested != 0;
}

/* func_00289768: mis-split handwritten stub-table fragment — `daddu $2,$0,$0;
 * nop; addiu $sp,+0x10; nop; addiu $sp,+0x10` with NO jr $ra (it falls through
 * into the following function's prologue). Not expressible as a returning C
 * function; left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00289768);

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

void func_002897B0(void) {
}

/* func_002897B8(): map the subtitle state-machine state (g_subtitleState[0],
 * 0..7) through jump table jtbl_0026C780_text to a status code; for state 1 it
 * also writes -1 to g_subtitleState+0x24, for state 4 it computes 4 - (count) at
 * +0x4DF8. Returns -1 / 6 / 7 depending on the branch taken.
 *
 * MATCH-WALL (jtbl): the matcher can't reproduce the compiler jump-table
 * layout/relocations from a C `switch`, so the #ifndef arm stays INCLUDE_ASM; the
 * #else arm is a faithful functional switch (engine 2.96 = no byte-match anyway).
 * State map (jtbl_0026C780_text): 0 -> showingIndex(+0x24) = -1, return -1;
 * 1/2/3 -> state = 7, animStep = 0, return 7; 4 -> animStep = 4 - old, state = 6,
 * return 6; 5 -> state = 6, animStep = 0, return 6; 6/7 -> return 1; else -> 0.
 * (The state/animStep writes lower as `sw ...,%lo(g_subtitleState)($page)` — the
 * 0x4DF8/0x4DFC in the .s ARE %lo(g_subtitleState)/+4, i.e. offsets 0 and 4, NOT a
 * +0x4DF8 struct offset — %hi/%lo page-base addressing.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002897B8);
#else
s32 func_002897B8(void) {
    switch (g_subtitleState.state) {
    case 0:
        g_subtitleState.showingIndex = -1;
        return -1;
    case 1:
    case 2:
    case 3:
        g_subtitleState.state = 7;
        g_subtitleState.animStep = 0;
        return 7;
    case 4: {
        s32 old = g_subtitleState.animStep;
        g_subtitleState.state = 6;
        g_subtitleState.animStep = 4 - old;
        return 6;
    }
    case 5:
        g_subtitleState.state = 6;
        g_subtitleState.animStep = 0;
        return 6;
    case 6:
    case 7:
        return 1;
    default:
        return 0;
    }
}
#endif

/* The per-line subtitle/voice timing table at g_health+0x66C, by the name the
 * ROM addresses it with. An unsized array is not small data to cc1 -G8, so the
 * address is compiler-split into lui/addiu and the scheduler can place the
 * index multiply between them as the ROM does (spelled g_health+0x66C it is
 * one `la` macro that nothing can be scheduled into). */
#ifndef TARGET_NATIVE
extern u16 D_18C958[];
#else
#define D_18C958 ((u16 *)((u8 *)&g_health + 0x66C))
#endif

/**
 * Arm a pending subtitle line: when the subtitle state machine is idle
 * (state 0), no line is already pending (showingIndex == -1), the area's voice
 * clip slot (g_saveImageArea+0x1000) is not busy and its current clip matches,
 * the line's timing entry is not 0xFFFF, the game is not in state 6 and the
 * game timer has reached 6, store the line and clear the phase fields.
 *
 *   textIndex    subtitle text index, stored at showingIndex (+0x24)
 *   voiceHandle  voice handle, stored at showingHandle (+0x28); also indexes the
 *                0xC-stride timing table D_18C958
 *   ->           1 when armed, 0 otherwise
 *
 * Every failing test branches to one shared `return 0` at the end (the goto
 * form); a `return 0` per test lets cc1 invert the clip test around an inline
 * return block. The empty volatile asm with a memory clobber before the clip
 * test keeps its load out of the busy test's delay slot, which the ROM leaves
 * as a `nop`.
 */
s32 func_00289840(s32 textIndex, s32 voiceHandle) {
    s32 pending;

    if (g_subtitleState.state != 0) {
        goto fail;
    }
    pending = g_subtitleState.showingIndex;
    if (pending != -1) {
        goto fail;
    }
    if (((AreaClipState *)&g_saveImageArea[0x1000])->voiceBusy != 0) {
        goto fail;
    }
    __asm__ __volatile__("" ::: "memory");
    if (((AreaClipState *)&g_saveImageArea[0x1000])->currentClip != pending) {
        goto fail;
    }
    if (D_18C958[voiceHandle * 6] == 0xFFFF) {
        goto fail;
    }
    if (g_nGameState == 6 || g_gameTime < 6) {
        goto fail;
    }
    g_subtitleState.showingIndex = textIndex;
    g_subtitleState.showingHandle = voiceHandle;
    g_subtitleState.phaseTimer = 0;
    g_subtitleState.phaseFlag = 0;
    return 1;
fail:
    return 0;
}

/* func_002898D8: 8-byte trailing-pad fragment (addiu $sp,+0x20; nop) of the
 * preceding function, pinned as its own symbol; the real function follows. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/188858", func_002898D8);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002898E0);
#else
/**
 * Tear down the currently-shown subtitle line.
 *
 * When a subtitle is active (state != 0): if its line has an associated voice
 * clip (g_pActiveTextTable[entryIndex].voice), that clip has a disc-TOC entry,
 * and the clip matches the area save-image's active clip
 * (voice == (s16)saveImage[+0x6C] - 0x1770), then bump the save-image's subtitle
 * sub-state (+0x72) to 5 unless it is already 6 or 7. In every active case the
 * subtitle state machine is then reset (state/_pad04 cleared, entry/showing
 * index set to -1). No-op when no subtitle is showing.
 */
void func_002898E0(void) {
    SubtitleState *ss = &g_subtitleState;
    s32 entryIndex;
    s32 voice;

    if (ss->state == 0) {
        return;
    }

    entryIndex = ss->entryIndex;
    if (entryIndex != -1) {
        voice = g_pActiveTextTable[entryIndex].voice;
        if (voice != -1 && *(s32 *)((u8 *)g_discToc + voice * 4 + 0x2A20) != 0) {
            u8 *saveImage = g_saveImageArea + 0x1000;
            if (voice == *(s16 *)(saveImage + 0x6C) - 0x1770
                && (u32)(*(u16 *)(saveImage + 0x72) - 6) >= 2) {
                *(s16 *)(saveImage + 0x72) = 5;
            }
        }
    }

    ss->state = 0;
    ss->animStep = 0;
    ss->entryIndex = -1;
    ss->showingIndex = -1;
}
#endif

/*
 * FindTextTableEntry(textId): linear-search the active localized text table for
 * the entry whose id (+0x4) matches `textId`; return its index, or -1 when the
 * table is empty or has no match.
 *
 * WALL (67.70%, LICM/aliasing): the first-element special-case + i++/re-read
 * index form
 *     if (g_subtitleState.tableCount > 0) {
 *         TextEntry *t = g_pActiveTextTable;
 *         if (t[0].id == textId) return 0;
 *         for (i = 1; i < g_subtitleState.tableCount; i++)
 *             if (t[i].id == textId) return i;
 *     } return -1;
 * gets the entry/first-element shape and the in-loop bnel branch-likely right,
 * but the original RE-READS g_subtitleState's +0x2C count from memory every
 * iteration and indexes the table as `base + i*0x10` (keeping the base live),
 * whereas this cc1 hoists the count load out of the loop (LICM — independent of
 * -fno-gcse, since it's loop-invariant load motion, not PRE) and walks a
 * `+= 0x10` pointer. The original treats the count load as if it could alias
 * the table writes (it doesn't), so the re-read can't be reproduced from clean
 * non-volatile C. Left INCLUDE_ASM for the matching build; the #else below is
 * the op-for-op faithful portable form (cmp-oracle: cmp_188858_text.c). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", FindTextTableEntry);
#else
s32 FindTextTableEntry(s32 textId) {
    s32 result = -1;
    s32 i = 0;

    if (g_subtitleState.tableCount > 0) {
        TextEntry *table = g_pActiveTextTable;
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

/* GetLocalizedString(textId): resolve `textId` to its localized C string. Looks
 * the id up via FindTextTableEntry; on a hit returns g_pActiveTextTable[idx].str.
 * On a miss returns the cached fallback string D_1A8D18 for the sentinel id
 * 0x9C40, otherwise a pointer to the empty-string buffer D_1A8D28.
 *
 * MATCHED on the s136os arm (FACT #8830). Record: cc1 2.9 scheduled the
 * gp-relative &D_1A8D28 tail address into a different delay slot; engine96
 * (task #469) reached 82.83%, its if-conversion selecting D_1A8D18 with movz
 * where the ROM keeps `beq $16,$2` + an out-of-line lw block.
 * GUARD (task #1269): on EE this C is the image's body, compiled alone by SN
 * 2.95.3 v1.36 -fopt-stack (tools/ee/s136os_functions.txt) and spliced over the
 * S136OS_SLOT line by tools/ee/s136os_splice.sh; the 2.9 compile sees only the
 * slot, so a build that skips the splice loses the function. On native it is
 * plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_GetLocalizedString)
S136OS_SLOT(GetLocalizedString);
#else
char *GetLocalizedString(s32 textId) {
    s32 idx = FindTextTableEntry(textId);
    if (idx >= 0) {
        return g_pActiveTextTable[idx].str;
    }
    if (textId == 0x9C40) {
        return D_1A8D18;
    }
    return (char *)&D_1A8D28;
}
#endif

/* func_00289A58: mis-split 1-instruction fragment — `sh $0,0x38($2)` (the
 * trailing store of the preceding function, pinned as its own symbol); no jr
 * $ra. Not a real function; left INCLUDE_ASM. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/188858", func_00289A58);

extern s32 g_bPalMode;          /* 0x1A7B98 - PAL flag (0 = NTSC) */
extern s32 g_screenHeight;      /* 0x1A7344 - active display height */
extern u16 D_1A7B9C;            /* 0x1A7B9C - subtitle-sound-enabled flag */
extern s32 D_1A8D20[];          /* gp small-data table indexed by g_bPalMode */
extern s32 PlayGlobalSound(s32 id, s32 a, s32 b);
extern void func_00280C98(s16 *layout, s16 clipX0, s16 clipX1, s16 left, s16 right,
                          s16 anchorX, s16 y, s16 lineHeight, s32 flags);
extern void func_00280BB8(void *layout, s32 color, const char *text, s32 arg4);
extern s32 func_001157AC(const char *s); /* SDK strlen */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", BeginSubtitleDisplay);
#else
/**
 * Start showing the current subtitle line: arm the subtitle state machine and
 * lay out its on-screen box.
 *
 * Sets g_subtitleState.state = 1, plays the subtitle blip (PlayGlobalSound) when
 * D_1A7B9C is set, then measures the text of the active line
 * (g_pActiveTextTable[entryIndex].str): func_00280C98 builds a layout descriptor
 * on the stack with the fixed subtitle margins, func_00280BB8 flows the text into
 * it, and the resulting half-width/half-height come back at layout bytes +0xC /
 * +0xE. Those (halved, + 10 / + 5) plus a fixed 0x100/8/8 are stored into the
 * box fields; textPixelWidth is strlen*7 (0 for an empty line). Finally the box Y
 * is positioned near the screen bottom (screenHeight-0x3C) and nudged up if it
 * would clip the bottom margin.
 */
void BeginSubtitleDisplay(void) {
    SubtitleState *ss = &g_subtitleState;
    u8 layout[0x40];
    const char *text;
    s16 measuredW;
    s16 measuredH;
    s32 screenH;

    ss->state = 1;
    ss->animStep = 0;

    if (D_1A7B9C != 0) {
        PlayGlobalSound(0, 1, 0);
    }

    text = g_pActiveTextTable[ss->entryIndex].str;

    func_00280C98((s16 *)layout, 0xF0, 0x1E0, 0x2C, 0x1D4, 0x100, 0x168, 0x10, 7);
    func_00280BB8(layout, 0x80FFA888, text, -1);

    /* measured box size returned by func_00280BB8 in the layout scratch */
    measuredW = *(s16 *)(layout + 0xC);
    measuredH = *(s16 *)(layout + 0xE);

    ss->boxHalfWidth = (measuredW >> 1) + 0xA;
    ss->boxHalfHeight = (measuredH >> 1) + 0x5;
    ss->field10 = 0x100;
    ss->boxPosY = D_1A8D20[g_bPalMode];
    ss->field1C = 8;
    ss->field18 = 8;
    ss->textPixelWidth = 0;

    if (text != 0) {
        s32 len = func_001157AC(text);
        ss->textPixelWidth = len * 8 - len; /* strlen * 7 */
    }

    screenH = g_screenHeight;
    ss->boxPosY = screenH - 0x3C;
    if (screenH - 0xC < (screenH - 0x3C) + ss->boxHalfHeight) {
        ss->boxPosY = screenH - (ss->boxHalfHeight + 0xC);
    }
}
#endif

/* UpdateSubtitleStateMachine(): per-frame tick of the subtitle/voice display
 * state machine (large dispatcher, ~0x928 bytes).
 *
 * MATCH-WALL: compiler jtbl dispatch (jtbl_0026C7A0_text) not reproducible from a
 * C switch; the #ifndef arm stays INCLUDE_ASM, the #else arm is the faithful
 * functional model (engine 2.96 = no byte-match; NB it is pure integer, no FP).
 * Addressing notes: g_subtitleState is at 0x254DF8 — its fields at +0x28..+0x48
 * (the "0x254e2x" globals) are read via the FULL address (real offsets); .state
 * (+0) is also written via the %hi page base (the .s `0x4DF8($17)` sites = offset
 * 0, NOT +0x4DF8). Save table = g_health+0x66C stride 0xC. Voice-clip flags =
 * g_discToc[voice]+0x2A20. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", UpdateSubtitleStateMachine);
#else
extern s32 g_playerProgress;             /* 0x1A79F8 first word: save-valid flag + bit index */
extern s32 g_gsPixelOffsetY;             /* 0x1A7354; +0x3C = current clip position value */
extern u16 D_138320, D_138324;           /* pad button state */
/* g_health (+0x66C save table stride 0xC), g_discToc (+0x2A20 per-voice flag
 * table), FindTextTableEntry: file-scope. */
extern s32 g_dialogVoiceId, g_dialogVoicePhase, g_dialogVoiceActive;
extern s32 g_pendingDialogVoiceId, g_pendingVoiceAux;
extern void BeginSubtitleDisplay(void);
extern void StopDialogVoice(void);
extern void func_0028AA70(s32 id);
extern s32 TickCountdownTimer(s32 p);
extern void func_002B1B48(s32 a, s32 b, s32 c);
void UpdateSubtitleStateMachine(void) {
    u8  *ss = (u8 *)&g_subtitleState;
    s32  slot;   /* g_subtitleState+0x28: current save-slot / voice index */
    u8  *save;
    s16  sVar;
    s32  clip;
    s32  iVar3;

    /* +0x30 = "system ready" gate; +0x34 = arm timer */
    if (*(s32 *)(ss + 0x30) == 0) {
        if (g_playerProgress < 0) {
            if (*(s32 *)(ss + 0x34) == 0) {
                if ((D_138320 & 0xF000) != 0) {
                    *(s32 *)(ss + 0x34) = 1;
                }
                if (*(s32 *)(ss + 0x34) == 0) {
                    goto gate;
                }
            }
            *(s32 *)(ss + 0x34) += 1;
            if (0x77 < *(s32 *)(ss + 0x34)) {
                *(s32 *)(ss + 0x30) = 1;
            }
        } else {
            *(s32 *)(ss + 0x34) = 0x78;
            *(s32 *)(ss + 0x30) = 1;
        }
    }
gate:
    if (((g_nGameState != 0) && (g_nGameState != 7)) || (*(s32 *)(ss + 0x30) == 0)) {
        g_subtitleState.state = 0;
        g_subtitleState.animStep = 0;
        g_subtitleState.showingIndex = -1;
        return;
    }
    g_subtitleState.animStep += 1;
    if (D_1A7B9C == 0 &&
        g_pActiveTextTable[g_subtitleState.entryIndex].voice == g_dialogVoiceId - 6000 &&
        (u32)(g_dialogVoicePhase - 6) >= 2) {
        g_dialogVoicePhase = 5;
    }

    slot = *(s32 *)(ss + 0x28);
    save = (u8 *)&g_health + 0x66C + slot * 0xC;

    switch (g_subtitleState.state) {
    case 0:
        if (g_subtitleState.showingIndex >= 0) {
            g_subtitleState.entryIndex = FindTextTableEntry(g_subtitleState.showingIndex);
            func_0028AA70(g_subtitleState.showingIndex);
            g_subtitleState.showingIndex = -1;
            if (g_subtitleState.entryIndex >= 0) {
                BeginSubtitleDisplay();
            }
        }
        break;
    case 1:
    case 2:
        if (D_1A7B9C != 0 &&
            g_pActiveTextTable[g_subtitleState.entryIndex].voice != -1 &&
            *(s32 *)((u8 *)g_discToc + g_pActiveTextTable[g_subtitleState.entryIndex].voice * 4 + 0x2A20) != 0 &&
            g_dialogVoiceActive == 0 && g_pendingDialogVoiceId == -1) {
            g_pendingVoiceAux = 0;
            g_pendingDialogVoiceId = slot + 6000;
        }
        if ((D_138324 & 0x10) != 0) {
            goto commit_state7;
        }
        if (g_subtitleState.animStep < 1) {
            return;
        }
        if (g_dialogVoicePhase != 3 || g_dialogVoiceId < 6000) {
            clip = g_pActiveTextTable[g_subtitleState.entryIndex].voice;
            if (clip != -1) {
                if (*(s32 *)((u8 *)g_discToc + clip * 4 + 0x2A20) == 0) {
                    iVar3 = 3;
                    goto set_state;
                }
                if (clip == g_dialogVoiceId - 6000) {
                    return;
                }
            }
        }
        iVar3 = 3;
        goto set_state;
    case 3:
        func_002B1B48(5, 0, 1);
        if ((D_138324 & 0x10) == 0) {
            if (g_subtitleState.animStep < 8) {
                return;
            }
            g_subtitleState.state = 4;
            g_subtitleState.animStep = 0;
            return;
        }
        goto commit_state7;
    case 4:
        func_002B1B48(5, 0, 1);
        if ((D_138324 & 0x10) != 0) {
            sVar = *(s16 *)(save + 0);
            if (sVar != -1) {
                *(s16 *)(save + 0) = sVar + 1;
            }
            if (*(u32 *)(save + 4) < *(u32 *)((u8 *)&g_gsPixelOffsetY + 0x3C)) {
                *(u32 *)(save + 4) = *(u32 *)((u8 *)&g_gsPixelOffsetY + 0x3C);
            }
            g_subtitleState.state = 6;
            g_subtitleState.animStep = 4 - g_subtitleState.animStep;
            *(u32 *)(save + 8) |= (1u << (g_playerProgress & 0x1F)) | 0x80000000;
            return;
        }
        if (g_subtitleState.animStep < 4) {
            return;
        }
        clip = g_pActiveTextTable[g_subtitleState.entryIndex].voice;
        if (clip != -1) {
            if (*(s32 *)((u8 *)g_discToc + clip * 4 + 0x2A20) != 0 && clip != g_dialogVoiceId - 6000 &&
                D_1A7B9C != 0) {
                g_subtitleState.state = 5;
                g_subtitleState.animStep = 0;
                return;
            }
            if (g_pActiveTextTable[g_subtitleState.entryIndex].voice != -1 &&
                *(s32 *)((u8 *)g_discToc + g_pActiveTextTable[g_subtitleState.entryIndex].voice * 4 + 0x2A20) != 0 &&
                (g_dialogVoicePhase != 3 || g_dialogVoiceId < 6000) && D_1A7B9C != 0) {
                return;
            }
        }
        if (D_1A7B9C != 0) {
            if (g_pActiveTextTable[g_subtitleState.entryIndex].voice != g_dialogVoiceId - 6000) {
                iVar3 = 5;
                goto set_state;
            }
            StopDialogVoice();
        }
        iVar3 = 5;
        goto set_state;
    case 5:
        func_002B1B48(5, 0, 1);
        if ((g_subtitleState.animStep < *(s32 *)(ss + 0x48) ||
             (g_pActiveTextTable[g_subtitleState.entryIndex].voice != -1 &&
              *(s32 *)((u8 *)g_discToc + g_pActiveTextTable[g_subtitleState.entryIndex].voice * 4 + 0x2A20) != 0 &&
              D_1A7B9C != 0)) &&
            ((D_1A7B9C == 0 ||
              (clip = g_pActiveTextTable[g_subtitleState.entryIndex].voice, clip == -1) ||
              *(s32 *)((u8 *)g_discToc + clip * 4 + 0x2A20) == 0 || clip == g_dialogVoiceId - 6000) &&
             (D_138324 & 0x10) == 0)) {
            return;
        }
        sVar = *(s16 *)(save + 0);
        if (sVar != -1) {
            *(s16 *)(save + 0) = sVar + 1;
        }
        if (*(u32 *)(save + 4) < *(u32 *)((u8 *)&g_gsPixelOffsetY + 0x3C)) {
            *(u32 *)(save + 4) = *(u32 *)((u8 *)&g_gsPixelOffsetY + 0x3C);
        }
        g_subtitleState.state = 6;
        g_subtitleState.animStep = 0;
        *(u32 *)(save + 8) |= (1u << (g_playerProgress & 0x1F)) | 0x80000000;
        break;
    case 6:
        if (D_1A7B9C != 0 && g_subtitleState.animStep < 4 && (D_138324 & 0x10) == 0) {
            return;
        }
        iVar3 = 7;
        g_subtitleState.animStep = 0;
        g_subtitleState.state = iVar3;
        break;
    case 7:
        func_002B1B48(5, 0, 1);
        if (g_pActiveTextTable[g_subtitleState.entryIndex].voice == g_dialogVoiceId - 6000 &&
            (u32)(g_dialogVoicePhase - 6) >= 2) {
            g_dialogVoicePhase = 5;
        }
        if (7 < g_subtitleState.animStep) {
            if (*(s32 *)(ss + 0x38) == 0) {
                g_subtitleState.state = 0;
                g_subtitleState.entryIndex = -1;
            } else {
                g_subtitleState.state = 8;
            }
            g_subtitleState.animStep = 0;
        }
        break;
    case 8:
        if (*(s32 *)(ss + 0x38) == 0) {
            if (g_subtitleState.entryIndex >= 0) {
                if (*(s16 *)(ss + 0x3A) == 0) {
                    if (TickCountdownTimer((s32)(ss + 0x3C)) == 0) {
                        return;
                    }
                    BeginSubtitleDisplay();
                    *(s16 *)(ss + 0x3A) += 1;
                    return;
                }
                sVar = *(s16 *)(save + 0);
                if (sVar != -1) {
                    *(s16 *)(save + 0) = sVar + 1;
                }
            }
            if (*(u32 *)(save + 4) < *(u32 *)((u8 *)&g_gsPixelOffsetY + 0x3C)) {
                *(u32 *)(save + 4) = *(u32 *)((u8 *)&g_gsPixelOffsetY + 0x3C);
            }
            g_subtitleState.entryIndex = -1;
            g_subtitleState.state = 0;
            *(s16 *)(ss + 0x3A) = 0;
            *(u32 *)(save + 8) |= (1u << (g_playerProgress & 0x1F)) | 0x80000000;
        }
        break;
    }
    return;

commit_state7:
    sVar = *(s16 *)(save + 0);
    if (sVar != -1) {
        *(s16 *)(save + 0) = sVar + 1;
    }
    if (*(u32 *)(save + 4) < *(u32 *)((u8 *)&g_gsPixelOffsetY + 0x3C)) {
        *(u32 *)(save + 4) = *(u32 *)((u8 *)&g_gsPixelOffsetY + 0x3C);
    }
    g_subtitleState.state = 7;
    g_subtitleState.animStep = 0;
    *(u32 *)(save + 8) |= (1u << (g_playerProgress & 0x1F)) | 0x80000000;
    return;

set_state:
    g_subtitleState.animStep = 0;
    g_subtitleState.state = iVar3;
}
#endif

void func_0028A4E0(void) {
}

/* func_0028A4E8(): per-frame subtitle-box renderer + open/close animator (~0x51C).
 *
 * Ticks the box phase timer (phaseFlag down-counts to 0, clamped; when it hits 0
 * the vertical slide offset phaseTimer is cleared), then bails unless the state
 * machine is live (state!=0), the box is active (boxActiveGate), and subtitles-
 * with-sound is enabled (D_1A7B9C). `state` (1..7) selects one of five paths via
 * jtbl_0026C7D0_text:
 *   1  pop-in: fixed 8px inset box.
 *   2  idle: func_0028A4E0() (currently a no-op).
 *   3  grow: box half-extents scaled by animStep k/8 (+0x20 base).
 *   4/5/6  full box + line text, text alpha faded by animStep (state 4 fades in,
 *          6 fades out, 5 is fully opaque).
 *   7  shrink/close: extents shrink toward 8, box height = (8-k)*12.
 * Any drawing path is skipped (-> func_0028A4E0()) when the box-visible byte
 * D_1A7B9D is 0. field10 (+0x10) is the box centre X, boxPosY (+0x14) the centre
 * Y; phaseTimer (+0x40) is added to every Y as a slide offset. field18/field1C
 * (+0x18/+0x1C) persist the last drawn half-width/half-height between frames.
 * The box quad is drawn by func_0027F208 (int coords) and func_0029C448 (float
 * coords); the line text is laid out by func_00280C98 and drawn by func_00280BB8.
 * Colours are GS-packed word constants.
 *
 * WALL: extensive FP scheduling + jr-through-jtbl dispatch cc1 does not
 * reproduce. Matching arm stays INCLUDE_ASM; the #else below is the portable
 * body.
 *
 * The four signed divide-by-8s are written as `/ 8`: the ROM carries cc1's own
 * slt/addiu/movn/sra expansion inline at each site. Through the former
 * `static div8_trunc()` helper cc1 emitted four `jal`s instead (task #1395;
 * SCREEN, s136 solo, relocated fields masked: 326/327 edit 386 -> 325/327 edit
 * 372). The residual is STRUCTURAL: the ROM saves 4 callee-saved registers
 * (frame 0x60), the arm 8 (frame 0xA0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028A4E8);
#else
extern u8 D_1A7B9D;   /* 0x1A7B9D - subtitle box-visible flag (byte) */
/* filled-quad draw (declared identically in text/1CA080.c) */
extern void func_0027F208(s32 y0, s32 y1, s32 x0, s32 x1, s32 h, s32 color);
/* border/outline quad draw: int mode+colour then four float box edges (y0,y1,x0,x1) */
extern void func_0029C448(s32 mode, s32 color, float y0, float y1, float x0, float x1);

void func_0028A4E8(void) {
    SubtitleState *ss = &g_subtitleState;
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
    if (D_1A7B9C == 0) {
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
        if (D_1A7B9D == 0) {
            func_0028A4E0();
            return;
        }
        ss->field1C = 8;
        ss->field18 = 8;
        func_0027F208(y - 8 + t, y + 8 + t, cx - 8, cx + 8, 0x60, 0x60442D00);
        func_0029C448(0x60, 0x55F0C070,
                      (float)((y - 8) + t), (float)((y + 8) + t),
                      (float)(cx - 8), (float)(cx + 8));
        return;
    }

    case 2:                                     /* idle */
        func_0028A4E0();
        return;

    case 3: {                                   /* grow: extents scaled by k/8 */
        s32 k = ss->animStep;
        s32 hh = ss->boxHalfHeight - 0x20;
        s32 hw = ss->boxHalfWidth - 0x20;
        s32 dy, dx;
        if (D_1A7B9D == 0) {
            func_0028A4E0();
            return;
        }
        if (k >= 9) k = 8;
        if (k < 0) k = 0;
        dy = hh * k / 8 + 0x20;
        dx = hw * k / 8 + 0x20;
        ss->field18 = dx;
        ss->field1C = dy;
        func_0027F208(y - dy + t, y + dy + t, cx - dx, cx + dx, 0x60, 0x60442D00);
        func_0029C448(0x60, 0x55F0C070,
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
        if (D_1A7B9D == 0) {
            func_0028A4E0();
            return;
        }
        ss->field18 = hw;
        ss->field1C = hh;
        func_0027F208(y - hh + t, y + hh + t, cx - hw, cx + hw, 0x60, 0x60442D00);
        func_0029C448(0x60, 0x55F0C070,
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

        str = g_pActiveTextTable[ss->entryIndex].str;
        func_00280C98((s16 *)layout, t + 0xF0, t + 0x1E0, 0x2C, 0x1D4,
                      0x100, y + t, 0x10, 3);
        func_00280BB8(layout, color, str, -1);
        return;
    }

    case 7: {                                   /* shrink/close */
        s32 f1c = ss->field1C;
        s32 f18 = ss->field18;
        s32 k = ss->animStep;
        s32 a, b, h;
        if (D_1A7B9D == 0) {
            func_0028A4E0();
            return;
        }
        if (k >= 9) k = 8;
        if (k < 0) k = 0;
        a = f1c - (f1c - 8) * k / 8;
        b = f18 - (f18 - 8) * k / 8;
        h = (8 - k) * 0xC;
        func_0027F208(y - a + t, y + a + t, cx - b, cx + b, h, 0x60442D00);
        func_0029C448(h, 0x55F0C070,
                      (float)((y - a) + t), (float)((y + a) + t),
                      (float)(cx - b), (float)(cx + b));
        return;
    }
    }
}
#endif

/* func_0028AA08(key, column, outValue): bidirectional key<->value lookup over
 * the D_254E48 table — 0xAA rows of two s16 columns each (row stride 4 bytes).
 * Sign-extend `key` to s16 and scan rows 0..0xA9, comparing the `column` s16 of
 * each row (column 0 -> +0, column 1 -> +2) against `key`. On the first match,
 * if `outValue` is non-NULL, write the row's OTHER column (column!=0 -> col0 at
 * +0; column==0 -> col1 at +2) through `outValue` (as a u16), and return the
 * matched row index. With no match across all 0xAA rows, return -1.
 *
 * Byte-exact on sdk29 (task #1009), plain C, no device. The old WALL note here
 * named the column*2 row-base striding and the i*4-vs-2 `movn` output-offset
 * selection. Task #978 reached both with the byte-offset `off` (2, +4 per row)
 * and `if (column) off = rowOff` ahead of the outValue test, leaving a pure
 * register residual at 97.00% (base/i/cell in t1/t0/v1, ROM t2/t1/t0). Computing
 * `rowOff = i * 4` at the TOP of each iteration, before the compare, closes
 * it. The ROM's `sll v1,i,2` in the bne delay slot then runs on every
 * iteration. The #978 `__asm__ volatile("")` at the head of the no-match path
 * is no longer needed.
 */
s32 func_0028AA08(s32 key, s32 column, s16 *outValue) {
    s16 keyHalf = (s16)key;
    u8 *base = (u8 *)D_254E48;
    s16 *cell = (s16 *)(base + column * 2); /* &row[0].col[column] */
    s32 i = 0;
    s32 off = 2; /* byte offset of row i's column 1 */

    do {
        s32 rowOff = i * 4; /* byte offset of row i's column 0 */
        if (*cell == keyHalf) {
            if (column != 0) {
                off = rowOff;
            }
            if (outValue != 0) {
                *outValue = *(u16 *)(off + (s32)base);
            }
            return i;
        }
        i++;
        off += 4;
        cell += 2;
    } while (i < 0xAA);
    return -1;
}

/* Recently-used area-id list: a byte ring at g_health+0xDFC; D_1A7C0C tracks the
 * live length. Touched only by func_0028AA70 (USA addr referenced both %gp_rel
 * and %hi/%lo — the gp/absolute split reload artifact). */
extern s32 D_1A7C0C;   /* area-LRU list length (0x1A7C0C) */

/* func_0028AA70(key): move the area-data record matching `key` to the MRU end of
 * the recently-used byte list at g_health+0xDFC (length D_1A7C0C). `key` is first
 * resolved to its area-data row id via func_0028AA08(key, column 1); a -1 (not
 * found) result is a no-op. The resolved id is located in the list (scanning from
 * index 0), the intervening bytes are shifted down to close the gap, and the id is
 * re-appended at the end. An id not yet in the list is simply appended (length
 * grows). The list is the most-recently-touched-area ordering used by the
 * area/save bookkeeping.
 *
 * WALL: single-$31 (sd) frame plus a jal gate, a branch-likely (bnel) scan and an
 * in-place byte-shift loop whose induction/branch colouring — together with the
 * g_health+0xDFC absolute-displacement fold and the gp/absolute split on D_1A7C0C
 * — cc1 does not reproduce. Left INCLUDE_ASM for the matching build; the #else is
 * the op-for-op faithful portable body. EU twin func_0028A9F8 (188748) is
 * byte-identical logic (g_health+0xDFC list, length D_1A7C8C) — region-agnostic. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028AA70);
#else
void func_0028AA70(s32 key) {
    u8 *lru;
    s32 len;
    s32 row;
    s32 i;

    row = func_0028AA08(key, 1, 0);
    if (row == -1) {
        return;
    }
    lru = (u8 *)&g_health + 0xDFC;
    len = D_1A7C0C;
    i = 0;
    if (lru[0] != (u8)row) {
        if (len <= 0) {
            goto append;          /* empty list: just append */
        }
        for (i = 1; i < len; i++) {
            if (lru[i] == (u8)row) {
                break;
            }
        }
    }
    if (i >= len) {
        goto append;              /* not present: append without removal */
    }
    /* present at index i: shift the tail down over it, then re-append */
    for (; i < len - 1; i++) {
        lru[i] = lru[i + 1];
    }
    lru[i] = 0;
    D_1A7C0C = len - 1;
append:
    len = D_1A7C0C;
    lru[len] = (u8)row;
    D_1A7C0C = len + 1;
}
#endif

/* Subtitle-event dispatch: for area-transition event 0x17 queue subtitle line
 * (0x9F3, voice 0x4A); for event 0x19 queue line (0xA35, voice 0x8C).
 *
 * MATCHED on the s136os arm (FACT #8830), as plain C with no sibling-call
 * guard. Record: 73% on the 2.9 arm; 94.47% on engine96 with an asm guard
 * (task #469), residual REORG — the ROM keeps a redundant epilogue `ld $16`
 * after the second jal, which those cc1s delete.
 * GUARD (task #1269): on EE this C is the image's body, compiled alone by SN
 * 2.95.3 v1.36 -fopt-stack (tools/ee/s136os_functions.txt) and spliced over the
 * S136OS_SLOT line by tools/ee/s136os_splice.sh; the 2.9 compile sees only the
 * slot, so a build that skips the splice loses the function. On native it is
 * plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0028AB70)
S136OS_SLOT(func_0028AB70);
#else
void func_0028AB70(s32 event) {
    if (event == 0x17) {
        func_0028ABC0(0x9F3, 0x4A);
    }
    if (event == 0x19) {
        func_0028ABC0(0xA35, 0x8C);
    }
}
#endif

/* func_0028ABC0(id, voice): enqueue a subtitle/voice request (id, voice) into a
 * 7-slot ring buffer that lives in the data block at &g_pActiveTextTable: the id
 * ring is at +0x8 (s16[7]), the voice ring at +0x18 (s16[7]), with a head index
 * at +0x27 and an entry count at +0x26. No-op when the ring is full (count >= 7)
 * or `id` is already queued (duplicate suppression scans the live entries
 * forward from head, modulo 7). On insert the new entry goes at slot
 * (head+count)%7 and count is incremented.
 *
 * NEAR-MISS: logic exact. The `div`/`mfhi` + `beql;break` divide-by-zero
 * scaffolding is NOT the obstacle: cc1 2.9 emits the same shape from the C `%`
 * (measured, task #1024). The best body measured is 94.67% (unit objdiff
 * report, solo sdk29; task #1076, NOTE #8535, replacing #1024's 76.17%). It
 * re-reads the count/head bytes absolutely and gets the loop-entry copies as
 * loop.c movables, so with two EE_REG pins everything through the scan loop is
 * the ROM instruction for instruction (#8535's normalised opcode+register
 * compare: 46 of 51 equal over the whole function). The residual is the tail
 * only: the ROM's dead `li $7,7` is missing (one word short), the shared 7's
 * `li $3,7` is emitted BEFORE the tail's `addu $2,$7,$11` where the ROM has
 * it after, and the voice-ring base lands in $7, not $8. The body is in NOTE
 * #8535.
 *
 * Why the two literal `% 7`s (which do give the dead li, in the ROM's slot)
 * still miss on registers (task #1100, cc1 .lreg dump): at local-alloc the
 * second li's pseudo is live only to its not-yet-folded div trap, because
 * cse's rerun after loop has already pointed div#2 at the FIRST 7. The short
 * pseudo out-ranks the first 7, takes $3, and pushes the first 7 to $4.
 * The ROM's allocation (first 7 in $3, dead li in $7, voice base in $8,
 * second remainder in $4) needs div#2 to still read the second li at
 * allocation and be retargeted to $3 afterwards. This cc1 never does that
 * retarget: with both 7s pinned ($3, $7), div#2 keeps reading $7.
 * -fno-rerun-cse-after-loop (diagnostic only; flags are per unit) restores
 * the first 7 in $3 and the second remainder in $4. That is consistent with
 * the compiler-revision class (RULING #7371), not a C spelling. 96 tail
 * spellings, register pins on every tail value, and an empty tied fence
 * (#8483) on a named second 7 were measured; none beats 94.67. Kept as the
 * portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028ABC0);
#else
void func_0028ABC0(s32 id, s32 voice) {
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

/* func_0028ACB0: mis-split 1-instruction fragment — `addiu $sp,+0x10` epilogue
 * tail of the preceding function, pinned as its own symbol; no jr $ra. Not a
 * real function; left INCLUDE_ASM. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028ACB0);

/* ResetBoltCounterHud(): snap the on-screen bolt counter to the true bolt total
 * (no roll animation) — seed both the shown and target values
 * (g_nBoltCounterDisplayed[0]/[1]) to g_boltCount, clear the roll accumulator
 * ([2]) and dirty index ([4]), set the pending flag ([3]) to -1, flush the HUD
 * value, and clear the two bolt-icon records at g_hudMobyAuxBlockBase+0x44.
 *
 * Byte-exact on sdk29 (task #948):
 *   - g_boltCountAbs / g_boltHudAbs are ASSEMBLER aliases (the #8036 construct,
 *     as g_pHudAssetHeaderAbs below): cc1-small, gas-absolute, giving the ROM's
 *     `lui v0; lw v0,%lo(v0)` load and its `lui $at; sw` stores;
 *   - the three stores before the call are volatile, which keeps them in ROM
 *     order and out of the jal delay slot (the ROM leaves it a nop);
 *   - the -1, the counter and the record pointer are held in the ROM's $2/$3;
 *   - the record-clear loop is `sh; addiu i; sw; nop; nop; bgez; addiu rec`:
 *     cc1 2.9 leaves a short loop's branch slot empty and pads it itself unless
 *     the loop counts about eight instructions, so the pads are written as
 *     counted asm statements and reorg then puts the pointer step in the slot
 *     (the same rule as 1CA080's func_002CAB50).
 */
#ifndef TARGET_NATIVE
__asm__(".extern g_boltCountAbs, 16\n\tg_boltCountAbs = g_boltCount");
extern s32 g_boltCountAbs;
__asm__(".extern g_boltHudAbs, 16\n\tg_boltHudAbs = g_nBoltCounterDisplayed");
extern s32 g_boltHudAbs[2];
#define BOLT_LOOP_FENCE(v) __asm__("" : "+r"(v) : : "memory")
#define BOLT_LOOP_NOP(v, next) \
    __asm__(".set noreorder\n\tnop\n\t.set reorder" : "+r"(v) : "r"(next))
#else
#define g_boltCountAbs g_boltCount
#define g_boltHudAbs g_nBoltCounterDisplayed
#define BOLT_LOOP_FENCE(v) ((void)0)
#define BOLT_LOOP_NOP(v, next) ((void)0)
#endif
void ResetBoltCounterHud(void) {
    s32 bolts = g_boltCountAbs;
    ((volatile s32 *)g_boltHudAbs)[1] = bolts;
    ((volatile s32 *)g_boltHudAbs)[0] = bolts;
    ((volatile s32 *)g_boltHudAbs)[2] = 0;
    FlushHudDisplayValue((s32)(u32)g_boltHudAbs);
    {
        register s32 pending EE_REG("$2") = -1;
        register s32 i EE_REG("$3");
        register u8 *rec EE_REG("$2");
        g_boltHudAbs[4] = 0;
        g_boltHudAbs[3] = pending;
        rec = (u8 *)&g_hudMobyAuxBlockBase + 0x44;
        i = 1;
        do {
            *(s16 *)(rec + 0xE) = 0;
            i--;
            *(s32 *)(rec + 0x0) = 0;
            BOLT_LOOP_FENCE(i);
            BOLT_LOOP_NOP(i, i);
            BOLT_LOOP_NOP(i, rec);
            rec += 0x10;
        } while (i >= 0);
    }
}

/* UpdateBoltCounterHud(): per-frame animation of the on-screen bolt counter. Rolls
 * the displayed value (g_nBoltCounterDisplayed[0]) toward the live g_boltCount using
 * up to two animation slots at g_hudMobyAuxBlockBase+0x44 (stride 0x10). Returns
 * early via the per-frame gate func_0029C570. Each frame:
 *   - flushes the current display value (FlushHudDisplayValue);
 *   - ARM: in game-state 5 with the live total below the display, hard-resets
 *     (ResetBoltCounterHud); else, when no slot is active (disp[3] == -1) and
 *     disp[4] == 0, arms a count-DOWN slot if boltCount < display, or a count-UP
 *     slot if display + accumulator < boltCount (picking slot 1 when slot 0 is busy);
 *   - HOLD: services the active slot, recomputing its remaining delta, and after
 *     ~61 held frames commits it to the rolling phase; fires the counter trigger
 *     func_0029C488(0xB4) whenever a slot is active or already rolling;
 *   - ROLL: for each of the two slots, promotes an arming slot to 'armed' once its
 *     tick passes 15, and steps a rolling slot by (s32)((f32)disp[2] * (1/14)) per
 *     frame, clamping the remaining delta at zero and retiring the slot (snapping to
 *     the final value) after ~15 roll frames.
 * disp[]: [0] displayed value, [1] roll base snapshot, [2] roll magnitude,
 * [3] active-slot index (-1 = none), [4] count-down direction flag.
 *
 * NOT MATCHED (task #1024, 90.88% solo on sdk29, 221/222 words). The ROM frame saves
 * only $ra; there are no callee-saves. A ROM-shaped body reached that figure; its
 * listing is in task #1024's store note. The one residual C cannot reach is an
 * assembler difference: after `mfc1 a0,$f2` (the roll delta) the ROM uses a0 at
 * once, while cc1 marks the hazard `#nop` and GNU as 2.40 turns it into a real
 * `nop`. The rest is register colouring in the arm/hold blocks and the shape of
 * the roll-clamp branches. Matching arm stays INCLUDE_ASM; #else is the
 * portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", UpdateBoltCounterHud);
#else
extern s32  func_0029C570(void);   /* per-frame gate: nonzero => skip this frame */
extern void func_0029C488(s32);    /* bolt-counter HUD trigger (always called with 0xB4) */

/* one bolt-roll animation slot: 2-entry array at g_hudMobyAuxBlockBase+0x44, stride 0x10 */
typedef struct HudRollSlot {
    s32 targetDelta;  /* +0x0  signed bolts still to roll onto the display */
    s32 holdTimer;    /* +0x4  arm-hold frame count, then roll-frame count */
    s32 tick;         /* +0x8  free-running frame tick (arming -> armed at 15) */
    s16 _pad0C;       /* +0xC  unused here */
    s16 phase;        /* +0xE  0 idle, 1 arming, 2 armed/hold, 3 rolling */
} HudRollSlot;

void UpdateBoltCounterHud(void) {
    s32         *disp  = g_nBoltCounterDisplayed;
    HudRollSlot *slots = (HudRollSlot *)((u8 *)&g_hudMobyAuxBlockBase + 0x44);
    s32 i;

    if (func_0029C570() != 0) {
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
        func_0029C488(0xB4);
    } else if (slots[0].phase == 3 || slots[1].phase == 3) {
        func_0029C488(0xB4);
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

/* func_0028B0B0(): per-frame render of the animated "±N bolts" popup for each of
 * the two bolt-roll animation slots (g_hudMobyAuxBlockBase+0x44, stride 0x10 —
 * shared with UpdateBoltCounterHud). For a slot in phase 1/2/3 it derives an
 * interpolation factor from the slot timers (arming ramp tick/14, hold ramp
 * holdTimer/14, roll fade 1-(holdTimer-7)/7), lerps the two glyph tints and the
 * number tint (ColorLerpPacked; the sign of targetDelta selects the palette),
 * formats targetDelta into a string, then draws the two "±" glyphs (font
 * codepoints 0xB2/0xB3) and the number. Phase 0 and phase >3 draw nothing.
 *
 * WALL (matching build): callee-saves + the heavy FP positioning schedule +
 * branch colouring cc1 does not reproduce. Matching arm stays INCLUDE_ASM; #else
 * is the portable body. Helper ABIs confirmed against the sibling draws in
 * 1CA080.cpp — func_003017F8 reads no $a2/$a3 argument (FACT #8918), so it is
 * declared with 2 ints + 5 floats. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028B0B0);
#else
extern void *g_guiInstance;            /* GUI singleton ptr (0x1A8D04) */
extern s16   g_swapGadgetItemIndex;    /* base whose +0x8A holds the HUD sprite y-scale (0x1B229A) */
extern f32   D_1A8DE0[];               /* per-digit-count x-scale table, 7 entries (0x1A8DE0) */
extern char  D_1A8E00[];               /* printf format for the bolt-delta number (0x1A8E00) */
extern s32   ColorLerpPacked(s32 colorA, s32 colorB, f32 t);        /* 0x2846E8 packed-RGBA lerp */
extern void  func_00115DA8(char *dst, const char *fmt, ...);      /* 0x115DA8 SDK sprintf */
extern s32   GuiFontAtlasLookupGlyph(void *atlas, s32 codepoint); /* 0x337BF8 */
/* 0x3017F8 glyph/sprite draw. Callee (text/2012B8) confirms the ABI: color0's
 * top byte alpha-gates the draw (sra 24; skip if 0); sx,sy are a scale pair each
 * multiplied by the glyph's intrinsic size at handle+0xC; px,py,v pass through to
 * func_003018A0. $a2/$a3 are never read (FACT #8918), so there are no pointer
 * params. */
extern void  func_003017F8(s32 handle, s32 color0, f32 px, f32 py, f32 sx, f32 syg, f32 v38);
extern void  func_00280090(s32 x, s32 y, u64 color, char *str, s64 wrap); /* 0x280090 text/number draw */

void func_0028B0B0(void) {
    HudRollSlot *slot = (HudRollSlot *)((u8 *)&g_hudMobyAuxBlockBase + 0x44);
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
                textColor = ColorLerpPacked(0x8000FFFF, 0x6029A1FF, t);
            } else {
                textColor = ColorLerpPacked(0x001010F0, 0x801010F0, t);
            }
            doDraw = 0x1F3;
        } else if (phase == 1) {
            t = (f32)slot->tick * (1.0f / 14.0f);
            color1 = ColorLerpPacked(0x00F0C070, 0x55F0C070, t);
            color2 = ColorLerpPacked(0x00442D00, 0x60442D00, t);
            if (slot->targetDelta >= 0) {
                textColor = ColorLerpPacked(0x0029A1FF, 0x6029A1FF, t);
            } else {
                textColor = ColorLerpPacked(0x001010F0, 0x801010F0, t);
            }
            doDraw = 0x1F3;
        } else if (phase == 3) {
            s32 hold = slot->holdTimer;
            t = (hold < 7) ? 1.0f
                           : 1.0f - ((f32)hold - 7.0f) * (1.0f / 7.0f);
            color1 = ColorLerpPacked(0x00F0C070, 0x55F0C070, t);
            color2 = ColorLerpPacked(0x00442D00, 0x60442D00, t);
            if (slot->targetDelta >= 0) {
                textColor = ColorLerpPacked(0x0029A1FF, 0x6029A1FF, t);
            } else {
                textColor = ColorLerpPacked(0x001010F0, 0x801010F0, t);
            }
            doDraw = 0x1F3;
        }

        if (doDraw != 0) {
            char buf[16];
            s32  len;
            func_00115DA8(buf, D_1A8E00, slot->targetDelta);
            len = func_001157AC(buf);
            if (len > 0) {
                void *atlas  = (u8 *)g_guiInstance + 0x8710;
                f32   xscale = D_1A8DE0[len - 1];
                f32   yfudge = *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8A);
                f32   glyphY = (f32)(s32)((f32)2 * yfudge + 0.5f);
                s32   textY  = (s32)((f32)0x24 * yfudge + 0.5f);
                s32   g1 = GuiFontAtlasLookupGlyph(atlas, 0xB2);
                s32   g2;
                func_003017F8(g1, color1,
                              (f32)0x1F3, glyphY, 1.0f, 1.0f, xscale);
                g2 = GuiFontAtlasLookupGlyph(atlas, 0xB3);
                func_003017F8(g2, color2,
                              (f32)0x1F3, glyphY, 1.0f, 1.0f, xscale);
                len = func_001157AC(buf);
                func_00280090(0x1BA, textY, textColor, buf, len);
            }
        }

        slot = (HudRollSlot *)((u8 *)slot + 0x10);
    }
}
#endif

/* func_0028B558: mis-split 1-instruction fragment — `addiu $sp,+0x10` epilogue
 * tail, pinned as its own symbol; no jr $ra. Not a real function; left
 * INCLUDE_ASM. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028B558);

/*
 * g_pHudAssetHeaderAbs: an ASSEMBLER alias of g_pHudAssetHeader (the #8036
 * construct, as for g_playerProgressAbs in text/191238.c). Declared 8 bytes so
 * cc1 -G8 treats it as small and emits one unsplit macro `lw rX, sym+4`; the
 * `.extern ,16` makes gas size the alias non-small, so it expands that macro
 * absolutely with the destination as its own %hi temp (`lui rX; lw rX,%lo(rX)`),
 * which is the ROM's form here. The relocation lands on g_pHudAssetHeader
 * itself, and every other reader of the real symbol is unaffected (gas decides
 * gp-rel vs absolute per symbol, per file).
 */
#ifndef TARGET_NATIVE
__asm__(".extern g_pHudAssetHeaderAbs, 16\n\tg_pHudAssetHeaderAbs = g_pHudAssetHeader");
extern void *g_pHudAssetHeaderAbs[2];
#else
#define g_pHudAssetHeaderAbs g_pHudAssetHeader
#endif

/**
 * Linear-scan the HUD icon-slot table (g_pHudAssetHeader[1]) for the entry
 * whose texture id equals `name`, stopping at the 0xFFFF sentinel.
 *
 *   name  the texture id to look for
 *   ->    its slot index, or the sentinel's index when absent
 *
 * The first entry is tested before the loop (both the sentinel and the name),
 * and the loop then walks a copy of the table pointer with the index
 * incremented in the sentinel branch's delay slot. That loop is 4
 * instructions, so the ROM carries two short-loop pad `nop`s between its two
 * exit tests (R5900_SHORT_LOOP_PAD2). The table pointer is read through
 * g_pHudAssetHeaderAbs for the ROM's `lui $5; lw $5` pair.
 */
s32 func_0028B560(s32 name) {
    HudIconSlot *table = (HudIconSlot *)g_pHudAssetHeaderAbs[1];
    HudIconSlot *p;
    s32 i = 0;
    s32 id;
    if (table[0].texId != 0xFFFF && table[0].texId != name) {
        p = table;
        do {
            p++;
            i++;
            id = p->texId;
            if (id == 0xFFFF) {
                break;
            }
            R5900_SHORT_LOOP_PAD2(id);
        } while (id != name);
    }
    return i;
}

extern void func_00283438(void *base, s32 size); /* clear/init a memory block */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", InitHudMobyTable);
#else
/**
 * First-time setup of the HUD moby-table context.
 *
 * Rebuilds all 13 D_2552B0 widget records (registering each via func_0028BE10 and
 * seeding its fields: key = -1, +0x20 = 0x10000, +0x6C = -6, +0x7C/field04/field24
 * = 0). Then, if the HUD moby table has not been allocated yet, DebugMalloc's the
 * 0x2800-byte table and the 0x1400-byte aux block. Records the table extent
 * (g_hudMobyTableBase..End, spawn start), clears + initialises it (func_00283438),
 * tags its +0x20 byte 0xFF, rebuilds the weapon-select wheel (func_0028C728) and
 * snaps the bolt counter (ResetBoltCounterHud).
 */
void InitHudMobyTable(void) {
    s32 i;

    *(s32 *)((u8 *)&g_pActiveTextTable + 0x30) = 0;
    *(s32 *)((u8 *)&g_pActiveTextTable + 0x34) = 0;

    for (i = 0; i < 13; i++) {
        D_2552B0[i].key = -1;
        *(s32 *)((u8 *)&D_2552B0[i] + 0x20) = 0x10000;
        func_0028BE10(i, 0xFFFF, 0, 0, 0, 0, 1);
        *(s32 *)((u8 *)&D_2552B0[i] + 0x7C) = 0;
        *(s32 *)((u8 *)&D_2552B0[i] + 0x6C) = -6;
        D_2552B0[i].field04 = 0;
        D_2552B0[i].field24 = 0;
    }

    if (g_hudMobyTableBase == 0) {
        g_hudMobyTableBase = DebugMalloc(0x2800);
        g_hudMobyAuxBlockBase = DebugMalloc(0x1400);
    }

    g_hudMobyTableEnd = (u8 *)g_hudMobyTableBase + 0x2800;
    g_hudMobySpawnStart = g_hudMobyTableBase;
    func_00283438(g_hudMobyTableBase, 0x2800);
    *((u8 *)g_hudMobyTableBase + 0x20) = 0xFF;
    func_0028C728();
    ResetBoltCounterHud();
}
#endif

/* func_002EFD28(dest, src, a, count, b): stage `count` (compressed size / 16)
 * units of the wad at `src` into the scratch buffer `dest`. Defined elsewhere. */
extern void func_002EFD28(void *dest, void *src, s32 a, s32 count, s32 b);
/* DecompressWad(src, dest): decompress the staged wad `src` into `dest` (writes
 * its output to the 2nd arg — see text/191238.c). Defined in text/198FA0. */
extern void DecompressWad(void *src, void *dest);
/* Reserve VRAM for a HUD moby-table entry (defined below). */
void RelocateHudBankGsSlots(s32 index, s32 size);
extern u8 g_menuScreenBlock[];                /* 0x menu-screen scratch/VRAM staging block */

/* ReloadAllHudBankTextures(mode): load and upload the HUD moby-table wads into VRAM.
 *
 * The header (g_pHudAssetHeader[0]) holds, per sub-bank, a decompressed size
 * (+0x54/+0x5C/+0x60/+0x64) and a compressed-source pointer (+0x94/+0x9C/+0xA0/
 * +0xA4); the matching compressed sizes live in g_hudMobySpawnStart (+0x8/+0x10/
 * +0x14/+0x18). Each present sub-bank (size != 0) is staged into the menu-screen
 * scratch buffer (g_menuScreenBlock+0x20) via func_002EFD28, decompressed into
 * the running VRAM address (g_menuScreenBlock+0x114, advanced by each bank's
 * size) via DecompressWad, then registered with RelocateHudBankGsSlots.
 *
 * Banks 0 and 1 are always loaded; bank 2 only for mode 0 or 2; bank 3 only for
 * mode 1 or 2. [SEEDABLE: mode] */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", ReloadAllHudBankTextures);
#else
void ReloadAllHudBankTextures(s32 mode) {
    u8   *hdr  = (u8 *)g_pHudAssetHeader[0];
    void *dest = *(void **)(g_menuScreenBlock + 0x20);
    s32   vram = *(s32 *)(g_menuScreenBlock + 0x114);
    s32   size, clen;

    /* bank 0 — always */
    size = *(s32 *)(hdr + 0x54);
    if (size != 0) {
        clen = *(s32 *)((u8 *)&g_hudMobySpawnStart + 0x8);
        func_002EFD28(dest, *(void **)(hdr + 0x94), 0, clen / 16, 0);
        DecompressWad(dest, (void *)vram);
        RelocateHudBankGsSlots(0, vram);
        vram += size;
    }
    /* bank 1 — always */
    size = *(s32 *)(hdr + 0x5C);
    if (size != 0) {
        clen = *(s32 *)((u8 *)&g_hudMobySpawnStart + 0x10);
        func_002EFD28(dest, *(void **)(hdr + 0x9C), 0, clen / 16, 0);
        DecompressWad(dest, (void *)vram);
        RelocateHudBankGsSlots(1, vram);
        vram += size;
    }
    /* bank 2 — mode 0 or 2 */
    if (mode == 0 || mode == 2) {
        size = *(s32 *)(hdr + 0x60);
        if (size != 0) {
            clen = *(s32 *)((u8 *)&g_hudMobySpawnStart + 0x14);
            func_002EFD28(dest, *(void **)(hdr + 0xA0), 0, clen / 16, 0);
            DecompressWad(dest, (void *)vram);
            RelocateHudBankGsSlots(3, vram);
            vram += size;
        }
    }
    /* bank 3 — mode 1 or 2 */
    if (mode == 1 || mode == 2) {
        size = *(s32 *)(hdr + 0x64);
        if (size != 0) {
            clen = *(s32 *)((u8 *)&g_hudMobySpawnStart + 0x18);
            func_002EFD28(dest, *(void **)(hdr + 0xA4), 0, clen / 16, 0);
            DecompressWad(dest, (void *)vram);
            RelocateHudBankGsSlots(4, vram);
        }
    }
}
#endif

/*
 * g_hudClutSlotsAbs / g_hudTextureSlotsAbs: ASSEMBLER aliases of the two slot-
 * array pointers (the #8036 construct, as g_pHudAssetHeaderAbs above), giving
 * the ROM's `lui rX; lw rX,%lo(rX)` read. They are declared as s32 rather than
 * as pointers: the ROM re-reads each pointer after every handle store, which is
 * what cc1 does when a word store may alias the word it read the pointer from.
 * Natively (-m32) they are the pointers' own storage, read as words.
 */
#ifndef TARGET_NATIVE
__asm__(".extern g_hudClutSlotsAbs, 16\n\tg_hudClutSlotsAbs = g_hudClutSlots");
extern s32 g_hudClutSlotsAbs;
__asm__(".extern g_hudTextureSlotsAbs, 16\n\tg_hudTextureSlotsAbs = g_hudTextureSlots");
extern s32 g_hudTextureSlotsAbs;
/* HUD_PREV_FENCE(prev) / HUD_PREV_FENCE_HDR(prev, hdr): SCHEDULING DEVICES, not
 * statements about the machine — the tied empty asm (as RequestLevelExit's).
 * Both keep cc1 from folding `(id - 1) * 4 + off` into the `id * 4` it already
 * holds. The _HDR form also reads `hdr`, which keeps the header load ahead of
 * the `id - 1` (the ROM's order in the CLUT arm; the texture arm has the plain
 * form's order). InvalidateHudBankGsSlots also applies the plain form to a
 * known-zero `id`, so cc1 cannot fold it into the constant it tests. */
#define HUD_PREV_FENCE(prev) __asm__("" : "+r"(prev))
#define HUD_PREV_FENCE_HDR(prev, hdr) __asm__("" : "+r"(prev) : "r"(hdr))
#else
#define g_hudClutSlotsAbs (*(s32 *)&g_hudClutSlots)
#define g_hudTextureSlotsAbs (*(s32 *)&g_hudTextureSlots)
#define HUD_PREV_FENCE(prev) ((void)0)
#define HUD_PREV_FENCE_HDR(prev, hdr) ((void)0)
#endif

/* The HUD asset header base (g_pHudAssetHeader[0]), read as a word. */
#define HUD_ASSET_HEADER ((u8 *)*(s32 *)&g_pHudAssetHeaderAbs[0])

/**
 * Relocate (and mark allocated) the GS handles for one HUD asset's CLUT and
 * texture slots, biasing them by an aligned base address.
 *
 * The HUD asset header (g_pHudAssetHeader[0]) holds three per-asset s32 arrays
 * indexed by assetId: cumulative CLUT-slot end indices at +0x14, cumulative
 * texture-slot end indices at +0x34, and the stored relocation base at +0x74.
 * If this asset's +0x74 word is already non-zero it is relocated -> no-op.
 * Otherwise the (16-byte aligned) base is stored, and every g_hudClutSlots /
 * g_hudTextureSlots handle in this asset's [prevEnd, end) range gets its sign
 * bit cleared (marking it allocated) and the base added.
 *
 *   assetId  index of the HUD asset
 *   baseAddr byte offset added to each handle (rounded up to a multiple of 16)
 *
 * Byte-exact on sdk29 (task #1024). Matching notes:
 *   - the header pointer is re-read after the +0x74 store and in both arms of
 *     the prevEnd test, and each slot-array pointer before each of its two
 *     handle updates, because the ROM treats those words as aliased by the
 *     stores (see g_hudClutSlotsAbs above);
 *   - the rounding goes through $3, and `assetId * 4` is kept in $12 for the
 *     texture half after being used from $2 (EE_REG). The ROM holds a copy
 *     there rather than recomputing it;
 *   - the prevEnd index is `hdr - -(prev * 4)`, the #804 operand-order spelling,
 *     so `addu` takes the header as its first operand;
 *   - the CLUT loop's entry test is a named local in $3 before a do-while, which
 *     is the register the ROM tests; the texture loop needs neither.
 */
void RelocateHudBankGsSlots(s32 assetId, s32 baseAddr) {
    register s32 base EE_REG("$10") = baseAddr;
    s32 id = assetId;
    s32 *relTable = (s32 *)(HUD_ASSET_HEADER + 0x74);
    s32 *relBase = &relTable[id];
    s32 texOff;
    u8 *hdr;
    s32 start;
    s32 end;
    s32 i;

    if (*relBase != 0) {
        return; /* asset already relocated */
    }
    {
        register s32 rounded EE_REG("$3") = base + 0xF;
        base = rounded & 0xFFFFFFF0u;
    }
    *relBase = base;

    /* CLUT slots owned by this asset: the range [prevEnd, end) */
    if (id != 0) {
        s32 prev;
        hdr = HUD_ASSET_HEADER;
        prev = id - 1;
        HUD_PREV_FENCE_HDR(prev, hdr);
        start = *(s32 *)((s32)hdr - -(prev * 4) + 0x14);
    } else {
        hdr = HUD_ASSET_HEADER;
        start = 0;
    }
    {
        register s32 off EE_REG("$2") = id * 4;
        register s32 keep EE_REG("$12");
        end = *(s32 *)(hdr + off + 0x14);
        keep = off;
        texOff = keep;
    }
    i = start;
    {
        register s32 more EE_REG("$3") = i < end;
        if (more) {
            do {
                *(s32 *)(g_hudClutSlotsAbs + i * 8) &= 0x7FFFFFFF;
                *(s32 *)(g_hudClutSlotsAbs + i * 8) += base;
                i++;
            } while (i < end);
        }
    }

    /* texture slots owned by this asset: the range [prevEnd, end) */
    if (id != 0) {
        s32 prev;
        hdr = HUD_ASSET_HEADER;
        prev = id - 1;
        HUD_PREV_FENCE(prev);
        start = *(s32 *)((s32)hdr - -(prev * 4) + 0x34);
    } else {
        hdr = HUD_ASSET_HEADER;
        start = 0;
    }
    end = *(s32 *)(hdr + texOff + 0x34);
    for (i = start; i < end; i++) {
        *(s32 *)(g_hudTextureSlotsAbs + i * 8) &= 0x7FFFFFFF;
        *(s32 *)(g_hudTextureSlotsAbs + i * 8) += base;
    }
}

/**
 * Inverse of RelocateHudBankGsSlots: un-relocate and free one HUD asset's CLUT and
 * texture GS handles. For each g_hudClutSlots / g_hudTextureSlots handle in this
 * asset's [prevEnd, end) range (bounds from the header arrays at +0x14 / +0x34),
 * subtract the stored base (+0x74) and set the sign bit to mark the slot
 * unallocated; then clear the stored base word. Asset 0 additionally clears the
 * +0x4 halfword of each of its texture slots up front.
 *
 *   assetId  index of the HUD asset to unload
 *
 * Byte-exact on sdk29 (task #1024), with RelocateHudBankGsSlots's levers plus:
 *   - asset 0's clear loop re-reads the header pointer in its test and the code
 *     after it uses that last read, so `hdr` is reassigned there. Its two reads
 *     and the halfword store are volatile: a SCHEDULING DEVICE (RULING #8404),
 *     not a statement about the machine. The ROM re-reads both pointers after the
 *     halfword store, which cc1's type-based aliasing would not, and a volatile
 *     final header read keeps reorg from pulling it into the texture loop's
 *     entry-branch slot;
 *   - HUD_PREV_FENCE(id) in the asset-0 path stops cc1 turning the loop's entry
 *     test on the known-zero id into a `blez` (the ROM keeps `slt v0,t1,v0`);
 *   - EE_REG pins for `assetId * 4` ($12, shared by the three later address
 *     sums), the clear loop's index ($4), its texture base ($3) and its test ($2).
 */
#define HUD_ASSET_HEADER_V ((u8 *)*(volatile s32 *)&g_pHudAssetHeaderAbs[0])
#define HUD_TEXTURE_SLOTS_V (*(volatile s32 *)&g_hudTextureSlotsAbs)
void InvalidateHudBankGsSlots(s32 assetId) {
    s32 id = assetId;
    u8 *hdr = HUD_ASSET_HEADER;
    register s32 off EE_REG("$12");
    s32 relBase;
    s32 start;
    s32 end;
    s32 i;

    relBase = *(s32 *)((s32)hdr - -(id * 4) + 0x74);
    if (id == 0) {
        /* asset 0 only: clear the +0x4 halfword of every one of its texture slots */
        register s32 slot EE_REG("$4") = 0;
        HUD_PREV_FENCE(id);
        off = 0;
        if (id < *(s32 *)(hdr + 0x34)) {
            register s32 more EE_REG("$2");
            do {
                register s32 tex EE_REG("$3") = HUD_TEXTURE_SLOTS_V;
                *(volatile s16 *)(tex + slot * 8 + 4) = 0;
                slot++;
                more = slot < *(s32 *)((hdr = HUD_ASSET_HEADER_V) + 0x34);
            } while (more);
        }
    }

    /* CLUT slots [prevEnd, end): remove the base and mark unallocated */
    if (id != 0) {
        s32 prev = id - 1;
        HUD_PREV_FENCE(prev);
        off = id * 4;
        start = *(s32 *)((s32)hdr - -(prev * 4) + 0x14);
    } else {
        start = 0;
    }
    end = *(s32 *)(hdr + off + 0x14);
    for (i = start; i < end; i++) {
        *(s32 *)(g_hudClutSlotsAbs + i * 8) -= relBase;
        *(s32 *)(g_hudClutSlotsAbs + i * 8) |= 0x80000000;
    }

    /* texture slots [prevEnd, end): remove the base and mark unallocated */
    if (id != 0) {
        s32 prev = id - 1;
        hdr = HUD_ASSET_HEADER;
        HUD_PREV_FENCE(prev);
        start = *(s32 *)((s32)hdr - -(prev * 4) + 0x34);
    } else {
        hdr = HUD_ASSET_HEADER;
        start = 0;
    }
    end = *(s32 *)(hdr + off + 0x34);
    for (i = start; i < end; i++) {
        *(s32 *)(g_hudTextureSlotsAbs + i * 8) -= relBase;
        *(s32 *)(g_hudTextureSlotsAbs + i * 8) |= 0x80000000;
    }

    /* clear the stored relocation base */
    *(s32 *)(HUD_ASSET_HEADER_V + off + 0x74) = 0;
}
#undef HUD_ASSET_HEADER_V
#undef HUD_TEXTURE_SLOTS_V

extern void UploadTextureToGs(s32 handle, s32 vramBlk, s32 fmt, s32 wLog, s32 hLog, s32 kickMode); /* UploadTextureToGs */
extern s32 g_vramTextureBase; /* 0x1A72E4 - VRAM static texture base */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", UploadHudBankTextures);
#else
/**
 * Upload a HUD asset's textures to GS VRAM.
 *
 * Relocates the shared HUD asset once (RelocateHudBankGsSlots(0, baseAddr)) if this
 * asset's header +0x74 flag is not yet set. Then, walking this asset's texture
 * slot range [prevEnd, end) (header +0x34 cumulative bounds), uploads each
 * g_hudTextureSlots entry to GS at the running VRAM cursor (UploadTextureToGs =
 * UploadTextureToGs, fmt 0x1B, dims from the slot's +0x6/+0x7 log2 bytes),
 * records the VRAM block in the slot's +0x4 field, and advances the cursor by
 * the texture size (1 << (wLog + hLog)) << 2.
 *
 *   assetId   HUD asset index
 *   baseAddr  relocation base handed to RelocateHudBankGsSlots
 *   kickMode  passed through to UploadTextureToGs
 */
void UploadHudBankTextures(s32 assetId, s32 baseAddr, s32 kickMode) {
    u8 *hdr = (u8 *)g_pHudAssetHeader[0];
    s32 vramCursor;
    s32 start;
    s32 end;
    s32 i;

    if (*(s32 *)(hdr + 0x74 + assetId * 4) == 0) {
        RelocateHudBankGsSlots(0, baseAddr);
    }

    vramCursor = *(s32 *)((u8 *)&g_vramTextureBase + 0x24);

    start = (assetId == 0) ? 0 : *(s32 *)(hdr + 0x34 + (assetId - 1) * 4);
    end = *(s32 *)(hdr + 0x34 + assetId * 4);
    for (i = start; i < end; i++) {
        HudGsSlot *tex = &g_hudTextureSlots[i];
        s32 wLog = ((u8 *)tex)[0x6];
        s32 hLog = ((u8 *)tex)[0x7];
        s32 vramBlk = vramCursor >> 8;

        UploadTextureToGs(tex->handle, vramBlk, 0x1B, wLog, hLog, kickMode);
        *(s16 *)((u8 *)tex + 0x4) = vramBlk;
        vramCursor += (1 << (wLog + hLog)) << 2;
    }
}
#endif

/*
 * g_debugMallocPoolBaseSplit: a second C name for g_debugMallocPoolBase (same
 * assembler symbol via the asm label). The file-scope `.extern ,16` above makes
 * every plain read of the real name an assembler-expanded `lui rX; lw rX` macro;
 * `section(".data")` tells cc1 -G8 this spelling is NOT small data, so cc1 splits
 * the address itself (`lui $2` ... `lw $4,%lo($2)`) and can schedule other work
 * between the two halves, which is the ROM's shape in ResetDebugHeap. No new
 * symbol reaches the object: the relocation names g_debugMallocPoolBase.
 */
#ifndef TARGET_NATIVE
extern u8 *g_debugMallocPoolBaseSplit __asm__("g_debugMallocPoolBase") __attribute__((section(".data")));
#else
#define g_debugMallocPoolBaseSplit g_debugMallocPoolBase
#endif

/**
 * (Re)initialise the DebugMalloc bump allocator: cursor back to the pool base,
 * end at base + 0x64000 (the 0x64000-byte debug pool).
 *
 * The pool base is read through g_debugMallocPoolBaseSplit so cc1 splits its
 * address; the scheduler then materialises the 0x64000 (`lui $3,6`) between the
 * %hi and the load, and the base lands in $a0, as in the ROM. Read through the
 * real name it is one `lw` macro, the constant follows the load, and the
 * function differs in 7 of 11 words.
 */
void ResetDebugHeap(void) {
    u8 *base = g_debugMallocPoolBaseSplit;
    g_debugMallocCursor = base;
    g_debugMallocEnd = base + 0x64000;
}

/**
 * Bump-allocate from the debug pool.
 *
 *   size  bytes requested
 *   ->    the old cursor, or 0 when fewer than `size` bytes remain
 *
 * Lazily initialises the pool (ResetDebugHeap) on first use. The fit test uses
 * the UNROUNDED size and the cursor then advances by the size rounded up to 16,
 * so an allocation can end up to 15 bytes past g_debugMallocEnd (NOTE #5689).
 *
 * MATCHED on the s136os arm (task #1505). Two spellings carry it:
 * - the 0xFFFFFFF0 mask (not ~0xF): cc1 then builds it with lui/ori, as the ROM
 *   does, instead of one `li -16`; and the rounded size goes back into `size`
 *   so it stays in $16.
 * - the cursor store is volatile, a CODEGEN DEVICE (RULING #8404), not a claim
 *   that anything else writes it: the only stores to g_debugMallocCursor in the
 *   USA asm are this function and ResetDebugHeap. cc1 thinks the store is one
 *   gp-relative instruction and would put it in the branch's delay slot; the
 *   ROM keeps it before the branch (gas expands it to lui $1 + sw) and fills
 *   the slot with the epilogue's `ld $16`.
 *
 * GUARD (task #1269): on EE this C is the image's body, compiled alone by SN
 * 2.95.3 v1.36 -fopt-stack (tools/ee/s136os_functions.txt) and spliced over the
 * S136OS_SLOT line by tools/ee/s136os_splice.sh. On native it is plain C.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_DebugMalloc)
S136OS_SLOT(DebugMalloc);
#else
void *DebugMalloc(s32 size) {
    u8 *result;

    if (g_debugMallocCursor == 0) {
        ResetDebugHeap();
    }
    if ((s32)(g_debugMallocEnd - g_debugMallocCursor) < size) {
        return 0;
    }
    result = g_debugMallocCursor;
    size = (size + 0xF) & 0xFFFFFFF0;
    *(u8 *volatile *)&g_debugMallocCursor = result + size;
    return result;
}
#endif

/*
 * g_mobyTableBaseGp: an ASSEMBLER alias of g_mobyTableBase sized 4, the
 * mirror image of g_pHudAssetHeaderAbs. The file-scope `.extern g_mobyTableBase,
 * 16` makes every access to the real name absolute; SwapMobyTableContext's
 * first read in the ROM is %gp_rel (and its write absolute), so that one read
 * goes through this alias, which gas sizes small and addresses off $gp. The
 * relocation names g_mobyTableBase.
 */
#ifndef TARGET_NATIVE
__asm__(".extern g_mobyTableBaseGp, 4\n\tg_mobyTableBaseGp = g_mobyTableBase");
extern void *g_mobyTableBaseGp;
#else
#define g_mobyTableBaseGp g_mobyTableBase
#endif

/* A volatile access to a pointer global: keeps the swap's loads and stores in
 * source order, which is the order the ROM issues them in. */
#define SWAP_VOLATILE(x) (*(void * volatile *)&(x))

/**
 * Toggle the active moby-table context between the live set and the 40-slot
 * HUD shadow set: flip g_activeMobyTableId and exchange the base, spawn-start,
 * end and aux-block pointers of the two sets. A no-op when `newId` is already
 * the active context.
 *
 *   newId  the context requested (0 or 1)
 *
 * Matching notes. The first g_mobyTableBase read is %gp_rel and sits in the
 * early-out branch's delay slot, so it is read through g_mobyTableBaseGp and is
 * the one non-volatile access (a volatile load cannot fill a delay slot). Every
 * other access is volatile, which pins the ROM's interleaving of loads and
 * stores; cc1 otherwise reorders the independent globals. The locals are bound
 * to the ROM's registers with EE_REG ($8 is reused for base, spawn and end).
 */
void SwapMobyTableContext(s32 newId) {
    register s32 id EE_REG("$6") = g_activeMobyTableId;
    register void *base EE_REG("$8");
    register void *hudBase EE_REG("$7");
    register void *spawn EE_REG("$8");
    register void *hudSpawn EE_REG("$4");
    register void *end EE_REG("$8");
    register void *hudEnd EE_REG("$2");
    register void *aux EE_REG("$5");
    register void *hudAux EE_REG("$3");

    if (newId == id) {
        return;
    }
    base = g_mobyTableBaseGp;
    hudBase = SWAP_VOLATILE(g_hudMobyTableBase);
    SWAP_VOLATILE(g_hudMobyTableBase) = base;
    spawn = SWAP_VOLATILE(g_mobySpawnStart);
    hudSpawn = SWAP_VOLATILE(g_hudMobySpawnStart);
    SWAP_VOLATILE(g_hudMobySpawnStart) = spawn;
    end = SWAP_VOLATILE(g_mobyTableEnd);
    hudEnd = SWAP_VOLATILE(g_hudMobyTableEnd);
    aux = SWAP_VOLATILE(g_mobyAuxBlockBase);
    hudAux = SWAP_VOLATILE(g_hudMobyAuxBlockBase);
    *(volatile s32 *)&g_activeMobyTableId = id ^ 1;
    SWAP_VOLATILE(g_mobyTableBase) = hudBase;
    SWAP_VOLATILE(g_mobySpawnStart) = hudSpawn;
    SWAP_VOLATILE(g_mobyTableEnd) = hudEnd;
    SWAP_VOLATILE(g_hudMobyTableEnd) = end;
    SWAP_VOLATILE(g_mobyAuxBlockBase) = hudAux;
    SWAP_VOLATILE(g_hudMobyAuxBlockBase) = aux;
}

/* func_0028BE10(packed, b, c, d, e, f, g): update HUD widget list slot
 * `packed & 0xF` of the D_2552B0 table (stride 0x90). Compares the slot's stored
 * layout fields (+0x20..+0x38) and the high nibble mask (packed & 0xFFF0)
 * against the new values; when any differ (or while not in gameState 5) it
 * rewrites them, bumps the slot's generation counter (+0x64) and the global
 * generation at g_pActiveTextTable+0x30, marks the slot dirty (+0x68=1, clears
 * +0x70/+0x7C) and, when the mask selects bit 0x20, re-inits it via
 * func_0028BF18. Returns the slot's generation counter.
 *
 * MATCH-WALL: a 7-way field-equality early-out chain (register colouring +
 * gameState-5/slot-2 branch-likely + the +0x30 gp/absolute-mix reload) cc1 won't
 * reproduce, so the #ifndef arm stays INCLUDE_ASM; the #else arm is the faithful
 * functional model (engine 2.96 = no byte-match anyway). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028BE10);
#else
s32 func_0028BE10(s32 typeAndSlot, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7) {
    s32 slot = typeAndSlot & 0xF;
    s32 mask = typeAndSlot & 0xFFF0;
    u8 *e = (u8 *)&D_2552B0[slot];
    s32 counter;

    if (g_nGameState == 5 && slot != 0 && slot != 2) {
        return 0;
    }

    if (*(s32 *)(e + 0x2C) == a6 && *(s32 *)(e + 0x28) == a7 &&
        *(s32 *)(e + 0x20) == a2 && *(s32 *)(e + 0x24) == mask &&
        *(s32 *)(e + 0x30) == a3 && *(s32 *)(e + 0x34) == a4 &&
        *(s32 *)(e + 0x38) == a5) {
        return *(s32 *)(e + 0x64);   /* already registered — return its generation */
    }

    counter = *(s32 *)((u8 *)&g_pActiveTextTable + 0x30);
    *(s32 *)(e + 0x64) = counter;
    *(s32 *)((u8 *)&g_pActiveTextTable + 0x30) = counter + 1;
    *(s32 *)(e + 0x2C) = a6;
    *(s32 *)(e + 0x28) = a7;
    *(s32 *)(e + 0x20) = a2;
    *(s32 *)(e + 0x24) = mask;
    *(s32 *)(e + 0x30) = a3;
    *(s32 *)(e + 0x34) = a4;
    *(s32 *)(e + 0x38) = a5;
    *(s32 *)(e + 0x68) = 1;
    *(s32 *)(e + 0x7C) = 0;
    *(s32 *)(e + 0x70) = 0;
    if ((mask & *(s32 *)(e + 0x4) & 0x20) != 0) {
        func_0028BF18((HudElement *)e);
    }
    return *(s32 *)(e + 0x64);
}
#endif

/* ActivateHudElement (USA 0x0028BF18): promote a HUD widget's staged
 * (pending) layout to its live slots and run its init callback.
 *
 * Params: w — the HudElement record to activate (one 0x90-byte registry
 *         entry; field map above the HudElement typedef).
 * Return: none.
 *
 * Binds the icon graphics via func_0028C090(w, the pending icon word at
 * +0x20), then copies the six pending words +0x24..+0x38 to their live slots
 * +0x04..+0x18 (ending with initFn/tickFn/drawFn), calls the new initFn
 * (+0x10) with `w` when it is non-null, and clears the dirty flag (+0x68)
 * last — after the callback, so a callback that re-dirties the widget is
 * overridden.
 *
 * MATCHED byte-exact on the s136os arm (SN 2.95.3 v1.36 -fopt-stack, matched
 * at -G8 -fno-gcse and unchanged at the unit's S136EXTRA="" -O2 default,
 * RULING #9450 / FACT #9348; tools/ee/s136os_functions.txt row, task #1493). The
 * final store MUST be spelled through `w`, not through the `b` byte view:
 * through `b` this compiler keeps a second callee-saved copy of the record in
 * $17 for that one store (0x20 frame, 28/30 words differ, verify_match_unit);
 * through `w` it reuses $16 and the ROM's 0x10 frame. Both spellings are the
 * same address, so the native arm is unaffected. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0028BF18)
S136OS_SLOT(func_0028BF18);
#else
void func_0028BF18(HudElement *w) {
    u8 *b = (u8 *)w;
    void (*init)(HudElement *);
    func_0028C090(w, *(s32 *)(b + 0x20));
    init = *(void (**)(HudElement *))(b + 0x30);
    *(s32 *)(b + 0x4) = *(s32 *)(b + 0x24);
    *(s32 *)(b + 0x14) = *(s32 *)(b + 0x34);
    *(s32 *)(b + 0x18) = *(s32 *)(b + 0x38);
    *(s32 *)(b + 0xC) = *(s32 *)(b + 0x2C);
    *(s32 *)(b + 0x8) = *(s32 *)(b + 0x28);
    *(void (**)(HudElement *))(b + 0x10) = init;
    if (init != 0) {
        init(w);
    }
    *(s32 *)((u8 *)w + 0x68) = 0;
}
#endif

/* (Re)initialise the whole 13-entry D_2552B0 HUD widget table: for each record
 * rebuild its element list via func_0028BE10(i, 0xFFFF, 0,0,0,0,1), clear the
 * record's +0x7C word and set its +0x6C word to -6, then run func_0028BF18 on
 * it.
 *
 * NEAR-MISS: logic exact, but cc1 allocates a 0x30 frame for the four
 * callee-saves ($16-$19) matching the original — only the trailing register
 * colouring / load scheduling differs. Kept as the portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028BF80);
#else
void func_0028BF80(void) {
    u8 *rec = (u8 *)&D_2552B0[0];
    s32 i;
    for (i = 0; i < 0xD; i++) {
        func_0028BE10(i, 0xFFFF, 0, 0, 0, 0, 1);
        *(s32 *)(rec + 0x7C) = 0;
        *(s32 *)(rec + 0x6C) = -6;
        func_0028BF18((HudElement *)rec);
        rec += 0x90;
    }
}
#endif

/**
 * Find the D_2552B0 HUD widget record whose key equals `key` and, if found,
 * rebuild its element list via func_0028BE10(index, 0xFFFF, 0, 0, 0, 0, 0).
 *
 *   key  the record key to look for (Rec2552B0.key, +0x64 in a 0x90-byte record)
 *   ->   1 if a record among the 13 matched (and was rebuilt), else 0
 *
 * Record 0 is tested before the loop, so the scan loop starts at index 1 and
 * walks a pointer to the key field. That loop is 5 instructions, so the ROM
 * carries one short-loop pad `nop` between the key load and its `bnel`
 * (R5900_SHORT_LOOP_PAD1); without it this body is the ROM less that one word.
 */
s32 func_0028C010(s32 key) {
    s32 i = 0;
    if (D_2552B0[0].key != key) {
        s32 recKey;
        do {
            if (++i >= 0xD) {
                return 0;
            }
            recKey = D_2552B0[i].key;
            R5900_SHORT_LOOP_PAD1(recKey, i);
        } while (recKey != key);
    }
    if (i >= 0xD) {
        return 0;
    }
    func_0028BE10(i, 0xFFFF, 0, 0, 0, 0, 0);
    return 1;
}

/**
 * Bind a HUD widget to an icon: resolve the icon slot for `iconName` and copy
 * the slot's texture id, palette id and base frame into the widget.
 *
 *   w         the widget record (+0x0 texture id, +0x40 slot index,
 *             +0x42 palette id, +0x44 base frame)
 *   iconName  texture id looked up by func_0028B560
 *
 * The icon-slot table pointer (g_pHudAssetHeader[1]) is re-read for every
 * field, as in the ROM: each widget store may alias it.
 *
 * MATCHED on the s136os arm (task #1505). The texture id is read BEFORE the
 * slot index is stored, which is the ROM's order (a u16 load and an s16 store
 * the scheduler may not swap). The table is read through g_pHudAssetHeaderAbs,
 * which gives the ROM's `lui rX; lw rX,%lo(g_pHudAssetHeader+4)(rX)` form.
 *
 * GUARD (task #1269): on EE this C is the image's body, compiled alone by SN
 * 2.95.3 v1.36 -fopt-stack (tools/ee/s136os_functions.txt) and spliced over the
 * S136OS_SLOT line by tools/ee/s136os_splice.sh. On native it is plain C.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0028C090)
S136OS_SLOT(func_0028C090);
#else
void func_0028C090(HudElement *w, s32 iconName) {
    u8 *b = (u8 *)w;
    s32 slot = func_0028B560(iconName);
    s32 texId = ((HudIconSlot *)g_pHudAssetHeaderAbs[1])[slot].texId;

    *(s16 *)(b + 0x40) = (s16)slot;
    *(s32 *)(b + 0x0) = texId;
    *(s8 *)(b + 0x42) = ((HudIconSlot *)g_pHudAssetHeaderAbs[1])[slot].paletteId;
    *(s32 *)(b + 0x44) = ((HudIconSlot *)g_pHudAssetHeaderAbs[1])[slot].baseFrame;
}
#endif

/* func_0028C100: mis-split 1-instruction fragment — `addiu $sp,+0x20` epilogue
 * tail, pinned as its own symbol; no jr $ra. Not a real function; left
 * INCLUDE_ASM. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C100);

/**
 * Find the D_2552B0 record whose key is `key` and store `value` into it.
 *
 *   key    the record key to search for (+0x64 of each record)
 *   value  stored into the found record's +0x24, and also its +0x4 when the
 *          record's +0x68 is zero
 *
 * The table holds 13 records of 0x90 bytes. Record 0 is tested before the
 * loop, and the call does nothing when no record matches.
 *
 * MATCHED on the s136os arm (task #1679). The unit's s136os compile runs at
 * the -O2 default, S136EXTRA="" (RULING #9450): the ROM copies the table's %hi
 * into a second register at entry and re-adds %lo to that copy after the loop,
 * a cross-block copy that is gcse's and that no spelling reproduces with gcse
 * off (FACT #9331). Under the unit's former s136 pin (-fno-gcse, which the 2.9
 * compile keeps) the same source builds 30 words against the ROM's 29, LONGER
 * (FACT #9442); vmu's old "19/30" for it was a positional count of that longer
 * body, not a same-length near-miss.
 * R5900_SHORT_LOOP_PAD1 is an RULING #8435 scheduling device. It places the
 * ROM's loop pad nop, and without it the body builds 28 words, not 29.
 *
 * GUARD (task #1269): on EE this C is the image's body, compiled alone by SN
 * 2.95.3 v1.36 -fopt-stack (tools/ee/s136os_functions.txt) and spliced over the
 * S136OS_SLOT line by tools/ee/s136os_splice.sh. On native it is plain C.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0028C108)
S136OS_SLOT(func_0028C108);
#else
void func_0028C108(s32 key, s32 value) {
    s32 i = 0;
    if (D_2552B0[0].key != key) {
        s32 recKey;
        do {
            if (++i >= 0xD) {
                return;
            }
            recKey = D_2552B0[i].key;
            R5900_SHORT_LOOP_PAD1(recKey, i);
        } while (recKey != key);
    }
    if (i < 0xD) {
        D_2552B0[i].field24 = value;
        if (D_2552B0[i].field68 == 0) {
            D_2552B0[i].field04 = value;
        }
    }
}
#endif

/**
 * Apply a layout record's alignment flags (+0x60) to an X (*pA) and Y (*pB)
 * coordinate using its extents (+0x58 X, +0x5C Y). Unless bit 0 or bit 1 is
 * set, Y moves up by half the Y extent. Unless bit 2 is set, X moves left by
 * the full X extent (bit 3 set) or by half of it (bit 3 clear).
 *
 *   rec  the layout record
 *   pA   X coordinate, adjusted in place
 *   pB   Y coordinate, adjusted in place
 *   ->   always 0
 *
 * The flags are read again after the *pB update (that store may alias the
 * record), on every path: the ROM reloads them in the second test's branch-
 * likely delay slot as well as at the join. The empty `"+r"` asm on the record
 * pointer stops cc1 proving the reload redundant on the skip paths, and splits
 * the Y-extent load from the first two so it lands in the first branch's delay
 * slot. Bit 0 is tested in value form (`!(flags & 1)`, the ROM's xori/andi).
 * The reloaded flags and the bit-3 test are EE_REG-bound to the ROM's $4/$2.
 */
s32 func_0028C180(HudElement *rec, s32 *pA, s32 *pB) {
    u8  *b = (u8 *)rec;
    s32  flags = *(s32 *)(b + 0x60);
    s32  xExtent = *(s32 *)(b + 0x58);
    s32  yExtent;
    s32  notHidden;
    register s32 flagsAgain EE_REG("$4");

    __asm__("" : "+r"(b));
    yExtent = *(s32 *)(b + 0x5C);
    notHidden = !(flags & 1);
    if (notHidden && (flags & 2) == 0) {
        *pB = *pB - (yExtent >> 1);
    }
    flagsAgain = *(s32 *)(b + 0x60);
    if ((flagsAgain & 4) == 0) {
        register s32 fullShift EE_REG("$2") = flagsAgain & 8;

        if (fullShift != 0) {
            *pA = *pA - xExtent;
        } else {
            *pA = *pA - (xExtent >> 1);
        }
    }
    return 0;
}

/* func_0028C1E8(rec, pX, pY, ...): apply a HUD layout record's alignment flags
 * (+0x60) to an (*pX,*pY) coordinate using fractional offset tables
 * (D_255A00/D_255A60) scaled by the record's half-extents (+0x58/+0x5C), via
 * IntToFloat/FloatToInt round-trips.
 *
 * The record's +0x7C tag selects both the offset table (set -> D_255A00,
 * clear -> D_255A60) and the sign of the table index (idxBase - idxDelta vs
 * idxBase + idxDelta), which is clamped into [0, 0x17]. The +0x60 flags then
 * pick exactly one axis/direction: bit 1/2 nudge Y by the +0x5C extent (biased
 * +52 px) negative/positive; bit 4/8 nudge X by the +0x58 extent (biased +20 px)
 * negative/positive; each nudge = round(table * (extent + bias)). The results
 * are ADDED into *pX / *pY.
 *
 * WALL (matching build): callee-saves incl. $f20 + the FP scaling schedule and
 * the movz/movz index clamp cc1 does not reproduce. Matching arm stays
 * INCLUDE_ASM; #else is the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C1E8);
#else
extern f32 IntToFloat(s32 v);      /* 0x2846D8 int->float (mtc1;cvt.s.w) */
extern s32 FloatToInt(f32 x);      /* 0x2846A0 float->int (cvt.w.s;mfc1) */
extern f32 D_255A00[];             /* fractional-offset table, rec+0x7C set (24 entries) */
extern f32 D_255A60[];             /* fractional-offset table, rec+0x7C clear (24 entries) */
void func_0028C1E8(HudElement *rec, s32 *pX, s32 *pY, s32 idxBase, s32 idxDelta) {
    u8 *r = (u8 *)rec;
    s32 idx;
    f32 scale;
    s32 flags;
    s32 dx = 0;   /* X nudge from the +0x58 half-extent (flags 4/8) */
    s32 dy = 0;   /* Y nudge from the +0x5C half-extent (flags 1/2) */

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
    scale = (*(s32 *)(r + 0x7C) != 0) ? D_255A00[idx] : D_255A60[idx];

    flags = *(s32 *)(r + 0x60);
    if (flags & 0x1) {
        dy = -FloatToInt(scale * (IntToFloat(*(s32 *)(r + 0x5C)) + 52.0f) + 0.5f);
    } else if (flags & 0x2) {
        dy = FloatToInt(scale * (IntToFloat(*(s32 *)(r + 0x5C)) + 52.0f) + 0.5f);
    } else if (flags & 0x4) {
        dx = -FloatToInt(scale * (IntToFloat(*(s32 *)(r + 0x58)) + 20.0f) + 0.5f);
    } else if (flags & 0x8) {
        dx = FloatToInt(scale * (IntToFloat(*(s32 *)(r + 0x58)) + 20.0f) + 0.5f);
    }

    *pX += dx;
    *pY += dy;
}
#endif

/* func_0028C390(rec): recompute a HUD widget's auto-sized extents. Resolves the
 * value pointer at rec+0xC (when valid and word-aligned) into the rec+0x78
 * working width, clamped against rec+0x8, else seeds 0x1869F; counts the decimal
 * digits of rec+0x8 (loop dividing by 10) and applies the rec+0x60 alignment
 * flags to derive the rec+0x5C/+0x58 half-extents (12 px per digit / 14 px caps).
 *   rec - the HudElement record; no return value
 *
 * Byte-exact on sdk29 (task #978), EE arm below. cc1 2.9 does emit the ROM's
 * `div`/`beql`/`break 7`/`mflo` sequence from C `/`. The levers:
 *   - volatile else-path stores keep the 0x1869F seed stores in their
 *     own block (#948);
 *   - three counted empty asms at the head of the digit loop lift it over
 *     cc1's short-loop threshold, so reorg fills the loop branch's slot with
 *     `digits++` (FACT #8384, #948);
 *   - f60 pinned to $8. Unpinned, it takes $7 and the div-by-zero guard's
 *     zero takes $8, the ROM's pair swapped. The use at the very end keeps f60
 *     live, so the `andi` tests go to v0 instead of reusing $8, which keeps
 *     the duplicated `slti v0,t1,12` the ROM carries at 0x28C45C.
 * The ROM's `bnel`/`beql` slots hold plain register stores and `break`, not
 * the symbolic macros asm_unit.sh hoists (FACT #8385). */
#ifndef TARGET_NATIVE
void func_0028C390(void *rec) {
    u8 *p = (u8 *)rec;
    s32 valPtr = *(s32 *)(p + 0xC);
    s32 n, f5C, digits;
    register s32 f60 __asm__("$8");

    if (valPtr != 0 && (valPtr & 3) == 0) {
        s32 value = *(s32 *)valPtr;
        s32 cap = *(s32 *)(p + 0x8);
        *(s32 *)(p + 0x78) = value;
        if (cap < value) {
            *(s32 *)(p + 0x78) = cap;
        }
        *(s32 *)(p + 0x74) = *(s32 *)(p + 0x78);
    } else {
        *(volatile s32 *)(p + 0x74) = 0x1869F;
        *(volatile s32 *)(p + 0x78) = 0x1869F;
    }

    n = *(s32 *)(p + 0x8);
    f60 = *(s32 *)(p + 0x60);
    f5C = *(s32 *)(p + 0x5C);
    digits = 0;
    if (n >= 0xA) {
        s32 t = n;
        do {
            __asm__ __volatile__("");
            __asm__ __volatile__("");
            __asm__ __volatile__("");
            t = t / 0xA;
            digits++;
        } while (t >= 0xA);
    }

    if ((f60 & 3) == 0 && (f60 & 0xC) != 0) {
        *(s32 *)(p + 0x5C) = (digits + 1) * 0xC + f5C;
        if (*(s32 *)(p + 0x58) < 0xE) {
            *(s32 *)(p + 0x58) = 0xE;
        }
        return;
    }

    if (f5C < 0xC) {
        *(s32 *)(p + 0x5C) = 0xC;
    }
    *(s32 *)(p + 0x58) += (digits + 1) * 0xE;
    __asm__("" : : "r"(f60));
}
#else
/* Native arm: the plain model of the EE arm above. The old MATCH-WALL note here
 * ("cc1 won't reproduce the div/break scaffolding from C `/`") was wrong: cc1
 * 2.9 does emit it (task #978). */
void func_0028C390(void *rec) {
    u8 *p = (u8 *)rec;
    s32 valPtr = *(s32 *)(p + 0xC);
    s32 n, f60, f5C, digits;

    if (valPtr != 0 && (valPtr & 3) == 0) {
        s32 value = *(s32 *)valPtr;
        s32 cap = *(s32 *)(p + 0x8);
        *(s32 *)(p + 0x78) = value;
        if (cap < value) {
            *(s32 *)(p + 0x78) = cap;
        }
        *(s32 *)(p + 0x74) = *(s32 *)(p + 0x78);
    } else {
        *(s32 *)(p + 0x74) = 0x1869F;
        *(s32 *)(p + 0x78) = 0x1869F;
    }

    n = *(s32 *)(p + 0x8);
    f60 = *(s32 *)(p + 0x60);
    f5C = *(s32 *)(p + 0x5C);
    digits = 0;
    if (n >= 0xA) {
        s32 t = n;
        do {
            t = t / 0xA;
            digits++;
        } while (t >= 0xA);
    }

    if ((f60 & 3) == 0 && (f60 & 0xC) != 0) {
        *(s32 *)(p + 0x5C) = (digits + 1) * 0xC + f5C;
        if (*(s32 *)(p + 0x58) < 0xE) {
            *(s32 *)(p + 0x58) = 0xE;
        }
        return;
    }

    if (f5C < 0xC) {
        *(s32 *)(p + 0x5C) = 0xC;
    }
    *(s32 *)(p + 0x58) += (digits + 1) * 0xE;
}
#endif

/* Reset a HUD widget record: set its type tag (+0x7C = 0xD2), clear the two
 * 16-bit cursor fields (+0x48/+0x4A) and re-init it via func_0028C390. */
void func_0028C490(HudElement *p) {
    *(s32 *)((u8 *)p + 0x7C) = 0xD2;
    *(s16 *)((u8 *)p + 0x48) = 0;
    *(s16 *)((u8 *)p + 0x4A) = 0;
    func_0028C390(p);
    __asm__ __volatile__("");
}

/* func_0028C4C0: 8-byte zero pad between functions — splat drops all-zero
 * inter-function regions, so recover it as a raw-word filler to keep the unit
 * size==span byte-exact (else downstream funcs shift, corrupting baked pointer
 * tables). 06333c5 precedent; NO re-split. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C4C0);

/* func_0028C4C8(w): per-frame smooth-roll update for a HUD counter widget.
 *
 * (1) Re-clamps the smoothed target (+0x78) from the live value pointer (+0xC):
 *     max(*valPtr, 0), bounded above by the raw value (+0x8).
 * (2) When the displayed value (+0x74) has not yet reached the target (+0x78)
 *     and the widget is "settled" (+0x6C >= 0x18), eases +0x74 toward +0x78:
 *     arms the +0x7C timer (0xB4), derives an easing step
 *         step = max( FloatToInt(func_002835C0(delta*0.04) * 5.0), delta/5 )
 *     clamped to [1,0x79] (delta = |+0x74 - +0x78|), advances the sub-step byte
 *     accumulator (+0x73) and, once it passes 3, moves +0x74 by (accum>>1)
 *     toward the target and drains the accumulator by twice that.
 * (3) Drives the two digit-roll byte counters (+0x70/+0x71): while the +0x7C
 *     timer is hot (>= 5) they ramp up (cap 8 each); otherwise they wind back
 *     down and re-arm the settle flag (+0x6C = 1).
 * (4) Refreshes the icon animation frame via func_0028E7E8(w+0x40).
 *
 * The FP easing (IntToFloat/func_002835C0/FloatToInt) is reproduced op-for-op;
 * the real R5900 helpers do the arithmetic (cmp-oracle validates the result).
 * [SEEDABLE] pure per-widget state machine over w's byte/word fields.
 *
 * Track-B note (+0x7C): written 0xB4 here and read back as a countdown timer,
 * which conflicts with the "+0x7C = element type tag" label in the HudElement
 * map above — same tension flagged in func_0028E9A0; left for Track-B. */
extern float IntToFloat(s32 x);       /* 0x284690: mtc1;cvt.s.w  int -> float */
extern s32   FloatToInt(float x);     /* 0x2846A0: cvt.w.s;mfc1  float -> int */
extern float func_002835C0(float x);
s32 func_0028E7E8(void *iconSlot);  /* returns frame index; real def below in this unit */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C4C8);
#else
void func_0028C4C8(HudElement *w) {
    u8  *b   = (u8 *)w;
    u8  *cnt = b + 0x70;                 /* byte counter block ($18 = w+0x70) */
    s32 *valPtr = *(s32 **)(b + 0xC);
    s32  cur, target, tag;

    /* (1) clamp the smoothed target from the live value pointer */
    if (valPtr != 0) {
        s32 v   = *valPtr;
        s32 raw = *(s32 *)(b + 0x8);
        if (v < 0) v = 0;
        *(s32 *)(b + 0x78) = v;
        if (raw < v) *(s32 *)(b + 0x78) = raw;
    }

    /* (2) ease the displayed value toward the target */
    cur    = *(s32 *)(b + 0x74);
    target = *(s32 *)(b + 0x78);
    if (cur != target) {
        *(s32 *)(b + 0x7C) = 0xB4;
        if (*(s32 *)(b + 0x6C) >= 0x18) {
            s32 delta = cur - target;
            s32 stepF, div5, step, accum;
            if (delta < 0) delta = -delta;

            stepF = FloatToInt(func_002835C0(IntToFloat(delta) * 0.04f) * 5.0f);
            div5  = delta / 5;
            step  = (stepF < div5) ? div5 : stepF;      /* max(stepF, delta/5) */
            if (step >= 0x7A)   step = 0x79;
            else if (step <= 0) step = 1;

            accum = (cnt[3] + step) & 0xFF;
            cnt[3] = (u8)accum;
            if (accum >= 3) {
                s32 aa   = *(s32 *)(b + 0x74);
                s32 tt   = *(s32 *)(b + 0x78);
                s32 half = accum >> 1;
                *(s32 *)(b + 0x74) = (tt < aa) ? (aa - half) : (aa + half);
                cnt[3] = (u8)(cnt[3] - (half << 1));
            }
        }
    }

    /* (3) digit-roll counters keyed on the +0x7C timer */
    tag = *(s32 *)(b + 0x7C);
    if (tag < 5) {
        u8 hi = cnt[1];
        u8 lo = cnt[0];
        if (hi != 0) {
            *(s32 *)(b + 0x6C) = 1;
            cnt[1] = hi - 1;
        } else if (lo != 0) {
            *(s32 *)(b + 0x6C) = 1;
            cnt[0] = lo - 1;
        }
    } else {
        u8 lo = cnt[0];
        if (lo < 8) {
            cnt[0] = lo + 1;
        } else {
            u8 hi = cnt[1];
            if (hi < 8) cnt[1] = hi + 1;
        }
    }

    /* (4) refresh the icon animation frame */
    func_0028E7E8(b + 0x40);
}
#endif

/* func_0028C698: 0x48-byte run of handwritten stub-table fragments (addiu
 * $sp,+N; nop pairs) preceding the real function, pinned as one symbol. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C698);

/* HUD-init forwarder: enable a HUD subsystem via func_0029DB10(arg, 1) then
 * register HUD element 0xD with func_002B1B48(0xD, 0, 1). */
void func_0028C6E0(s32 arg) {
    func_0029DB10(arg, 1);
    func_002B1B48(0xD, 0, 1);
    __asm__ __volatile__("");
}

/* func_0028C710: mis-split handwritten stub-table fragment — three `addiu
 * $sp,+N; nop` epilogue tails (+0x10/+0x10/+0x50) with NO jr $ra. Not a real
 * returning function; left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C710);

/* func_0028C728(): rebuild the 8-slot weapon-select wheel icon list. Stores the
 * slot count (8) and the &D_1A8DD0 callback into g_hudMobySpawnStart+0x28/+0x2C,
 * resets the wheel cursor (D_1A8D48=0), then for each of the 8 source item ids
 * in the g_gsPixelOffsetY+0x64 table copies the id into the wheel record
 * (D_002550F0, stride 0x1C, +0x18) and resolves the weapon's nameStringId
 * (g_weaponTable[g_itemEquippedSlot[id]].nameStringId) into the record's +0x0.
 *
 * WALL (gp/absolute-mix + reloaded-ptr): the source table is addressed as the
 * named sub-object g_gsPixelOffsetY+0x64 with a running displacement, and the
 * wheel-record base is reloaded each iteration; the mixed %gp_rel/%hi-lo
 * addressing and induction colouring are fixed SN-cc1 behaviour. Left
 * INCLUDE_ASM. */
extern s32 g_equippedItemSlots[8];            /* currently-equipped item ids (0x1A73B8) */

typedef struct WheelRecord {
    s32 nameStringId;   /* +0x00 */
    u8  _pad04[0x14];
    s32 itemId;         /* +0x18: resolved item id for this wheel slot */
} WheelRecord;                                /* stride 0x1C */

extern WheelRecord D_002550F0[];              /* wheel icon-list record array (0x2550F0) */
extern u8 D_1A8DD0;                            /* wheel record header base (0x1A8DD0) */

/* g_equippedItemSlotsAbs: an assembler alias of g_equippedItemSlots (the #8036
 * construct). Declared 8 bytes so cc1 -G8 emits one `la` macro, sized 16 for gas
 * so that macro expands absolutely with the destination as its own %hi temp
 * (`lui $4; addiu $4,$4`), the ROM's form in func_0028C728 and func_0028C7A8. */
#ifndef TARGET_NATIVE
__asm__(".extern g_equippedItemSlotsAbs, 16\n\tg_equippedItemSlotsAbs = g_equippedItemSlots");
extern s32 g_equippedItemSlotsAbs[2];
#else
#define g_equippedItemSlotsAbs g_equippedItemSlots
#endif

/**
 * Build the weapon-select wheel's icon list: publish the list header (8 slots,
 * record array &D_1A8DD0) at g_hudMobySpawnStart+0x28/+0x2C, clear the wheel
 * cursor D_1A8D48, then fill one D_002550F0 record per equipped slot with the
 * slot's item id (+0x18) and that item's active variant's display-name string
 * id (+0x00).
 *
 * Matching notes. The loop is the ROM's: a count-down from 7 with the source,
 * record and table pointers stepping in place. The slot and table bases and
 * the 0xE0 stride are named locals so their loop-invariant loads are placed
 * as the ROM places them; `id + (s32)slots` gives the ROM's id-first `addu`,
 * and g_equippedItemSlotsAbs its `la` macro pair.
 */
void func_0028C728(void) {
    s32 n;
    s32 *src;
    WheelRecord *rec;
    u8 *slots;
    WeaponDef *table;
    s32 stride;

    *(s32 *)((u8 *)&g_hudMobySpawnStart + 0x28) = 8;
    *(void **)((u8 *)&g_hudMobySpawnStart + 0x2C) = &D_1A8DD0;
    slots = g_itemEquippedSlot;
    table = g_weaponTable;
    rec = D_002550F0;
    D_1A8D48 = 0;
    stride = 0xE0;
    src = g_equippedItemSlotsAbs;
    n = 7;
    do {
        s32 id = *src;

        n--;
        src++;
        rec->itemId = id;
        rec->nameStringId = *(u16 *)((u8 *)table + *(u8 *)(id + (s32)slots) * stride + 0x3C);
        rec++;
    } while (n >= 0);
}

/* WheelRecord + g_equippedItemSlots already declared by func_0028C728's block above. */

/**
 * SyncEquippedItemSlots: refresh the 8-entry equipped-item cache
 * (g_equippedItemSlots[0..7]) from the weapon-select wheel records. The record
 * array base is the pointer stored at g_hudMobySpawnStart+0x2C (== &D_002550F0,
 * stride 0x1C) and each record's +0x18 holds the resolved item id; every slot
 * whose cached id differs is overwritten with the record's.
 *
 * Matching notes. The ROM re-reads the record-array base on every iteration,
 * as if the slot store could alias it; cc1 would hoist it (type-based aliasing),
 * so the read is volatile. The loop is the ROM's: a byte offset stepping by
 * 0x1C, a count-down from 7 and a post-incremented slot pointer. The record
 * address is spelled `off - -(s32)base` because the ROM's `addu` takes the
 * offset as its first operand and only the negated form puts it there (the
 * #804 operand-order lever). The slot array is read through
 * g_equippedItemSlotsAbs for the ROM's `la` macro pair.
 */
void func_0028C7A8(void) {
    WheelRecord *volatile *pRec = *(WheelRecord * volatile **)((u8 *)&g_hudMobySpawnStart + 0x2C);
    s32 off = 0;
    s32 *slot = g_equippedItemSlotsAbs;
    s32 n = 7;

    do {
        WheelRecord *base = *pRec;
        s32 cur;
        s32 id;

        n--;
        cur = *slot;
        id = *(s32 *)(off - -(s32)base + 0x18);
        off += 0x1C;
        if (cur != id) {
            *slot = id;
        }
        slot++;
    } while (n >= 0);
}

/* Build the weapon-select wheel widget `w`: rebuild its icon list
 * (func_0028C728), then seed the wheel geometry/state (half-extents 0xD2/0xC8
 * at +0x58/+0x5C, type tag -2 at +0x74, mode 0x1E at +0x78, cleared +0x70 word
 * and the two +0x48/+0x4A cursor halves).
 *
 * BYTE-EXACT on the s136os arm (task #1329): SN 2.95.3 v1.36 -fopt-stack
 * reproduces all 20 ROM words. Two phrasings carry the match and are not
 * cosmetic: the four constants are materialised into locals BEFORE their
 * stores (the ROM builds $v0/$v1/$a0/$a1 first, then stores them), and the
 * three zero stores are written +0x48, +0x4A, +0x70 because SN 1.36 emits the
 * LAST of them first and the others in source order, giving the ROM's +0x70,
 * +0x48, +0x4A (FACT #8947's store-order rule; here it holds for mixed sh/sw
 * widths, while the four register-valued stores above stay in source order).
 * The trailing `__asm__ __volatile__("")` keeps $16 restored before $31.
 * (It was first matched on the engine96 arm behind MATCH_func_0028C7F0, task
 * #469, with the zero stores in +0x70, +0x48, +0x4A order.)
 *
 * GUARD (task #1329): on EE this C is the image's body, compiled alone by the
 * s136os arm (FACT #8810; row in tools/ee/s136os_functions.txt) and spliced
 * over S136OS_SLOT by tools/ee/s136os_splice.sh. There is no asm fallback: a
 * build that skips the splice drops the function. On native it is plain C.
 * Its MATCH_func_0028C7F0 engine96 guard is retired with this promotion: the
 * function is image-resident, no longer arm-scored (RULING #8118), and it was
 * the unit's last MATCH_ member (RULING #8915 item 2). */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_0028C7F0)
S136OS_SLOT(func_0028C7F0);
#else
void func_0028C7F0(HudElement *w) {
    u8 *b = (u8 *)w;
    s32 halfW, halfH, typeTag, mode;
    func_0028C728();
    halfW = 0xD2;
    halfH = 0xC8;
    typeTag = -2;
    mode = 0x1E;
    *(s32 *)(b + 0x58) = halfW;
    *(s32 *)(b + 0x5C) = halfH;
    *(s32 *)(b + 0x74) = typeTag;
    *(s32 *)(b + 0x78) = mode;
    *(s16 *)(b + 0x48) = 0;
    *(s16 *)(b + 0x4A) = 0;
    *(s32 *)(b + 0x70) = 0;
    __asm__ __volatile__("");
}
#endif

/* func_0028C840(wheel) = UpdateQuickSelectWheelInput: per-frame handler for the
 * quick-select item wheel. Active while the open button is held (g_padButtonsHeld
 * & 0x10); navigates by dpad (g_padButtonsPressed & 0xF000 -> neighbor links) or by
 * analog-stick angle (atan2 of D_1382C8/CC, snapped to the nearest of the wheel's
 * item-count slots). Grid entries: gridBase = *(g_hudMobySpawnStart+0x2C), gridPage
 * = *(gridBase + D_1A8D48*4), entry = gridPage + slot*0x1C (itemId +0x18, dpad
 * neighbors +0x8/+0xC/+0x10/+0x14). On a confirmed selection plays a cue, sets
 * g_activeGadgetItem to the chosen item, clamps its ammo to the variant capacity,
 * and (if the class isn't resident) triggers a gadget-class load. Save-slot fields
 * at g_health+0x554 (+0 s16 seen-counter, +4 u32 clip pos, +8 u32 flags).
 * MATCH-WALL only (FP scheduling); un-walled as faithful #else (engine 2.96 = no
 * byte-match). Float literals are verbatim; the FP-idiom call args + the three
 * addressing modes (gp-rel / page-base-cleared / weapon-table) are .s-verified. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C840);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void *g_guiInstance;
extern s32 g_gsPixelOffsetY;
extern s32 g_playerProgress;
/* (end of this body's declarations) */
/* g_guiInstance (void*), PlayGlobalSound, IntToFloat(s32), FloatToInt: file-scope. */
extern s32 g_padButtonsHeld, g_padButtonsPressed, g_fileLoadState;
extern u8  g_soundBankHandlesBlk[];  /* +0x1248 = last-selected item id */
extern s32 D_1A8C64;                 /* input-lock gate */
extern u8  D_1A7BBB;                 /* inventory-overlay-mode flag */
extern f32 D_1382C8, D_1382CC;       /* analog stick x / y */
extern s32 D_13834C, D_138358;       /* input/mode flags */
extern s32 D_1A8D48;                 /* wheel grid-page index (gp-rel) */
extern s32 D_1A8DA0, D_1A8DA4, D_1A8D98, D_1A8D9C;   /* dpad-neighbor fallbacks */
extern f32 D_1A8E10;                 /* wheel-angle snap residual (gp-rel scratch) */
extern s32 D_1B187C, D_1B1878, D_1B1518;
extern s32 D_1A8D30;                 /* inventory-overlay mode value */
extern s32 g_activeGadgetItem;
extern s32 IsInventoryOverlayMode(void);
extern s32 IsGadgetClassResident(s32 cls);
extern void EnsureGadgetClassResident(s32 cls);
extern void FreeHudElementByHandle(s32 handle);
extern void AdvanceInventoryOverlayToActive(void);
extern void StopFileLoad(void);
extern void DebugPrintStub(s32 p);
extern f32 SqrtfVu0(f32 x);
extern f32 Atan2fPoly(f32 y, f32 x);
extern f32 WrapAnglePiDiff(f32 a, f32 b);
extern f32 AngleShortestDiff(f32 a, f32 b);
extern f32 WrapAngleToPiRange(f32 a);
extern void func_00288888(HudElement *w);
extern s32 func_00283328(s32 p);
extern void func_0034D8C8(s32 a, s32 b);
extern s32 func_00290F98(void);
extern s32 func_00290FA0(void);
extern void func_00290FA8(void);
extern s32 func_00290FC0(void);
void func_0028C840(void *wheel) {
    u8  *p = (u8 *)wheel;
    u8  *save = (u8 *)&g_health + 0x554;   /* +0 s16 counter, +4 u32 clip, +8 u32 flags */
    s32  gridBase, gridPage, item, sel, cand, dir;
    u8  *pbVar9 = p + 0x70;
    f32  ang, mag, snap;
    s32  bActive;

    if (D_1A8C64 != 0) {
        return;
    }

    bActive = 0;
    if (((D_1A7BBB == 0 || IsInventoryOverlayMode() != 0) || D_1A7BBB == 0) &&
        (bActive = 1, D_1A7BBB == 0)) {
        func_00290FA8();
        if (func_00290FC0() == 0) {
            bActive = 0;
        }
    }
    if (bActive && g_guiInstance != 0) {
        s32 mode = 2;
        if (IsInventoryOverlayMode() != 0) {
            mode = D_1A8D30;
        }
        func_0034D8C8((s32)g_guiInstance + 0x376C8, mode);
        /* UNCONFIRMED arg: the .s call has a nop delay slot with no arg set;
           passed the wheel obj to match the existing HudElement* decl. */
        func_00288888((HudElement *)p);
    }

    *(s32 *)(p + 0x7C) = 0xB4;
    *(s32 *)(p + 0x6C) = 0x18;
    if (func_00283328(0x18C316) != 0 && D_1A7BBB == 0) {
        D_13834C = 2;
    }

    mag = SqrtfVu0(D_1382C8 * D_1382C8 + D_1382CC * D_1382CC);
    ang = D_1382C8;
    snap = D_1382CC;
    if (mag != 0.0f) {
        ang = ang / mag;
        snap = snap / mag;
    }
    ang = Atan2fPoly(ang, snap);

    if ((*(s32 *)(p + 0x78) >> 0x18) == 0 &&
        ((g_padButtonsPressed & 0xF000) != 0 || mag < 0.5f ||
         (*(s32 *)(p + 0x78) = *(s32 *)(p + 0x78) - 1, *(s32 *)(p + 0x78) == -1))) {
        *(s32 *)(p + 0x74) = -1;
        *(s32 *)(p + 0x78) = 0x10000FF;
    }

    dir = 0;
    sel = *(s32 *)(p + 0x74);
    gridBase = *(s32 *)((u8 *)&g_hudMobySpawnStart + 0x2C);
    gridPage = *(s32 *)(gridBase + D_1A8D48 * 4);

    if (*(char *)(p + 0x7B) == 1) {
        if ((g_padButtonsHeld & 0x10) == 0) {
            dir = *(s32 *)(p + 0x74);
        } else if (bActive) {
            s32 count = *(s32 *)((u8 *)&g_hudMobySpawnStart + 0x28);
            if (0.9f < mag) {
                f32 wrapped = ang + 3.1415927f + 3.1415927f + 1.5707964f + 3.1415927f / (f32)count;
                f32 pos = wrapped * 0.15915494f * (f32)count;
                f32 total = IntToFloat(count);
                u32 idx;
                if (sel != -1) {
                    f32 d = WrapAnglePiDiff(ang, 1.5707964f);
                    f32 diff = AngleShortestDiff(((f32)sel * 6.2831855f) / total - 3.1415927f, d);
                    if (diff <= 0.5890487f) {
                        goto after_nav;
                    }
                }
                idx = (u32)(s32)pos % (u32)count;
                if ((((idx ^ 1) & 1)) != 0) {
                    s32 ipos = FloatToInt(pos);
                    if (0.8f < pos - (f32)ipos) {
                        idx = idx + 1;
                    } else if (pos - (f32)ipos < 0.2f) {
                        idx = idx - 1;
                    }
                    idx = (u32)((s32)(idx + count)) % (u32)count;
                }
                dir = -1;
                *(s32 *)(p + 0x74) = idx;
                D_1A8E10 = WrapAngleToPiRange(wrapped - ((f32)(s32)idx * 0.78539824f + 0.39269912f));
                if (0.0f < D_1A8E10) {
                    dir = 1;
                }
            } else if (D_138358 == 0 && (g_padButtonsPressed & 0xF000) != 0) {
                cand = sel;
                if ((g_padButtonsPressed & 0x1000) != 0 && (cand = D_1A8DA0, sel != -1)) {
                    cand = *(s32 *)(sel * 0x1C + gridPage + 0x10);
                }
                if ((g_padButtonsPressed & 0x4000) != 0 && (item = D_1A8DA4, cand != -1)) {
                    item = *(s32 *)(cand * 0x1C + gridPage + 0x14);
                    cand = item;
                }
                if ((g_padButtonsPressed & 0x8000) != 0 && (item = D_1A8D98, cand != -1)) {
                    item = *(s32 *)(cand * 0x1C + gridPage + 8);
                    cand = item;
                }
                if ((g_padButtonsPressed & 0x2000) != 0 && (item = D_1A8D9C, cand != -1)) {
                    item = *(s32 *)(cand * 0x1C + gridPage + 0xC);
                    cand = item;
                }
                *(s32 *)(p + 0x74) = cand;
            }
        after_nav:
            if (*(s32 *)(*(s32 *)(p + 0x74) * 0x1C + gridPage) == 0) {
                s32 count = *(s32 *)((u8 *)&g_hudMobySpawnStart + 0x28);
                if (dir != 0) {
                    s32 nidx = (*(s32 *)(p + 0x74) + dir) % count;
                    if (*(s32 *)(nidx * 0x1C + gridPage) != 0) {
                        *(s32 *)(p + 0x74) = nidx;
                    }
                }
                goto load_check;
            }
            dir = *(s32 *)(p + 0x74);
        } else {
            dir = *(s32 *)(p + 0x74);
        }
    } else {
    load_check:
        dir = *(s32 *)(p + 0x74);
    }

    if (dir == sel) {
        if (*(u8 *)(p + 0x71) < 8) {
            *(u8 *)(p + 0x71) += 1;
        }
        sel = *(s32 *)(p + 0x74);
    } else {
        DebugPrintStub(0x1A8E18);
        StopFileLoad();
        *(u8 *)(p + 0x71) = 0;
        PlayGlobalSound(3, 0, 0);
        sel = *(s32 *)(p + 0x74);
    }

    item = (sel < 0) ? -1 :
        *(s32 *)(sel * 0x1C + gridPage + 0x18);
    if (sel < 0 ||
        IsGadgetClassResident(*(s32 *)((u8 *)&g_weaponTable[g_itemEquippedSlot[item]] + 0x14)) != 0) {
        u8 bVal;
        if ((g_padButtonsHeld & 0x10) == 0 && g_fileLoadState == 0) {
            bVal = *pbVar9 - 1;
            if ((*pbVar9 == 0 || (*pbVar9 = bVal, bVal == 0)) && *(s32 *)(p + 0x74) >= 0) {
                s32 chosen = *(s32 *)(*(s32 *)(p + 0x74) * 0x1C + gridPage + 0x18);
                if (chosen != 0) {
                    if (*(s32 *)((u8 *)g_soundBankHandlesBlk + 0x1248) != chosen &&
                        *(s16 *)(save + 0) != -1) {
                        *(s16 *)(save + 0) += 1;
                    }
                    if (*(u32 *)(save + 4) < *(u32 *)((u8 *)&g_gsPixelOffsetY + 0x3C)) {
                        *(u32 *)(save + 4) = *(u32 *)((u8 *)&g_gsPixelOffsetY + 0x3C);
                    }
                    *(u32 *)(save + 8) |= (1u << (g_playerProgress & 0x1F)) | 0x80000000;
                    g_activeGadgetItem = chosen;
                    if (*(s16 *)((u8 *)&g_weaponTable[g_itemEquippedSlot[chosen]] + 0x88) != 0) {
                        s32 cap = *(u16 *)((u8 *)&g_weaponTable[g_itemEquippedSlot[chosen]] + 0x8E);
                        if (cap < g_weaponAmmo[chosen]) {
                            g_weaponAmmo[chosen] = cap;
                        }
                    }
                }
            }
            if (D_1A7BBB == 0) {
                func_00290FA0();
                func_00290F98();
            } else {
                AdvanceInventoryOverlayToActive();
            }
            bVal = *pbVar9;
        } else {
            bVal = *pbVar9;
        }
        if (bVal < 8) {
            *pbVar9 = bVal + 1;
        }
        bVal = *pbVar9;
        if (bVal == 0) {
            D_1B187C = *(s32 *)(p + 0x78);
*(s32 *)((u8 *)&g_hudMobyAuxBlockBase + 0x1C) = *(s32 *)(p + 0x74);
            FreeHudElementByHandle(*(s32 *)(p + 0x64));
            D_1B1878 = D_1B1518;
            *(s32 *)(p + 0x6C) = -6;
        }
    } else {
        EnsureGadgetClassResident(
            *(s32 *)((u8 *)&g_weaponTable[g_itemEquippedSlot[
                *(s32 *)(*(s32 *)(p + 0x74) * 0x1C + gridPage + 0x18)]] + 0x14));
        if (*pbVar9 == 0) {
            D_1B187C = *(s32 *)(p + 0x78);
*(s32 *)((u8 *)&g_hudMobyAuxBlockBase + 0x1C) = *(s32 *)(p + 0x74);
            FreeHudElementByHandle(*(s32 *)(p + 0x64));
            D_1B1878 = D_1B1518;
            *(s32 *)(p + 0x6C) = -6;
        }
    }
}
#endif

/* DrawWeaponSelectWheel(w): render the weapon quick-select wheel widget (~0x5C4).
 *
 * `w` is the wheel HudElement: w+0x70/+0x71 are the open/close animation counters
 * (each /8, clamped to [0,1] -> fillA/fillB); w+0x74 is the highlighted item
 * index; w+0x50/+0x54 are the screen anchor (x,y), aligned in place by
 * func_0028C180. For each wheel item (count/callback-array at
 * g_hudMobySpawnStart+0x28/+0x2C, 0x1C stride, indexed base = array[D_1A8D48]) it
 * computes a per-item angle (WrapAnglePiSum, result unused here), drives the
 * highlight tween for the selected item (func_0034DAB0), and pokes the GUI alpha
 * slot (FloatToInt(alpha*0.5) while func_00290FC0(), else 0). For the highlighted
 * item — unless the popup gate D_1A8C64 is set — it draws the equipped weapon's
 * ammo counter "%d/%d" (red 0x804040FF when out of ammo, white gradient
 * otherwise) and the localized weapon name, split onto two lines at the first '-'
 * or ' '. Returns w+0x58 (the widget status word).
 *
 * WALL: extensive float geometry math; FP scheduling not reproducible from C.
 * Matching arm stays INCLUDE_ASM; the #else below is the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", DrawWeaponSelectWheel);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void *g_guiInstance;
/* (end of this body's declarations) */
extern f32   WrapAnglePiSum(f32 a, f32 b);   /* 0x284548 wrap a+b into [-pi,pi] */
extern void  func_0034DAB0(void *elem, s32 flag, f32 t);       /* HUD highlight tween */
extern s32   func_00290FC0(void);                              /* GUI state gate */
extern void  func_002801B8(s32 x, s32 y, u32 color, const char *text, s32 flag);
extern char  g_szAmmoFraction[];             /* "%d/%d" ammo-count format string */
extern s32   D_1A8C64;                        /* GUI popup-busy gate */

s32 DrawWeaponSelectWheel(HudElement *w) {
    char buf[0x50];
    s32 posX, posY;
    f32 fillA, fillB, combined;
    s32 alpha, selectedIndex, n, i, off;
    void **table;

    /* open/close animation fractions, each clamped to [0,1] */
    fillA = IntToFloat(*((u8 *)w + 0x70)) / IntToFloat(8);
    if (1.0f < fillA) {
        fillA = 1.0f;
    } else if (fillA < 0.0f) {
        fillA = 0.0f;
    }

    fillB = IntToFloat(*((u8 *)w + 0x71)) / IntToFloat(8);
    if (1.0f < fillB) {
        fillB = 1.0f;
    } else if (fillB < 0.0f) {
        fillB = 0.0f;
    }

    alpha = FloatToInt(fillA * 128.0f);

    /* copy the widget's anchor and let func_0028C180 align it in place */
    posX = *(s32 *)((u8 *)w + 0x50);
    posY = *(s32 *)((u8 *)w + 0x54);
    func_0028C180(w, &posX, &posY);

    n = *(s32 *)((u8 *)&g_hudMobySpawnStart + 0x28);
    if (n > 0) {
        table = *(void ***)((u8 *)&g_hudMobySpawnStart + 0x2C);
        off = 0;
        i = 0;
        do {
            u8 *rec;

            /* per-item angle; the original discards the result (extern call kept) */
            (void)WrapAnglePiSum(2.0f * (f32)i * 3.14159274f / (f32)n - 3.14159274f,
                                 1.57079637f);

            if (*(s32 *)((u8 *)w + 0x74) == i) {                 /* highlighted item */
                rec = (u8 *)table[D_1A8D48] + off;
                if (*(s32 *)rec != 0 && g_guiInstance != 0) {
                    f32 fi = (f32)(i + 7);
                    if (fi > 7.0f) {
                        fi -= 8.0f;
                    }
                    func_0034DAB0((u8 *)g_guiInstance + 0x376C8, 1, fi);
                }
            }

            rec = (u8 *)table[D_1A8D48] + off;
            if (*(s32 *)rec != 0 && g_guiInstance != 0) {
                s32 v = (func_00290FC0() != 0) ? FloatToInt((f32)alpha * 0.5f) : 0;
                *(s32 *)((u8 *)g_guiInstance + 0x38000 + 0x79DC) = v;
            }

            n = *(s32 *)((u8 *)&g_hudMobySpawnStart + 0x28);
            i++;
            off += 0x1C;
        } while (i < n);
    }

    selectedIndex = *(s32 *)((u8 *)w + 0x74);
    if (selectedIndex < 0) {
        return *(s32 *)((u8 *)w + 0x58);
    }

    table = *(void ***)((u8 *)&g_hudMobySpawnStart + 0x2C);
    if (*(s32 *)((u8 *)table[D_1A8D48] + selectedIndex * 0x1C) == 0) {
        return *(s32 *)((u8 *)w + 0x58);
    }
    if (D_1A8C64 != 0) {
        return *(s32 *)((u8 *)w + 0x58);
    }

    combined = fillA * fillB;

    /* ammo counter for the equipped weapon in the highlighted slot */
    {
        s32 itemId = g_equippedItemSlots[selectedIndex];
        WeaponDef *wd = &g_weaponTable[g_itemEquippedSlot[itemId]];

        if (wd->exists != 0 && wd->sellsAmmoFlag != 0) {
            s32 curAmmo = g_weaponAmmo[itemId];
            u16 capacity = wd->ammoCapacity;
            s32 color;

            func_00115DA8(buf, g_szAmmoFraction, curAmmo, capacity);
            if (curAmmo == 0) {
                color = ColorLerpPacked(0x004040FF, 0x804040FF, combined);   /* out of ammo: red */
            } else {
                color = ColorLerpPacked(0x00F0F0F0, 0x80F0F0F0, combined);   /* white gradient */
            }
            func_002801B8(posX + 0x69, posY + 0x70, color, buf, -1);
        }
    }

    /* localized weapon name, from the wheel record's item id at +0x18 */
    {
        s32 textColor = ColorLerpPacked(0x00F0F0F0, 0x80F0F0F0, combined);
        u8 *rec = (u8 *)table[D_1A8D48] + selectedIndex * 0x1C;
        s32 itemId2 = *(s32 *)(rec + 0x18);
        s16 nameId = *(s16 *)((u8 *)&g_weaponTable[g_itemEquippedSlot[itemId2]] + 0x48);
        char *name = GetLocalizedString(nameId);
        s32 splitLen = 0;
        char *tail = name;
        s32 len, j, lineY;

        if (name == 0) {
            return *(s32 *)((u8 *)w + 0x58);
        }
        if (func_001157AC(name) == 0) {
            return *(s32 *)((u8 *)w + 0x58);
        }
        len = func_001157AC(name);

        /* first pass: split at the first '-' (checked from index 1) */
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

        /* if no '-', second pass: split at the first ' ' */
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

        /* baseline Y depends on whether the equipped weapon sells ammo */
        {
            s32 itemId3 = g_equippedItemSlots[selectedIndex];
            if (g_weaponTable[g_itemEquippedSlot[itemId3]].sellsAmmoFlag != 0) {
                lineY = posY + 0x4B;
            } else {
                lineY = posY + 0x64;
                if (splitLen != 0) {
                    lineY = posY + 0x55;
                }
            }
        }

        if (splitLen != 0) {
            func_002801B8(posX + 0x69, lineY, textColor, name, splitLen);
            func_002801B8(posX + 0x69, lineY + 0x10, textColor, tail, -1);
        } else {
            func_002801B8(posX + 0x69, lineY, textColor, name, -1);
        }
    }

    return *(s32 *)((u8 *)w + 0x58);
}
#endif

/**
 * Seed a HUD list widget `w`: register the list descriptor (8 entries,
 * callback &D_1A8DD8) in g_hudMobySpawnStart+0x28/+0x2C, set the widget
 * geometry (+0x58 = 0xD2, +0x5C = 0xC8), type tag (+0x74 = -2) and mode
 * (+0x78 = 0x1E), clear its two 16-bit cursors (+0x48/+0x4A) and reset the
 * wheel cursor D_1A8D48.
 *
 *   w  the widget record being initialised
 *
 * Matching notes. Every store is volatile so the stores issue in source order,
 * which is the ROM's; the constants are EE_REG-bound locals assigned in the
 * ROM's order (two of them reuse $2/$3 after the descriptor stores), and the
 * empty volatile asm stops the scheduler sinking the last three constant loads
 * in between the widget stores.
 */
void func_0028D6D8(HudElement *w) {
    volatile u8 *b = (volatile u8 *)w;
    register s32 count EE_REG("$2");
    register void *callback EE_REG("$3");
    register s32 width EE_REG("$5");
    register s32 tag EE_REG("$6");
    register s32 height EE_REG("$3");
    register s32 mode EE_REG("$2");

    count = 8;
    callback = &D_1A8DD8;
    *(volatile s32 *)((u8 *)&g_hudMobySpawnStart + 0x28) = count;
    width = 0xD2;
    *(void *volatile *)((u8 *)&g_hudMobySpawnStart + 0x2C) = callback;
    tag = -2;
    height = 0xC8;
    mode = 0x1E;
    __asm__ __volatile__("");
    *(volatile s32 *)(b + 0x58) = width;
    *(volatile s32 *)(b + 0x78) = mode;
    *(volatile s32 *)(b + 0x5C) = height;
    *(volatile s32 *)(b + 0x74) = tag;
    *(volatile s16 *)(b + 0x48) = 0;
    *(volatile s16 *)(b + 0x4A) = 0;
    D_1A8D48 = 0;
}

/* func_0028D720(w): per-frame input + selection update for the weapon/quick-select
 * wheel (w = the HUD widget). Reads controller port-0 (D_138180): normalises the
 * right-stick and, when deflected past 0.9, converts its angle to a wheel slot
 * ((((angle+2pi)+pi/2)+pi/n) / 2pi * n, mod n); when the stick is centred it does
 * face-button graph navigation instead — triangle/circle/cross/square each step
 * to the neighbour link (offsets +0x10/+0x0C/+0x14/+0x08) of the current slot's
 * 0x1C-stride record, or seed a default (D_1A8DC0/DBC/DC4/DB8) from -1. On a
 * selection change it resets the colour pulse (func_002AA3F0) and plays the nav
 * sound (PlayGlobalSound 3). The D-pad-Up edge confirms the selection into
 * g_soundBankHandlesBlk+0x24E4 (via the slot's +0x18 link); an Up-held/‑release
 * counter (w+0x70) gates the final publish of the selection to the HUD moby
 * (g_hudMobyAuxBlockBase+0x14/0x18/0x1C + func_0028C010). The committed category
 * is invalidated (→0) if its icon slot isn't loaded (g_hudClutSlots flags).
 *
 * WALL (matching build): callee-saves + the FP angle math schedule + branch
 * colouring cc1 does not reproduce. Matching arm stays INCLUDE_ASM; #else is the
 * portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028D720);
#else
extern u8   D_138180[];                /* controller port-0 state (0x138180) */
extern u8   g_soundBankHandlesBlk[];   /* g_soundBankHandles+0x20 (0x189E20) — declared again below for func_0028EB10 */
extern s32  g_renderLayerMask;         /* +0x4 = current render layer (0x1B1878-... adj global) */
extern f32  Atan2fPoly(f32 a, f32 b);            /* 0x283BF8 Atan2fPoly(x,y) */
extern void func_00288888(HudElement *w);           /* 0x288888 (w live at call) */
extern void func_00288840(void);                    /* 0x288840 */
extern s32 func_002AA3F0(s32 a, s32 b, s32 c, s32 d, s32 e); /* 0x2AA3F0 colour-pulse reset; returns the pulsed colour */
extern s32  D_1A8DC0;                  /* default wheel target: triangle (0x1A8DC0) */
extern s32  D_1A8DC4;                  /* default wheel target: cross    (0x1A8DC4) */
extern s32  D_1A8DB8;                  /* default wheel target: square   (0x1A8DB8) */
extern s32  D_1A8DBC;                  /* default wheel target: circle   (0x1A8DBC) */
extern u8   D_1A7BBB;                  /* gate byte -> func_00288840 (0x1A7BBB) */

/* neighbour/target link at record[slot]+off within the wheel record block selected
 * by the cursor D_1A8D48 (block-base array at g_hudMobySpawnStart+0x2C; 0x1C stride). */
static s32 HudWheelLink(s32 slot, s32 off) {
    void **blockPtrs = *(void ***)((u8 *)&g_hudMobySpawnStart + 0x2C);
    u8    *block     = (u8 *)blockPtrs[D_1A8D48];
    return *(s32 *)(block + slot * 0x1C + off);
}

void func_0028D720(HudElement *w) {
    u8 *pw  = (u8 *)w;
    u8 *pad = D_138180;                          /* controller port-0 state */
    f32 stickX, stickY, mag, angle;
    s32 v78, oldSel, newSel;

    func_00288888(w);

    *(s32 *)(pw + 0x7C) = 0xB4;
    *(s32 *)(pw + 0x6C) = 0x18;
    *(s32 *)(pad + 0x1CC) = 2;

    stickX = *(f32 *)(pad + 0x148);
    stickY = *(f32 *)(pad + 0x14C);
    mag = func_002835C0(stickX * stickX + stickY * stickY);
    if (mag != 0.0f) {
        stickX = stickX / mag;
        stickY = stickY / mag;
    }
    angle = Atan2fPoly(stickX, stickY);

    /* +0x78: top-byte state gate — arm/refresh the "selection open" latch */
    v78 = *(s32 *)(pw + 0x78);
    if ((v78 >> 24) == 0 && *(s16 *)((u8 *)&g_hudClutSlots + 0x4) != 0) {
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
                s32 n = *(s32 *)((u8 *)&g_hudMobySpawnStart + 0x28);
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
                    sel = (oldSel == -1) ? D_1A8DC0 : HudWheelLink(oldSel, 0x10);
                } else {
                    sel = oldSel;
                }
                if ((*(s32 *)(pad + 0x1C4) & 0x4000) != 0) {
                    sel = (sel == -1) ? D_1A8DC4 : HudWheelLink(sel, 0x14);
                }
                if ((*(s32 *)(pad + 0x1C4) & 0x8000) != 0) {
                    sel = (sel == -1) ? D_1A8DB8 : HudWheelLink(sel, 0x08);
                }
                if ((*(s32 *)(pad + 0x1C4) & 0x2000) != 0) {
                    sel = (sel == -1) ? D_1A8DBC : HudWheelLink(sel, 0x0C);
                }
                *(s32 *)(pw + 0x74) = sel;
            }
        }
    }

    newSel = *(s32 *)(pw + 0x74);
    if (newSel != oldSel) {
        func_002AA3F0(0, 0, 1, 0, 1);
        *(u8 *)(pw + 0x71) = 0;
        PlayGlobalSound(3, 0, 0);
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
            *(s32 *)(g_soundBankHandlesBlk + 0x24E4) =
                (sel == -1) ? 0 : HudWheelLink(sel, 0x18);
        }
        if (D_1A7BBB != 0) {
            func_00288840();
        }
        {
            u8 up = *(u8 *)(pw + 0x70);
            if (up != 0) {
                *(u8 *)(pw + 0x70) = up - 1;
            }
        }
    }

    /* invalidate the committed category if its icon slot isn't loaded */
    {
        s32 *cat = (s32 *)(g_soundBankHandlesBlk + 0x24E4);
        if (*cat == 6 && *(s16 *)((u8 *)&g_hudClutSlots + 0x8) == 0) *cat = 0;
        if (*cat == 7 && *(s16 *)((u8 *)&g_hudClutSlots + 0xC) == 0) *cat = 0;
        if (*cat == 5 && *(s16 *)((u8 *)&g_hudClutSlots + 0xA) == 0) *cat = 0;
    }

    /* once the up-counter has drained, publish the selection to the HUD moby */
    if (*(u8 *)(pw + 0x70) == 0) {
        *(s32 *)((u8 *)&g_hudMobyAuxBlockBase + 0x18) = *(s32 *)(pw + 0x78);
        *(s32 *)((u8 *)&g_hudMobyAuxBlockBase + 0x1C) = *(s32 *)(pw + 0x74);
        func_0028C010(*(s32 *)(pw + 0x64));
        *(s32 *)(pw + 0x6C) = -6;
        *(s32 *)((u8 *)&g_hudMobyAuxBlockBase + 0x14) =
            *(s32 *)((u8 *)&g_renderLayerMask + 0x4);
    }
}
#endif

/* func_0028DC28(hud): draw the weapon-select wheel / item ring (~0xA18 bytes).
 *
 * (1) computes two open/close fill fractions from hud+0x70/+0x71 (each /8, clamped
 * to [0,1]); (2) unconditionally draws the 12 chrome/frame glyphs of the wheel
 * (codepoints 0x08-0x0F, 0x07 x2, 0x86 x2); (3) if hud+0x74 (the active slot) is
 * >= 0, pulses the selection colour (func_002AA3F0) and draws the selection cursor
 * box + a count glyph; (4) walks the item ring (g_hudMobySpawnStart+0x28/+0x2C,
 * stride 0x1C, base table[D_1A8D48]): for each populated slot it computes a ring
 * position, then a per-category colour via jtbl_0026C7F0_text (gated on the
 * g_hudClutSlots availability flags), and draws the slot background (when lit) +
 * the item icon (func_0028EDF0 -> func_0028F2C0); (5) if the active slot was drawn,
 * draws its localized name (main + shadow, colour cross-faded by fillB*fillA).
 * Returns the widget status word at hud+0x58.
 *
 * WALL: extensive float math + jtbl dispatch; neither reproducible from C.
 * Matching arm stays INCLUDE_ASM; the #else below is the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028DC28);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern s16 g_swapGadgetItemIndex;
extern void *g_guiInstance;
extern void func_003017F8(s32 handle, s32 color0, f32 px, f32 py, f32 sx, f32 syg, f32 v38);
extern s32 GuiFontAtlasLookupGlyph(void *atlas, s32 codepoint);
extern s32 func_002AA3F0(s32 a, s32 b, s32 c, s32 d, s32 e);
extern f32 WrapAnglePiSum(f32 a, f32 b);
extern char *GetLocalizedString(s32 textId);
extern s32 ColorLerpPacked(s32 colorA, s32 colorB, f32 t);
extern void func_002801B8(s32 x, s32 y, u32 color, const char *text, s32 flag);
/* (end of this body's declarations) */
/* later-defined / asm-only helpers, forward-declared for this arm */
extern s32  func_0028EDF0(s32 name, s32 level);
extern void func_002904B0(s32 x0, s32 y0, s32 x1, s32 y1, u64 reg4, s32 mode);
extern void func_0028F2C0(s32 icon, s32 x, s32 y, s32 w, s32 h, void *colorArr); /* icon block draw */
extern f32  func_00283B30(f32 angle);   /* 0x283B30 ring X term */
extern f32  func_00283B48(f32 angle);   /* 0x283B48 ring Y term */
typedef struct WheelCursorRect { s32 x, y; } WheelCursorRect;
extern WheelCursorRect D_1A8E30[8];     /* 0x1A8E30 selection-cursor box positions */
extern s32  D_1A8E70[8];                /* 0x1A8E70 per-slot label text ids */

/* g_hudClutSlots is read here as absolute s16 availability flags at +0x4../+0xC */
#define CLUT16(off) (*(s16 *)((u8 *)&g_hudClutSlots + (off)))

s32 func_0028DC28(HudElement *hud) {
    static const struct { s32 cp; u32 color; f32 sx; } topGlyphs[12] = {
        {0x08, 0x60442D00,  1.0f}, {0x09, 0x60442D00,  1.0f},
        {0x0A, 0x60442D00,  1.0f}, {0x0B, 0x60442D00,  1.0f},
        {0x0C, 0x60442D00,  1.0f}, {0x0D, 0x60442D00,  1.0f},
        {0x0E, 0x60442D00,  1.0f}, {0x0F, 0x60442D00,  1.0f},
        {0x07, 0x60442D00,  1.0f}, {0x07, 0x60442D00, -1.0f},
        {0x86, 0x55F0C070,  1.0f}, {0x86, 0x55F0C070, -1.0f},
    };
    f32   yfudge  = *(f32 *)((u8 *)&g_swapGadgetItemIndex + 0x8E);
    s32   clutSum = CLUT16(0x8) + CLUT16(0xC) + CLUT16(0xA);
    s32   active  = *(s32 *)((u8 *)hud + 0x74);
    void *atlas   = (u8 *)g_guiInstance + 0x8710;
    f32   fillA, fillB;
    s32   selectedDrawn = 0;
    s32   n, i, k;

    /* open/close fractions, each clamped to [0,1] */
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

    /* 12 chrome/frame glyphs (unconditional) */
    for (k = 0; k < 12; k++) {
        func_003017F8(GuiFontAtlasLookupGlyph(atlas, topGlyphs[k].cp),
                      (s32)topGlyphs[k].color,
                      129.0f, 208.0f, topGlyphs[k].sx, yfudge, 0.0f);
    }

    /* selection cursor + count glyph */
    if (active >= 0) {
        s32 pulse = func_002AA3F0(0x80442D00, 0x80FFDE8D, 0x19, 0, 0);
        s32 count = (active - 1 > -1) ? (active - 1) : 7;
        s32 glyph = GuiFontAtlasLookupGlyph(atlas, 0x10);
        func_003017F8(glyph, pulse,
                      129.0f, 208.0f, 1.0f, yfudge, (f32)count);
        func_002904B0(D_1A8E30[active].x, D_1A8E30[active].y,
                      D_1A8E30[active].x + 0x21, D_1A8E30[active].y + 7,
                      (u64)(u32)pulse, 0);
    }

    /* item ring */
    n = *(s32 *)((u8 *)&g_hudMobySpawnStart + 0x28);
    for (i = 0; i < n; i++) {
        void **table = *(void ***)((u8 *)&g_hudMobySpawnStart + 0x2C);
        u8   *rec    = (u8 *)table[D_1A8D48] + i * 0x1C;
        f32   angle  = WrapAnglePiSum(2.0f * (f32)i * 3.14159274f / (f32)n - 3.14159274f,
                                      1.57079637f);
        s32   ringX  = (s32)(func_00283B30(angle) * 80.0f) + 0x72;
        s32   ringY  = (s32)(func_00283B48(angle) * 75.0f) + 0xC0;

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
                iconA = func_0028EDF0(*(s32 *)rec, 0xA);
                colorBg[0] = colorBg[1] = colorBg[2] = colorBg[3] = color;
                func_0028F2C0(iconA, ringX + 2, ringY + 2, 0x1C, 0x1A, colorBg);
            }

            color2 = drawIcon ? (s32)0x80FFDE8D : 0x40808080;
            colorGlyph[0] = colorGlyph[1] = colorGlyph[2] = colorGlyph[3] = (u32)color2;
            iconB = func_0028EDF0(*(s32 *)rec, kind);
            func_0028F2C0(iconB, ringX, ringY, 0x1E, 0x1C, colorGlyph);
        }
        n = *(s32 *)((u8 *)&g_hudMobySpawnStart + 0x28);   /* reloaded each iteration */
    }

    /* active slot's localized label (main + shadow) */
    active = *(s32 *)((u8 *)hud + 0x74);
    if (selectedDrawn && active >= 0 && active < 8) {
        s32   textId = D_1A8E70[active];
        char *str;

        if (textId == 0x2DCA && CLUT16(0x6) != 0) {
            textId = 0x2DCB;
        }
        str = GetLocalizedString(textId);
        if (str != 0 && func_001157AC(str) != 0) {
            f32 t = fillB * fillA;
            s32 colorMain   = ColorLerpPacked(0x00E0C0A0, (s32)0x80E0C0A0, t);
            s32 colorShadow = ColorLerpPacked(0x00000000, (s32)0x80000000, t);
            func_002801B8(0x80, 0xC8, (u32)colorShadow, str, -1);
            func_002801B8(0x81, 0xC9, (u32)colorMain, str, -1);
        }
    }

    return *(s32 *)((u8 *)hud + 0x58);
}
#undef CLUT16
#endif

/* func_0028E640(...): HUD widget helper (~0x15C bytes).
 *
 * WALL: callee-saves + jal gates; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028E640);

/* Seed a HUD widget record's geometry (+0x5C/+0x58 = 0x20, type tag +0x7C =
 * 0x96) then re-init it via func_0028C390 (empty-asm guard keeps the jal). */
void func_0028E7A0(HudElement *p) {
    u8 *b = (u8 *)p;
    *(s32 *)(b + 0x58) = 0x20;
    *(s32 *)(b + 0x7C) = 0x96;
    *(s32 *)(b + 0x5C) = 0x20;
    func_0028C390(p);
    __asm__ __volatile__("");
}

/* Stub returning 0. */
s32 func_0028E7D0(void) {
    return 0;
}

/* func_0028E7D8: mis-split handwritten stub-table fragment — `addiu $sp,+0x70;
 * nop; addiu $sp,+0x80` epilogue tails with NO jr $ra. Not a real returning
 * function; left INCLUDE_ASM. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028E7D8);

extern s32 GetRandomInt(s32 max);   /* uniform [0, max) */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028E7E8);
#else
/**
 * Pick the current animation frame for a HUD icon widget and cache it.
 *
 * `slot` is the widget: +0x0 (s16) icon id, +0x2 (u8) animation mode, +0x4 (s32)
 * cached current frame (also this function's output), +0xC (s32) animation start
 * time. The icon record comes from the table at g_pHudAssetHeader[1] indexed by
 * the icon id; from it we use maxLevel (+0x2) as the frame count, baseFrame
 * (+0x4) as the first map index, and the byte at +0x7 as the per-frame duration.
 *
 * Modes: 0 = hold baseFrame; 1 = loop (baseFrame + elapsed/dur mod count);
 * 2 = ping-pong (reflect through 2*count-2); 3 = one-shot ping-pong that, once
 * finished, holds baseFrame and reschedules the start time 2*count + rand(30) +
 * 10 ticks ahead. An icon with texId 0xFFFF, or any other mode, yields frame 0.
 * The chosen frame is written back to slot+0x4 and returned.
 */
s32 func_0028E7E8(void *slot) {
    u8 *w = (u8 *)slot;
    HudIconSlot *icon = &((HudIconSlot *)g_pHudAssetHeader[1])[*(s16 *)(w + 0x0)];
    s32 dur;
    s32 count;
    s32 phase;
    s32 frame = 0;

    if (icon->texId == 0xFFFF) {
        *(s32 *)(w + 0x4) = 0;
        return 0;
    }

    switch (*(u8 *)(w + 0x2)) {
    case 0: /* static: hold the base frame */
        frame = icon->baseFrame;
        break;
    case 1: /* loop on a fixed cadence */
        dur = ((u8 *)icon)[0x7];
        frame = icon->baseFrame + (g_gameTime - *(s32 *)(w + 0xC)) / dur % icon->maxLevel;
        break;
    case 2: /* ping-pong: run forward then back */
        dur = ((u8 *)icon)[0x7];
        count = icon->maxLevel;
        phase = (g_gameTime - *(s32 *)(w + 0xC)) / dur % (2 * count - 2);
        frame = icon->baseFrame + (phase < count ? phase : 2 * count - (phase + 2));
        break;
    case 3: /* one-shot ping-pong, then hold + schedule a random restart */
        if (*(s32 *)(w + 0xC) < g_gameTime) {
            dur = ((u8 *)icon)[0x7];
            count = icon->maxLevel;
            phase = (g_gameTime - *(s32 *)(w + 0xC)) / dur;
            if (phase < 2 * count - 2) {
                frame = icon->baseFrame + (phase < count ? phase : 2 * count - (phase + 2));
            } else {
                frame = icon->baseFrame;
                *(s32 *)(w + 0xC) = 2 * count + GetRandomInt(0x1E) + 0xA;
            }
        } else {
            frame = *(s32 *)(w + 0x4);
        }
        break;
    }

    *(s32 *)(w + 0x4) = frame;
    return frame;
}
#endif

/* func_0028E9A0(runTickCallbacks): per-frame update pass over the 13-entry HUD
 * element registry D_2552B0 (stride 0x90). After a one-shot (re)bind pass
 * (func_0028EB10), for each record it:
 *   - clamps the +0x7C per-frame countdown up to 0xA when the record's +0x04
 *     live-flags has bit 0x10 set OR the global text gate (g_pActiveTextTable+0x34)
 *     is active;
 *   - if that countdown is >= 2, decrements it and (while the +0x6C phase < 0x1E)
 *     bumps the phase, counting the record as "active" in the return value;
 *   - otherwise (countdown <= 1, still decremented toward 0) counts the +0x6C phase
 *     DOWN, floored at -6;
 *   - when the record is dirty (+0x68 != 0) and its phase has reached the -6 floor,
 *     re-activates it via func_0028BF18;
 *   - when runTickCallbacks is set and the record has a +0x14 tick callback, invokes
 *     it with the record.
 * Returns the count of records whose countdown was still running (>= 2).
 *
 * NOTE: +0x14/+0x6C/+0x7C are unnamed in Rec2552B0 and reached by raw offset here.
 * This function mutates +0x7C as a per-frame countdown, which conflicts with the
 * "+0x7C = element type tag" note in the HudElement field map above — flagged for a
 * Track-B revisit; the #else body faithfully mirrors the asm regardless.
 *
 * Matching build stays INCLUDE_ASM (callee-saves + gp/absolute-mix + a jalr callback
 * and peeled likely-branch scan cc1 won't reproduce); #else is the portable body. */
extern void func_0028EB10(void); /* text/188858, defined below (gp-gated rebind) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028E9A0);
#else
s32 func_0028E9A0(s32 runTickCallbacks) {
    Rec2552B0 *rec = &D_2552B0[0];
    Rec2552B0 *end = (Rec2552B0 *)((u8 *)&D_2552B0[0] + 0x750);
    s32 activeCount = 0;

    func_0028EB10();

    do {
        u8  *r = (u8 *)rec;
        s32 *countdown = (s32 *)(r + 0x7C); /* per-frame countdown */
        s32 *phase = (s32 *)(r + 0x6C);     /* animation phase, floored at -6 */
        s32  t;

        if ((rec->field04 & 0x10) ||
            (*(s32 *)((u8 *)&g_pActiveTextTable + 0x34) != 0)) {
            if (*countdown < 0xA) {
                *countdown = 0xA;
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

        if (rec->field68 != 0 && *phase == -6) {
            func_0028BF18((HudElement *)rec);
        }

        if (runTickCallbacks != 0) {
            void (*tickFn)(HudElement *) = *(void (**)(HudElement *))(r + 0x14);
            if (tickFn != 0) {
                tickFn((HudElement *)rec);
            }
        }

        rec++;
    } while (rec < end);

    return activeCount;
}
#endif

/* func_0028EAC8(): return the active weapon's item id when that weapon sells
 * ammo (g_weaponTable[slot].sellsAmmoFlag != 0) and the "no ammo sale" gate is
 * clear, else 0.
 *   returns the item id held at g_soundBankHandlesBlk+0x1248, or 0
 *
 * Byte-exact on sdk29 (task #1025), plain C, no device. The old WALL note here
 * said cc1 folds the block's +0x20 into the %lo relocation and lowers the test
 * as a branch. Measured: reading the block through a local pointer keeps the
 * ROM's lui/addiu base with +0x1248/+0x22B4 displacements, and two separate
 * `result = 0` assignments become the ROM's movz/movn pair (one `||` test
 * compiles to branches). Spelling the base as `g_soundBankHandlesBlk + 0x1248`
 * directly folds it; laundering the pointer through an empty asm issues three
 * adjacent pairs in reverse order, the residual shape task #978 recorded.
 * sellsAmmoFlag must be u16: as s16 the load is `lh`, the ROM's is `lhu`.
 */
s32 func_0028EAC8(void) {
    u8 *blk = g_soundBankHandlesBlk;
    s32 itemId = *(s32 *)(blk + 0x1248);
    u8  noSale = blk[0x22B4];
    s32 result = itemId;
    u8  slot = g_itemEquippedSlot[itemId];

    if (g_weaponTable[slot].sellsAmmoFlag == 0) {
        result = 0;
    }
    if (noSale != 0) {
        result = 0;
    }
    return result;
}

/* func_0028EB10(): one-shot ammo-vendor HUD-widget (re)bind pass, run at the top
 * of func_0028E9A0's per-frame update. No-op while the hard-disable gate D_1A8FBC
 * or the "already armed" gate D_1A8FB0 is set. Otherwise it asks func_0028EAC8
 * for the active ammo-selling weapon's item id:
 *   - if a real ammo weapon is equipped (itemId != 0 && its g_weaponTable entry
 *     exists), it registers the ammo-vendor widget via func_0028BE10 (feeding the
 *     weapon's name-string id + ammo-capacity and the vendor tick/draw callbacks
 *     func_0028E7A0 / func_0028E7D0 + the D_002907C0 layout blob), remembering the
 *     returned handle in D_1A8FB8 and the item in D_1A8FB4;
 *   - otherwise (no ammo weapon): if the sound-bank state byte
 *     g_soundBankHandlesBlk[0x22B4] == 2 it first tears down any live widget
 *     (func_0028C108(handle, 0)) then spawns the generic bank widget (id 0xFFFF,
 *     priority 0xC8, callbacks func_0028C490 / func_0028C4C8 / func_0028E640+0x40),
 *     stashing that handle at g_soundBankHandlesBlk[0x1878]; if the byte != 2 it
 *     just tears the live widget down.
 * D_1A8FB8 == -1 means "no live widget", so the teardown is skipped in that case.
 *
 * WALL (matching build): callee-saves + the gp-rel/absolute global mix + the
 * peeled likely-branch teardown and multiple jal gates cc1 won't reproduce.
 * Matching arm stays INCLUDE_ASM; #else is the portable body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028EB10);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void func_0028C4C8(HudElement *w);
/* (end of this body's declarations) */
extern s32 D_1A8FBC;                  /* gp hard-disable gate (0x1A8FBC) */
extern u8  g_soundBankHandlesBlk[];   /* g_soundBankHandles+0x20 (0x189E20) */
extern u8  D_002907C0[];              /* ammo-vendor widget layout blob (0x2907C0) */
extern void func_0028E640(void);      /* generic-bank widget draw callback (INCLUDE_ASM) */
void func_0028EB10(void) {
    s32 itemId;
    u8  slot;
    WeaponDef *w;

    if (D_1A8FBC != 0) {
        return;                       /* hard-disabled */
    }
    if (D_1A8FB0 != 0) {
        return;                       /* countdown already armed */
    }

    itemId = func_0028EAC8();
    slot = g_itemEquippedSlot[itemId];
    w = &g_weaponTable[slot];
    if (itemId != 0 && w->exists != 0) {
        D_1A8FB4 = itemId;
        D_1A8FB8 = func_0028BE10(0x10, w->nameStringId,
                                 (s32)&func_0028E7A0, (s32)D_002907C0,
                                 (s32)&func_0028E7D0, (s32)&g_weaponAmmo[itemId],
                                 w->ammoCapacity);
        return;
    }

    if (g_soundBankHandlesBlk[0x22B4] == 2) {
        if (D_1A8FB8 != -1) {
            func_0028C108(D_1A8FB8, 0);
            D_1A8FB8 = -1;
        }
        *(s16 *)(g_soundBankHandlesBlk + 0x1878) =
            (s16)func_0028BE10(0x10, 0xFFFF,
                               (s32)&func_0028C490, (s32)&func_0028C4C8,
                               (s32)&func_0028E640 + 0x40,
                               (s32)(g_soundBankHandlesBlk + 0x1874), 0xC8);
    } else {
        if (D_1A8FB8 != -1) {
            func_0028C108(D_1A8FB8, 0);
            D_1A8FB8 = -1;
        }
    }
}
#endif

/* Re-seed the HUD widget table (func_0028BF80), then arm the countdown gate
 * (D_1A8FB0 = 1) and its companion latches (D_1A8FB8 = -1, D_1A8FB4 = 0). */
void func_0028EC70(void) {
    func_0028BF80();
    D_1A8FB0 = 1;
    D_1A8FB8 = -1;
    D_1A8FB4 = 0;
}

/* Decrement the countdown gate at D_1A8FB0, clamping at 0. */
void func_0028ECA8(void) {
    if (D_1A8FB0 != 0) {
        D_1A8FB0 = D_1A8FB0 - 1;
    }
}

/* func_0028ECC0(): per-frame HUD tick (0x28ECC0). Bails (clearing the HUD CLUT
 * slot) if that slot is already busy or the screen is mid white-fade. Otherwise
 * runs each of the 13 D_2552B0 widget records' +0x18 init callback (via jalr,
 * passing the record), then advances the nanotech-orb HUD counters:
 *   - orb+0xC = 0x64 when both orb counters are idle OR we're not in gameplay
 *     (g_nGameState != 0);
 *   - in gameplay (g_nGameState == 0) with a counter active: ramp orb+0x8 toward
 *     0x80 (when orb+0x4 is running) or back to 0, tick orb+0x4 down, and reset
 *     it from the 0x3E8 sentinel.
 *
 * WALL (matching build): callee-saves + the indirect-call (jalr) callback loop
 * over gp/absolute-mixed globals cc1 does not reproduce — stays INCLUDE_ASM.
 * This #else is faithful COVERAGE only. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028ECC0);
#else
extern u8 g_screenFadeWhite[];      /* +0x4 = white-fade level (0 = no fade) */
extern u8 g_pActiveNanotechOrb[];   /* nanotech-orb HUD counters at +0x4/+0x8/+0xC */

void func_0028ECC0(void)
{
    s32 *hudClut = (s32 *)((u8 *)&g_hudClutSlots + 0x10);
    s32 *orb4 = (s32 *)(g_pActiveNanotechOrb + 0x4);
    s32 *orb8 = (s32 *)(g_pActiveNanotechOrb + 0x8);
    s32 *orbC = (s32 *)(g_pActiveNanotechOrb + 0xC);
    s32 i;

    if (*hudClut != 0 || *(s32 *)(g_screenFadeWhite + 0x4) != 0) {
        *hudClut = 0;
        return;
    }

    *(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) = 0xFFFFF0;
    for (i = 0; i < 13; i++) {
        void (*cb)(Rec2552B0 *) = *(void (**)(Rec2552B0 *))((u8 *)&D_2552B0[i] + 0x18);
        if (cb != 0) {
            cb(&D_2552B0[i]);
        }
    }

    if ((*orb4 == 0 && *orb8 == 0) || g_nGameState != 0) {
        *orbC = 0x64;
    } else {
        if (*orb4 != 0) {
            s32 v = *orb8 + 0x10;
            *orb8 = (v < 0x81) ? v : 0x80;   /* ramp up, clamp to 0x80 */
        } else {
            s32 v = *orb8 - 0x10;
            *orb8 = (v >= 0) ? v : 0;        /* ramp down, clamp to 0 */
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

/* Resolve the live HUD icon-map index for icon `name` at upgrade `level`.
 * Looks the icon up in the slot table (func_0028B560), bounds-checks `level`
 * against the slot's frame count, then verifies both the CLUT and texture GS
 * slots are allocated. Returns the map index (baseFrame + level) on success,
 * else 0.
 *
 * NEAR-MISS (55%): logic correct, but the original threads the branch-likely
 * (bltzl) clut check and the `movz` result-select tail through a register
 * allocation cc1 doesn't reproduce, plus the two-callee-save 0x20-vs-0x10 frame.
 * Kept as the portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028EDF0);
#else
s32 func_0028EDF0(s32 name, s32 level) {
    HudIconSlot *table = (HudIconSlot *)g_pHudAssetHeader[1];
    HudIconSlot *slot = &table[func_0028B560(name)];
    if (slot->texId == 0xFFFF) {
        return 0;
    }
    if (level < (s32)slot->maxLevel) {
        s32 mapIndex = slot->baseFrame + level;
        HudIconMapEntry *e = &g_hudIconMap[mapIndex];
        if (g_hudClutSlots[e->clutSlot].handle < 0) {
            return 0;
        }
        if ((g_hudTextureSlots[e->textureSlot].handle & 0x80000000) == 0) {
            return mapIndex;
        }
    }
    return 0;
}
#endif

/* GetHudIconTex0(iconId): resolve one HUD icon's GS TEX0 register (~0x224
 * bytes). Looks the icon up in g_hudIconMap to get its CLUT and texture slots;
 * if either is not currently resident in VRAM (vramAddr == 0) it allocates a
 * VRAM block (bumping g_vramAllocCursor) and queues an upload into
 * g_texUploadQueue. Returns the assembled 64-bit GS TEX0 register describing
 * the texture (TBP0/TBW/PSM/TW/TH) and its CLUT (CBP), with the high bit set.
 *
 * Pure leaf (no calls, no frame). The matching build stays INCLUDE_ASM: the
 * CLUT/texture tables are reached through gp/absolute-mixed %hi/%lo addressing
 * the compiler will not reproduce from this C. */
typedef struct HudTexUploadEntry {            /* stride 0x10 */
    s32 clutSrc;    /* +0x0: CLUT source GS handle */
    u16 clutDst;    /* +0x4: CLUT dest VRAM block addr (0 for descriptor) */
    u16 clutWidth;  /* +0x6: CLUT width sentinel 0x3FF0, or dest addr on alloc */
    s32 texSrc;     /* +0x8: texture source GS handle */
    u8  texLogW;    /* +0xC */
    u8  texLogH;    /* +0xD */
    u16 texDst;     /* +0xE: texture dest VRAM block addr (0x3FF0 descriptor) */
} HudTexUploadEntry;
extern HudTexUploadEntry g_texUploadQueue[];  /* 0x1B92C0 */
extern s32 g_texUploadCount;                  /* 0x1B157C: entries pending */
extern u32 g_vramAllocCursor;                 /* 0x1A72D0: byte VRAM bump cursor */
extern s32 g_vramFrameBufB;                   /* 0x1A72DC: VRAM frame buffer B base */
/* DLI lever MEASURED (task #1220; unit objdiff report, objdiff_build.sh, this #else body promoted
 * SOLO, sdk29 arm, colima-ee-x86; every other row in the unit unchanged). cc1 emits
 * `dli $2,0x8000000000000000`; the ROM holds Ps2EeAs's expansion of that value at 0x28F0BC ($3),
 * but in a different register. No allowlist row applies to the body AS COMPILED: the checker
 * refuses a row carrying cc1's register (demonstrated, NOTE #8789). A #8598 pin moves the register
 * onto the ROM's (FACT #8791); pin+row is not sufficient: residual body shape - structure /
 * C-shape / addressing (NOTE #8789), built 125 words vs the ROM's 137. Solo score 47.82%. Residual
 * class: REGALLOC at the dli site (pin-reachable, above). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", GetHudIconTex0);
#else
u64 GetHudIconTex0(s32 iconId) {
    HudIconMapEntry *e = &g_hudIconMap[iconId];
    HudGsSlot *clut = &g_hudClutSlots[e->clutSlot];
    HudGsSlot *tex  = &g_hudTextureSlots[e->textureSlot];
    s32 queued = 0;

    /* Re-upload whenever either the CLUT or the texture is not resident. */
    if (clut->vramAddr == 0 || tex->vramAddr == 0) {
        /* Pre-fill the descriptor half of the pending entry. Both alloc paths
         * below target this same slot; g_texUploadCount advances only once. */
        HudTexUploadEntry *ent = &g_texUploadQueue[g_texUploadCount];
        ent->clutSrc   = clut->handle;
        ent->clutDst   = 0;
        ent->clutWidth = 0x3FF0;
        ent->texSrc    = clut->handle;
        ent->texDst    = 0x3FF0;
        ent->texLogH   = 5;
        ent->texLogW   = 5;

        if (clut->vramAddr == 0) {
            u32 cursor = g_vramAllocCursor;
            s32 count  = g_texUploadCount;
            clut->vramAddr = (u16)(cursor >> 8);
            g_vramAllocCursor = cursor + 0x400;           /* CLUT is 0x400 bytes */
            if (count < 0x40) {
                HudTexUploadEntry *ce = &g_texUploadQueue[count];
                ce->clutSrc   = clut->handle;
                ce->clutDst   = 0;
                ce->clutWidth = clut->vramAddr;
                queued = 1;
            }
        }
        if (tex->vramAddr == 0) {
            u32 cursor = g_vramAllocCursor;
            u32 maxLog = (tex->logH < tex->logW) ? tex->logW : tex->logH;
            s32 count  = g_texUploadCount;
            tex->vramAddr = (u16)(cursor >> 8);
            g_vramAllocCursor = cursor + (1u << (2 * maxLog));
            if (count < 0x40) {
                HudTexUploadEntry *te = &g_texUploadQueue[count];
                te->texSrc  = tex->handle;
                te->texLogW = tex->logW;
                te->texLogH = tex->logH;
                te->texDst  = tex->vramAddr;
                queued = 1;
            }
        }
        if (queued) {
            g_texUploadCount++;
        }
    }

    /* Assemble the GS TEX0_1 register. */
    {
        s32 logW      = tex->logW;
        s32 tbwShift  = (logW >= 6) ? (logW - 6) : 0;
        u64 tex0      = (u64)tex->vramAddr;                      /* TBP0  bits 0-13 */
        tex0 |= (u64)(1u << tbwShift) << 14;                     /* TBW   bits 14-19 */
        tex0 |= (u64)(((s32)tex->vramAddr < (g_vramFrameBufB >> 8)) ? 27 : 19) << 20; /* PSM bits 20-25 */
        tex0 |= (u64)tex->logW << 26;                           /* TW    bits 26-29 */
        tex0 |= (u64)tex->logH << 30;                           /* TH    bits 30-33 */
        tex0 |= ((u64)clut->vramAddr << 37) | ((u64)0x8000 << 19); /* CBP + CLUT bits */
        tex0 |= (u64)0x8000000000000000ULL;                     /* high control bit */
        return tex0;
    }
}
#endif

#ifdef TARGET_NATIVE
/* Externs referenced only by the faithful #else coverage bodies in the
 * GS/GIF sprite-packet cluster below (func_0028F0D0 .. func_0028FFF0). Guarded
 * under TARGET_NATIVE so the INCLUDE_ASM (byte-match) arm stays byte-neutral —
 * the -G8 %gp_rel sizing hacks are the matcher's concern, not the native arm's.
 * (g_pActiveTextTable, func_00283B30/B48 are already declared file-scope.) */
extern u32 *g_frameDmaCursor;       /* 0x1B2228 - per-frame render-DMA write ptr */
extern s32 g_gsPixelOffsetX;        /* 0x1A7350 - GS window X offset */
extern s32 g_gsPixelOffsetY;        /* 0x1A7354 - GS window Y offset */
extern u64 GetHudIconTex0(s32 iconIndex);                 /* resolve HUD icon TEX0 */
extern void Vec4AddVu0(f32 *dst, const f32 *a, const f32 *b); /* 0x283670 */
extern void Vec4SubVu0(f32 *dst, const f32 *a, const f32 *b); /* 0x2836A0 */
#endif

/* func_0028F0D0 = DrawHudIconQuadTiled: draw a HUD icon as a textured-sprite GIF
 * packet (0x60-byte, NLOOP=5) to the frame render-DMA chain. Sibling of
 * func_0028F540/func_0028F700: screen coords are 1/16-pixel subpixel (coord<<4,
 * like func_0028F540), while the UV rectangle spans the icon's full texture
 * (2^logW x 2^logH texels, like func_0028F700) rather than the passed size.
 *
 *   iconIndex  index into g_hudIconMap (and GetHudIconTex0)
 *   x, y       top-left screen position, tile units (multiplied by 16)
 *   w, h       size in the same tile units
 *   alpha      tint alpha (RGB fixed 0x7F7F7F)
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F0D0);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern u32 *g_frameDmaCursor;
extern s32 g_gsPixelOffsetX;
extern s32 g_gsPixelOffsetY;
/* (end of this body's declarations) */
void func_0028F0D0(s32 iconIndex, s32 x, s32 y, s32 w, s32 h, s32 alpha) {
    u8 *p = (u8 *)g_frameDmaCursor;
    s32 offX = g_gsPixelOffsetX;
    s32 offY = g_gsPixelOffsetY;
    u64 z = (u64)(u32)*(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) << 32;
    HudGsSlot *tex = &g_hudTextureSlots[g_hudIconMap[iconIndex].textureSlot];
    s32 uExtent = 1 << tex->logW; /* 2^texWidthLog2 texels */
    s32 vExtent = 1 << tex->logH; /* 2^texHeightLog2 texels */

    *(u32 *)(p + 0x0) = 0x10000005;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000005;
    g_frameDmaCursor = (u32 *)(p + 0x10);

    *(u64 *)(p + 0x10) = ((u64)0xE800 << 47) | 0x8001;
    *(u64 *)(p + 0x18) = 0x5353106;
    *(u64 *)(p + 0x20) = GetHudIconTex0(iconIndex);
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

    g_frameDmaCursor = (u32 *)((u8 *)g_frameDmaCursor + 0x50);
}
#endif

/* func_0028F2C0: draw a HUD icon as a Gouraud-shaded textured quad — a 0x70-byte
 * NLOOP=8 GIF packet (the GIFtag qword is the static blob D_1AC8B0) with a
 * distinct RGBA at each of the 4 corners taken from colorArr[0..3]. UV spans the
 * icon's full texture (2^logW x 2^logH); screen coords are 1/16-pixel subpixel
 * (coord<<4). The near/far UV split (u at +0x50, v at +0x68, combined at +0x80)
 * follows the register order encoded in the D_1AC8B0 GIFtag.
 *
 *   iconIndex  index into g_hudIconMap (and GetHudIconTex0)
 *   x, y       top-left screen position, tile units (multiplied by 16)
 *   w, h       size in the same tile units
 *   colorArr   pointer to 4 packed RGBA words, one per corner
 *              (top-left, top-right, bottom-left, bottom-right)
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F2C0);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern u32 *g_frameDmaCursor;
extern s32 g_gsPixelOffsetX;
extern s32 g_gsPixelOffsetY;
/* (end of this body's declarations) */
void func_0028F2C0(s32 iconIndex, s32 x, s32 y, s32 w, s32 h, void *colorArr) {
    u8 *p = (u8 *)g_frameDmaCursor;
    s32 *colors = (s32 *)colorArr;
    s32 offX = g_gsPixelOffsetX;
    s32 offY = g_gsPixelOffsetY;
    u64 z = (u64)(u32)*(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) << 32;
    HudGsSlot *tex = &g_hudTextureSlots[g_hudIconMap[iconIndex].textureSlot];
    s32 uExtent = 1 << tex->logW;
    s32 vExtent = 1 << tex->logH;

    *(u32 *)(p + 0x0) = 0x10000008;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000008;
    g_frameDmaCursor = (u32 *)(p + 0x10);

    /* GIFtag qword (copied 128-bit from D_1AC8B0 in the original) */
    *(u64 *)(p + 0x10) = 0xE400000000008001ULL;
    *(u64 *)(p + 0x18) = 0x0053153153153106ULL;
    g_frameDmaCursor = (u32 *)(p + 0x20);

    *(u64 *)(p + 0x20) = GetHudIconTex0(iconIndex);
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

    g_frameDmaCursor = (u32 *)((u8 *)g_frameDmaCursor + 0x70);
}
#endif

__asm__(".extern g_frameDmaCursor, 16");
extern u32 *g_frameDmaCursor;   /* 0x1B2228 - per-frame render-DMA write pointer */
__asm__(".extern g_gsPixelOffsetX, 16");
extern s32 g_gsPixelOffsetX;    /* 0x1A7350 - GS window X offset */
__asm__(".extern g_gsPixelOffsetY, 16");
extern s32 g_gsPixelOffsetY;    /* 0x1A7354 - GS window Y offset */
extern u64 GetHudIconTex0(s32 iconIndex);   /* resolve a HUD icon's GS tex0 register */

/* DLI lever MEASURED (task #1220; unit objdiff report, objdiff_build.sh, this #else body promoted
 * SOLO, sdk29 arm, colima-ee-x86; every other row in the unit unchanged). cc1 emits
 * `dli $10,0x7400000000008001`; the ROM holds Ps2EeAs's expansion of that value at 0x28F570 ($11),
 * but in a different register. No allowlist row applies to the body AS COMPILED: a row must carry
 * the ROM's words for cc1's register. A #8598 pin + row is UNTRIED. Solo score 24.56%. Residual
 * class: REGALLOC at the dli site, plus PACKED-SAVE (ROM saves at stride 8, cc1 2.9 at 16; NOTE
 * #8777). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F540);
#else
/**
 * Draw a 1:1 HUD icon: append a textured-sprite GIF packet (0x60-byte NLOOP=5)
 * to the frame render-DMA chain (g_frameDmaCursor) blitting the icon's texture
 * over screen rect (x,y)..(x+w,y+h). The tex0 register is resolved via
 * GetHudIconTex0(iconIndex); UV spans (0,0)..(w,h) texels (fixed-point <<4 for U,
 * <<20 for V); RGB tint 0x7F7F7F with the given alpha; Z from
 * &g_pActiveTextTable+0x3C. Screen coords are 1/16-pixel subpixel (coord<<4, -8
 * bias, off = g_gsPixelOffset). Advances the cursor by 0x60.
 */
void func_0028F540(s32 iconIndex, s32 x, s32 y, s32 w, s32 h, s32 alpha) {
    u8 *p = (u8 *)g_frameDmaCursor;
    s32 offX = g_gsPixelOffsetX;
    s32 offY = g_gsPixelOffsetY;
    u64 z = (u64)(u32)*(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) << 32;

    *(u32 *)(p + 0x0) = 0x10000005;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000005;
    g_frameDmaCursor = (u32 *)(p + 0x10);

    *(u64 *)(p + 0x10) = ((u64)0xE800 << 47) | 0x8001;
    *(u64 *)(p + 0x18) = 0x5353106;
    *(u64 *)(p + 0x20) = GetHudIconTex0(iconIndex);
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

    g_frameDmaCursor = (u32 *)((u8 *)g_frameDmaCursor + 0x50);
}
#endif

/* func_0028F6E8(...): HUD icon slot setup thunk (0x14 bytes) that falls into
 * DrawHudIconQuadPixel at 0x28F700.
 *
 * WALL: callee-saves + jal gates; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F6E8);

/* DLI lever MEASURED (task #1220; unit objdiff report, objdiff_build.sh, this #else body promoted
 * SOLO, sdk29 arm, colima-ee-x86; every other row in the unit unchanged). cc1 emits
 * `dli $10,0x7400000000008001`; the ROM holds Ps2EeAs's expansion of that value at 0x28F740 ($15),
 * but in a different register. No allowlist row applies to the body AS COMPILED: a row must carry
 * the ROM's words for cc1's register. A #8598 pin + row is UNTRIED. Solo score 16.50%. Residual
 * class: REGALLOC at the dli site, plus PACKED-SAVE (ROM saves at stride 8, cc1 2.9 at 16; NOTE
 * #8777). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F700);
#else
/**
 * func_0028F700 = DrawHudIconQuadPixel: draw a HUD icon as a textured-sprite GIF
 * packet (0x60-byte NLOOP=5) at whole-pixel screen coords. Sibling of
 * func_0028F540; the two differences are that screen coordinates are whole
 * pixels here (no <<4 subpixel), and the UV rectangle spans the icon's full
 * texture - 2^wLog2 x 2^hLog2 texels, where wLog2/hLog2 are the bytes at +0x6/
 * +0x7 of the icon's g_hudTextureSlots entry (indexed through
 * g_hudIconMap[iconIndex].textureSlot) - rather than the passed size.
 *
 *   iconIndex  index into g_hudIconMap (and GetHudIconTex0)
 *   x, y       top-left screen pixel
 *   w, h       screen size in pixels
 *   alpha      tint alpha (RGB fixed 0x7F7F7F)
 */
void func_0028F700(s32 iconIndex, s32 x, s32 y, s32 w, s32 h, s32 alpha) {
    u8 *p = (u8 *)g_frameDmaCursor;
    s32 offX = g_gsPixelOffsetX;
    s32 offY = g_gsPixelOffsetY;
    u64 z = (u64)(u32)*(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) << 32;
    HudGsSlot *tex = &g_hudTextureSlots[g_hudIconMap[iconIndex].textureSlot];
    s32 uExtent = 1 << ((u8 *)tex)[0x6]; /* 2^texWidthLog2 texels */
    s32 vExtent = 1 << ((u8 *)tex)[0x7]; /* 2^texHeightLog2 texels */

    *(u32 *)(p + 0x0) = 0x10000005;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000005;
    g_frameDmaCursor = (u32 *)(p + 0x10);

    *(u64 *)(p + 0x10) = ((u64)0xE800 << 47) | 0x8001;
    *(u64 *)(p + 0x18) = 0x5353106;
    *(u64 *)(p + 0x20) = GetHudIconTex0(iconIndex);
    *(u64 *)(p + 0x28) = 0x156;
    *(u64 *)(p + 0x30) = ((u64)(u32)alpha << 24) | 0x7F7F7F;
    *(u64 *)(p + 0x38) = 0; /* near UV (0,0) */
    *(u64 *)(p + 0x40) = (u64)(u32)(x + offX - 8)
                       | ((u64)(u32)(y + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x48) = (u64)(u32)((uExtent << 4) + (vExtent << 20)); /* far UV = full texture */
    *(u64 *)(p + 0x50) = (u64)(u32)((x + w) + offX - 8)
                       | ((u64)(u32)((y + h) + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x58) = 0;

    g_frameDmaCursor = (u32 *)((u8 *)g_frameDmaCursor + 0x50);
}
#endif

/* func_0028F8E0: draw a HUD icon as a textured-sprite GIF packet (0x60-byte,
 * NLOOP=5) with a caller-supplied RGB tint. Same shape as func_0028F700
 * (whole-pixel screen coords, UV spanning the icon's full 2^logW x 2^logH
 * texture) except the packed colour is (rgb & 0xFFFFFF) | (alpha << 24) built by
 * assembling the three rgb bytes plus alpha, rather than the fixed 0x7F7F7F.
 *
 *   iconIndex  index into g_hudIconMap (and GetHudIconTex0)
 *   x, y       top-left screen pixel
 *   w, h       screen size in pixels
 *   alpha      tint alpha
 *   rgb        packed 0x00BBGGRR tint colour (low 24 bits used)
 */
/* DLI lever MEASURED (task #1220; unit objdiff report, objdiff_build.sh, this #else body promoted
 * SOLO, sdk29 arm, colima-ee-x86; every other row in the unit unchanged). cc1 emits
 * `dli $11,0x7400000000008001`; the ROM holds Ps2EeAs's expansion of that value at 0x28F928 ($25),
 * but in a different register. No allowlist row applies to the body AS COMPILED: a row must carry
 * the ROM's words for cc1's register. A #8598 pin + row is UNTRIED. Solo score 19.80%. Residual
 * class: REGALLOC at the dli site, plus PACKED-SAVE (ROM saves at stride 8, cc1 2.9 at 16; NOTE
 * #8777). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F8E0);
#else
void func_0028F8E0(s32 iconIndex, s32 x, s32 y, s32 w, s32 h, s32 alpha, s32 rgb) {
    u8 *p = (u8 *)g_frameDmaCursor;
    s32 offX = g_gsPixelOffsetX;
    s32 offY = g_gsPixelOffsetY;
    u64 z = (u64)(u32)*(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) << 32;
    HudGsSlot *tex = &g_hudTextureSlots[g_hudIconMap[iconIndex].textureSlot];
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
    g_frameDmaCursor = (u32 *)(p + 0x10);

    *(u64 *)(p + 0x10) = ((u64)0xE800 << 47) | 0x8001;
    *(u64 *)(p + 0x18) = 0x5353106;
    *(u64 *)(p + 0x20) = GetHudIconTex0(iconIndex);
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

    g_frameDmaCursor = (u32 *)((u8 *)g_frameDmaCursor + 0x50);
}
#endif

/* DLI lever MEASURED (task #1220; unit objdiff report, objdiff_build.sh, this #else body promoted
 * SOLO, sdk29 arm, colima-ee-x86; every other row in the unit unchanged). cc1 emits
 * `dli $13,0x7400000000008001`; the ROM holds Ps2EeAs's expansion of that value at 0x28FB2C ($18),
 * but in a different register. No allowlist row applies to the body AS COMPILED: a row must carry
 * the ROM's words for cc1's register. A #8598 pin + row is UNTRIED. Solo score 12.25%. Residual
 * class: REGALLOC at the dli site, plus PACKED-SAVE (ROM saves at stride 8, cc1 2.9 at 16; NOTE
 * #8777). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028FAE0);
#else
/**
 * Append a textured-sprite GIF packet (a 0x60-byte, NLOOP=5 draw) to the frame
 * render-DMA chain (g_frameDmaCursor), then advance the cursor by 0x60. Draws a
 * quad from screen corner (x0,y0) to (x0+w, y0+h) mapped to UV (u0,v0) .. plus
 * the texture extents 2^(uShift+4) / 2^(vShift+4), tinted RGB 0x7F7F7F with the
 * given alpha. The packed Z comes from the render-depth field at
 * &g_pActiveTextTable + 0x3C.
 *
 * Layout at cursor p:
 *   p+0x00 : chain tags 0x10000005 / 0x50000005
 *   p+0x10 : GIFtag (NLOOP=5) + register data (0x5353106, reg1, 0x156)
 *   p+0x30 : colour (0x7F7F7F | alpha<<24)
 *   p+0x38 : near UV (u0 | v0<<16)
 *   p+0x40 : near XY (x0+off-8 | (y0+off-8)<<16 | z<<32)
 *   p+0x48 : far UV  (u0+2^(uShift+4) | (v0+2^(vShift+4))<<16)
 *   p+0x50 : far XY  (x0+w+off-8 | (y0+h+off-8)<<16 | z<<32)
 * off = g_gsPixelOffset{X,Y}.
 */
void func_0028FAE0(s32 reg1, s32 x0, s32 y0, s32 uShift, s32 vShift, s32 w, s32 h, s32 u0, s32 v0, s32 alpha) {
    u8 *p = (u8 *)g_frameDmaCursor;
    s32 offX = g_gsPixelOffsetX;
    s32 offY = g_gsPixelOffsetY;
    u64 z = (u64)(u32)*(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) << 32;

    /* DMA/GIF chain header (NLOOP=5) */
    *(u32 *)(p + 0x0) = 0x10000005;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000005;
    g_frameDmaCursor = (u32 *)(p + 0x10);

    /* GIFtag + fixed register data */
    *(u64 *)(p + 0x10) = ((u64)0xE800 << 47) | 0x8001;
    *(u64 *)(p + 0x18) = 0x5353106;
    *(u64 *)(p + 0x20) = (u64)(u32)reg1;
    *(u64 *)(p + 0x28) = 0x156;

    /* colour, near UV, near XY */
    *(u64 *)(p + 0x30) = ((u64)(u32)alpha << 24) | 0x7F7F7F;
    *(u64 *)(p + 0x38) = (u64)(u32)u0 | ((u64)(u32)v0 << 16);
    *(u64 *)(p + 0x40) = (u64)(u32)(x0 + offX - 8)
                       | ((u64)(u32)(y0 + offY - 8) << 16)
                       | z;

    /* far UV, far XY */
    *(u64 *)(p + 0x48) = (u64)(u32)(u0 + (1 << (uShift + 4)))
                       | ((u64)(u32)(v0 + (1 << (vShift + 4))) << 16);
    *(u64 *)(p + 0x50) = (u64)(u32)((x0 + w) + offX - 8)
                       | ((u64)(u32)((y0 + h) + offY - 8) << 16)
                       | z;
    *(u64 *)(p + 0x58) = 0;

    g_frameDmaCursor = (u32 *)((u8 *)g_frameDmaCursor + 0x50);
}
#endif

/* func_0028FC70: mis-split 1-instruction fragment — `addiu $sp,+0xE0` epilogue
 * tail, pinned as its own symbol; no jr $ra. Not a real function; left
 * INCLUDE_ASM. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028FC70);

/* func_0028FC78: draw a rotated (oriented) HUD texture quad — a 0x70-byte
 * NLOOP=7 GIF packet whose four corners are the centre offset by two rotated
 * half-extent basis vectors. The caller passes a GS TEX0/texbuffer handle
 * directly (no g_hudIconMap lookup), so this is the raw-texture cousin of the
 * icon draws above.
 *
 *   cx, cy      quad centre, screen pixels (float; truncated to int per corner)
 *   halfW       half-width  half-extent (scales the horizontal basis vector)
 *   halfH       half-height half-extent (scales the vertical basis vector)
 *   angle       rotation angle fed to the VU0 trig pair
 *   uExtent     U texel extent (near UV u = uExtent<<4)
 *   vExtent     V texel extent (far UV v = vExtent<<4)
 *   texReg      GS register value written at packet +0x20 (TEX0/handle)
 *
 * UNCONFIRMED: func_00283B48/func_00283B30 are the sin/cos VU0 pair (per the
 * atan2-to-cartesian pattern noted elsewhere); the halfW<->halfH assignment is
 * inferred from the angle=0 case (basis vectors collapse to the screen axes).
 * The colour is the fixed 0x807F7F7F (alpha 0x80, RGB 0x7F7F7F). */
/* DLI lever MEASURED (task #1220; unit objdiff report, objdiff_build.sh, this #else body promoted
 * SOLO, sdk29 arm, colima-ee-x86; every other row in the unit unchanged). cc1 emits
 * `dli $5,0xb400000000008001` and `dli $6,0x53535353106`; the ROM holds Ps2EeAs's expansion of
 * that value at 0x28FDBC ($9) and 0x28FDD4 ($5), but in a different register. No allowlist row
 * applies to the body AS COMPILED: a row must carry the ROM's words for cc1's register. A #8598
 * pin + row is UNTRIED. Solo score 3.70%. Residual class: REGALLOC at the dli site, plus
 * PACKED-SAVE (ROM saves at stride 8, cc1 2.9 at 16; NOTE #8777). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028FC78);
#else
void func_0028FC78(f32 cx, f32 cy, f32 halfW, f32 halfH, f32 angle,
                   s32 uExtent, s32 vExtent, s32 texReg) {
    u8 *p;
    s32 offX = g_gsPixelOffsetX;
    s32 offY = g_gsPixelOffsetY;
    u64 z = (u64)(u32)*(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) << 32;
    f32 vecV[4];   /* vertical (height) basis vector    */
    f32 vecH[4];   /* horizontal (width) basis vector   */
    f32 center[4];
    f32 c0[4], c1[4], c2[4], c3[4];

    /* Rotated half-extent basis vectors (sin = func_00283B48, cos = func_00283B30). */
    vecV[0] = halfH * func_00283B48(angle);
    vecV[1] = halfH * func_00283B30(angle);
    vecH[0] = -halfW * func_00283B30(angle);
    vecH[1] = halfW * func_00283B48(angle);
    center[0] = cx;
    center[1] = cy;

    /* Four corners: centre +/- vecV +/- vecH (VU0 vec4 add/sub). */
    Vec4AddVu0(c0, center, vecV); Vec4SubVu0(c0, c0, vecH);
    Vec4AddVu0(c1, center, vecV); Vec4AddVu0(c1, c1, vecH);
    Vec4SubVu0(c2, center, vecV); Vec4SubVu0(c2, c2, vecH);
    Vec4SubVu0(c3, center, vecV); Vec4AddVu0(c3, c3, vecH);

    p = (u8 *)g_frameDmaCursor;
    *(u32 *)(p + 0x0) = 0x10000007;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000007;
    g_frameDmaCursor = (u32 *)(p + 0x10);

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

    g_frameDmaCursor = (u32 *)((u8 *)g_frameDmaCursor + 0x70);
}
#endif

/* func_0028FFF0: draw a HUD icon as a textured-sprite GIF packet (0x60-byte,
 * NLOOP=5) with fully explicit corner coordinates AND texture coordinates — the
 * most general of the cluster. Unlike func_0028F540/func_0028F8E0 (which derive
 * the far UV from the texture size), the caller supplies both the near and far
 * UV directly. Whole-pixel screen coords; fixed RGB 0x7F7F7F with alpha.
 *
 *   iconIndex   index into g_hudIconMap (and GetHudIconTex0)
 *   x0, y0      near (top-left) screen pixel
 *   x1, y1      far (bottom-right) screen pixel
 *   u0, v0      near texel coordinate
 *   u1, v1      far texel coordinate
 *   alpha       tint alpha (RGB fixed 0x7F7F7F)
 */
/* DLI lever MEASURED (task #1220; unit objdiff report, objdiff_build.sh, this #else body promoted
 * SOLO, sdk29 arm, colima-ee-x86; every other row in the unit unchanged). cc1 emits
 * `dli $14,0x7400000000008001`; the ROM holds Ps2EeAs's expansion of that value at 0x290020 ($13),
 * but in a different register. No allowlist row applies to the body AS COMPILED: a row must carry
 * the ROM's words for cc1's register. A #8598 pin + row is UNTRIED. Solo score 23.65%. Residual
 * class: REGALLOC at the dli site, plus PACKED-SAVE (ROM saves at stride 8, cc1 2.9 at 16; NOTE
 * #8777). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028FFF0);
#else
void func_0028FFF0(s32 iconIndex, s32 x0, s32 y0, s32 x1, s32 y1,
                   s32 u0, s32 v0, s32 u1, s32 v1, s32 alpha) {
    u8 *p = (u8 *)g_frameDmaCursor;
    s32 offX = g_gsPixelOffsetX;
    s32 offY = g_gsPixelOffsetY;
    u64 z = (u64)(u32)*(s32 *)((u8 *)&g_pActiveTextTable + 0x3C) << 32;

    *(u32 *)(p + 0x0) = 0x10000005;
    *(u32 *)(p + 0x4) = 0;
    *(u32 *)(p + 0x8) = 0;
    *(u32 *)(p + 0xC) = 0x50000005;
    g_frameDmaCursor = (u32 *)(p + 0x10);

    *(u64 *)(p + 0x10) = ((u64)0xE800 << 47) | 0x8001;
    *(u64 *)(p + 0x18) = 0x5353106;
    *(u64 *)(p + 0x20) = GetHudIconTex0(iconIndex);
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

    g_frameDmaCursor = (u32 *)((u8 *)g_frameDmaCursor + 0x50);
}
#endif

/* UploadTextureToGs(src, a2, a3, logW, logH, kickNow) = UploadTextureToGs: build a GS
 * image-upload GIF packet for one texture (TRXPOS/TRXREG/TRXDIR) via
 * func_00126288 (BuildGsImageUploadPacket). The transfer dimensions come from the
 * log2 dims: width = 1<<logW, height = 1<<logH, GS-buffer-width v12 =
 * max((1<<logW)>>6, 1), and the qword count tag = 1<<(logW+logH-4). When kickNow==0
 * it splices a DMA tag chain into g_frameDmaCursor for deferred upload (DMAtag
 * 0x10000006 + GIFtag 0x50000006 header, then a 0x30000000|tag transfer + trailing
 * 0x50000000|tag), advancing the cursor by 0x70 then 0x10; otherwise it builds into
 * a local packet and issues it immediately (func_0011AEA0/FlushCache +
 * KickGifImageUpload). MATCH-WALL only (callee-save/register-colouring); un-walled
 * as faithful #else (engine 2.96 = no byte-match). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", UploadTextureToGs);
#else
extern void func_00126288(void *buf, s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_0011AEA0(s32 mode);   /* FlushCache */
extern void KickGifImageUpload(void *packet, s32 handle);
void UploadTextureToGs(s32 handle, s32 vramBlk, s32 fmt, s32 wLog, s32 hLog,
                   s32 kickMode) {
    s32 v12 = (1 << wLog) >> 6;
    s32 tag = 1 << (wLog + hLog - 4);
    u8 *buf;
    u8 packet[0x60];

    if (v12 <= 0) {
        v12 = 1;
    }

    if (kickMode == 0) {
        u32 *c = g_frameDmaCursor;
        c[0] = 0x10000006;
        c[1] = 0;
        c[2] = 0;
        c[3] = 0x50000006;
        buf = (u8 *)c + 0x10;
        g_frameDmaCursor = (u32 *)((u8 *)c + 0x70);
    } else {
        buf = packet;
    }

    func_00126288(buf, (s16)vramBlk, (s16)v12, (s16)fmt, 0, 0, (s16)(1 << wLog),
                (s16)(1 << hLog));

    if (kickMode == 0) {
        u32 *c = g_frameDmaCursor;
        c[0] = tag | 0x30000000;
        c[1] = (u32)handle;
        c[2] = 0;
        c[3] = tag | 0x50000000;
        g_frameDmaCursor = (u32 *)((u8 *)c + 0x10);
    } else {
        func_0011AEA0(0);
        KickGifImageUpload(buf, handle);
    }
}
#endif

/*
 * The GS-rectangle GIF-packet builders func_00290320, func_002904B0 and
 * func_00290640. All three are byte-exact on sdk29, promoted under RULING
 * #8549's per-site `dli` allowlist (task #1127; C bodies from task #1024's
 * NOTE #8489).
 *
 * Each builds 64-bit constants with cc1's `dli` (0x4400000000008001, and in the
 * first two also 0x00FFFFF000000000). cc1 2.9 has no DImode `ori` (FACT
 * #8524), so ordinary and fenced C cannot spell the ROM's form (#8524's bound:
 * the spellings it tried; mode punning and emitting inline-asm devices were not
 * tried, and an emitting asm is forbidden by RULING #8549). The ROM's words there
 * equal the `dli` expansion SN's Ps2EeAs.exe produces (FACT #8518, which does
 * not show that Ps2EeAs produced the ROM; GNU and SN as.exe 2.9 differ, #8524):
 * `ori 0x8800; dsll32 15; ori 0x8001` and `ori 0xFFFF; dsll 16; ori 0xF000;
 * dsll 24`, where GNU as picks another (`lui 0x4400; dsll32 0; ori`,
 * `li -1; dsll32 12; dsrl 8`). In this build that expansion comes from the
 * ASSEMBLER step: tools/ee/asm_unit.sh (tools/ee/ps2eeas_dli.awk) rewrites a
 * cc1 `dli` into those words only at the (region, function, operands) rows of
 * tools/ee/ps2eeas_dli_sites.txt (task #1105). These three functions are the
 * only ones promoted under that allowlist; 7 of its rows are exactly their 7
 * `dli` sites (3 + 3 + 1), the 8th is unpromoted func_0027DF80's (task #1159).
 * RULING #8549 rev 3 froze it until landing_gate checked every row (a919f3a9,
 * #1116) and asm_unit.sh refused a listed `dli` directly before a reorder-mode
 * branch (f6c2bae9, #1124); both landed, a new row needs `--ps2eeas` proof. No C
 * here writes an `ori`/`dsll`, and no asm below emits an instruction: the
 * fences have empty templates and the alias directive only defines an
 * assembler symbol.
 *
 * Devices (EE arm only unless marked; the native arm is plain C). Each one was
 * removed alone and re-measured (task #1127, verify_match_unit): every removal
 * breaks the match, so none is redundant.
 *   - GS_RECT_CURSOR, GS_RECT_PIX_X/Y and GS_RECT_VOL are `volatile` CODEGEN
 *     DEVICES (RULING #8404), not a claim that the cursor or the pixel offsets
 *     change asynchronously. The ROM re-loads g_frameDmaCursor (`lui; lw`)
 *     before each header store and before taking `p`, re-loads
 *     g_gsPixelOffsetX/Y for each corner, and keeps the packet stores in source
 *     order. Without the cursor volatile 90/92 words differ, without the offset
 *     volatile 75/92, without the store volatile 93/100 (func_00290320).
 *     Writers of g_frameDmaCursor: 85 asm functions by operand name (FACT #8647);
 *     87 by ROM effective address (FACT #8676). The 85 (DEMONSTRATED: 60 in
 *     nonmatchings/ plus 25 in the whole-unit segments text/1849B0, 1812A8,
 *     1FD030, 2012B8, 1B8FA8; a count of FILES gives 65) come from this census,
 *     a direct `sw` to the symbol, one row per glabel:
 *       for f in $(/usr/bin/grep -rlE '[[:space:]]sw[[:space:]].*g_frameDmaCursor\)'
 *         going-decompiled/asm/usa); do awk '/^glabel/{fn=$2}
 *         /[ \t]sw[ \t].*g_frameDmaCursor[)]/{print fn}' $f; done | sort -u
 *     The other 2 are func_002E1A58 (3 sites) and func_002821C0 (1 site): they
 *     store through a register holding the address and their stores never
 *     name the symbol, so no store-operand census reaches them (they do name
 *     it in the address-forming addiu) (149 sites in all). #8676's ROM
 *     decoder propagates constants linearly per function with no CFG, its 4
 *     extra sites were read by eye, and its 87 is not shown to be complete.
 *     By name most of the 85 are draw-list, packet or texture-upload builders
 *     (e.g. BeginFrameDrawList, BuildShrubDrawSegment, UploadTextureToGs), but
 *     not all: they also hold the frame-arena functions ResetFrameArenas and
 *     FlipFrameArena, FadeOutToBlackBlocking, and 28 unnamed func_ rows
 *     (these three functions among them). g_gsPixelOffsetX/Y are stored
 *     by InitScreenGeometry, SetupGsDisplayBuffers,
 *     RecomputeScreenViewportFromGsContext and func_0027B858. None of those is
 *     named as an interrupt or DMA handler. That census is by name, not a
 *     call-graph trace.
 *   - g_frameDmaCursorGp is an ASSEMBLER ALIAS of g_frameDmaCursor (FACT #8036's
 *     construct, sized 4 so gas makes it %gp_rel). It gives the ROM's final
 *     `jr $31; sw $2,%gp_rel(g_frameDmaCursor)($28)`, while every other cursor
 *     access is absolute. Without it the final store is `lui $1; sw %lo` and the
 *     function is 2 words longer. The relocation names g_frameDmaCursor and the
 *     object has no g_frameDmaCursorGp symbol.
 *   - the `reg4` fence is an EMPTY operand-tied asm (RULING #8483, both arms): a
 *     SCHEDULING FENCE. It keeps the store of `reg4` (`sd $8,0x28`) after the
 *     cursor write-back, in the `beqz` delay slot. Without it cc1 hoists that
 *     store above the write-back and 74/100 words differ.
 *   - func_00290640 carries one EE_REG pin (RULING #8598), see its comment.
 */
#ifndef TARGET_NATIVE
__asm__(".extern g_frameDmaCursorGp, 4\n\tg_frameDmaCursorGp = g_frameDmaCursor");
extern u32 *g_frameDmaCursorGp; // alias
#define GS_RECT_VOL volatile
#define GS_RECT_CURSOR (*(u32 *volatile *)&g_frameDmaCursor)
#define GS_RECT_PIX_X (*(volatile s32 *)&g_gsPixelOffsetX)
#define GS_RECT_PIX_Y (*(volatile s32 *)&g_gsPixelOffsetY)
#else
#define g_frameDmaCursorGp g_frameDmaCursor
#define GS_RECT_VOL
#define GS_RECT_CURSOR g_frameDmaCursor
#define GS_RECT_PIX_X g_gsPixelOffsetX
#define GS_RECT_PIX_Y g_gsPixelOffsetY
#endif

/**
 * Append a GIF/DMA packet to the frame render-DMA chain (g_frameDmaCursor)
 * that programs a GS rectangle from two corner points.
 *
 * Packet at the cursor `p`:
 *   p+0x00  DMA/GIF chain tags 0x10000003, 0, 0, 0x50000003; the cursor moves to
 *           p+0x10 here
 *   p+0x10  GIFtag 0x4400000000008001, then the register data 0x4410, 0x41, reg4
 *   p+0x30  corner 0: x | y << 16 | 0x00FFFFF000000000 (fixed Z)
 *   p+0x38  corner 1, packed the same way
 * The cursor is then advanced by another 0x30, to p+0x40.
 *
 *   x0,y0,x1,y1  the corners
 *   reg4         the fourth register-data qword, stored as is
 *   mode         != 0: whole pixels, coord + g_gsPixelOffset - 8;
 *                == 0: 1/16-pixel, (coord << 4) + g_gsPixelOffset - 0x10
 *
 * Each coordinate is a 32-bit sum, sign-extended to 64 bits before the shift
 * and OR, as the ROM's `addiu; dsll 16; or` does. The two corners re-read the
 * pixel offsets.
 * Byte-exact on sdk29 (task #1127): devices and dli expansion above.
 */
void func_00290320(s32 x0, s32 y0, s32 x1, s32 y1, u64 reg4, s32 mode) {
    u8 *p;

    ((GS_RECT_VOL u32 *)GS_RECT_CURSOR)[0] = 0x10000003;
    ((GS_RECT_VOL u32 *)GS_RECT_CURSOR)[1] = 0;
    ((GS_RECT_VOL u32 *)GS_RECT_CURSOR)[2] = 0;
    ((GS_RECT_VOL u32 *)GS_RECT_CURSOR)[3] = 0x50000003;
    p = (u8 *)GS_RECT_CURSOR;
    GS_RECT_CURSOR = (u32 *)(p + 0x10);
    *(GS_RECT_VOL u64 *)(p + 0x10) = ((u64)0x8800 << 47) | 0x8001;
    *(GS_RECT_VOL u64 *)(p + 0x18) = 0x4410;
    *(GS_RECT_VOL u64 *)(p + 0x20) = 0x41;
    __asm__ __volatile__("" : "+r"(reg4));
    *(u64 *)(p + 0x28) = reg4;

    if (mode != 0) {
        {
            s32 oy = GS_RECT_PIX_Y;
            s32 ox = GS_RECT_PIX_X;
            *(GS_RECT_VOL s64 *)(p + 0x30) = (s64)(x0 + ox - 8)
                                           | ((s64)(y0 + oy - 8) << 16)
                                           | ((s64)0xFFFFF000u << 24);
        }
        {
            s32 oy = GS_RECT_PIX_Y;
            s32 ox = GS_RECT_PIX_X;
            *(s64 *)(p + 0x38) = (s64)(x1 + ox - 8) | ((s64)(y1 + oy - 8) << 16)
                               | ((s64)0xFFFFF000u << 24);
        }
    } else {
        {
            s32 oy = GS_RECT_PIX_Y;
            s32 ox = GS_RECT_PIX_X;
            *(GS_RECT_VOL s64 *)(p + 0x30) = (s64)((x0 << 4) + ox - 0x10)
                                           | ((s64)((y0 << 4) + oy - 0x10) << 16)
                                           | ((s64)0xFFFFF000u << 24);
        }
        {
            s32 oy = GS_RECT_PIX_Y;
            s32 ox = GS_RECT_PIX_X;
            *(s64 *)(p + 0x38) = (s64)((x1 << 4) + ox - 0x10)
                               | ((s64)((y1 << 4) + oy - 0x10) << 16)
                               | ((s64)0xFFFFF000u << 24);
        }
    }
    g_frameDmaCursorGp = (u32 *)((u8 *)GS_RECT_CURSOR + 0x30);
}

/**
 * Twin of func_00290320: the same GS-rectangle packet and fixed Z, except the
 * register data at p+0x20 is 0x46 rather than 0x41.
 *
 *   x0,y0,x1,y1, reg4, mode  as func_00290320
 *
 * Byte-exact on sdk29 (task #1127): devices and dli expansion above.
 */
void func_002904B0(s32 x0, s32 y0, s32 x1, s32 y1, u64 reg4, s32 mode) {
    u8 *p;

    ((GS_RECT_VOL u32 *)GS_RECT_CURSOR)[0] = 0x10000003;
    ((GS_RECT_VOL u32 *)GS_RECT_CURSOR)[1] = 0;
    ((GS_RECT_VOL u32 *)GS_RECT_CURSOR)[2] = 0;
    ((GS_RECT_VOL u32 *)GS_RECT_CURSOR)[3] = 0x50000003;
    p = (u8 *)GS_RECT_CURSOR;
    GS_RECT_CURSOR = (u32 *)(p + 0x10);
    *(GS_RECT_VOL u64 *)(p + 0x10) = ((u64)0x8800 << 47) | 0x8001;
    *(GS_RECT_VOL u64 *)(p + 0x18) = 0x4410;
    *(GS_RECT_VOL u64 *)(p + 0x20) = 0x46;
    __asm__ __volatile__("" : "+r"(reg4));
    *(u64 *)(p + 0x28) = reg4;

    if (mode != 0) {
        {
            s32 oy = GS_RECT_PIX_Y;
            s32 ox = GS_RECT_PIX_X;
            *(GS_RECT_VOL s64 *)(p + 0x30) = (s64)(x0 + ox - 8)
                                           | ((s64)(y0 + oy - 8) << 16)
                                           | ((s64)0xFFFFF000u << 24);
        }
        {
            s32 oy = GS_RECT_PIX_Y;
            s32 ox = GS_RECT_PIX_X;
            *(s64 *)(p + 0x38) = (s64)(x1 + ox - 8) | ((s64)(y1 + oy - 8) << 16)
                               | ((s64)0xFFFFF000u << 24);
        }
    } else {
        {
            s32 oy = GS_RECT_PIX_Y;
            s32 ox = GS_RECT_PIX_X;
            *(GS_RECT_VOL s64 *)(p + 0x30) = (s64)((x0 << 4) + ox - 0x10)
                                           | ((s64)((y0 << 4) + oy - 0x10) << 16)
                                           | ((s64)0xFFFFF000u << 24);
        }
        {
            s32 oy = GS_RECT_PIX_Y;
            s32 ox = GS_RECT_PIX_X;
            *(s64 *)(p + 0x38) = (s64)((x1 << 4) + ox - 0x10)
                               | ((s64)((y1 << 4) + oy - 0x10) << 16)
                               | ((s64)0xFFFFF000u << 24);
        }
    }
    g_frameDmaCursorGp = (u32 *)((u8 *)GS_RECT_CURSOR + 0x30);
}

/**
 * The same GS-rectangle packet as func_00290320, with register data 0x46 at
 * p+0x20 and a caller-supplied Z: each corner packs as
 * x | y << 16 | zHigh << 32.
 *
 *   x0,y0,x1,y1, reg4, mode  as func_00290320
 *   zHigh                    the corners' upper 32 bits
 *
 * Byte-exact on sdk29 (task #1127): devices and dli expansion above, plus one
 * EE_REG REGISTER-PIN DEVICE (RULING #8598). The ROM re-loads the cursor into
 * $2 for the p[2] store and into $3 for the p[3] store. Unpinned, cc1 swaps the
 * two (6/94 words differ). `c` is the live cursor copy the p[3] store uses;
 * pinning it to $3 is enough, and cc1 then picks $2 for p[2] itself. A second
 * pin on the p[2] copy was measured and bought nothing, so it was dropped.
 */
void func_00290640(s32 x0, s32 y0, s32 x1, s32 y1, u64 reg4, s32 zHigh, s32 mode) {
    u8 *p;

    ((GS_RECT_VOL u32 *)GS_RECT_CURSOR)[0] = 0x10000003;
    ((GS_RECT_VOL u32 *)GS_RECT_CURSOR)[1] = 0;
    ((GS_RECT_VOL u32 *)GS_RECT_CURSOR)[2] = 0;
    {
        register u32 *c EE_REG("$3") = GS_RECT_CURSOR;
        ((GS_RECT_VOL u32 *)c)[3] = 0x50000003;
    }
    p = (u8 *)GS_RECT_CURSOR;
    GS_RECT_CURSOR = (u32 *)(p + 0x10);
    *(GS_RECT_VOL u64 *)(p + 0x10) = ((u64)0x8800 << 47) | 0x8001;
    *(GS_RECT_VOL u64 *)(p + 0x18) = 0x4410;
    *(GS_RECT_VOL u64 *)(p + 0x20) = 0x46;
    __asm__ __volatile__("" : "+r"(reg4));
    *(u64 *)(p + 0x28) = reg4;

    if (mode != 0) {
        {
            s32 oy = GS_RECT_PIX_Y;
            s32 ox = GS_RECT_PIX_X;
            *(GS_RECT_VOL s64 *)(p + 0x30) = (s64)(x0 + ox - 8)
                                           | ((s64)(y0 + oy - 8) << 16)
                                           | ((s64)zHigh << 32);
        }
        {
            s32 oy = GS_RECT_PIX_Y;
            s32 ox = GS_RECT_PIX_X;
            *(s64 *)(p + 0x38) = (s64)(x1 + ox - 8) | ((s64)(y1 + oy - 8) << 16)
                               | ((s64)zHigh << 32);
        }
    } else {
        {
            s32 oy = GS_RECT_PIX_Y;
            s32 ox = GS_RECT_PIX_X;
            *(GS_RECT_VOL s64 *)(p + 0x30) = (s64)((x0 << 4) + ox - 0x10)
                                           | ((s64)((y0 << 4) + oy - 0x10) << 16)
                                           | ((s64)zHigh << 32);
        }
        {
            s32 oy = GS_RECT_PIX_Y;
            s32 ox = GS_RECT_PIX_X;
            *(s64 *)(p + 0x38) = (s64)((x1 << 4) + ox - 0x10)
                               | ((s64)((y1 << 4) + oy - 0x10) << 16)
                               | ((s64)zHigh << 32);
        }
    }
    g_frameDmaCursorGp = (u32 *)((u8 *)GS_RECT_CURSOR + 0x30);
}
#undef GS_RECT_VOL
#undef GS_RECT_CURSOR
#undef GS_RECT_PIX_X
#undef GS_RECT_PIX_Y
#ifdef TARGET_NATIVE
#undef g_frameDmaCursorGp
#endif

/* func_002907B8(...): HUD element helper / unit tail (~0xB8 bytes).
 *
 * WALL: callee-saves + jal gate; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002907B8);
