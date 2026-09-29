#include "common.h"

/*
 * text/1907F0 — level-init / screen-fade helper TU (carved out of the .text
 * asm segment on 2026-06-11, TU-cluster scan candidate C). This is an
 * original separate translation unit built at nonzero -G: its small-data
 * globals (sdata cluster D_1A9000..D_1A9020 plus D_1A8D40) are accessed
 * uniformly via %gp_rel($gp), which -G0 cannot express. The matcher builds
 * THIS unit at -O2 -G8 -fno-gcse (the later SN cc1 that built this TU lacks load-PRE - -fno-gcse reproduces it) (see the per-unit GFLAG override in
 * tools/ee/objdiff_build.sh / diff.sh / build.sh); every other text unit
 * stays -O2 -G0.
 *
 * -G8 extern-sizing rules for this file (same as cod/0321A0):
 *   - a complete extern object of size <= 8 bytes is placed in small data
 *     (gp-relative access);
 *   - an object the original reads with the ADJACENT lui/%lo "assembler
 *     macro" shape (dest-reg-based loads, $at-based stores) is declared as a
 *     small scalar PLUS a file-scope `.extern sym,16` override: cc1 then
 *     emits the one-insn symbolic macro (so it schedules like one insn,
 *     matching the SN codegen) and GNU as expands it absolutely.
 *
 * KNOWN MIXED-ATTRIBUTION CAVEAT (verified in the raw bytes, not splat
 * noise): g_playerProgress (0x1A79F8) is read via %gp_rel in func_002911F0
 * but via an absolute %hi/%lo pair in func_00290FD0 — the same 4-byte
 * global, both ways, inside one original TU (the proven reload-artifact
 * wall). One declaration expresses only one side: g_playerProgress is
 * declared small here (favouring func_002911F0), and func_00290FD0 reads it
 * through a 16-byte-sized zero-offset assembler alias (g_playerProgressAbs)
 * that gas expands absolutely. (func_00290FD0's div-by-28 `break 0,7` is not a
 * wall: cc1 2.9 emits the ROM's guard shape and tools/ee/move_fixup.sed
 * spells the trap as SN ee-as encoded it.)
 *
 * SAVE-LAYOUT WALL (measured 2026-06-11, blocks every multi-save function
 * here): this gameplay-text TU was built by a later SN cc1 that packs
 * callee-saved GPR/FPR stack slots 8-byte (sd at sp+0x0/0x8/0x10/...); the
 * pinned 2.9-ee-991111 cc1 reserves a 16-byte slot per save (proven on
 * func_003475F0: 99.89%, every byte equal except the save offsets/frame
 * size). Functions whose only callee save is $ra are unaffected.
 */

/* Original cc1-small / assembler-absolute symbols (see header). */
__asm__(".extern g_nGameState, 16");

/* Small-data globals (gp-relative; complete <=8-byte declarations so -G8
 * places them in small data). */
extern s32 D_1A9000;  /* fade/transition frame countdown (stepped by TickCountdownTimer) */
extern s32 D_1A9004;  /* fade mode/owner id (2 = level-transition fade) */
extern f32 D_1A9008;  /* fade progress/intensity captured at request time */
extern s32 D_1A900C;  /* fade colour A (0xRRGGBB) */
extern s32 D_1A9010;  /* fade colour B (0xRRGGBB) */
extern s32 D_1A9014;  /* full-screen tint level 0..10 */
extern s32 D_1A9018;  /* tint hold-this-frame request flag */
extern s32 D_1A901C;  /* cleared-only flag (set elsewhere) */
extern s32 D_1A9020;  /* small state machine 0..3 (3 = done) */
extern s32 D_1A8D40;  /* shared status value returned by the pump callbacks */

/* cc1-small / assembler-absolute scalar (paired with the override above). */
extern s32 g_nGameState;     /* top-level game state id (0 = in-game) */

extern s32 TickCountdownTimer(s32 *counter);   /* shared countdown step (text/183178) */
extern void func_0029C600(s32 arg0);
extern s32 DrawFullScreenTint(s32 r, s32 g, s32 b, s32 a);

/* Forward decls for the screen-fade / level-init helpers and their callees. */
extern s32  func_00290EA0(void);            /* fade pump status callback */
extern s32  func_00290920(void);            /* fade draw callback */
extern void func_002911F0(void);            /* grant default gadget loadout (defined below) */
extern s32  func_00291148(void);            /* validate inventory display order (defined below) */
extern void func_0029C5B0(s32 (*pump)(void), s32 (*draw)(void), s32 slot); /* register pump+draw callbacks */
extern void func_002AB1A8(s32 *counter, s32 start, s32 mode);              /* arm a countdown */

