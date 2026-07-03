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
 * text unit keeps its own flag model.
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
    s32 _pad4;    /* +0x4 */
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
    s16 sellsAmmoFlag;   /* +0x88 */
    u8  _pad8A[0x4];
    u16 ammoCapacity;    /* +0x8E */
    u16 ammoStartGrant;  /* +0x90 */
    u8  _pad92[0x4E];
} WeaponDef;                                  /* stride 0xE0 */

extern u8 g_itemEquippedSlot[0x38];           /* itemId -> active variant slot (0x139568) */
extern WeaponDef g_weaponTable[];             /* per-variant def/state table (0x239B20) */
extern s32 g_weaponXp[0x38];                  /* per-item XP/upgrade accumulator (0x139868) */
extern s32 g_weaponAmmo[0x38];                /* per-item current ammo (0x139688) */

/* Sound-bank / weapon-context block accessed by func_0028EAC8 at base+0x20.
 * +0x1268 holds the active weapon's item id; +0x22D4 is a "no ammo sale" gate. */
extern u8 g_soundBankHandles[];               /* 0x18FC60 (base) */

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
    s32 _pad04;         /* +0x04 */
    u8  _pad08[0x18];
    s32 entryIndex;     /* +0x20: index into g_pActiveTextTable of shown line */
    s32 showingIndex;   /* +0x24: pending/showing line index (-1 = none) */
    s32 showingHandle;  /* +0x28: voice/clip handle of the shown line */
    s32 tableCount;     /* +0x2C */
    u8  _pad30[0x10];
    s32 phaseTimer;     /* +0x40: per-line phase timer (cleared on arm) */
    s32 phaseFlag;      /* +0x44: per-line phase flag  (cleared on arm) */
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
extern u8  D_259F38[];   /* 6-byte header + 0xA-stride {s16 key,...} records, -1 sentinel */
extern u8  D_259CC0[];   /* same layout as D_259F38 */
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
void func_0028BE10(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
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

/* func_00288B08(key): copy a 0x30-byte word table from D_1A8B10 onto the stack
 * (via ldl/ldr/sdl/sdr unaligned block moves) and linear-scan it for `key`,
 * returning 1 when present (before a zero terminator), else 0.
 *
 * WALL: the unaligned ldl/ldr + sdl/sdr block copy of the source table is a
 * memcpy-style idiom the matcher does not emit from struct/array C. Left
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00288B08);

/*
 * func_00288BB0(key): return 1 if `key` is present (as the first s16 of any
 * 0xA-stride record after a 6-byte header, -1 sentinel) in either D_259F38 or
 * D_259CC0, else 0.
 *
 * WALL (69.84%): logic exact, but the original materialises &D_259F38 /
 * &D_259CC0 once and reaches the +6 records by a displacement (lh 0x6(base) /
 * addiu base,0x6); our cc1 folds the +6 into the %lo relocation (one symbolic
 * address per access). The address-fold-vs-displacement wall. Left INCLUDE_ASM.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00288BB0);
#else
s32 func_00288BB0(s32 key) {
    s16 *p;
    for (p = (s16 *)(D_259F38 + 6); *p != -1; p = (s16 *)((u8 *)p + 0xA)) {
        if ((s32)*p == key) {
            return 1;
        }
    }
    for (p = (s16 *)(D_259CC0 + 6); *p != -1; p = (s16 *)((u8 *)p + 0xA)) {
        if ((s32)*p == key) {
            return 1;
        }
    }
    return 0;
}
#endif

/* func_00288C30(itemId): register an item pickup in the recent-items slot table
 * (g_gsPixelOffsetY+0x64, 8 slots). Gated by func_00288BB0 (D_25E308 membership)
 * and func_00288B08, and only when the item's variant exists; finds a free /
 * matching slot and records the item id there.
 *
 * WALL: two callee-saves + two jal gates, a sentinel scan over D_25E308 and a
 * branch-likely (beql/bnel) slot search whose register colouring cc1 does not
 * reproduce. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00288C30);

/* GiveInventoryItem(itemId, ...): mark an item owned (g_inventoryOwned) and run
 * the downstream registration (variant advance / order-list insertion). Single
 * callee-save + jal gates.
 *
 * WALL: a jal-driven gate chain over g_inventoryOwned / g_weaponTable with the
 * 0xE0-stride `mult` indexing and branch colouring cc1 does not reproduce. Left
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", GiveInventoryItem);

/* AddItemToInventoryOrder(itemId): place `itemId` into the inventory quick-select
 * order list (g_inventoryOrder, bounded by D_1A7B90). The item must exist, be at
 * upgrade level 0, and either have its +0x80 flag set or sell ammo. Scans the
 * order list for an existing entry of this item (low 6 bits) or the first free
 * (0xFF) slot and writes `itemId | (owned ? 0x40 : 0)` there. Returns 1 when a
 * slot was written, else 0.
 *
 * WALL: frameless leaf, but the duplicate/free-slot scan uses branch-likely
 * (beql/bnel) and a movz to fold the owned-flag into bit 6, neither of which cc1
 * reproduces from the equivalent C. Kept as the portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", AddItemToInventoryOrder);
#else
s32 AddItemToInventoryOrder(s32 itemId) {
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
    for (order = g_inventoryOrder; order < &D_1A7B90; order++) {
        u8 entry = *order;
        if ((entry & 0x3F) == itemId) {
            *order = (u8)(itemId | (owned != 0 ? 0x40 : 0));
            return 1;
        }
        if (entry == 0xFF) {
            /* both paths converge to itemId | (owned ? 0x40 : 0) in the asm */
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

/* func_00288F30(itemId): test/iterate the item's progress-gate table (10-entry,
 * walks via a callee-saved cursor). Single callee-save (0x30 frame) + jal.
 *
 * WALL: callee-save + jal gate with a 0xA-bounded scan whose register colouring
 * cc1 does not reproduce. Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00288F30);

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
 *
 * WALL (93.12%, register-colouring + sentinel CSE): with the explicit
 * pre-loop-load / do-while / re-load form
 *     s32 *p = D_240340; s32 cur = *p;
 *     if (cur != -2) do { cur = *p; p += 2; if (key==cur) return 1; cur = *p; }
 *                    while (cur != -2);
 *     return 0;
 * cc1 reproduces the original's exact branch/instruction layout. The only
 * residual delta is register allocation: the original keeps the loaded value in
 * $2 and materialises the -2 sentinel TWICE (once in $2 for the first-element
 * compare, once in $5 for the loop test), whereas -O2 CSEs the sentinel into a
 * single $2 and colours the loaded value into $5. The original is effectively
 * *less* optimised here (un-CSE'd constant) — not reachable from clean C at -O2.
 * Left INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00289190);
#else
s32 func_00289190(s32 key) {
    s32 *p = D_240340;
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
 * Compute the scene-arena address that would remain after carving `size`
 * bytes off the decompressed-scene region, written to *out. Refuses sizes over
 * 0x40000 (writes 0, returns -1); otherwise returns 0.
 *
 * WALL (74.69%): logic + the bnezl branch-likely form match exactly, but the
 * original interleaves the two global loads (lui base; lui cursor; lw cursor;
 * lw base) where our cc1 schedules each lui/lw pair together — a fixed
 * load-scheduling difference unaffected by operand order. Left INCLUDE_ASM.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00289398);
#else
s32 func_00289398(s32 size, s32 *out) {
    if ((u32)0x40000 < (u32)size) {
        *out = 0;
        return -1;
    }
    *out = (g_sceneDecompressBase + g_sceneArenaCursor) - size;
    return 0;
}
#endif

/* Handwritten no-return fragment: `sh $0,0x1C($a0); nop` with NO jr $ra (it
 * falls through). Not expressible as a returning C function — INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002893D8);

/* ResetCinematicQueue(q): clear the cinematic ring buffer — zero the write/read
 * cursors and count, poison the 5 slots (0x28 bytes) with 0xCD, clear the
 * active flag and set the queue's game-state mode to 1.
 *
 * NEAR-MISS: logic exact, but the memset call forces `q` into the callee-saved
 * $16 and cc1 schedules the trailing cursor/flag stores in a different order
 * than the original's interleave with the jal. Kept as the portable #else
 * body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", ResetCinematicQueue);
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
 * slot's {id, flag} to *outId/*outFlags, mark that slot free (-1/-1), advance the
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
 * NEAR-MISS (85%): logic exact, but cc1 allocates a 0x20-byte frame for the
 * two callee-saves where the original uses 0x10, and sibling-call-optimises the
 * tail func_00289540 into a `j`. Both are fixed cc1 codegen choices; kept as the
 * portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002895E0);
#else
void func_002895E0(CinematicQueue *q) {
    func_00289560(q);
    *(s32 *)q = 0;
    func_00289540(q);
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

/* RequestLevelExit(destination, doSave): raise the in-level exit flag
 * (g_nLevelExitRequested) and record the destination; when destination == -1
 * (the "no explicit destination" sentinel) also resets a transition latch
 * (D_1393E0+0x17C/+0x18) and clears a save/load status bit
 * (g_nSaveLoadStatusCode+0x4 & ~0x200). When doSave is set, clears
 * the save-prompt gate and commits a progress checkpoint.
 *
 * The matching build stays INCLUDE_ASM (a single-$31 frame, but mixed %gp_rel
 * (exit flag / destination) + %hi/%lo (status code, latch) addressing and two
 * tail jal gates whose branch colouring cc1 does not reproduce). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", RequestLevelExit);
#else
void RequestLevelExit(s32 destination, s32 doSave) {
    g_nLevelExitRequested = 1;
    g_nLevelExitDestination = destination;
    if (destination == -1) {
        if (*(s32 *)(D_1393E0 + 0x17C) != 0) {
            *(s32 *)(D_1393E0 + 0x17C) = 0;
        }
        if (*(s16 *)(D_1393E0 + 0x18) >= 0) {
            *(s16 *)(D_1393E0 + 0x18) = (s16)destination;
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
 * WALL (jtbl): a compiler-emitted `switch` jump table (jr through
 * jtbl_0026C780_text). The matcher cannot reproduce the original jump-table
 * layout/relocations from C `switch`; left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002897B8);

/* func_00289840(textIndex, voiceHandle): arm a pending subtitle line — only when
 * the subtitle state machine is idle (g_subtitleState[0]==0), no line is already
 * pending (+0x24 == -1), the area save image's current clip is free and matches,
 * the resolved health/clip entry isn't 0xFFFF, the game isn't in state 6 and the
 * game timer has advanced past 6 — stores textIndex/voiceHandle into
 * g_subtitleState +0x24/+0x28 and clears +0x40/+0x44. Returns 1 when armed, else
 * 0.
 *
 * WALL (81.58%): the C below is op-for-op faithful and reproduces every load,
 * guard branch, the g_saveImageArea+0x1000 / g_health+0x66C absolute-displacement
 * folds and the combined `state==6 || time<6` exit exactly — the SOLE residual
 * difference is the EE 3-operand `mult`: the original schedules it between the
 * table-base `lui` and `addiu` and reuses the constant-12 register (v0) for the
 * product, whereas this cc1 emits the `addiu` first and allocates a fresh temp
 * (a0). That is a pure instruction-scheduling / register-allocation artifact of
 * the multiply, not expressible from semantically-equivalent C. Left INCLUDE_ASM
 * for the matching build; the #else is the cmp-oracle'd portable body
 * (cmp_188858_text.c, 95/95 on real R5900). EU twin func_00289730 (188748) is
 * byte-identical logic — region-agnostic, no divergence. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00289840);
#else
s32 func_00289840(s32 textIndex, s32 voiceHandle) {
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

/* func_002898D8: 8-byte trailing-pad fragment (addiu $sp,+0x20; nop) of the
 * preceding function, pinned as its own symbol; the real function follows. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002898D8);

/*
 * func_002898E0(): tear down the currently-shown subtitle line — if a voice
 * clip is associated (via g_pActiveTextTable[entryIndex].voice), its disc-TOC
 * entry is present and it matches the area save image's current clip, bump the
 * save image's subtitle sub-state to 5; then reset the subtitle state machine.
 *
 * WALL (84.30%): logic exact, but the original holds &g_subtitleState's %hi in
 * one register across the whole function (CSE of the high half), whereas our
 * cc1 re-materialises `lui %hi(g_subtitleState)` per access — the %hi-CSE the
 * unit-wide -fno-gcse (needed by most of this TU) disables. Left INCLUDE_ASM.
 */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002898E0);

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
 * NEAR-MISS: logic exact, but the FindTextTableEntry call forces `textId` into
 * the callee-saved $16 (0x10 frame) and the gp-relative &D_1A8D28 tail address
 * is computed in a branch-delay slot that cc1 schedules differently. Kept as the
 * portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", GetLocalizedString);
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00289A58);

