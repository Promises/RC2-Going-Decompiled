#include "common.h"

/*
 * EU mirror of USA text/1907F0 (this is the EU file-offset twin text/190808 —
 * see config/eu split). Level-init / screen-fade helper TU; original separate
 * translation unit built at nonzero -G, so its small-data globals are accessed
 * via %gp_rel($gp). The matcher builds THIS unit at -O2 -G8 -fno-gcse (the
 * per-unit GFLAG override in tools/ee/objdiff_build.sh / diff.sh / build.sh);
 * every other text unit stays -O2 -G0.
 *
 * The C bodies are region-agnostic (objdiff masks gp/reloc deltas); the only
 * EU-specific work is retargeting the referenced global/function NAMES to their
 * EU symbols. EU sdata cluster mirrors USA D_1A9000.. at D_1A90B0..; see the
 * per-function notes for the delta. Functions left INCLUDE_ASM are blocked for
 * the same toolchain reasons documented on the USA twin (src/usa/text/1907F0.c).
 *
 * -G8 extern-sizing rules: a complete extern object of size <= 8 bytes is
 * placed in small data (gp-relative access), so the gp-relative globals below
 * are declared as small scalars.
 */

/* Small-data globals (gp-relative; complete <=8-byte declarations so -G8 places
 * them in small data). EU addresses; USA equivalent in the comment. */
extern s32 D_1A90B0;  /* USA D_1A9000 — fade/transition frame countdown */
extern s32 D_1A8DF0;  /* USA D_1A8D40 — shared status value from the pump callbacks */
extern s32 D_1A90C8;  /* USA D_1A9018 — tint hold-this-frame request flag */
extern s32 D_1A90C4;  /* USA D_1A9014 — full-screen tint level 0..10 */
extern s32 D_1A90CC;  /* USA D_1A901C — cleared-only flag (set elsewhere) */
extern s32 D_1A90D0;  /* USA D_1A9020 — small state machine 0..3 (3 = done) */

/* cc1-small / assembler-absolute scalar: top-level game state id (0 = in-game).
 * Named the same in both regions (EU 0x1A8C60, USA 0x1A8BB0). */
__asm__(".extern g_nGameState, 16");
extern s32 g_nGameState;

extern s32 func_00283208(s32 *counter);   /* EU twin of USA func_002832F8 (shared countdown step) */
extern void func_0029C1A8(s32 arg0);       /* EU twin of USA func_0029C600 */
extern s32 func_0027E2A8(s32 r, s32 g, s32 b, s32 a);  /* EU twin of USA DrawFullScreenTint */

/* func_00290888: EU twin of USA func_00290878 (fade-request entry). Blocked by
 * the 8-byte-packed callee-save layout + sibling-call tail (see USA twin). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/190808", func_00290888);

/* func_00290938: EU twin of USA func_00290920 (fade draw callback). Blocked by
 * the 8-byte-packed callee-save layout (see USA twin). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/190808", func_00290938);

/**
 * Fade pump status callback (EU twin of USA func_00290EA0): while in-game
 * (g_nGameState == 0), step the countdown; when it expires, stop the pump via
 * func_0029C1A8(1). Returns the shared status value.
 */
s32 func_00290EA8(void) {
    if (g_nGameState == 0) {
        if (func_00283208(&D_1A90B0)) {
            func_0029C1A8(1);
        }
    }
    return D_1A8DF0;
}

/* func_00290EE8: EU twin of USA func_00290EE8 (request tint hold this frame,
 * return 1). Blocked: the original cc1 reuses one `li v0,1` for both store and
 * return; the pinned cc1 materialises two (see USA twin). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/190808", func_00290EE8);

/**
 * Per-frame full-screen tint pump (EU twin of USA func_00290EF8): raise the tint
 * level toward 10 while requested (D_1A90C8), decay toward 0 otherwise, then draw
 * a black full-screen tint with alpha = level * 0.1 * 48. Forwards the draw
 * call's status when it draws (falling through otherwise, like the original).
 */
s32 func_00290F00(void) {
    s32 alpha;

    if (D_1A90C8) {
        s32 v = D_1A90C4 + 1;
        D_1A90C8 = 0;
        D_1A90C4 = v;
        if (!(v < 11)) {
            D_1A90C4 = 10;
        }
    } else {
        s32 v = D_1A90C4 - 1;
        D_1A90C4 = v;
        if (v < 0) {
            D_1A90C4 = 0;
        }
    }
    alpha = (f32)D_1A90C4 * 0.1f * 48.0f;
    if (alpha) {
        return func_0027E2A8(0, 0, 0, alpha);
    }
}

