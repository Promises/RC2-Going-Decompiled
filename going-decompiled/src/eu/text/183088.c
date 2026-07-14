#include "common.h"

/*
 * text/183088 — EU (SCES_516.07) twin of USA text/183178: the tiny
 * unit-conversion / timer-helper TU. Same source as the USA sibling; only the
 * gp-relative small-data scale globals sit at different EU addresses
 * (D_1A7990..D_1A79AC instead of USA D_1A7910..D_1A792C). Built at -O2 -G8
 * (per-unit GFLAG override in tools/ee/objdiff_build.sh: "eu/text/183088 ->
 * USA 183178 twin"); objdiff masks the gp/reloc target deltas, so the C bodies
 * are region-agnostic — only the extern global NAMES are retargeted to EU.
 *
 * The TU is three families of paired helpers over per-unit float scale
 * factors: for each sdata float scale a `scale * x` float helper and a
 * round-to-nearest-int twin, plus hand-written countdown/decay primitives.
 *
 * Region-delta note: the EU splat boundary merged the 0x38-byte handwritten
 * stub block with the FIRST scale helper (D_1A7990 * x) into a single glabel
 * func_00283108. Because that glabel contains the handwritten stub words it
 * cannot be expressed as pure C, so unlike the USA build (where the stub and
 * the first helper are separate glabels and the helper matches) the EU build
 * holds func_00283108 as INCLUDE_ASM. The four remaining standalone scale
 * helpers port byte-exact.
 */

/* Per-unit scale factors (EU sdata, gp-relative). EU addresses of the USA
 * D_1A7910.. cluster; initial values measured in the USA rom carry over. */
extern f32 D_1A7990; /* = 1.0    (time-scale a) */
extern f32 D_1A7994; /* = 1.0    (time-scale b) */
extern f32 D_1A7998; /* = 1.0    (time-scale c) */
extern f32 D_1A799C; /* = 1/60   (frames -> seconds @60Hz) */
extern f32 D_1A79A0; /* = 1/3600 (frames -> minutes @60Hz) */

/* func_00283108: 0x38-byte handwritten stub block (paired `addiu $sp,+0xN` / nop
 * words, no prologue/return — not compiler output), split off via the
 * symbol_addrs pin so the first scale helper below starts clean. Kept as raw
 * asm (un-C-able stub words). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/183088", func_00283108);

/** func_00283140 (USA func_00283230): scale x by the D_1A7990 conversion factor.
 * Recovered from the EU stub-fusion mis-split. */
f32 func_00283140(f32 x) {
    return D_1A7990 * x;
}

/* func_00283150: (s32)((f32)x * D_1A7990 + 0.5f) rounded-conversion twin —
 * blocked by the adda.s/madd.s ACC-constant fused-madd codegen wall (the
 * pinned cc1 only emits mul.s + add.s). Same wall as the USA sibling. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/183088", func_00283150);
#else
/* TODO(match): functional equivalent - not byte-exact; adda.s/madd.s ACC-constant
 * rounding idiom this cc1 won't emit (same wall as USA func_00283240). */
s32 func_00283150(s32 x) {
    return (s32)((f32)x * D_1A7990 + 0.5f);
}
#endif

/** Scale x by the D_1A7994 conversion factor. */
f32 func_00283178(f32 x) {
    return D_1A7994 * x;
}

/* func_00283188: rounded conversion by D_1A7994 — same adda.s/madd.s wall. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/183088", func_00283188);
#else
/* TODO(match): functional equivalent - not byte-exact; adda.s/madd.s wall
 * (same as USA func_00283278). */
s32 func_00283188(s32 x) {
    return (s32)((f32)x * D_1A7994 + 0.5f);
}
#endif

/** Scale x by the D_1A7998 conversion factor. */
f32 func_002831B0(f32 x) {
    return D_1A7998 * x;
}

/* func_002831C0: rounded conversion by D_1A7998 — same adda.s/madd.s wall. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/183088", func_002831C0);
#else
/* TODO(match): functional equivalent - not byte-exact; adda.s/madd.s wall
 * (same as USA func_002832B0). */
s32 func_002831C0(s32 x) {
    return (s32)((f32)x * D_1A7998 + 0.5f);
}
#endif

/** Convert x from frames to seconds (x * 1/60). */
f32 func_002831E8(f32 x) {
    return D_1A799C * x;
}

/** Convert x from frames to minutes (x * 1/3600). */
f32 func_002831F8(f32 x) {
    return D_1A79A0 * x;
}

/* func_00283208: hand-written s32 countdown step (pmaxw + addi). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/183088", func_00283208);
#else
/* TODO(match): functional equivalent - not byte-exact; handwritten pmaxw + addi
 * countdown form (USA func_002832F8). Steps an s32 timer at *p: if already 0 -> 1
 * (idle); else clamp to >=1, decrement, store; -> 0 while running, 2 on expiry. */
s32 func_00283208(s32 *p) {
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

/* func_00283238: hand-written s16 countdown step (pmaxw + addi). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/183088", func_00283238);
#else
/* TODO(match): functional equivalent - not byte-exact; handwritten pmaxw + addi
 * countdown (USA func_00283328). s16 twin of func_00283208: same idle/run/expire
 * return triple (1 idle, 0 running, 2 on expiry). */
s32 func_00283238(s16 *p) {
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

/* func_00283268: hand-written u8 countdown step (pmaxw + addi). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/183088", func_00283268);
#else
/* TODO(match): functional equivalent - not byte-exact; handwritten pmaxw + addi
 * countdown (USA func_00283358). u8 twin of func_00283208: value is zero-extended
 * (lbu) so the clamp only matters when v==0 -> idle return 1; else decrement,
 * store, return 0 while running / 2 on expiry. */
s32 func_00283268(u8 *p) {
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

/* func_00283298: hand-written float countdown step against the D_1A79AC
 * threshold (addi/sub handwritten forms). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/183088", func_00283298);
#else
/* TODO(match): functional equivalent - not byte-exact; handwritten dsll32/slti
 * sign-bit test + sub return form (USA func_00283388). Float countdown timer at *p
 * against threshold D_1A79AC:
 *   - if *p > threshold: subtract threshold, store, return 0 (still running);
 *   - else: snap *p to 0.0, return a sign-of-remaining code: the original
 *     reinterprets the float bits, shifts them into the high word and signed-
 *     compares (slti ,1) so the test is true iff the value was negative or exactly
 *     +0.0 -> returns (test ? 1 : 0) - 2, i.e. -1 for non-positive remaining, -2
 *     for a strictly positive remaining (0 < *p <= threshold). */
extern f32 D_1A79AC; /* = 1/60 (float countdown threshold) */
s32 func_00283298(f32 *p) {
    f32 v = *p;
    f32 thr = D_1A79AC;
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
