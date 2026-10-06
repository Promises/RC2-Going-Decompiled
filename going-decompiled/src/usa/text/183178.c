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
extern f32 D_1A792C; /* = 1/60 (float countdown threshold, func_00283388) */

/* func_002831F8: 0x38 bytes of hand-written 8-byte stub entries (paired
 * `addiu $sp, +0xN` / nop words, one all-nop pair, no prologue, no return) —
 * not compiler output and not a callable function (no jr/return), so there is
 * no C / functional-equivalent body to write. Documented handwritten fragment;
 * kept as raw asm for both build targets. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/183178", func_002831F8);

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
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183178", func_00283240);
#else
/* TODO(match): functional equivalent - not byte-exact; adda.s/madd.s
 * ACC-constant rounding idiom this cc1 won't emit. */
s32 func_00283240(s32 x) {
    return (s32)((f32)x * D_1A7910 + 0.5f);
}
#endif

/** Scale x by the D_1A7914 conversion factor. */
f32 func_00283268(f32 x) {
    return D_1A7914 * x;
}

/* func_00283278: rounded conversion by D_1A7914 — same adda.s/madd.s
 * ACC-constant wall as func_00283240. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183178", func_00283278);
#else
/* TODO(match): functional equivalent - not byte-exact; adda.s/madd.s wall. */
s32 func_00283278(s32 x) {
    return (s32)((f32)x * D_1A7914 + 0.5f);
}
#endif

/** Scale x by the D_1A7918 conversion factor. */
f32 func_002832A0(f32 x) {
    return D_1A7918 * x;
}

/* func_002832B0: rounded conversion by D_1A7918 — same adda.s/madd.s
 * ACC-constant wall as func_00283240. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183178", func_002832B0);
#else
/* TODO(match): functional equivalent - not byte-exact; adda.s/madd.s wall. */
s32 func_002832B0(s32 x) {
    return (s32)((f32)x * D_1A7918 + 0.5f);
}
#endif

/** Convert x from frames to seconds (x * 1/60). */
f32 func_002832D8(f32 x) {
    return D_1A791C * x;
}

/** Convert x from frames to minutes (x * 1/3600). */
f32 func_002832E8(f32 x) {
    return D_1A7920 * x;
}

/* TickCountdownTimer: hand-written s32 countdown step (pmaxw + addi, "handwritten
 * instruction" forms this compiler never emits from C). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183178", TickCountdownTimer);
#else
/* TODO(match): functional equivalent - not byte-exact; handwritten pmaxw +
 * `addi` countdown form. Steps an s32 timer at *p: if already 0 -> 1 (idle);
 * else clamp to >=1, decrement, store; -> 0 while still running, 2 on the
 * frame it hits 0. */
s32 TickCountdownTimer(s32 *p) {
    s32 v = *p;
    if (v == 0) {
        return 1;
    }
    if (v < 1) {
        v = 1;
    }
    v -= 1;
    *p = v;
    if (v > 0) {
        return 0;
    }
    return 2;
}
#endif

/* func_00283328: hand-written s16 countdown step (pmaxw + addi). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183178", func_00283328);
#else
/* TODO(match): functional equivalent - not byte-exact; handwritten pmaxw +
 * `addi` countdown form. s16 twin of TickCountdownTimer: same idle/run/expire
 * return triple (1 idle, 0 running, 2 on expiry). */
s32 func_00283328(s16 *p) {
    s32 v = *p;
    if (v == 0) {
        return 1;
    }
    if (v < 1) {
        v = 1;
    }
    v -= 1;
    *p = (s16)v;
    if (v > 0) {
        return 0;
    }
    return 2;
}
#endif

/* func_00283358: hand-written u8 countdown step (pmaxw + addi). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183178", func_00283358);
#else
/* TODO(match): functional equivalent - not byte-exact; handwritten pmaxw +
 * `addi` countdown form. u8 twin of TickCountdownTimer: value is zero-extended
 * (lbu) so the pmaxw clamp only matters when v==0 -> idle return 1; otherwise
 * decrement, store, return 0 while running / 2 on expiry. */
s32 func_00283358(u8 *p) {
    s32 v = *p;
    if (v == 0) {
        return 1;
    }
    if (v < 1) {
        v = 1;
    }
    v -= 1;
    *p = (u8)v;
    if (v > 0) {
        return 0;
    }
    return 2;
}
#endif

/* func_00283388: hand-written float countdown step against the D_1A792C
 * threshold (addi/sub "handwritten instruction" forms). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183178", func_00283388);
#else
/* TODO(match): functional equivalent - not byte-exact; handwritten
 * dsll32/slti sign-bit test + `sub` return form. Float countdown timer at *p
 * against threshold D_1A792C:
 *   - if *p > threshold: subtract threshold, store, return 0 (still running);
 *   - else: snap *p to 0.0, return a sign-of-remaining code: the original
 *     reinterprets the float bits, shifts them into the high word and signed-
 *     compares (slti ... ,1) so the test is true iff the value was negative or
 *     exactly +0.0 -> returns (test ? 1 : 0) - 2, i.e. -1 for non-positive
 *     remaining, -2 for a strictly positive remaining (0 < *p <= threshold). */
s32 func_00283388(f32 *p) {
    f32 v = *p;
    f32 thr = D_1A792C;
    if (v <= thr) {
        union { f32 f; s32 i; } u;
        s64 hi;
        s32 test;
        u.f = v;
        *p = 0.0f;
        hi = (s64)u.i << 32;       /* dsll32 of the sign-extended float bits */
        test = (hi < 1) ? 1 : 0;   /* slti $2, $1, 1 */
        return test - 2;
    }
    *p = v - thr;
    return 0;
}
#endif