/* func_00290FD0 level-init callees (Track-B names where known). */
extern void InstallFileLoadPump(void);
extern void func_002FCFC8(void);
extern void SetupMemoryArenaTable(void);
extern void FillMemory32(void *dst, s32 pattern, s32 nbytes);
extern void InitScreenGeometry(void);
extern void BuildCameraProjection(void);
extern void func_00280FE0(void);
extern void ResetFrameArenas(void);
extern void RegisterParticleUpdateHandlers(void);
extern void InstallVif1DmacHandlers(void);
extern void MarkLevelAvailable(s32 level);
extern s32  g_vramTextureBase;
extern s32  g_vramDynamicBase;
extern s32  g_vramAllocCursor;
extern u8   g_mobyClassSlotRemap[];   /* 0x1CE460 */
extern u8   g_mobyClassBounds[];      /* 0x1D1500 */
extern u8   g_mobyClassDataSizes[];   /* 0x1D0D80 */
extern u8   g_tieClassQueue[];        /* 0x216200 */
extern u8   g_tieVisibleClassList[];  /* 0x21D600 */
extern u8   g_tieTexVramTable[];      /* 0x21D000 */
extern u8   g_shrubClassTable[];      /* 0x2125D0 */
extern u8   g_shrubRelightList[];     /* 0x2140D0 */
extern u8   g_shrubTexVramTable[];    /* 0x213AD0 */
extern u8   g_pLastOcclusionMask[];   /* 0x1B1698 */
extern s32  D_1A9A88;                  /* cleared at the end of the init pass */

/* Inventory display-order / ownership system (text/183178 data block). */
extern u8   g_inventoryOrder[];        /* 0x1A7B70: byte[] low6=itemId, bit0x40=owned, 0xFF terminator */
extern u8   g_inventoryOwned[];        /* 0x1A7B00: u8[0x38] per-item have-flag */
extern u8   g_inventoryNewFlag[];      /* 0x1A7B38: u8[0x38] newly-acquired flag */
extern s32  func_00289190(s32 itemClass);   /* true when the item class is still ownable */
extern void GiveInventoryItem(s32 itemId);
extern void func_002AE6C8(s32 itemId);      /* EquipGadgetItem */
extern void func_002912B8(s32 progress);    /* re-insert everything unlocked at the player's progress */
extern s32  g_playerProgress;          /* 0x1A79F8 */
extern u32  g_itemEquipSlotTable[];    /* 0x1A7398: equip-slot table written for items 0x1E/0x2F (idx 0 / idx 2) */

#define INVENTORY_ORDER_COUNT 0x20      /* loop walks g_inventoryOrder[0 .. 0x1F] (end = &g_inventoryOrder[0x20]) */
#define ITEM_HELIPACK         0x1E

/* func_00290870: 8 bytes of inter-function padding (addiu $sp,+0x90 / nop)
 * split off via symbol_addrs size:0x8; the real function begins at
 * func_00290878. Pure padding, no C. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1907F0", func_00290870);

/* func_00290878: fade-request entry (take ownership of the fade state slot
 * D_1A9004, register the func_00290EA0/func_00290920 pump callbacks, then
 * store colours/progress and arm the 10-frame countdown at D_1A9000).
 * Body reproduces 1:1 at 84% — blocked by the 8-byte-packed callee-save
 * layout (saves s0/s1/s2/ra/f20 at sp+0x0..0x20; see header) plus a void
 * tail call this cc1 sibling-call-optimises. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1907F0", func_00290878);
#else
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed
 * callee-save layout (s0/s1/s2/ra/f20) + a void sibling-call tail. */
void func_00290878(s32 owner, s32 colourA, s32 colourB, f32 progress) {
    s32 current;

    /* Register the pump+draw callbacks the first time, or when a new owner
     * pre-empts a non-transition fade (D_1A9004 != 2). */
    if (D_1A9000 == 0 || (D_1A9004 != 2 && owner == 2)) {
        D_1A9004 = owner;
        func_0029C5B0(func_00290EA0, func_00290920, 1);
    }
    current = D_1A9004;

    /* Only the slot's current owner may (re)arm the fade. */
    if (owner == current) {
        D_1A900C = colourA;
        D_1A9010 = colourB;
        D_1A9008 = progress;
        func_002AB1A8(&D_1A9000, 0xA, 2);
    }
}
#endif

