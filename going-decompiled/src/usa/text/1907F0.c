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
 * wall). Only one side is expressible per declaration; g_playerProgress is
 * declared small here (favouring func_002911F0), so func_00290FD0 stays
 * INCLUDE_ASM (it is additionally blocked by the `break 0,7` div-by-28
 * encoding this toolchain cannot reproduce).
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
extern s32 D_1A9000;  /* fade/transition frame countdown (stepped by func_002832F8) */
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

extern s32 func_002832F8(s32 *counter);   /* shared countdown step (text/183178) */
extern void func_0029C600(s32 arg0);
extern s32 DrawFullScreenTint(s32 r, s32 g, s32 b, s32 a);

/* func_00290870: 8 bytes of inter-function padding (addiu $sp,+0x90 / nop)
 * split off via symbol_addrs size:0x8; the real function begins at
 * func_00290878. Pure padding, no C. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1907F0", func_00290870);

/* func_00290878: fade-request entry (take ownership of the fade state slot
 * D_1A9004, register the func_00290EA0/func_00290920 pump callbacks, then
 * store colours/progress and arm the 10-frame countdown at D_1A9000).
 * Body reproduces 1:1 at 84% — blocked by the 8-byte-packed callee-save
 * layout (saves s0/s1/s2/ra/f20 at sp+0x0..0x20; see header) plus a void
 * tail call this cc1 sibling-call-optimises. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1907F0", func_00290878);

/* func_00290920: the fade draw callback (rebuilds the camera projection with
 * a pinched FOV, draws the letterbox/fade rectangles via func_0027E4D0 and
 * the two colour overlays via func_002846E8/func_003017F8). Blocked by the
 * 8-byte-packed callee-save layout (s0..s5+ra+f20..f25 packed at sp+0x0..,
 * see header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1907F0", func_00290920);

/**
 * Fade pump status callback: while in-game (g_nGameState == 0), step the
 * D_1A9000 countdown; when it expires, stop the pump via func_0029C600(1).
 * Returns the shared D_1A8D40 status value.
 */
s32 func_00290EA0(void) {
    if (g_nGameState == 0) {
        if (func_002832F8(&D_1A9000)) {
            func_0029C600(1);
        }
    }
    return D_1A8D40;
}

/* func_00290EE0: 4 bytes of inter-function fill (`addiu $sp,+0x30`), no
 * prologue/return — not compiler output, no C can produce it. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1907F0", func_00290EE0);

/* func_00290EE8: request the full-screen tint to hold/raise this frame and
 * return 1 (`D_1A9018 = 1; return 1;`). Best attempt 63% - the original
 * (later SN) cc1 reuses ONE `li v0,1` for both the store and the return
 * value; the pinned cc1 always materialises two (no source shape or flag
 * found that shares them; a volatile re-read shape adds an lw instead).
 * Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1907F0", func_00290EE8);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1907F0", func_00290F90);

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

/* func_00290FD0: one-shot render/moby-table init (fills the moby class
 * tables, vram bases, screen geometry, then MarkLevelAvailable(progress %
 * 28)). Blocked twice over: the `break 0,7` div-guard encoding (documented
 * toolchain wall) AND the absolute %hi/%lo read of g_playerProgress that
 * this TU's small declaration cannot express (see header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1907F0", func_00290FD0);

/* func_00291148: validate the inventory display order (0xff-out entries no
 * longer ownable per func_00289190, Heli-Pack 0x1E exempt; returns 1 when
 * already clean). Blocked by the 8-byte-packed callee-save layout (saves
 * s0..s5+ra at sp+0x0..0x30; see header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1907F0", func_00291148);

/* func_002911F0: grant the always-owned starting items if missing (0x1E
 * Heli-Pack with its order entry + equip slot, 0x2A with its order entry,
 * 0x2F with its equip slot, the 0x06 flag pair), then re-validate the
 * display order (func_00291148) and re-insert everything unlocked at
 * g_playerProgress (func_002912B8 — read via %gp_rel, the small side of the
 * header's mixed-attribution caveat). Best attempt 99.40% (volatile
 * condition bytes pin the delay slots, the value-returning tail fixes the
 * sibling call — every insn/reloc exact) — the residue is pure register
 * COLORING: the original (later SN) cc1 colours the three single-use
 * byte-store temps (0x5E/0x6A) v1-then-v0, the pinned cc1 v0-then-v1; no
 * source shape found that flips it. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1907F0", func_002911F0);