/* BeginSubtitleDisplay(...): start showing a subtitle line — resolve the text
 * entry, allocate the on-screen handle and seed the subtitle state machine.
 *
 * WALL: multiple callee-saves + jal gates over g_subtitleState / the text table
 * with branch colouring cc1 does not reproduce. Left INCLUDE_ASM (not yet fully
 * traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", BeginSubtitleDisplay);

/* UpdateSubtitleStateMachine(): per-frame tick of the subtitle/voice display
 * state machine (large dispatcher, ~0x928 bytes).
 *
 * WALL: compiler-emitted jump tables (jtbl) + float timing math; the jr-through-
 * jtbl dispatch and FP scheduling are not reproducible from C. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", UpdateSubtitleStateMachine);

void func_0028A4E0(void) {
}

/* func_0028A4E8(...): subtitle layout/positioning helper (~0x51C bytes, heavy FP).
 *
 * WALL: extensive float math + jump tables; FP scheduling and jr-through-jtbl
 * dispatch are not reproducible from C. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028A4E8);

/* func_0028AA08(key, column, outValue): bidirectional key<->value lookup over
 * the D_254E48 table — 0xAA rows of two s16 columns each (row stride 4 bytes).
 * Sign-extend `key` to s16 and scan rows 0..0xA9, comparing the `column` s16 of
 * each row (column 0 -> +0, column 1 -> +2) against `key`. On the first match,
 * if `outValue` is non-NULL, write the row's OTHER column (column!=0 -> col0 at
 * +0; column==0 -> col1 at +2) through `outValue` (as a u16), and return the
 * matched row index. With no match across all 0xAA rows, return -1.
 *
 * WALL (frameless leaf, but the column*2 row-base striding + an i*4-vs-2 movn
 * output-offset selection that cc1's `?:` does not reproduce, plus the D_254E48
 * %hi/%lo displacement fold). Left INCLUDE_ASM for the matching build; the
 * #else below is the op-for-op faithful portable form (cmp_188858_text.c). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028AA08);
#else
s32 func_0028AA08(s32 key, s32 column, s16 *outValue) {
    s16 keyHalf = (s16)key;
    s16 *base = D_254E48;
    s16 *cell = base + column;   /* &row[0].col[column]; advances by 2 s16 (4 bytes) per row */
    s32 i = 0;

    do {
        if (*cell == keyHalf) {
            if (outValue != 0) {
                /* searched col0 -> return col1 (+1 s16); searched col!=0 -> col0 (+0) */
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
 * NEAR-MISS (73%): logic exact, but cc1 uses a 0x20-byte frame (orig 0x10) and
 * sibling-call-optimises the second func_0028ABC0 into a `j`. Kept as #else. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028AB70);
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
 * NEAR-MISS: logic exact, but the original lowers the (head+i)%7 indexing with
 * `div`/`mfhi` + break-on-div-by-zero scaffolding and a peeled register
 * colouring that cc1 doesn't reproduce from the C `%`. Kept as the portable
 * #else body. */
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028ACB0);

/* ResetBoltCounterHud(): snap the on-screen bolt counter to the true bolt total
 * (no roll animation) — seed both the shown and target values
 * (g_nBoltCounterDisplayed[0]/[1]) to g_boltCount, clear the roll accumulator
 * ([2]) and dirty index ([4]), set the pending flag ([3]) to -1, flush the HUD
 * value, and clear the two bolt-icon records at g_hudMobyAuxBlockBase+0x44.
 *
 * NEAR-MISS: logic exact, but the FlushHudDisplayValue call (0x10 frame, $31
 * save) and the trailing record-clear loop's per-iteration nops/branch colouring
 * cc1 schedules differently. Kept as the portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", ResetBoltCounterHud);
#else
void ResetBoltCounterHud(void) {
    s32 bolts = g_boltCount;
    s32 i;
    u8 *rec;
    g_nBoltCounterDisplayed[1] = bolts;
    g_nBoltCounterDisplayed[0] = bolts;
    g_nBoltCounterDisplayed[2] = 0;
    FlushHudDisplayValue((s32)(u32)g_nBoltCounterDisplayed);
    g_nBoltCounterDisplayed[4] = 0;
    g_nBoltCounterDisplayed[3] = -1;
    rec = (u8 *)&g_hudMobyAuxBlockBase + 0x44;
    for (i = 1; i >= 0; i--) {
        *(s16 *)(rec + 0xE) = 0;
        *(s32 *)(rec + 0x0) = 0;
        rec += 0x10;
    }
}
#endif

/* UpdateBoltCounterHud(): per-frame animation of the on-screen bolt counter —
 * rolls the displayed value toward g_boltCount and updates the digit sprites.
 *
 * WALL: callee-saves + float roll math whose FP scheduling cc1 does not
 * reproduce. Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", UpdateBoltCounterHud);

/* func_0028B0B0(...): bolt-counter digit-sprite layout/draw (~0x4A8 bytes, very
 * heavy FP).
 *
 * WALL: extensive float positioning math; FP scheduling not reproducible from C.
 * Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028B0B0);

/* func_0028B558: mis-split 1-instruction fragment — `addiu $sp,+0x10` epilogue
 * tail, pinned as its own symbol; no jr $ra. Not a real function; left
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028B558);

/* Linear-scan the HUD icon-slot table for the entry whose texture id equals
 * `name`, stopping at the 0xFFFF sentinel; return its index (or the sentinel
 * index when not found).
 *
 * NEAR-MISS (89%): frameless leaf, logic + the peeled first iteration match,
 * but the original schedules two padding `nop`s between the two loop-exit
 * branches that cc1 doesn't emit. Fixed scheduling difference; kept as #else. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028B560);
#else
s32 func_0028B560(s32 name) {
    HudIconSlot *table = (HudIconSlot *)g_pHudAssetHeader[1];
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

/* InitHudMobyTable(): first-time setup of the HUD moby-table context — rebuild
 * all 13 D_2552B0 widget records (func_0028BE10 per slot), then DebugMalloc the
 * HUD moby table / spawn / aux blocks when not already allocated and record them
 * in g_hudMobyTableBase etc.
 *
 * WALL: five callee-saves (0x30 frame) + multiple jal allocations and the
 * gp/absolute-mixed global stores; not reproducible from C. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", InitHudMobyTable);

/* func_0028B6F0(...): HUD moby-table population helper (~0x1D4 bytes).
 *
 * WALL: multiple callee-saves + jal gates with gp/absolute-mixed addressing;
 * not reproducible from C. Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028B6F0);

/* func_0028B8C8(...): HUD moby-table accessor/populate helper (~0x160 bytes).
 *
 * WALL: callee-saves + jal gate; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028B8C8);

/* func_0028BA28(...): HUD moby-table accessor/populate helper (~0x174 bytes).
 *
 * WALL: callee-saves + jal gate; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028BA28);

/* func_0028BBA0(...): HUD moby-table accessor/populate helper (~0x124 bytes).
 *
 * WALL: callee-saves + jal gate; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028BBA0);

/*
 * ResetDebugHeap(): (re)initialise the DebugMalloc bump allocator — cursor back
 * to the pool base, end at base + 0x64000 (the 0x64000-byte debug pool).
 *
 * WALL (77.27%): logic identical, but the original hoists the `lui 0x6`
 * constant-materialisation of 0x64000 ABOVE the pool-base load (and colours the
 * base into $a0); our cc1 schedules the constant after the load. A fixed
 * instruction-scheduling difference. Left INCLUDE_ASM.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", ResetDebugHeap);
#else
void ResetDebugHeap(void) {
    u8 *base = g_debugMallocPoolBase;
    g_debugMallocCursor = base;
    g_debugMallocEnd = base + 0x64000;
}
#endif

/* DebugMalloc(size): bump-allocate `size` bytes (rounded up to 16) from the
 * debug pool. Lazily (re)initialises the pool on first use, and returns 0 when
 * the remaining space is smaller than `size`. Returns the old cursor on success.
 *
 * NEAR-MISS: logic exact, but the call to ResetDebugHeap forces `size` into a
 * callee-saved register ($16), and the success/fail merge cc1 schedules the
 * cursor reload differently than the original's branch layout. Kept as the
 * portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", DebugMalloc);
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
    g_debugMallocCursor += (size + 0xF) & ~0xF;
    return result;
}
#endif

/*
 * SwapMobyTableContext(newId): toggle the active moby-table context (live vs
 * the 40-slot HUD shadow set), exchanging the base/spawn-start/end/aux-block
 * pointers between the two sets; no-op when the requested context is active.
 *
 * WALL (78.51%): the original reads the FIRST g_mobyTableBase via %gp_rel in
 * the beq branch-delay slot but WRITES the same symbol via %hi/%lo later — the
 * mixed gp_rel/absolute reload artifact (only one addressing form is
 * expressible per declaration, and the gp_rel-in-delay-slot form is fixed
 * SN-cc1 behaviour). Left INCLUDE_ASM.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", SwapMobyTableContext);
#else
void SwapMobyTableContext(s32 newId) {
    void *base, *spawn, *end, *aux;
    if (newId == g_activeMobyTableId) {
        return;
    }
    base  = g_mobyTableBase;
    spawn = g_mobySpawnStart;
    end   = g_mobyTableEnd;
    aux   = g_mobyAuxBlockBase;

    g_activeMobyTableId = g_activeMobyTableId ^ 1;

    g_mobyTableBase    = g_hudMobyTableBase;
    g_mobySpawnStart   = g_hudMobySpawnStart;
    g_mobyTableEnd     = g_hudMobyTableEnd;
    g_mobyAuxBlockBase = g_hudMobyAuxBlockBase;

    g_hudMobyTableBase    = base;
    g_hudMobySpawnStart   = spawn;
    g_hudMobyTableEnd     = end;
    g_hudMobyAuxBlockBase = aux;
}
#endif

/* func_0028BE10(packed, b, c, d, e, f, g): update HUD widget list slot
 * `packed & 0xF` of the D_2552B0 table (stride 0x90). Compares the slot's stored
 * layout fields (+0x20..+0x38) and the high nibble mask (packed & 0xFFF0)
 * against the new values; when any differ (or while not in gameState 5) it
 * rewrites them, bumps the slot's generation counter (+0x64) and the global
 * generation at g_pActiveTextTable+0x30, marks the slot dirty (+0x68=1, clears
 * +0x70/+0x7C) and, when the mask selects bit 0x20, re-inits it via
 * func_0028BF18. Returns the slot's generation counter.
 *
 * WALL: multiple callee-saves + a 7-way field-equality early-out chain whose
 * register colouring and the gameState-5/slot-2 branch-likely guards cc1 does
 * not reproduce; the +0x30 global is also read %gp_rel but written %hi/%lo
 * (the gp/absolute-mix reload artifact). Left INCLUDE_ASM (functional model in
 * the doc above; not fabricated as C to avoid a wrong-field defect). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028BE10);

/* Initialise a HUD widget `w`: bind its icon graphics (func_0028C090 using the
 * icon id at +0x20), unpack the staged layout fields (+0x24/+0x28/+0x2C/+0x30/
 * +0x34/+0x38) into their live slots, run the widget's init callback (+0x30)
 * when present, and clear the dirty flag (+0x68).
 *
 * NEAR-MISS (99.4%): logic byte-identical except cc1's epilogue restores
 * $31 before $16 where the original restores $16 first. A fixed cc1 epilogue
 * register-restore ordering; kept as the portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028BF18);
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
    *(s32 *)(b + 0x68) = 0;
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

/*
 * func_0028C010(key): search the D_2552B0 record table (13 entries, stride
 * 0x90, key at +0x64) for `key`; when present rebuild a HUD list via
 * func_0028BE10 and return 1, else 0.
 *
 * WALL (83.06%): logic + the displacement form match, but the original copies
 * the key argument into a saved register up-front and advances its scan pointer
 * to base+0x64 (reading 0x0(ptr)) where our cc1 keeps the key in $a0 and reads
 * 0x64(base) — an induction-variable / arg-colouring choice. Left INCLUDE_ASM.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C010);
#else
s32 func_0028C010(s32 key) {
    s32 i = 0;
    if (D_2552B0[0].key != key) {
        do {
            if (++i >= 0xD) {
                return 0;
            }
        } while (D_2552B0[i].key != key);
    }
    if (i >= 0xD) {
        return 0;
    }
    func_0028BE10(i, 0xFFFF, 0, 0, 0, 0, 0);
    return 1;
}
#endif

/* Resolve the HUD icon slot for `iconName` (via func_0028B560), then copy its
 * texture id, palette id and base-frame field out of the icon-slot table into
 * the widget record `w`, recording the slot index in w[+0x40].
 *
 * NEAR-MISS (59%): logic correct (the original re-reads the table pointer per
 * field access, modelled here by the per-access cast), but cc1 still allocates a
 * 0x20-byte frame for the two callee-saves (orig 0x10) and schedules the loads
 * differently. Fixed cc1 codegen; kept as the portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C090);
#else
void func_0028C090(HudElement *w, s32 iconName) {
    u8 *b = (u8 *)w;
    s32 slot = func_0028B560(iconName);
    *(s16 *)(b + 0x40) = (s16)slot;
    *(s32 *)(b + 0x0) = ((HudIconSlot *)g_pHudAssetHeader[1])[slot].texId;
    *(s8 *)(b + 0x42) = ((HudIconSlot *)g_pHudAssetHeader[1])[slot].paletteId;
    *(s32 *)(b + 0x44) = ((HudIconSlot *)g_pHudAssetHeader[1])[slot].baseFrame;
}
#endif

/* func_0028C100: mis-split 1-instruction fragment — `addiu $sp,+0x20` epilogue
 * tail, pinned as its own symbol; no jr $ra. Not a real function; left
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C100);

/* func_0028C108(key, value): linear-search the D_2552B0 record table (13
 * entries, stride 0x90, key at +0x64) for `key`; when found within the table,
 * store `value` into that record's +0x24 field, and additionally into its +0x4
 * field when its +0x68 field is zero. No-op when not found.
 *
 * NEAR-MISS (74.7%): the peeled bnel search loop and the &D_2552B0 base-hoist
 * match, but the original keeps the matched record pointer in a dedicated
 * register and copies it twice (store +0x24, reload +0x68, store +0x4 — store
 * BEFORE load), whereas cc1 reorders the +0x68 load ahead of the +0x24 store
 * (no-alias) and drops the pointer copies. Kept as the portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C108);
#else
void func_0028C108(s32 key, s32 value) {
    s32 i = 0;
    if (D_2552B0[0].key != key) {
        do {
            if (++i >= 0xD) {
                return;
            }
        } while (D_2552B0[i].key != key);
    }
    if (i < 0xD) {
        D_2552B0[i].field24 = value;
        if (D_2552B0[i].field68 == 0) {
            D_2552B0[i].field04 = value;
        }
    }
}
#endif

/*
 * func_0028C180(rec, pA, pB): apply a layout record's edge/centre alignment
 * flags (+0x60) to an X (*pA) and Y (*pB) coordinate, using its half-extents at
 * +0x58 (X) / +0x5C (Y): bit 1 / bit 2 gate the Y shift by half the +0x5C
 * extent; bit 4 / bit 8 gate the X shift by the full / half +0x58 extent.
 * Always returns 0.
 *
 * WALL (85.69%): logic exact, but the original lowers the `(flags & 1) == 0`
 * test as `xori;andi;beqz` + a branch-likely (bnezl) and keeps the +0x58 extent
 * live in a saved register; our cc1 emits `andi;bnez`, no branch-likely, and
 * re-reads the extent — a fixed branch/colouring heuristic. Left INCLUDE_ASM.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C180);
#else
s32 func_0028C180(HudElement *rec, s32 *pA, s32 *pB) {
    u8  *b = (u8 *)rec;
    s32  flags = *(s32 *)(b + 0x60);
    s32  xExtent = *(s32 *)(b + 0x58);
    s32  yExtent = *(s32 *)(b + 0x5C);

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

/* func_0028C1E8(rec, pX, pY, ...): apply a HUD layout record's alignment flags
 * (+0x60) to an (*pX,*pY) coordinate using fractional offset tables
 * (D_255A00/D_255A60) scaled by the record's half-extents (+0x58/+0x5C), via
 * IntToFloat/FloatToInt round-trips.
 *
 * WALL: callee-saves incl. $f20 + float scaling math (IntToFloat/FloatToInt
 * round-trips); FP scheduling not reproducible from C. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C1E8);

/* func_0028C390(rec): recompute a HUD widget's auto-sized extents. Resolves the
 * value pointer at rec+0xC (when valid and word-aligned) into the rec+0x78
 * working width, clamped against rec+0x8, else seeds 0x1869F; counts the decimal
 * digits of rec+0x8 (loop dividing by 10) and applies the rec+0x60 alignment
 * flags to derive the rec+0x5C/+0x58 half-extents (12 px per digit / 14 px caps).
 *
 * WALL: the digit-count and modulo steps lower as `div`/`mflo` with
 * break-on-div-by-zero scaffolding and branch-likely guards that cc1 does not
 * reproduce from the C `/`,`%`. Left INCLUDE_ASM (functional model documented;
 * not fabricated to avoid a wrong-arithmetic defect). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C390);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C4C0);

/* func_0028C4C8(...): HUD widget reset/layout helper (~0x1CC bytes, some FP).
 *
 * WALL: callee-saves + float math + jal gates; not reproducible from C. Left
 * INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C4C8);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C728);

/* SyncEquippedItemSlots / func_0028C7A8(): refresh the 8-entry equipped-item
 * cache (g_equippedItemSlots[0..7]) from the live weapon-select wheel records.
 * The wheel-record array base is the pointer stored at g_hudMobySpawnStart+0x2C
 * (== &D_002550F0, stride 0x1C); each record's +0x18 holds the resolved item id.
 * For each slot whose cached id differs from the wheel record's +0x18 id, write
 * the record id into g_equippedItemSlots[i].
 *
 * WALL (54%, LICM / reloaded-ptr): the original RE-READS the record-array base
 * pointer (*(g_hudMobySpawnStart+0x2C)) from memory on every iteration, treating
 * it as if the g_equippedItemSlots store could alias it; this cc1 proves the two
 * objects disjoint and hoists the invariant base load out of the loop (loop-
 * invariant code motion, independent of -fno-gcse). The same reloaded-ptr idiom
 * walls the sibling func_0028C728. Left INCLUDE_ASM for the matching build; the
 * #else is the cmp-oracle'd portable body (cmp_188858_wheel.c, 32/32 on real
 * R5900). EU twin func_0028C730 (188748) is byte-identical logic — region-
 * agnostic, no divergence. */
extern s32 g_equippedItemSlots[8];            /* currently-equipped item ids (0x1A73B8) */

typedef struct WheelRecord {
    s32 nameStringId;   /* +0x00 */
    u8  _pad04[0x14];
    s32 itemId;         /* +0x18: resolved item id for this wheel slot */
} WheelRecord;                                /* stride 0x1C */

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C7A8);
#else
void func_0028C7A8(void) {
    WheelRecord **pRec = *(WheelRecord ***)((u8 *)&g_hudMobySpawnStart + 0x2C);
    s32 *slot = g_equippedItemSlots;
    s32  i;

    for (i = 0; i < 8; i++) {
        s32 id = pRec[0][i].itemId;
        if (slot[i] != id) {
            slot[i] = id;
        }
    }
}
#endif

/* Build the weapon-select wheel widget `w`: rebuild its icon list
 * (func_0028C728), then seed the wheel geometry/state (half-extents 0xD2/0xC8,
 * type tag -2, mode 0x1E, cleared cursors).
 *
 * NEAR-MISS (89%): logic exact, but cc1 uses a 0x20-byte frame (orig 0x10) for
 * the two callee-saves and schedules the trailing zero-stores in a different
 * order. Fixed cc1 codegen; kept as the portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C7F0);
#else
void func_0028C7F0(HudElement *w) {
    u8 *b = (u8 *)w;
    func_0028C728();
    *(s32 *)(b + 0x58) = 0xD2;
    *(s32 *)(b + 0x5C) = 0xC8;
    *(s32 *)(b + 0x74) = -2;
    *(s32 *)(b + 0x78) = 0x1E;
    *(s32 *)(b + 0x70) = 0;
    *(s16 *)(b + 0x48) = 0;
    *(s16 *)(b + 0x4A) = 0;
}
#endif

/* func_0028C840(...): weapon-select wheel slot layout/draw (~0x8CC bytes, very
 * heavy FP).
 *
 * WALL: extensive float geometry math; FP scheduling not reproducible from C.
 * Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C840);

/* DrawWeaponSelectWheel(...): render the weapon-select wheel widget (~0x5C4
 * bytes, heavy FP).
 *
 * WALL: extensive float geometry math; FP scheduling not reproducible from C.
 * Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", DrawWeaponSelectWheel);

/* func_0028D6D8(w): seed a HUD list widget `w` — register the list descriptor
 * (8 entries, callback &D_1A8DD8) in g_hudMobySpawnStart+0x28/+0x2C, set the
 * widget geometry (+0x58=0xD2, +0x5C=0xC8, type tag +0x74=-2, mode +0x78=0x1E),
 * clear its two 16-bit cursors (+0x48/+0x4A) and reset the wheel cursor
 * (D_1A8D48=0).
 *
 * NEAR-MISS: frameless and straight-line, but the +0x28/+0x2C descriptor writes
 * use the named-sub-object (g_hudMobySpawnStart+0x28) %hi/%lo form while
 * D_1A8DD8/D_1A8D48 are %gp_rel; cc1 schedules the trailing zero-stores in a
 * different order. Kept as the portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028D6D8);
#else
void func_0028D6D8(HudElement *w) {
    u8 *b = (u8 *)w;
    *(s32 *)((u8 *)&g_hudMobySpawnStart + 0x28) = 8;
    *(void **)((u8 *)&g_hudMobySpawnStart + 0x2C) = &D_1A8DD8;
    *(s32 *)(b + 0x58) = 0xD2;
    *(s32 *)(b + 0x78) = 0x1E;
    *(s32 *)(b + 0x5C) = 0xC8;
    *(s32 *)(b + 0x74) = -2;
    *(s16 *)(b + 0x48) = 0;
    *(s16 *)(b + 0x4A) = 0;
    D_1A8D48 = 0;
}
#endif

/* func_0028D720(...): weapon-wheel input/selection update (~0x504 bytes, heavy FP).
 *
 * WALL: extensive float math; FP scheduling not reproducible from C. Left
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028D720);

/* func_0028DC28(...): weapon-wheel draw/animation (~0xA18 bytes, very heavy FP +
 * jump tables).
 *
 * WALL: extensive float math + jtbl dispatch; neither reproducible from C. Left
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028DC28);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028E7D8);

/* func_0028E7E8(slot): pick the current animation frame for HUD icon `slot`
 * (mode at +0x2 selects: static frame, time-driven mod cycle, ping-pong, one-shot
 * or random) from the icon-slot table at g_pHudAssetHeader[1], writing the
 * chosen map index back into the widget at +0x4. Returns the frame index.
 *
 * WALL: three callee-saves (0x20 frame) + several div/mfhi/mflo modulo steps
 * (with break-on-div-by-zero) and a GetRandomInt jal whose register colouring cc1
 * does not reproduce. Left INCLUDE_ASM (functional model documented; not
 * fabricated to avoid a wrong-modulo defect). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028E7E8);

/* func_0028E9A0(...): HUD icon-frame helper (~0x128 bytes).
 *
 * WALL: callee-saves + modulo/jal scaffolding; not reproducible from C. Left
 * INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028E9A0);

/* func_0028EAC8(): return the active weapon's item id when that weapon sells
 * ammo (g_weaponTable[slot].sellsAmmoFlag != 0) and the "no ammo sale" gate is
 * clear, else 0.
 *
 * WALL (gp/absolute-fold): the original addresses the sound-bank block as the
 * named sub-object g_soundBankHandles+0x20 with +0x1248/+0x22B4 displacements,
 * and finishes with a movz/movn conditional-move pair; cc1 folds the +0x20 into
 * the %lo relocation (losing the displacement split) and lowers the ?: as a
 * branch. Kept as the portable #else body. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028EAC8);
#else
s32 func_0028EAC8(void) {
    s32 itemId = *(s32 *)(g_soundBankHandles + 0x1268);
    u8  noSale = g_soundBankHandles[0x22D4];
    u8  equippedSlot = g_itemEquippedSlot[itemId];
    u16 sellsAmmo = g_weaponTable[equippedSlot].sellsAmmoFlag;
    if (sellsAmmo == 0 || noSale != 0) {
        return 0;
    }
    return itemId;
}
#endif

/* func_0028EB10(...): HUD widget update helper (~0x15C bytes).
 *
 * WALL: callee-saves + jal gates; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028EB10);

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

/* func_0028ECC0(): per-frame HUD tick — when the HUD CLUT slot is allocated and
 * not faded, run each of the 13 D_2552B0 widget init callbacks (+0x18, via jalr)
 * and refresh state gated on g_nGameState / nanotech-orb visibility.
 *
 * WALL: callee-saves + an indirect-call (jalr) callback loop over many globals
 * addressed with gp/absolute-mixed forms; not reproducible from C. Left
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028ECC0);

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

/* GetHudIconTex0(...): resolve a HUD icon's GS tex0 register/handle (~0x224
 * bytes).
 *
 * WALL: callee-saves + jal gates over the CLUT/texture slot tables with
 * gp/absolute-mixed addressing; not reproducible from C. Left INCLUDE_ASM (not
 * yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", GetHudIconTex0);

/* func_0028F0D0(...): HUD icon GS-handle helper (~0x1F0 bytes).
 *
 * WALL: callee-saves + jal gates; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F0D0);

/* func_0028F2C0(...): HUD icon upload/draw helper (~0x27C bytes; 128-bit block
 * moves).
 *
 * WALL (sq/lq 128-bit): GS packet assembly via 128-bit lq/sq the matcher cannot
 * reproduce from C. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F2C0);

/* func_0028F540(...): HUD icon slot helper (~0x1A8 bytes).
 *
 * WALL: callee-saves + jal gates; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F540);

/* func_0028F6E8(...): HUD icon slot setup thunk (0x14 bytes) that falls into
 * DrawHudIconQuadPixel at 0x28F700.
 *
 * WALL: callee-saves + jal gates; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F6E8);

/* func_0028F700 = DrawHudIconQuadPixel (0x1E0 bytes): HUD icon GS sprite quad at
 * pixel coords. A real function (jal'd by func_002D9D60), split out of the
 * preceding func_0028F6E8 thunk via a type:func pin. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F700);

/* func_0028F8E0(...): HUD icon upload/draw helper (~0x200 bytes; 128-bit block
 * moves).
 *
 * WALL (sq/lq 128-bit): GS packet assembly via 128-bit lq/sq the matcher cannot
 * reproduce from C. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F8E0);

/* func_0028FAE0(...): HUD icon slot helper (~0x18C bytes).
 *
 * WALL: callee-saves + jal gates; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028FAE0);

/* func_0028FC70: mis-split 1-instruction fragment — `addiu $sp,+0xE0` epilogue
 * tail, pinned as its own symbol; no jr $ra. Not a real function; left
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028FC70);

/* func_0028FC78(...): HUD element draw/layout helper (~0x374 bytes, heavy FP).
 *
 * WALL: extensive float math; FP scheduling not reproducible from C. Left
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028FC78);

/* func_0028FFF0(...): HUD element helper (~0x1C0 bytes).
 *
 * WALL: callee-saves + jal gates; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028FFF0);

/* func_002901B0(...): HUD element helper (~0x170 bytes).
 *
 * WALL: callee-saves + jal gates; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002901B0);

/* func_00290320(...): HUD element helper (~0x18C bytes; near-twin of
 * func_002904B0).
 *
 * WALL: callee-saves + jal gates; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00290320);

/* func_002904B0(...): HUD element helper (~0x18C bytes; near-twin of
 * func_00290320).
 *
 * WALL: callee-saves + jal gates; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002904B0);

/* func_00290640(...): HUD element helper (~0x174 bytes).
 *
 * WALL: callee-saves + jal gates; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00290640);

/* func_002907B8(...): HUD element helper / unit tail (~0xB8 bytes).
 *
 * WALL: callee-saves + jal gate; register colouring not reproducible from C.
 * Left INCLUDE_ASM (not yet fully traced). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002907B8);