/* func_00290920: the fade draw callback (rebuilds the camera projection with
 * a pinched FOV, draws the letterbox/fade rectangles via func_0027E4D0 and
 * the two colour overlays via ColorLerpPacked/func_003017F8). Blocked by the
 * 8-byte-packed callee-save layout (s0..s5+ra+f20..f25 packed at sp+0x0..,
 * see header).
 * NO #else: this is 0x57C bytes of dense, opaque letterbox/bar interpolation
 * geometry (a dozen cvt.w.s/magic-constant FP chains feeding func_0027E4D0
 * rect draws) whose per-bar coordinate math is not recovered to the fidelity
 * a faithful functional-equivalent would need — left bare with this wall note
 * per the "don't fabricate speculative C for opaque functions" rule. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1907F0", func_00290920);

/**
 * Fade pump status callback: while in-game (g_nGameState == 0), step the
 * D_1A9000 countdown; when it expires, stop the pump via func_0029C600(1).
 * Returns the shared D_1A8D40 status value.
 */
s32 func_00290EA0(void) {
    if (g_nGameState == 0) {
        if (TickCountdownTimer(&D_1A9000)) {
            func_0029C600(1);
        }
    }
    return D_1A8D40;
}

/* func_00290EE0: 4 bytes of inter-function fill (`addiu $sp,+0x30`), no
 * prologue/return — not compiler output, no C can produce it. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1907F0", func_00290EE0);

/**
 * func_00290EE8 - request the full-screen tint to hold/raise this frame.
 *
 * Sets the tint hold-this-frame flag D_1A9018 to 1 and returns 1.
 *
 * Byte-exact on the sdk29 arm (task #895). The ROM materialises ONE
 * `li v0,1` for both the store and the return value; cc1 2.9 left to itself
 * loads the constant twice (li v1 for the store, li v0 for the return). Binding
 * the value to $2 (empty on native) makes the store read the return register,
 * which gives the ROM's `li v0,1; jr ra; sw v0,%gp_rel(D_1A9018)`. The old note
 * here ("no source shape shares them") was a lever that had not been tried.
 */
#ifndef TARGET_NATIVE
#define EE8_IN_V0 __asm__("$2")
#else
#define EE8_IN_V0
#endif
s32 func_00290EE8(void) {
    register s32 result EE8_IN_V0 = 1;

    D_1A9018 = result;
    return result;
}

/**
 * Per-frame full-screen tint pump: raise the tint level toward 10 while
 * requested (D_1A9018), decay it toward 0 otherwise, then draw a black
 * full-screen tint with alpha = level * 0.1 * 48. Forwards the draw call's
 * status when it draws (falling through otherwise, like the original — the
 * value-returning call is also what keeps cc1 from sibling-call-optimising
 * the tail).
 */
s32 func_00290EF8(void) {
    s32 alpha;

    if (D_1A9018) {
        s32 v = D_1A9014 + 1;
        D_1A9018 = 0;
        D_1A9014 = v;
        if (!(v < 11)) {
            D_1A9014 = 10;
        }
    } else {
        s32 v = D_1A9014 - 1;
        D_1A9014 = v;
        if (v < 0) {
            D_1A9014 = 0;
        }
    }
    alpha = (f32)D_1A9014 * 0.1f * 48.0f;
    if (alpha) {
        return DrawFullScreenTint(0, 0, 0, alpha);
    }
}

/* func_00290F90: 4-byte fragment (`sw $v0, %gp_rel(D_1AB2DC)($gp)` with no
 * return) — a handwritten patch-stub, not compiler output. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1907F0", func_00290F90);

/** Clear the D_1A901C flag. */
void func_00290F98(void) {
    D_1A901C = 0;
}

/** Reset the D_1A9020 state machine. */
void func_00290FA0(void) {
    D_1A9020 = 0;
}

/** Kick the D_1A9020 state machine (0 -> 1; later states untouched). */
void func_00290FA8(void) {
    if (D_1A9020 == 0) {
        D_1A9020 = 1;
    }
}

/** True when the D_1A9020 state machine reached its final state (3). */
s32 func_00290FC0(void) {
    return D_1A9020 == 3;
}

