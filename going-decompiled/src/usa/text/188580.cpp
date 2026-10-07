#include "common.h"

/*
 * text/188580 — camera-aux helper TU (carved out of the .text asm segment on
 * 2026-06-10, TU-cluster scan candidate B). This is an original separate
 * translation unit built at nonzero -G: its small-data globals (sdata cluster
 * D_1A8A60..D_1A8AE0) are accessed uniformly via %gp_rel($gp), which -G0
 * cannot express. The matcher builds THIS unit at -O2 -G8 (see the per-unit
 * GFLAG override in tools/ee/objdiff_build.sh / diff.sh / build.sh); every
 * other text unit stays -O2 -G0.
 *
 * -G8 rule for externs in this file: a complete extern object of size <= 8
 * bytes is placed in small data (gp-relative access); anything that the
 * original accesses with absolute %hi/%lo pairs must be declared with an
 * incomplete array type (extern T sym[];) or a complete type > 8 bytes so
 * cc1 cannot prove it small.
 */

/* Anonymous .bss state block at 0x1BAC00 (sized in symbol_addrs so the asm
 * resolves its fields to this symbol). +0x40 is compared against mode/state 7
 * and +0x58 stepped 1->2 by the helpers below; also driven by the screen/
 * cinematic code around func_00287140. Field semantics not yet traced. */
typedef struct {
    /* 0x00 */ u8 unk00[0x40];
    /* 0x40 */ s32 unk40; /* mode/state id; 7 gates the helpers below */
    /* 0x44 */ u8 unk44[0x14];
    /* 0x58 */ s32 unk58; /* sub-step: 1 -> 2 handshake */
    /* 0x5C */ u8 unk5C[0x28];
    /* 0x84 */ s16 unk84; /* counter/flag, reset by func_002888A8 */
} UnkCamAuxState;
extern UnkCamAuxState D_001BAC00;

/* Small-data globals (gp-relative; complete <=8-byte declarations so -G8
 * places them in small data). Semantics not yet traced. */
extern s32 D_1A8A60;
extern s32 D_1A8A64;

/* 16-byte aligned float vector / matrix-row type (matches text/183558's Vec4f).
 * Used only to type the camera-math externs and locals below; decays to f32*
 * so it interoperates with the plain-array globals. */
typedef f32 Vec4f[4] __attribute__((aligned(16)));

/* Camera state (see symbol_addrs): g_cameraPos is the world-position vec4 at
 * 0x1B52C0; g_cameraMatrix is the 3-row rotation matrix at 0x1B54F0
 * (== g_cameraPos + 0x230). D_001B1750 is a fixed anchor vec4. */
extern f32 g_cameraPos[4];
extern f32 g_cameraMatrix[12];
extern Vec4f D_001B1750;

/* g_cameraAnchor: the C spelling of D_001B1750 at a call argument.
 * ADDRESSING-MODEL DEVICE (RULING #8620, FACT #8036; EE arm only): the ROM
 * forms the anchor's address as an adjacent `lui $6 / addiu $6` pair at EVERY
 * use and forms it again after each call, which is the assembler's one-insn
 * `la` macro. cc1 prints `la` only for a symbol it believes small, so the EE
 * arm reaches the anchor through an 8-byte view, and `.extern D_001B1750, 16`
 * makes GNU as expand the macro absolutely. Through the full-size Vec4f cc1
 * instead keeps the address live in a callee-saved register across the calls
 * (one more save, a 4-register frame). Both lines emit no instruction and the
 * relocations name D_001B1750 itself; precedent 178E88.cpp's
 * g_blobShadowState view of g_blobShadowCount. */
#ifndef TARGET_NATIVE
__asm__(".extern D_001B1750, 16");
extern f32 g_cameraAnchor[2] __asm__("D_001B1750");
#else
#define g_cameraAnchor D_001B1750
#endif

/* func_00288748 length/angle scalars (gp-relative). D_1A8A78 is an integer
 * (loaded then cvt.s.w'd to a target length); D_1A8A80 is a float scale. */
extern s32 D_1A8A78;
extern f32 D_1A8A80;
/* D_1A8AE0: source vec4 for func_00283DA0 in the camera rebuild. */
extern Vec4f D_1A8AE0;
/* g_cameraRebuildSrc: the C spelling of D_1A8AE0 at a call argument.
 * ADDRESSING-MODEL DEVICE (RULING #8620; EE arm only): the ROM passes its
 * address as one `addiu $5,$28,%gp_rel(D_1A8AE0)` (it sits in the unit's
 * small-data cluster), and under -G8 cc1 emits that only for a symbol it
 * believes small, so the EE arm reaches it through an 8-byte view. Through
 * the 16-byte Vec4f cc1 forms it absolutely (lui/addiu, one word longer).
 * Emits no instruction; the relocation names D_1A8AE0 itself. */
