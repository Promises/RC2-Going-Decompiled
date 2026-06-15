#include "common.h"

/*
 * text/183558 - the VU0 3D-math primitive band (vaddr 0x283558..0x2849AF).
 *
 * This translation unit is hand-written R5900 / VU0 macro-mode assembly: the
 * vector ops are COP2 instructions (lqc2/sqc2/vadd/vmul/vdiv/vsqrt/vopmula...)
 * and the scalar helpers use R5900-native FPU ops (min.s/max.s/abs.s) plus
 * delay-slot-packed schedules that ee-gcc 2.9 (-O2) never emits. The handful of
 * genuinely *compiled* functions in the band (e.g. SetVideoMode, func_00283B60)
 * use %gp_rel small-data access, which needs -G8 — but this unit is pinned at
 * -G0 — so NONE of them byte-match here. The unit therefore stays INCLUDE_ASM
 * for the matching build (verified instruction-by-instruction via objdiff: the
 * scalar conversions miss on scheduling/coloring, the min/max/abs miss because
 * the patterns don't exist, the vector ops are pure COP2).
 *
 * Per docs/PORTING.md these are tier-2 "pure-computation" functions: each VU0
 * math op has an exact portable C equivalent, kept in the #else branch so the
 * native (TARGET_NATIVE) build has a working implementation and a future
 * match-seed. The matching build always takes the INCLUDE_ASM branch.
 */

#ifdef TARGET_NATIVE
/* Portable 4-lane vector (PS2 VU0 quadword: x,y,z,w; 16-byte aligned). */
typedef f32 Vec4f[4] __attribute__((aligned(16)));
#endif

/*
 * Reinterpret a 32-bit integer's bit pattern as a float (raw mtc1, no
 * conversion). Hand-written: the original schedules the `mtc1` into the `jr`
 * delay slot (jr ra; mtc1 a0,f0) — a form this ee-gcc never emits — so the
 * matching build keeps the asm; the #else is the portable equivalent.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002835D8);
#else
f32 func_002835D8(s32 bits) {
    return *(f32 *)&bits;
}
#endif
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002835E0);
/*
 * Absolute value of a float. Hand-written: the original is a single R5900
 * `abs.s` in the jr delay slot; ee-gcc 2.9 has no abssf2 pattern (it lowers to
 * a compare + branch + neg), so it cannot reproduce these bytes. #else is the
 * portable equivalent.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", GetFloatAbs);
#else
f32 GetFloatAbs(f32 x) {
    return (x < 0.0f) ? -x : x;
}
#endif

/*
 * Smaller of two floats. Hand-written: the original is a single R5900 `min.s`
 * in the jr delay slot; ee-gcc 2.9 has no sminsf3 pattern (it lowers to a
 * compare + branch + moves), so no byte match. #else is the portable form.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283600);
#else
f32 func_00283600(f32 a, f32 b) {
    return (a < b) ? a : b;
}
#endif
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283608);
/*
 * Clamp x into [lo, hi]. Hand-written: the original is `max.s; min.s` (R5900
 * native min/max), which ee-gcc 2.9 cannot emit (no sminsf3/smaxsf3 — it
 * lowers to compares + branches). #else is the portable equivalent.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283618);
#else
f32 func_00283618(f32 x, f32 lo, f32 hi) {
    f32 t = (x > lo) ? x : lo;
    return (t < hi) ? t : hi;
}
#endif
/** dst = |src| componentwise (VU0 vabs.xyzw). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283628);
#else
void func_00283628(Vec4f dst, const Vec4f src) {
    int i;
    for (i = 0; i < 4; i++) dst[i] = (src[i] < 0.0f) ? -src[i] : src[i];
}
#endif

/** Zero a 16-byte quadword at dst (sq $0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283638);
#else
void func_00283638(Vec4f dst) {
    dst[0] = dst[1] = dst[2] = dst[3] = 0.0f;
}
#endif

/** Zero a 4-float vector at dst (sqc2 $vf0 — vf0 reads as 0,0,0,1, but the
 *  store here writes the all-zero pattern of the cleared quadword). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283640);
#else
void func_00283640(Vec4f dst) {
    dst[0] = 0.0f; dst[1] = 0.0f; dst[2] = 0.0f; dst[3] = 1.0f;
}
#endif

/** Set dst to the rotated unit vector (VU0 vmr32 of vf0 = (0,0,0,1)). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", SetVec4UnitZ);
#else
void SetVec4UnitZ(Vec4f dst) {
    /* vmr32 rotates the lanes of vf0=(0,0,0,1) up by one: -> (0,0,1,0). */
    dst[0] = 0.0f; dst[1] = 0.0f; dst[2] = 1.0f; dst[3] = 0.0f;
}
#endif