/*
 * func_00290FD0 — per-level render / moby-table init. Sole caller is
 * LoadLevelAndInitHealth (FACT #5672), so it runs on every level load, not
 * only at boot. Installs the file-load pump, sets up the memory-arena table,
 * points the dynamic-texture VRAM cursor at the static-texture base, fills
 * the moby-class / tie / shrub lookup tables with their empty patterns
 * (-1 = unused slot, 0 = no size / no VRAM), rebuilds screen geometry and the
 * camera projection, then grants the default gadget loadout. When the player
 * has progress, re-marks level (progress % 28) as available. Clears
 * D_1A9A88 last. No params, no return value.
 *
 * Symbol views (see the header's -G8 rules): the four scalars are read and
 * written with the adjacent lui/%lo macro shape, so they carry `.extern ,16`
 * overrides; g_playerProgress is read ABSOLUTELY here but %gp_rel in
 * func_002911F0 (the mixed-attribution caveat), so this function reads it
 * through g_playerProgressAbs, a zero-offset assembler alias sized 16 bytes
 * (the #8386 alias pattern; relocations still name g_playerProgress). The
 * div-by-28 guard is cc1's own `div; beql; break 7` — move_fixup.sed spells
 * the trap `break 0,7` as SN ee-as did. The two VRAM-cursor stores are
 * written in reverse of the ROM's order because cc1 swaps them back.
 */
__asm__(".extern g_vramTextureBase, 16");
__asm__(".extern g_vramDynamicBase, 16");
__asm__(".extern g_vramAllocCursor, 16");
__asm__(".extern D_1A9A88, 16");
__asm__(".extern g_playerProgressAbs, 16\n\tg_playerProgressAbs = g_playerProgress");
extern s32 g_playerProgressAbs;         /* absolute view of g_playerProgress */

void func_00290FD0(void) {
    InstallFileLoadPump();
    func_002FCFC8();
    SetupMemoryArenaTable();

    /* Reset the dynamic-texture VRAM cursor to the static-texture base. */
    g_vramAllocCursor = g_vramTextureBase;
    g_vramDynamicBase = g_vramTextureBase;

    FillMemory32(&g_pLastOcclusionMask[0x28], 0x87654321, 0x10);
    FillMemory32(g_mobyClassSlotRemap,        -1, 0x2000);
    FillMemory32(g_mobyClassBounds,           -1, 0xF00);
    FillMemory32(g_mobyClassDataSizes,         0, 0xF0);
    FillMemory32(&g_tieClassQueue[0x200],     -1, 0x2000);
    FillMemory32(&g_tieVisibleClassList[0x200], -1, 0x800);
    FillMemory32(&g_tieTexVramTable[0x400],    0, 0x80);
    FillMemory32(&g_shrubClassTable[0x100],   -1, 0x1000);
    FillMemory32(&g_shrubRelightList[0x400],  -1, 0x400);
    FillMemory32(&g_shrubTexVramTable[0x400],  0, 0x40);

    InitScreenGeometry();
    BuildCameraProjection();
    func_00280FE0();
    ResetFrameArenas();
    RegisterParticleUpdateHandlers();
    InstallVif1DmacHandlers();
    func_002911F0();

    if (g_playerProgressAbs != 0) {
        MarkLevelAvailable(g_playerProgressAbs % 0x1C);
    }
    D_1A9A88 = 0;
}

/* func_00291148: validate the inventory display order (0xff-out entries no
 * longer ownable per func_00289190, Heli-Pack 0x1E exempt; returns 1 when
 * already clean). Blocked by the 8-byte-packed callee-save layout (saves
 * s0..s5+ra at sp+0x0..0x30; see header). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1907F0", func_00291148);
#else
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed
 * callee-save layout (s0..s5+ra). Walks the inventory display order and
 * 0xFF-clears any entry whose item class is no longer ownable. Returns 1 if
 * the order was already clean, 0 if it had to clear an entry. */
s32 func_00291148(void) {
    s32 i;
    s32 clean = 1;

    for (i = 0; i < INVENTORY_ORDER_COUNT; i++) {
        u8 entry = g_inventoryOrder[i];
        s32 itemClass;

        if (entry == 0xFF) {
            continue;                      /* terminator/empty slot */
        }
        itemClass = entry & 0x3F;
        if (itemClass == ITEM_HELIPACK) {
            continue;                      /* Heli-Pack is always exempt */
        }
        /* Clear the entry when the class is no longer ownable, or when it
         * resolves to class 0 (an invalid order byte). */
        if (func_00289190(itemClass) == 0 || itemClass == 0) {
            g_inventoryOrder[i] = 0xFF;
            clean = 0;
        }
    }
    return clean;
}
#endif

