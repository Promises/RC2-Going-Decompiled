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
extern f32 D_1A7994; /* = 1.0    (time-scale b) */
extern f32 D_1A7998; /* = 1.0    (time-scale c) */
extern f32 D_1A799C; /* = 1/60   (frames -> seconds @60Hz) */
extern f32 D_1A79A0; /* = 1/3600 (frames -> minutes @60Hz) */

/* func_00283108: EU merge of the 0x38-byte handwritten stub block (paired
 * `addiu $sp,+0xN` / nop words, no prologue/return — not compiler output) with
 * the first scale helper (D_1A7990 * x). The stub words make it un-C-able, so
 * the leading scale helper that matches in USA (func_00283230) cannot be
 * carved out here. Kept as raw asm. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/183088", func_00283108);

/* func_00283150: (s32)((f32)x * D_1A7990 + 0.5f) rounded-conversion twin —
 * blocked by the adda.s/madd.s ACC-constant fused-madd codegen wall (the
 * pinned cc1 only emits mul.s + add.s). Same wall as the USA sibling. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/183088", func_00283150);

/** Scale x by the D_1A7994 conversion factor. */
f32 func_00283178(f32 x) {
    return D_1A7994 * x;
}

/* func_00283188: rounded conversion by D_1A7994 — same adda.s/madd.s wall. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/183088", func_00283188);

/** Scale x by the D_1A7998 conversion factor. */
f32 func_002831B0(f32 x) {
    return D_1A7998 * x;
}

/* func_002831C0: rounded conversion by D_1A7998 — same adda.s/madd.s wall. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/183088", func_002831C0);

/** Convert x from frames to seconds (x * 1/60). */
f32 func_002831E8(f32 x) {
    return D_1A799C * x;
}

/** Convert x from frames to minutes (x * 1/3600). */
f32 func_002831F8(f32 x) {
    return D_1A79A0 * x;
}

/* func_00283208: hand-written s32 countdown step (pmaxw + addi). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/183088", func_00283208);

/* func_00283238: hand-written s16 countdown step (pmaxw + addi). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/183088", func_00283238);

/* func_00283268: hand-written u8 countdown step (pmaxw + addi). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/183088", func_00283268);

/* func_00283298: hand-written float countdown step against the D_1A79AC
 * threshold (addi/sub handwritten forms). */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/183088", func_00283298);