/** dst = a + b, all four lanes (VU0 vadd.xyzw). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283658);
#else
void func_00283658(Vec4f dst, const Vec4f a, const Vec4f b) {
    int i;
    for (i = 0; i < 4; i++) dst[i] = a[i] + b[i];
}
#endif

/** dst = a + b on x,y,z; dst.w = a.w (VU0 vadd.xyz). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", Vec4AddVu0);
#else
void Vec4AddVu0(Vec4f dst, const Vec4f a, const Vec4f b) {
    dst[0] = a[0] + b[0]; dst[1] = a[1] + b[1]; dst[2] = a[2] + b[2];
    dst[3] = a[3];
}
#endif

/** In-place a += b on x,y,z (VU0 vadd.xyz, stored back into b's slot 'a'). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283688);
#else
void func_00283688(Vec4f unused, Vec4f a, const Vec4f b) {
    (void)unused;
    a[0] = a[0] + b[0]; a[1] = a[1] + b[1]; a[2] = a[2] + b[2];
}
#endif

/** dst = a - b on x,y,z; dst.w = a.w (VU0 vsub.xyz). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", Vec4SubVu0);
#else
void Vec4SubVu0(Vec4f dst, const Vec4f a, const Vec4f b) {
    dst[0] = a[0] - b[0]; dst[1] = a[1] - b[1]; dst[2] = a[2] - b[2];
    dst[3] = a[3];
}
#endif

/** dst = lerp(a, b, t) on x,y,z: a + (b - a)*t; dst.w = a.w (VU0 vsub/vmulx/vadd). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002836B8);
#else
void func_002836B8(Vec4f dst, f32 t, const Vec4f a, const Vec4f b) {
    dst[0] = a[0] + (b[0] - a[0]) * t;
    dst[1] = a[1] + (b[1] - a[1]) * t;
    dst[2] = a[2] + (b[2] - a[2]) * t;
    dst[3] = a[3];
}
#endif

/** dst = src * s on x,y,z; dst.w = src.w (VU0 vmulx.xyz). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", Vec4ScaleVu0);
#else
void Vec4ScaleVu0(Vec4f dst, f32 s, const Vec4f src) {
    dst[0] = src[0] * s; dst[1] = src[1] * s; dst[2] = src[2] * s;
    dst[3] = src[3];
}
#endif

/** In-place src *= s on x,y,z (VU0 vmulx.xyz, stored back into src). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002836F8);
#else
void func_002836F8(Vec4f unused, f32 s, Vec4f src) {
    (void)unused;
    src[0] = src[0] * s; src[1] = src[1] * s; src[2] = src[2] * s;
}
#endif

/** dst = src * s, all four lanes including w (VU0 vmulx.xyzw). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", ScaleVec4IncludingW);
#else
void ScaleVec4IncludingW(Vec4f dst, f32 s, const Vec4f src) {
    int i;
    for (i = 0; i < 4; i++) dst[i] = src[i] * s;
}
#endif

/** dst = a * b componentwise, all four lanes (VU0 vmul.xyzw). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283728);
#else
void func_00283728(Vec4f dst, const Vec4f a, const Vec4f b) {
    int i;
    for (i = 0; i < 4; i++) dst[i] = a[i] * b[i];
}
#endif

/** dst = src / d on x,y,z; dst.w = src.w (VU0 vdiv Q then vmulq.xyz). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283740);
#else
void func_00283740(Vec4f dst, f32 d, const Vec4f src) {
    f32 q = 1.0f / d;
    dst[0] = src[0] * q; dst[1] = src[1] * q; dst[2] = src[2] * q;
    dst[3] = src[3];
}
#endif

/** Return the 3D dot product a.x*b.x + a.y*b.y + a.z*b.z (VU0 vmul/vadda/vmadd). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", Vec3DotVu0);
#else
f32 Vec3DotVu0(const Vec4f a, const Vec4f b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}
#endif

/** dst = a x b (3D cross product; VU0 vopmula/vopmsub). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", Vec3CrossVu0);
#else
void Vec3CrossVu0(Vec4f dst, const Vec4f a, const Vec4f b) {
    dst[0] = a[1] * b[2] - a[2] * b[1];
    dst[1] = a[2] * b[0] - a[0] * b[2];
    dst[2] = a[0] * b[1] - a[1] * b[0];
}
#endif

/** Return the 3D length sqrt(x*x + y*y + z*z) (VU0 vmul/vadd/vsqrt). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", Vec3LengthVu0);
#else
f32 Vec3LengthVu0(const Vec4f v) {
    return __builtin_sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}
#endif
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002837D0);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002837F8);
/** Return the 2D distance in the x,y plane between a and b (VU0 vsub/vmul/vsqrt). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", DistXYVu0);
#else
f32 DistXYVu0(const Vec4f a, const Vec4f b) {
    f32 dx = a[0] - b[0];
    f32 dy = a[1] - b[1];
    return __builtin_sqrtf(dx * dx + dy * dy);
}
#endif
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283860);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283888);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", Vec3RescaleToLenVu0);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283920);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283968);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002839D8);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283A48);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283A70);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283AA0);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283AB8);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283AE0);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283B00);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283B30);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283B48);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283B60);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283BF8);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283D10);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", MatrixIdentityVu0);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283D68);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283DA0);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283DC0);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283DE0);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284008);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284028);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284048);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284098);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002840E8);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", MatrixMultiplyVu0);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284180);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002841C0);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284248);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284308);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284380);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284408);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002844A0);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002844F8);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284548);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284590);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002845D8);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284630);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284668);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284678);
/*
 * Convert a signed integer to a float. Hand-written: the original packs
 * `mtc1; cvt.s.w` with no mtc1->cvt hazard nop and a bare `nop` delay slot;
 * ee-gcc 2.9 inserts the hazard nop and sinks the cvt into the jr delay slot,
 * so it cannot reproduce the schedule. #else is the portable equivalent.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", IntToFloat);
#else
f32 IntToFloat(s32 x) {
    return (f32)x;
}
#endif

/*
 * Truncate a float to a signed integer. Hand-written: the original does the
 * `cvt.w.s; mfc1` in place on the argument register ($f12), whereas ee-gcc 2.9
 * always allocates a fresh result register ($f0) — a register-coloring delta
 * (objdiff 97.5%). #else is the portable equivalent.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", FloatToInt);
#else
s32 FloatToInt(f32 x) {
    return (s32)x;
}
#endif
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002846B0);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002846E8);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284730);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284768);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284860);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284998);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", SetVideoMode);
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284A20);
