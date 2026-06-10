#include "common.h"

/*
 * text/183178 — tiny unit-conversion/timer-helper TU (carved out of the .text
 * asm segment on 2026-06-10, TU-cluster scan candidate A). This is an
 * original separate translation unit built at nonzero -G: its small-data
 * globals (sdata cluster D_1A7910..D_1A792C) are accessed uniformly via
 * %gp_rel($gp), which -G0 cannot express. The matcher builds THIS unit at
 * -O2 -G8 (see the per-unit GFLAG override in tools/ee/objdiff_build.sh /
 * diff.sh / build.sh); every other text unit stays -O2 -G0.
 *
 * -G8 rule for externs in this file: a complete extern object of size <= 8
 * bytes is placed in small data (gp-relative access); anything that the
 * original accesses with absolute %hi/%lo pairs must be declared with an
 * incomplete array type (extern T sym[];) so cc1 cannot prove it small.
 *
 * The TU is three families of paired helpers over per-unit scale factors:
 * for each sdata float scale there is a `scale * x` float helper and a
 * round-to-nearest-int helper, plus hand-written countdown/decay primitives.
 * Measured initial values (rom @0x1A7910): 1.0, 1.0, 1.0, 1/60, 1/3600
 * (and 1/216000 at +0x14, 1/60 again as the D_1A792C countdown threshold) —
 * i.e. a frame/time-conversion TU: three unit time-scale factors plus
 * frames-to-seconds / frames-to-minutes converters at the 60 Hz frame rate.
 */

/* Per-unit scale factors (sdata, gp-relative). Initial values measured in
 * the rom; identities/writers not yet traced, so the D_ names stay. */
extern f32 D_1A7910; /* = 1.0  (time-scale a) */
extern f32 D_1A7914; /* = 1.0  (time-scale b) */
extern f32 D_1A7918; /* = 1.0  (time-scale c) */
extern f32 D_1A791C; /* = 1/60   (frames -> seconds @60Hz) */
extern f32 D_1A7920; /* = 1/3600 (frames -> minutes @60Hz) */

/* func_002831F8: 0x38 bytes of hand-written 8-byte stub entries (paired
 * `addiu $sp, +0xN` / nop words, one all-nop pair, no prologue, no return) —
 * not compiler output, no C can produce it. Kept as raw asm. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183178", func_002831F8);

/** Scale x by the D_1A7910 conversion factor. */
f32 func_00283230(f32 x) {
    return D_1A7910 * x;
}

/* func_00283240: (s32)((f32)x * D_1A7910 + 0.5f) rounded-conversion twin of
 * func_00283230 — blocked by a fused-madd codegen wall: the original emits
 * `li.s $f3,0.25; adda.s $f3,$f3; madd.s` (ACC loaded as half the rounding
 * constant, then doubled — a later SN ee-gcc ACC-constant idiom). The pinned
 * 2.9-ee-991111 cc1 (and the 2.95.2 one) only ever emits mul.s + add.s here;
 * no flag (-mmad, -ffast-math, -mcpu=r5900, -O3) or source shape
 * (+0.25f+0.25f) produces adda/madd. Same wall for the other two rounding
 * twins below. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183178", func_00283240);

/** Scale x by the D_1A7914 conversion factor. */
f32 func_00283268(f32 x) {
    return D_1A7914 * x;
}

/* func_00283278: rounded conversion by D_1A7914 — same adda.s/madd.s
 * ACC-constant wall as func_00283240. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183178", func_00283278);

/** Scale x by the D_1A7918 conversion factor. */
f32 func_002832A0(f32 x) {
    return D_1A7918 * x;
}

/* func_002832B0: rounded conversion by D_1A7918 — same adda.s/madd.s
 * ACC-constant wall as func_00283240. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183178", func_002832B0);

/** Convert x from frames to seconds (x * 1/60). */
f32 func_002832D8(f32 x) {
    return D_1A791C * x;
}

/** Convert x from frames to minutes (x * 1/3600). */
f32 func_002832E8(f32 x) {
    return D_1A7920 * x;
}

/* func_002832F8: hand-written s32 countdown step (pmaxw + addi, "handwritten
 * instruction" forms this compiler never emits from C). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183178", func_002832F8);

/* func_00283328: hand-written s16 countdown step (pmaxw + addi). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183178", func_00283328);

/* func_00283358: hand-written u8 countdown step (pmaxw + addi). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183178", func_00283358);

/* func_00283388: hand-written float countdown step against the D_1A792C
 * threshold (addi/sub "handwritten instruction" forms). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183178", func_00283388);