/**
 * func_002911F0 - grant the always-owned starting items the save is missing
 * (GrantDefaultGadgetLoadout), then re-validate and rebuild the display order.
 *
 * - 0x1E Heli-Pack: give + equip, display-order slot 0 = 0x5E (0x1E | 0x40
 *   owned), equip-slot[0] = 0x1E;
 * - 0x2A: give + equip, display-order slot 1 = 0x6A;
 * - 0x2F: give, equip-slot[2] = 0x2F;
 * - 0x06: set its owned and newly-acquired flags.
 * Then func_00291148 re-validates the order and func_002912B8 re-inserts
 * everything unlocked at g_playerProgress (read via %gp_rel, the small side of
 * the header's mixed-attribution caveat).
 *
 * Byte-exact on the sdk29 arm (task #895). The ROM reaches every byte with the
 * one-insn assembler macro (`lbu v0,sym+N` / `sb v1,sym+N` through $at), so
 * each table is indexed from a cc1-SMALL scalar alias of its symbol (the
 * #889 offset-0 construct): cc1 emits the macro and the file's
 * `.extern sym, 16` makes GNU as expand it absolutely. Indexing the unsized `u8[]` instead made
 * cc1 keep the table base in s0 (a 32-byte frame). The "pure register
 * colouring" residual of the earlier 99.40 attempt is fixed by binding the
 * byte-store constants to $3 and the equip-slot value to $2. The +6 flag pair
 * is written owned-first so cc1 issues the newly-acquired store first, as the
 * ROM does. The empty asm after the final call keeps it a `jal` (FACT #8177).
 */
#ifndef TARGET_NATIVE
__asm__(".extern g_inventoryOwned, 16");
__asm__(".extern g_inventoryOrder, 16");
__asm__(".extern g_inventoryNewFlag, 16");
__asm__(".extern g_itemEquipSlotTable, 16");
extern u8  g_inventoryOwnedSmall __asm__("g_inventoryOwned");
extern u8  g_inventoryOrderSmall __asm__("g_inventoryOrder");
extern u8  g_inventoryNewFlagSmall __asm__("g_inventoryNewFlag");
extern u32 g_itemEquipSlotSmall __asm__("g_itemEquipSlotTable");
#define OWNED_AT(i)      ((&g_inventoryOwnedSmall)[i])
#define ORDER_AT(i)      ((&g_inventoryOrderSmall)[i])
#define NEW_FLAG_AT(i)   ((&g_inventoryNewFlagSmall)[i])
#define EQUIP_SLOT_AT(i) ((&g_itemEquipSlotSmall)[i])
#define F1F0_IN_V0 __asm__("$2")
#define F1F0_IN_V1 __asm__("$3")
#else
#define OWNED_AT(i)      g_inventoryOwned[i]
#define ORDER_AT(i)      g_inventoryOrder[i]
#define NEW_FLAG_AT(i)   g_inventoryNewFlag[i]
#define EQUIP_SLOT_AT(i) g_itemEquipSlotTable[i]
#define F1F0_IN_V0
#define F1F0_IN_V1
#endif
void func_002911F0(void) {
    if (OWNED_AT(0x1E) == 0) {
        register s32 orderEntry F1F0_IN_V1;
        register s32 equipItem F1F0_IN_V0;

        GiveInventoryItem(0x1E);
        func_002AE6C8(0x1E);
        orderEntry = 0x5E; /* 0x1E | 0x40 (owned) */
        equipItem = 0x1E;
        ORDER_AT(0) = orderEntry;
        EQUIP_SLOT_AT(0) = equipItem;
    }
    if (OWNED_AT(0x2A) == 0) {
        register s32 orderEntry F1F0_IN_V1;

        GiveInventoryItem(0x2A);
        func_002AE6C8(0x2A);
        orderEntry = 0x6A; /* 0x2A | 0x40 (owned) */
        ORDER_AT(1) = orderEntry;
    }
    if (OWNED_AT(0x2F) == 0) {
        GiveInventoryItem(0x2F);
        EQUIP_SLOT_AT(2) = 0x2F;
    }
    if (OWNED_AT(0x06) == 0) {
        OWNED_AT(0x06) = 1;
        NEW_FLAG_AT(0x06) = 1;
    }

    func_00291148();
    func_002912B8(g_playerProgress);
    __asm__ __volatile__(""); /* sibling-call guard: the ROM keeps the jal */
}
