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

/* func_00288748 length/angle scalars (gp-relative). D_1A8A78 is an integer
 * (loaded then cvt.s.w'd to a target length); D_1A8A80 is a float scale. */
extern s32 D_1A8A78;
extern f32 D_1A8A80;
/* D_1A8AE0: source vec4 for func_00283DA0 in the camera rebuild. */
extern Vec4f D_1A8AE0;

/* VU0 vector-math helpers (text/183558) and quaternion/rotation builders. */
extern void Vec4SubVu0(Vec4f dst, const Vec4f a, const Vec4f b);
extern void Vec4AddVu0(Vec4f dst, const Vec4f a, const Vec4f b);
extern void Vec3CrossVu0(Vec4f dst, const Vec4f a, const Vec4f b);
extern f32  Vec3LengthVu0(const Vec4f v);
extern void Vec3RescaleToLenVu0(Vec4f dst, f32 len, const Vec4f src);
extern void func_00283DA0(Vec4f dst, const Vec4f src);   /* matrix-row builder */
extern void func_002840E8(Vec4f dst, const Vec4f a, const Vec4f b); /* 3x3 a*b */
extern void QuatToMatrix3(const Vec4f src, Vec4f dst);   /* quat -> 3x3 matrix */
extern void func_002ADCE0(Vec4f dst, const Vec4f axis, f32 len); /* axis -> quat */
extern void func_002ABAE8(const f32 *p, f32 a, f32 b, f32 c, f32 d);

/* Reinterpret an IEEE-754 bit pattern as f32 (for the exact roll constants). */
static __inline__ f32 bits_to_f32(u32 bits) {
    union { u32 u; f32 f; } v;
    v.u = bits;
    return v.f;
}

/*
 * func_00288600: rebuild the camera rotation matrix from the player/target.
 * $4 (target) = position to look toward, $5 (flag) selects the roll source:
 *   - forward = normalize(g_cameraPos - D_001B1750)
 *   - up      = func_00283DA0(D_1A8AE0); right3x3 = g_cameraMatrix * up
 *   - cross   = forward x right; len = |cross|
 *   - flag!=0: roll quat from cross by -(len*0.5)         [func_002ADCE0]
 *   - flag==0: roll via func_002ABAE8(target, len*0.5, k0, k0, k1)
 *   - quat -> 3x3 (QuatToMatrix3); g_cameraMatrix = roll3x3 * g_cameraMatrix
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188580", func_00288600);
#else
/* TODO(match): functional equivalent - not byte-exact; blocked by the proven
 * 16-byte callee-save-slot layout wall (this cc1 reserves 16 bytes per callee
 * save; the original packs s0..s5+ra 8-byte at sp+0xB0..0xE0). */
void func_00288600(const Vec4f target, s32 flag) {
    Vec4f forward;   /* sp+0xA0 */
    Vec4f up;        /* sp+0x60 */
    Vec4f right;     /* sp+0x30 (3x3) */
    Vec4f cross;     /* sp+0x90 */
    Vec4f quat;      /* sp+0x00 == cross reused as quaternion + roll matrix */
    f32 len;

    Vec4SubVu0(forward, g_cameraPos, D_001B1750);
    Vec3RescaleToLenVu0(forward, 1.0f, forward);

    func_00283DA0(up, D_1A8AE0);
    func_002840E8(right, g_cameraMatrix, up);

    Vec3CrossVu0(cross, right, forward);
    len = Vec3LengthVu0(cross);

    if (flag != 0) {
        func_002ADCE0(cross, cross, -(len * 0.5f));
    } else {
        func_002ABAE8(target, len * 0.5f,
                      bits_to_f32(0x3A18825C), bits_to_f32(0x3A18825C),
                      bits_to_f32(0x3D567752));
        func_002ADCE0(cross, cross, -target[0]);   /* build the roll quat (both paths) */
    }

    QuatToMatrix3(cross, quat);
    func_002840E8(g_cameraMatrix, quat, g_cameraMatrix);
}
#endif

/*
 * func_00288748: apply a roll step to the camera matrix. p[0] is an arcmin
 * delta; angle = p[0] * (pi/180/60) * D_1A8A80. Build a quaternion about the
 * camera right axis (matrix row 0) and pre-multiply it into g_cameraMatrix.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188580", func_00288748);
#else
/* TODO(match): functional equivalent - not byte-exact; same 16-byte
 * callee-save-slot wall (original packs s0/s1/ra at sp+0x40/0x48/0x50). */
void func_00288748(const Vec4f p) {
    Vec4f quat;  /* sp+0x00 */
    Vec4f mat;   /* sp+0x10 (3x3) */
    f32 angle = p[0] * bits_to_f32(0x3998825C) * D_1A8A80;

    func_002ADCE0(quat, g_cameraMatrix, angle);
    QuatToMatrix3(quat, mat);
    func_002840E8(g_cameraMatrix, mat, g_cameraMatrix);
}
#endif

/*
 * func_002887C0: nudge the camera position by 'offset', re-anchored about the
 * fixed point D_001B1750 and clamped back to a fixed radius (f32)D_1A8A78.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/188580", func_002887C0);
#else
/* TODO(match): functional equivalent - not byte-exact; saves s0/s1/ra packed
 * at sp+0x0/0x8/0x10 in a 0x20 frame and sibling-call-optimises the void tail
 * call, both of which this cc1 expands differently (16-byte save-slot wall). */
void func_002887C0(const Vec4f offset) {
    Vec4SubVu0(g_cameraPos, g_cameraPos, D_001B1750);
    Vec4AddVu0(g_cameraPos, g_cameraPos, offset);
    Vec3RescaleToLenVu0(g_cameraPos, (f32)D_1A8A78, g_cameraPos);
    Vec4AddVu0(g_cameraPos, g_cameraPos, D_001B1750);
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