#ifndef TARGET_NATIVE
extern f32 g_cameraRebuildSrc[2] __asm__("D_1A8AE0");
#else
#define g_cameraRebuildSrc D_1A8AE0
#endif
/* The camera matrix addressed from the camera position: g_cameraMatrix is
 * g_cameraPos + 0x230 bytes, and the ROM's first matrix argument in
 * func_00288600 is `addiu $5,$16,0x230` off the register already holding
 * &g_cameraPos (its last argument names g_cameraMatrix afresh). Spelled from
 * g_cameraPos on EE only; native keeps the named array. */
#ifndef TARGET_NATIVE
#define CAMERA_MATRIX_FROM_POS (g_cameraPos + 0x230 / sizeof(f32))
#else
#define CAMERA_MATRIX_FROM_POS g_cameraMatrix
#endif

/* VU0 vector-math helpers (text/183558) and quaternion/rotation builders. */
extern void Vec4SubVu0(Vec4f dst, const Vec4f a, const Vec4f b);
extern void Vec4AddVu0(Vec4f dst, const Vec4f a, const Vec4f b);
extern void Vec3CrossVu0(Vec4f dst, const Vec4f a, const Vec4f b);
extern f32  Vec3LengthVu0(const Vec4f v);
extern void Vec3RescaleToLenVu0(Vec4f dst, f32 len, const Vec4f src);
extern void func_00283DA0(Vec4f dst, const Vec4f src);   /* matrix-row builder */
extern void func_002840E8(Vec4f dst, const Vec4f a, const Vec4f b); /* 3x3 a*b */
extern void QuatToMatrix3(const Vec4f src, Vec4f dst);   /* quat -> 3x3 matrix */
extern void func_002ADCE0(Vec4f dst, f32 len, const Vec4f axis); /* axis -> quat; def 1A8180.c (dst, len, src) */
extern void func_002ABAE8(const f32 *p, f32 a, f32 b, f32 c, f32 d);

/* Reinterpret an IEEE-754 bit pattern as f32 (for the exact roll constants). */
static __inline__ f32 bits_to_f32(u32 bits) {
    union { u32 u; f32 f; } v;
    v.u = bits;
    return v.f;
}

/*
 * func_00288600: re-level the camera rotation (g_cameraMatrix) by a roll
 * about the axis between the current frame and the anchor direction.
 *   - forward = normalize(g_cameraPos - anchor)      (anchor = D_001B1750)
 *   - up      = func_00283DA0(D_1A8AE0); right = g_cameraMatrix * up (3x3)
 *   - cross   = right x forward; half = |cross| * 0.5
 *   - flag!=0: roll quaternion about cross by -half          [func_002ADCE0]
 *   - flag==0: func_002ABAE8(target, half, k0, k0, k1) first, then the roll
 *              quaternion about cross by -target[0]
 *   - g_cameraMatrix = QuatToMatrix3(roll) * g_cameraMatrix
 *   target  2-way roll state func_002ABAE8 updates (only target[0] is read
 *           back here, as the roll angle)
 *   flag    non-zero: roll straight by -half; zero: route it through target
 *   ->      nothing (g_cameraMatrix is rotated in place)
 * The quaternion is built in place over `cross` (the ROM passes sp+0x90 as
 * both dst and axis). k0 = 0x3A18825C (~5.818e-4), k1 = 0x3D567752 (~0.0524).
 * GUARD (task #1805): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback. On native it is plain C.
 * MATCHED on the s136os arm (task #1805). What closed it, from NOTE #8954's
 * 74/81 row (frame 0x90 vs the ROM's 0xF0): `up`, `right` and `roll` are 3x3
 * (0x30-byte) buffers, which gives the ROM's frame; the anchor and D_1A8AE0
 * views and CAMERA_MATRIX_FROM_POS give its addressing; and `half` is formed
 * once before the branch, as the ROM's mul.s in the beqz delay slot is.
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00288600)
S136OS_SLOT(func_00288600);
#else
void func_00288600(const Vec4f target, s32 flag) {
    Vec4f roll[3];    /* sp+0x00 (3x3) */
    Vec4f right[3];   /* sp+0x30 (3x3) */
    Vec4f up[3];      /* sp+0x60 (3x3) */
    Vec4f cross[1];   /* sp+0x90, also the roll quaternion */
    Vec4f forward[1]; /* sp+0xA0 */
    f32 half;

    Vec4SubVu0(forward[0], g_cameraPos, g_cameraAnchor);
    Vec3RescaleToLenVu0(forward[0], 1.0f, forward[0]);

    func_00283DA0(up[0], g_cameraRebuildSrc);
    func_002840E8(right[0], CAMERA_MATRIX_FROM_POS, up[0]);

    Vec3CrossVu0(cross[0], right[0], forward[0]);
    half = Vec3LengthVu0(cross[0]) * 0.5f;

    if (flag != 0) {
        func_002ADCE0(cross[0], -half, cross[0]);
    } else {
        func_002ABAE8(target, half,
                      bits_to_f32(0x3A18825C), bits_to_f32(0x3A18825C),
                      bits_to_f32(0x3D567752));
        func_002ADCE0(cross[0], -target[0], cross[0]);
    }

    QuatToMatrix3(cross[0], roll[0]);
    func_002840E8(g_cameraMatrix, roll[0], g_cameraMatrix);
}
#endif

