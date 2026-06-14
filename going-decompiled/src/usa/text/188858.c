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

/* gp-relative small-data globals (read %gp_rel in the original). */
extern s32 g_nSavePromptPending;    /* show-saving-prompt gate (0x1A7B94) */
extern s32 g_nLevelExitRequested;   /* in-level frame-loop exit flag (0x1A8B84) */
extern s32 g_nGameState;            /* top-level game state id */

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

/* Larger %hi/%lo globals (arrays/structs — already non-small, no override). */
extern void *g_pHudAssetHeader;     /* DebugMalloc'd HUD asset header (0x1B1808) */

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
    u8  _pad4E[0x3A];
    s16 sellsAmmoFlag;   /* +0x88 */
    u8  _pad8A[0x4];
    u16 ammoCapacity;    /* +0x8E */
    u16 ammoStartGrant;  /* +0x90 */
    u8  _pad92[0x4E];
} WeaponDef;                                  /* stride 0xE0 */

extern u8 g_itemEquippedSlot[0x38];           /* itemId -> active variant slot (0x139568) */
extern WeaponDef g_weaponTable[];             /* per-variant def/state table (0x239B20) */

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
    s32 showingIndex;   /* +0x24 */
    s32 showingHandle;  /* +0x28 */
    s32 tableCount;     /* +0x2C */
} SubtitleState;
extern SubtitleState g_subtitleState;
extern s32 g_discToc[];                       /* master disc asset directory (0x14B540) */
extern u8  g_saveImageArea[];                 /* per-area save image RAM buffer (0x1A53A8) */

extern s32 g_cinematicUnlockedFlags[];        /* cinematics-watched bitfield (0x139768) */

/* Tables in other text/data segments. */
extern s32 D_240340[];   /* {key, _} pairs (stride 8), -2 sentinel (0x240340) */
extern u8  D_259F38[];   /* 6-byte header + 0xA-stride {s16 key,...} records, -1 sentinel */
extern u8  D_259CC0[];   /* same layout as D_259F38 */
typedef struct Rec2552B0 {
    u8  _pad00[0x64];
    s32 key;             /* +0x64 */
    u8  _pad68[0x28];
} Rec2552B0;                                  /* stride 0x90 */
extern Rec2552B0 D_2552B0[];   /* 13-entry record table (0x2552B0) */

/* Forward declarations of unit-local callees. */
void ResetCinematicQueue(CinematicQueue *q);
void func_0028C390(void *p);
void func_0028BE10(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
void func_0029DB10(s32 a, s32 b);
void func_002B1B48(s32 a, s32 b, s32 c);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002888D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", GetWeaponUpgradeLevel);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", SetWeaponUpgradeSlot);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00288BB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00288C30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", GiveInventoryItem);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", AddItemToInventoryOrder);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00288F30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", IsItemUnlockedAtProgress);

/*
 * func_00289190(key): return 1 if `key` appears as the first word of any
 * {key,_} pair in D_240340 (stride 8, -2 sentinel), else 0.
 *
 * WALL (~5%): logic matches but cc1 lays out the loop / sentinel preload with a
 * different register assignment and branch structure (the original threads the
 * %lo-immediate first-element load into the loop differently). Left INCLUDE_ASM.
 */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00289190);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", UpgradeWeaponToMax);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", GetWeaponStatsAtLevel);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00289398);

/* Handwritten no-return fragment: `sh $0,0x1C($a0); nop` with NO jr $ra (it
 * falls through). Not expressible as a returning C function — INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002893D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", ResetCinematicQueue);

/*
 * DequeueCinematic(q, outId, outFlags): pop the slot at the read cursor into
 * *outId/*outFlags, clear it to -1, advance the read cursor mod 5; returns 1
 * unless the queue was empty. EnqueueCinematic(q, id): push id at the write
 * cursor (refused when full/playing), set the "standard cinematic id" flag,
 * advance mod 5. (See func_00289560 for the matched companion.)
 *
 * WALL (94.87% / 77.24%): logic + the (x+1 != 5)?x+1:0 cursor wrap are exact,
 * but the final `q->cursor = wrap` store sinks into the jr-delay slot with the
 * movz immediately before jr in the original; GNU as schedules jr ahead of the
 * movz here. Enqueue additionally drifts the queue-pointer register colouring
 * off its branch-likely early checks. The jr-delay conditional-move tail wall.
 */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", DequeueCinematic);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", EnqueueCinematic);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002895E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", StartCinematicFromQueue);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", RequestLevelExit);

/* Non-zero while the main in-level frame loop has been asked to exit. */
s32 IsLevelExitRequested(void) {
    return g_nLevelExitRequested != 0;
}

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002897B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00289840);

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
 * WALL (53.67%): logic correct, but the original RE-READS g_subtitleState's
 * +0x2C count from memory every loop iteration (recomputing &g_subtitleState)
 * and indexes the table as `base + i*0x10`, whereas our cc1 caches the count in
 * a register (CSE) and walks a `+= 0x10` pointer — a fixed CSE / loop-strength
 * idiom difference. Left INCLUDE_ASM.
 */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", FindTextTableEntry);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", GetLocalizedString);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00289A58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", BeginSubtitleDisplay);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", UpdateSubtitleStateMachine);

void func_0028A4E0(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028A4E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028AA08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028AA70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028AB70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028ABC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028ACB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", ResetBoltCounterHud);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", UpdateBoltCounterHud);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028B0B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028B558);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028B560);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", InitHudMobyTable);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028B6F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028B8C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028BA28);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", ResetDebugHeap);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", DebugMalloc);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", SwapMobyTableContext);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028BE10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028BF18);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028BF80);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C010);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C090);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C100);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C108);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C180);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C1E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C390);

/* Reset a HUD widget record: set its type tag (+0x7C = 0xD2), clear the two
 * 16-bit cursor fields (+0x48/+0x4A) and re-init it via func_0028C390. */
void func_0028C490(void *p) {
    *(s32 *)((u8 *)p + 0x7C) = 0xD2;
    *(s16 *)((u8 *)p + 0x48) = 0;
    *(s16 *)((u8 *)p + 0x4A) = 0;
    func_0028C390(p);
    __asm__ __volatile__("");
}

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C710);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C728);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C7A8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C7F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028C840);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", DrawWeaponSelectWheel);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028D6D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028D720);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028DC28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028E640);

/* Seed a HUD widget record's geometry (+0x5C/+0x58 = 0x20, type tag +0x7C =
 * 0x96) then re-init it via func_0028C390 (empty-asm guard keeps the jal). */
void func_0028E7A0(u8 *p) {
    *(s32 *)(p + 0x58) = 0x20;
    *(s32 *)(p + 0x7C) = 0x96;
    *(s32 *)(p + 0x5C) = 0x20;
    func_0028C390(p);
    __asm__ __volatile__("");
}

/* Stub returning 0. */
s32 func_0028E7D0(void) {
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028E7D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028E7E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028E9A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028EAC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028EB10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028EC70);

/* Decrement the countdown gate at D_1A8FB0, clamping at 0. */
void func_0028ECA8(void) {
    if (D_1A8FB0 != 0) {
        D_1A8FB0 = D_1A8FB0 - 1;
    }
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028ECC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028EDF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", GetHudIconTex0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F0D0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F2C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F540);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F6E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028F8E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028FAE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028FC70);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028FC78);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_0028FFF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002901B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00290320);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002904B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_00290640);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188858", func_002907B8);