/* func_00290F98: EU twin of USA func_00290F90 — 4-byte handwritten patch-stub
 * fragment (`sw $v0, %gp_rel(D_1AB35C)($gp)` with no return), not compiler
 * output. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/190808", func_00290F98);

/** Clear the D_1A90CC flag (EU twin of USA func_00290F98). */
void func_00290FA0(void) {
    D_1A90CC = 0;
}

/** Reset the D_1A90D0 state machine (EU twin of USA func_00290FA0). */
void func_00290FA8(void) {
    D_1A90D0 = 0;
}

/** Kick the D_1A90D0 state machine (0 -> 1; EU twin of USA func_00290FA8). */
void func_00290FB0(void) {
    if (D_1A90D0 == 0) {
        D_1A90D0 = 1;
    }
}

/** True when the D_1A90D0 state machine reached its final state (3).
 *  EU twin of USA func_00290FC0. */
s32 func_00290FC8(void) {
    return D_1A90D0 == 3;
}

/* func_00290FD8: EU twin of USA func_00290FD0 (one-shot render/moby-table init).
 * Blocked by the `break 0,7` div-guard encoding + the absolute %hi/%lo read of
 * g_playerProgress that the small declaration cannot express (see USA twin). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/190808", func_00290FD8);

/* func_00291150: EU twin of USA func_00291148 (validate inventory display order).
 * Blocked by the 8-byte-packed callee-save layout (see USA twin). */
#ifdef TARGET_NATIVE
#define INVENTORY_ORDER_COUNT 0x20
#define ITEM_HELIPACK         0x1E
extern u8  g_inventoryOrder[];            /* EU 0x1A7BF0 (USA 0x1A7B70, +0x80) */
extern s32 func_00289080(s32 itemClass); /* EU twin of USA func_00289190 (ownable check) */
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/190808", func_00291150);
#else
/* TODO(match): functional equivalent - not byte-exact; 8-byte-packed callee-save
 * layout. Walks the inventory display order and 0xFF-clears any entry whose item
 * class is no longer ownable. Returns 1 if already clean, 0 if it cleared one. */
s32 func_00291150(void) {
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
        if (func_00289080(itemClass) == 0 || itemClass == 0) {
            g_inventoryOrder[i] = 0xFF;
            clean = 0;
        }
    }
    return clean;
}
#endif

/* func_002911F8: EU twin of USA func_002911F0 (grant always-owned starting
 * items, re-validate display order). Blocked by register-coloring residue (see
 * USA twin). */
#ifdef TARGET_NATIVE
extern u8  g_inventoryOwned[];            /* EU 0x1A7B80 (USA 0x1A7B00, +0x80) */
extern u8  g_inventoryNewFlag[];          /* EU 0x1A7BB8 (USA 0x1A7B38, +0x80) */
extern u32 g_itemEquipSlotTable[];        /* EU 0x1A7418 (USA 0x1A7398, +0x80) */
extern s32 g_playerProgress;
extern void GiveInventoryItem(s32 itemId);
extern void func_002AE3C0(s32 itemId);    /* EU twin of USA func_002AE6C8 (EquipGadgetItem) */
extern void func_002912C0(s32 progress);  /* EU twin of USA func_002912B8 (re-insert unlocked) */
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/190808", func_002911F8);
#else
/* TODO(match): functional equivalent - not byte-exact; 99.40% wall is pure
 * register-coloring of the three byte-store temps (see USA twin).
 * GrantDefaultGadgetLoadout: grant the always-owned starting items the save is
 * missing, then re-validate and rebuild the display order. */
void func_002911F8(void) {
    /* 0x1E Heli-Pack: owned bit in display-order slot 0, equip-slot[0]. */
    if (g_inventoryOwned[0x1E] == 0) {
        GiveInventoryItem(0x1E);
        func_002AE3C0(0x1E);
        g_inventoryOrder[0]      = 0x5E;   /* 0x1E | 0x40 (owned) */
        g_itemEquipSlotTable[0]  = 0x1E;
    }
    /* 0x2A: owned bit in display-order slot 1. */
    if (g_inventoryOwned[0x2A] == 0) {
        GiveInventoryItem(0x2A);
        func_002AE3C0(0x2A);
        g_inventoryOrder[1]      = 0x6A;   /* 0x2A | 0x40 (owned) */
    }
    /* 0x2F: equip-slot[2]. */
    if (g_inventoryOwned[0x2F] == 0) {
        GiveInventoryItem(0x2F);
        g_itemEquipSlotTable[2]  = 0x2F;
    }
    /* 0x06: ensure the owned + newly-acquired flag pair is set. */
    if (g_inventoryOwned[0x06] == 0) {
        g_inventoryNewFlag[0x06] = 1;
        g_inventoryOwned[0x06]   = 1;
    }

    func_00291150();
    func_002912C0(g_playerProgress);
}
#endif