/*
 * func_00288748: apply a roll step to the camera matrix. p[0] is an arcmin
 * delta; angle = p[0] * (pi/180/60) * D_1A8A80. Build a quaternion about the
 * camera right axis (matrix row 0) and pre-multiply it into g_cameraMatrix.
 *   p   the step vector; only p[0] (arcmin) is read
 *   ->  nothing (g_cameraMatrix is rotated in place)
 */
/* GUARD (task #1324): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00288748)
S136OS_SLOT(func_00288748);
#else
/* MATCHED on the s136os arm (task #1324): byte-exact solo under SN 2.95.3 v1.36
 * -fopt-stack, which packs s0/s1/ra at sp+0x40/0x48/0x50 as the ROM does (the
 * 16-byte-slot wall of cc1 2.9). The closing lever was the matrix SIZE: QuatToMatrix3
 * writes a 3x3 matrix (three Vec4 rows, 0x30 bytes), so `mat` is Vec4f[3] and the
 * frame is the ROM's 0x60; a single Vec4f gave a 0x40 frame. */
void func_00288748(const Vec4f p) {
    Vec4f quat;    /* sp+0x00 */
    Vec4f mat[3];  /* sp+0x10 (3x3) */
    f32 angle = p[0] * bits_to_f32(0x3998825C) * D_1A8A80;

    func_002ADCE0(quat, angle, g_cameraMatrix);
    QuatToMatrix3(quat, mat[0]);
    func_002840E8(g_cameraMatrix, mat[0], g_cameraMatrix);
}
#endif

/*
 * func_002887C0: nudge the camera position by 'offset' while keeping it at a
 * fixed distance from the anchor: re-centre g_cameraPos on the anchor
 * (D_001B1750), add the offset, rescale the result to length (f32)D_1A8A78,
 * and move it back out of anchor space.
 *   offset  world-space displacement (vec4) added in anchor space
 *   ->      nothing (g_cameraPos is updated in place)
 * GUARD (task #1805): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback. On native it is plain C.
 * MATCHED on the s136os arm (task #1805). The closing lever is the anchor's
 * addressing model (g_cameraAnchor, above): with the plain Vec4f the solo
 * compile held the address in $18 (4 saves, frame layout off from word 1,
 * NOTE #8954's 29/31 row).
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002887C0)
S136OS_SLOT(func_002887C0);
#else
void func_002887C0(const Vec4f offset) {
    Vec4SubVu0(g_cameraPos, g_cameraPos, g_cameraAnchor);
    Vec4AddVu0(g_cameraPos, g_cameraPos, offset);
    Vec3RescaleToLenVu0(g_cameraPos, (f32)D_1A8A78, g_cameraPos);
    Vec4AddVu0(g_cameraPos, g_cameraPos, g_cameraAnchor);
}
#endif

/** AdvanceInventoryOverlayToActive: in the in-game inventory/weapon-wheel overlay
 * (menu-overlay mode 7), advance the open sub-state 1->2 (shown/accept-input).
 * D_001BAC00 is the menu-overlay state block: .unk40 = live mode (== g_nMenuOverlayModeLive,
 * 7=inventory), .unk58 = sub-state (== g_nMenuOverlaySubState). */
void func_00288840(void) {
    UnkCamAuxState *state = &D_001BAC00;

    if (state->unk40 == 7 && state->unk58 == 1) {
        state->unk58 = 2;
    }
}

/** IsInventoryOverlayMode: true while the menu-overlay is in mode 7
 * (the in-game inventory / weapon-wheel overlay). */
s32 func_00288870(void) {
    return D_001BAC00.unk40 == 7;
}

/** Set D_1A8A60 to 3. */
void func_00288888(void) {
    D_1A8A60 = 3;
}

/** Read the 0x1BAC00 block's +0x84 counter/flag. */
s16 func_00288898(void) {
    return D_001BAC00.unk84;
}

/** Clear the 0x1BAC00 block's +0x84 counter/flag. */
void func_002888A8(void) {
    D_001BAC00.unk84 = 0;
}

/** Read the D_1A8A64 flag. */
s32 func_002888B8(void) {
    return D_1A8A64;
}

/** Raise the D_1A8A64 flag. */
void func_002888C0(void) {
    D_1A8A64 = 1;
}

/** Clear the D_1A8A64 flag. */
void func_002888D0(void) {
    D_1A8A64 = 0;
}
