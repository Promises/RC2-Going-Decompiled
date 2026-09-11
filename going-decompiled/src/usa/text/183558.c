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
/*
 * Absolute value of a signed integer. Hand-written: `bgez; neg; addi` with
 * the result move in the jr delay slot - a schedule ee-gcc never emits.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002835E0);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
s32 func_002835E0(s32 x) {
    return (x < 0) ? -x : x;
}
#endif
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
/*
 * Smaller of three signed integers: min(a, min(b, c)). Hand-written: two
 * R5900 `pminw` (parallel min word) ops that ee-gcc 2.9 cannot emit.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283608);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
s32 func_00283608(s32 a, s32 b, s32 c) {
    s32 m = (a < b) ? a : b;
    return (m < c) ? m : c;
}
#endif
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

/** dst = -(a x b) (3D cross product; VU0 vopmula ACC,b,a / vopmsub vf3,a,b yields
 *  the canonical PS2 NEGATED outer product). dst.w is left undefined by the VU
 *  path (vf3.w holds a stale lane). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", Vec3CrossVu0);
#else
void Vec3CrossVu0(Vec4f dst, const Vec4f a, const Vec4f b) {
    dst[0] = a[2] * b[1] - a[1] * b[2];
    dst[1] = a[0] * b[2] - a[2] * b[0];
    dst[2] = a[1] * b[0] - a[0] * b[1];
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
/** Return the 2D length sqrt(v.x*v.x + v.y*v.y) of the xy components (VU0 vmul/vaddy/vsqrt). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002837D0);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
f32 func_002837D0(const Vec4f v) {
    return __builtin_sqrtf(v[0] * v[0] + v[1] * v[1]);
}
#endif
/** Return the 3D distance sqrt(|a-b|^2) between points a and b (VU0 vsub/vmul/vmadd/vsqrt). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002837F8);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
f32 func_002837F8(const Vec4f a, const Vec4f b) {
    f32 dx = a[0] - b[0];
    f32 dy = a[1] - b[1];
    f32 dz = a[2] - b[2];
    return __builtin_sqrtf(dx * dx + dy * dy + dz * dz);
}
#endif
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
/** Return the squared 3D distance |a-b|^2 between points a and b (VU0 vsub/vmul/vmadd). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283860);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
f32 func_00283860(const Vec4f a, const Vec4f b) {
    f32 dx = a[0] - b[0];
    f32 dy = a[1] - b[1];
    f32 dz = a[2] - b[2];
    return dx * dx + dy * dy + dz * dz;
}
#endif

/**
 * Sphere overlap test: return 1 if the two spheres overlap, else 0. Sphere a
 * is centred at a.xyz with radius a.w, sphere b at b.xyz with radius b.w. The
 * VU0 code compares the squared centre distance against the squared radius sum
 * (vsub/vadd.w/vmul/vmadd) and returns 1 when dist^2 < (a.w+b.w)^2.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283888);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
s32 func_00283888(const Vec4f a, const Vec4f b) {
    f32 dx = a[0] - b[0];
    f32 dy = a[1] - b[1];
    f32 dz = a[2] - b[2];
    f32 rsum = a[3] + b[3];
    f32 d = (dx * dx + dy * dy + dz * dz) - rsum * rsum;
    return (d < 0.0f) ? 1 : 0;
}
#endif

/**
 * Rescale the xyz of src to length 'len' and store into dst (VU0 vrsqrt). The
 * VU seeds the reciprocal-sqrt numerator with 'len' so the result is
 * src * (len / |src|). A zero-length src yields the zero vector.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", Vec3RescaleToLenVu0);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void Vec3RescaleToLenVu0(Vec4f dst, f32 len, const Vec4f src) {
    f32 len2 = src[0] * src[0] + src[1] * src[1] + src[2] * src[2];
    if (len2 != 0.0f) {
        f32 q = len / __builtin_sqrtf(len2);
        dst[0] = src[0] * q;
        dst[1] = src[1] * q;
        dst[2] = src[2] * q;
    } else {
        dst[0] = 0.0f;
        dst[1] = 0.0f;
        dst[2] = 0.0f;
    }
}
#endif

/** Rescale the xy of src to length 'len' and store into dst (2D variant of Vec3RescaleToLenVu0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283920);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_00283920(Vec4f dst, f32 len, const Vec4f src) {
    f32 len2 = src[0] * src[0] + src[1] * src[1];
    if (len2 != 0.0f) {
        f32 q = len / __builtin_sqrtf(len2);
        dst[0] = src[0] * q;
        dst[1] = src[1] * q;
    } else {
        dst[0] = 0.0f;
        dst[1] = 0.0f;
    }
    /* Only .xy is computed; the full sqc2 stores back .z/.w unchanged from the
     * lqc2'd src (preserved on both the compute and zero paths). */
    dst[2] = src[2];
    dst[3] = src[3];
}
#endif

/**
 * Normalize src.xyz to length 'minLen' into dst if |src| >= minLen; return 1 on
 * success, 0 if the vector is too short (degenerate). The VU0 code computes the
 * length, then divides only when the length is non-zero and >= the threshold.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283968);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
s32 func_00283968(Vec4f dst, f32 minLen, const Vec4f src) {
    f32 len = __builtin_sqrtf(src[0] * src[0] + src[1] * src[1] + src[2] * src[2]);
    if (len != 0.0f && (minLen - len) <= 0.0f) {
        f32 q = minLen / len;
        dst[0] = src[0] * q;
        dst[1] = src[1] * q;
        dst[2] = src[2] * q;
        /* .w is not scaled; the full sqc2 stores it back unchanged from src. */
        dst[3] = src[3];
        return 1;
    }
    return 0;
}
#endif

/**
 * Move point 'src' toward the surface of the unit sphere about it by reflecting
 * across the normalized direction: builds a normalized direction from 'dir',
 * projects (src) onto it, and reflects. If the projection is negative the point
 * is left unchanged (copied verbatim). VU0 vrsqrt/vmul/vmadd/vsub.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002839D8);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_002839D8(Vec4f dst, const Vec4f src, const Vec4f dir) {
    f32 vx = -src[0], vy = -src[1], vz = -src[2];
    f32 dlen2 = dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2];
    f32 inv = 1.0f / __builtin_sqrtf(dlen2);
    f32 nx = dir[0] * inv, ny = dir[1] * inv, nz = dir[2] * inv;
    f32 proj = vx * nx + vy * ny + vz * nz;
    if (proj >= 0.0f) {
        f32 px = nx * proj, py = ny * proj, pz = nz * proj;
        px = (px - vx) * 2.0f;
        py = (py - vy) * 2.0f;
        pz = (pz - vz) * 2.0f;
        dst[0] = vx + px;
        dst[1] = vy + py;
        dst[2] = vz + pz;
        /* Reflection touches only .xyz; vf1.w is never negated (the vsub/vadd
         * are .xyz), so the full sqc2 stores the original src.w — not -src.w. */
        dst[3] = src[3];
    } else {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
    }
}
#endif

/**
 * Transform vector v by the 3x3 rotation part of matrix m (rows m[0..2]); the
 * w lane of the result comes from v.w (no translation). out = m * v.
 * VU0 vmulax/vmadday/vmaddaz/vmaddw.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283A48);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_00283A48(Vec4f out, const Vec4f v, const Vec4f m) {
    const Vec4f *r = (const Vec4f *)m;
    int i;
    for (i = 0; i < 4; i++)
        out[i] = r[0][i] * v[0] + r[1][i] * v[1] + r[2][i] * v[2];
    out[3] += v[3];
}
#endif

/**
 * Transform point v by the full 4x4 matrix m (rows m[0..3]); the translation
 * row m[3] is scaled by v.w. out = m * v. VU0 vmulax/vmadday/vmaddaz/vmaddw.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283A70);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_00283A70(Vec4f out, const Vec4f v, const Vec4f m) {
    const Vec4f *r = (const Vec4f *)m;
    int i;
    for (i = 0; i < 4; i++)
        out[i] = r[0][i] * v[0] + r[1][i] * v[1] + r[2][i] * v[2] + r[3][i] * v[3];
}
#endif

/** Unpack 4 unsigned bytes (packed in 's') to a float vector in dst (VU0 vitof0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283AA0);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_00283AA0(Vec4f dst, u32 packed) {
    dst[0] = (f32)(packed & 0xFF);
    dst[1] = (f32)((packed >> 8) & 0xFF);
    dst[2] = (f32)((packed >> 16) & 0xFF);
    dst[3] = (f32)((packed >> 24) & 0xFF);
}
#endif

/** Convert the 4 floats at src to integers and pack them down to 4 bytes (VU0 vftoi0/ppach/ppacb). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283AB8);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
u32 func_00283AB8(const Vec4f src) {
    u32 b0 = (u32)(s32)src[0] & 0xFF;
    u32 b1 = (u32)(s32)src[1] & 0xFF;
    u32 b2 = (u32)(s32)src[2] & 0xFF;
    u32 b3 = (u32)(s32)src[3] & 0xFF;
    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}
#endif

/** Unpack 4 signed shorts to a float vector in dst (VU0 pextlh/psraw/vitof0).
 *  The asm unpacks all four s16 lanes from the low 64 bits of the SINGLE packed
 *  doubleword arg (caller 0x2A053C passes one 64-bit value); there is no second
 *  source register. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283AE0);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_00283AE0(Vec4f dst, u64 packed4) {
    dst[0] = (f32)(s16)( packed4        & 0xFFFF);
    dst[1] = (f32)(s16)((packed4 >> 16) & 0xFFFF);
    dst[2] = (f32)(s16)((packed4 >> 32) & 0xFFFF);
    dst[3] = (f32)(s16)((packed4 >> 48) & 0xFFFF);
}
#endif

/** Sum 'count' bytes starting at 'data' and return the total (R5900 2-at-a-time add loop). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283B00);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
s32 func_00283B00(const u8 *data, s32 count) {
    s32 sum = 0;
    s32 i;
    for (i = 0; i < count; i++)
        sum += data[i];
    return sum;
}
#endif

/*
 * COSINE via VU microprogram upload (vcallms 0xC80). Genuinely hardware: the value
 * goes through an uploaded VU0 microcode routine, not portable VU macro-mode math.
 *
 * The sin/cos assignment of this pair is READ FROM THE MICROCODE, not inferred. The
 * VIF MPG upload at offset 0x00E96C of assets/cod/000000.textbin.bin loads 94 VU
 * instructions to micro-memory address 400 -- and vcallms 0xC80 / 8 == 400, so this
 * pair's two entry points sit at the very start of that upload:
 *
 *   instr 400  LOI  1.57079625  (PI/2)   <- vcallms 0xC80 enters HERE
 *   instr 401  (consumes I)
 *   instr 402  LOI  3.1415925   (PI)     <- vcallms 0xC90 enters HERE
 *   instr 403  LOI -3.1415925   (-PI)      [shared range reduction from here on]
 *   instr 414..417  LOI -1/6, +1/120, -1/5040, +1/362880, E-bit terminator
 *
 * There is ONE program with ONE terminator and ONE polynomial -- and that polynomial
 * is the odd (SINE) series. The only thing 0xC80 does that 0xC90 does not is take on
 * a PI/2 phase first. sin(x + PI/2) == cos(x), so:
 *
 *   vcallms 0xC80 (this function) = COS      vcallms 0xC90 = SIN
 *
 * Corroboration: those four coefficients match the EE-side trig table at vaddr
 * 0x1AC420 to every printed digit, and there is no cosine series anywhere in the
 * image -- because cos is this phase shift into the shared sine kernel.
 *
 * This corrects a long-standing inversion: the bodies below and both plate comments
 * previously asserted the opposite pairing, as do ~14 `extern` comments elsewhere in
 * src/usa and the names SinfVu0/CosfVu0 (which are swapped and are NOT pinned here).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283B30);
#else
/* TODO(hle): VU0 microprogram (vcallms 0xC80) - cosine; needs VU backend. Scalar stand-in.
 * DIVERGENCE, UNCONDITIONAL: this is a scalar libm call, NOT the VU0 microprogram. Even
 * with the pairing correct, precision and rounding differ from the hardware -- the VU
 * runs a 4-term minimax polynomial in 32-bit float with its own range reduction, while
 * __builtin_cosf is the host libm. This is not a latent concern: build_conly.sh compiles
 * the #else arm into <unit>.calt.o and Attempt B injects it BEFORE <unit>.o, so on the
 * C-only shipping path THIS body is what runs. Results will not be bit-identical to the
 * ROM; anything comparing against captured hardware output must expect that. */
f32 func_00283B30(f32 x) {
    return __builtin_cosf(x);
}
#endif

/*
 * SINE via VU microprogram upload (vcallms 0xC90). Hardware VU0 microcode. This is the
 * bare entry into the shared kernel described above: 0xC90 skips the PI/2 phase that
 * 0xC80 applies, so it evaluates the sine series directly.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283B48);
#else
/* TODO(hle): VU0 microprogram (vcallms 0xC90) - sine; needs VU backend. Scalar stand-in.
 * DIVERGENCE, UNCONDITIONAL: see func_00283B30 above -- a scalar libm call is not the VU0
 * microprogram, and this body is the one that runs on the C-only path. */
f32 func_00283B48(f32 x) {
    return __builtin_sinf(x);
}
#endif

/*
 * Arc-sine-style approximation: returns the inverse-trig value of x using a
 * polynomial in sqrt(1-|x|) (coefficient table D_1AC440) with sign handling.
 * The VU0/FPU schedule (madda chains + vsqrt) is hand-written. The portable
 * standard-library form is functionally equivalent.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283B60);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
f32 func_00283B60(f32 x) {
    return __builtin_asinf(x);
}
#endif

/*
 * atan2(y, x): four-quadrant arc-tangent built from a rational/polynomial
 * approximation selected by sign and magnitude (tables D_1AC460/480) with VU0
 * Horner evaluation. Portable library form is functionally equivalent.
 * Arguments: $f12 = y-like, $f13 = x-like.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283BF8);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
f32 func_00283BF8(f32 y, f32 x) {
    return __builtin_atan2f(y, x);
}
#endif

/** Build a 3x3 identity matrix (3 rows of 16 bytes) at dst (VU0 vmulx/vmr32/vaddw). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283D10);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_00283D10(Vec4f dst) {
    Vec4f *r = (Vec4f *)dst;
    int i, j;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 4; j++)
            r[i][j] = (i == j) ? 1.0f : 0.0f;
}
#endif

/** Build a 4x4 identity matrix at dst (VU0 vmulx/vmr32/vmove/vaddw). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", MatrixIdentityVu0);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void MatrixIdentityVu0(Vec4f dst) {
    Vec4f *r = (Vec4f *)dst;
    int i, j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            r[i][j] = (i == j) ? 1.0f : 0.0f;
}
#endif

/**
 * Build a 4x4 uniform-scale matrix with scale 's' on the diagonal (x,y,z) and
 * 1 in [3][3]; translation row is identity. VU0 vmulx/vmove/vaddx.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283D68);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_00283D68(Vec4f dst, f32 s) {
    Vec4f *r = (Vec4f *)dst;
    int i, j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            r[i][j] = 0.0f;
    r[0][0] = s;
    r[1][1] = s;
    r[2][2] = s;
    r[3][3] = 1.0f;
}
#endif

/*
 * Build a rotation matrix via VU microprogram (vcallms 0xD18) producing the
 * first 3 rows from a vector in src. Hardware VU0 microcode upload.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283DA0);
#else
/* TODO(hle): VU0 microprogram (vcallms 0xD18) builds rotation matrix; needs VU backend. */
void func_00283DA0(Vec4f dst, const Vec4f src);
#endif

/*
 * Build a rotation matrix via VU microprogram (vcallms 0xD18) producing all 4
 * rows from a vector in src. Hardware VU0 microcode upload.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283DC0);
#else
/* TODO(hle): VU0 microprogram (vcallms 0xD18) builds rotation matrix; needs VU backend. */
void func_00283DC0(Vec4f dst, const Vec4f src);
#endif

/*
 * Build a 4x4 rotation matrix from Euler angles (radians) stored in angles[0..2]
 * (z at +0, y at +4, x at +8): R = Rz * Ry * Rx applied incrementally. Each
 * non-zero angle's sine/cosine come from an inline range-reduced polynomial
 * (coefficients 0xBE2AAAA4 etc. = the -1/6, 1/120,... Taylor terms); the matrix
 * starts as identity and is composed by the per-axis rotations. The handwritten
 * VU0/FPU schedule has no compiler equivalent.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00283DE0);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_00283DE0(Vec4f dst, const f32 *angles) {
    Vec4f *r = (Vec4f *)dst;
    f32 az = angles[2], ay = angles[1], ax = angles[0];
    int i, j, k;
    f32 m[4][4];
    /* start from identity */
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            m[i][j] = (i == j) ? 1.0f : 0.0f;
    /* compose Rx, Ry, Rz only for the non-zero angles, in the asm's order */
    if (ax != 0.0f) {
        f32 c = __builtin_cosf(ax), s = __builtin_sinf(ax);
        f32 rot[4][4] = {{1,0,0,0},{0,c,s,0},{0,-s,c,0},{0,0,0,1}};
        f32 tmp[4][4];
        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++) {
                tmp[i][j] = 0.0f;
                for (k = 0; k < 4; k++) tmp[i][j] += rot[i][k] * m[k][j];
            }
        for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) m[i][j] = tmp[i][j];
    }
    if (ay != 0.0f) {
        f32 c = __builtin_cosf(ay), s = __builtin_sinf(ay);
        f32 rot[4][4] = {{c,0,-s,0},{0,1,0,0},{s,0,c,0},{0,0,0,1}};
        f32 tmp[4][4];
        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++) {
                tmp[i][j] = 0.0f;
                for (k = 0; k < 4; k++) tmp[i][j] += rot[i][k] * m[k][j];
            }
        for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) m[i][j] = tmp[i][j];
    }
    if (az != 0.0f) {
        f32 c = __builtin_cosf(az), s = __builtin_sinf(az);
        f32 rot[4][4] = {{c,s,0,0},{-s,c,0,0},{0,0,1,0},{0,0,0,1}};
        f32 tmp[4][4];
        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++) {
                tmp[i][j] = 0.0f;
                for (k = 0; k < 4; k++) tmp[i][j] += rot[i][k] * m[k][j];
            }
        for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) m[i][j] = tmp[i][j];
    }
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            r[i][j] = m[i][j];
}
#endif
/** Copy the 3x3 (3 rows) of matrix src into dst and set dst row 3 to (0,0,0,1). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284008);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_00284008(Vec4f dst, const Vec4f src) {
    int i, j;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 4; j++)
            dst[i * 4 + j] = src[i * 4 + j];
    dst[12] = 0.0f; dst[13] = 0.0f; dst[14] = 0.0f; dst[15] = 1.0f;
}
#endif

/** Copy the 3x3 (3 rows) of matrix src into dst, leaving row 3 untouched. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284028);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_00284028(Vec4f dst, const Vec4f src) {
    int i, j;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 4; j++)
            dst[i * 4 + j] = src[i * 4 + j];
}
#endif

/** Transpose the 3x3 rotation part of src into dst, clearing each row's w; sets dst row 3 = (0,0,0,1). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284048);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_00284048(Vec4f dst, const Vec4f src) {
    int i, j;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            dst[i * 4 + j] = src[j * 4 + i];
    dst[3] = 0.0f; dst[7] = 0.0f; dst[11] = 0.0f;
    dst[12] = 0.0f; dst[13] = 0.0f; dst[14] = 0.0f; dst[15] = 1.0f;
}
#endif

/** Transpose the 3x3 rotation part of src into dst, clearing each row's w; row 3 untouched. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284098);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_00284098(Vec4f dst, const Vec4f src) {
    int i, j;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            dst[i * 4 + j] = src[j * 4 + i];
    dst[3] = 0.0f; dst[7] = 0.0f; dst[11] = 0.0f;
}
#endif

/** Multiply two 3x3 matrices (3 rows each): dst = a * b. VU0 vmulax/vmadday/vmaddz. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002840E8);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_002840E8(Vec4f dst, const Vec4f a, const Vec4f b) {
    const Vec4f *ar = (const Vec4f *)a;
    const Vec4f *br = (const Vec4f *)b;
    Vec4f *dr = (Vec4f *)dst;
    int i, j;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 4; j++)
            dr[i][j] = ar[0][j] * br[i][0] + ar[1][j] * br[i][1] + ar[2][j] * br[i][2];
}
#endif

/** Multiply two 4x4 matrices: dst = a * b (loops over b's 4 rows). VU0 vmulax/vmadda chain. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", MatrixMultiplyVu0);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void MatrixMultiplyVu0(Vec4f dst, const Vec4f a, const Vec4f b) {
    const Vec4f *ar = (const Vec4f *)a;
    const Vec4f *br = (const Vec4f *)b;
    Vec4f *dr = (Vec4f *)dst;
    int i, j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            dr[i][j] = ar[0][j] * br[i][0] + ar[1][j] * br[i][1]
                     + ar[2][j] * br[i][2] + ar[3][j] * br[i][3];
}
#endif

/**
 * Hamilton quaternion product: dst = a * b (xyz = vector part, w = scalar).
 * dst = (a.w*b.xyz + b.w*a.xyz + a.xyz x b.xyz, a.w*b.w - dot(a.xyz, b.xyz)).
 * VU0 vopmula/vopmsub (cross) + vmulw + vmaddz (dot). Args: $5=a, $6=b.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284180);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_00284180(Vec4f dst, const Vec4f a, const Vec4f b) {
    f32 ax = a[0], ay = a[1], az = a[2], aw = a[3];
    f32 bx = b[0], by = b[1], bz = b[2], bw = b[3];
    dst[0] = aw * bx + bw * ax + (ay * bz - az * by);
    dst[1] = aw * by + bw * ay + (az * bx - ax * bz);
    dst[2] = aw * bz + bw * az + (ax * by - ay * bx);
    dst[3] = aw * bw - (ax * bx + ay * by + az * bz);
}
#endif

/**
 * Normalized linear interpolation of two quaternions: blends a and b by factor
 * t (q = a*(1-t) + b*t), takes the short arc by negating the b contribution when
 * the dot is negative, then normalizes. VU0 vmulw/vmulx/vadd/vrsqrt/vmulq.
 * Args: $f12=t, $5=a, $6=b.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002841C0);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_002841C0(Vec4f dst, f32 t, const Vec4f a, const Vec4f b) {
    f32 wa = 1.0f - t;
    f32 q[4];
    f32 sum2 = 0.0f, dot = 0.0f;
    int i;
    for (i = 0; i < 4; i++) {
        f32 s = a[i] * wa + b[i] * t;
        q[i] = s;
        sum2 += s * s;
        dot += (a[i] * wa) * (b[i] * t);
    }
    if (dot < 0.0f) {
        sum2 = 0.0f;
        for (i = 0; i < 4; i++) {
            f32 s = a[i] * wa - b[i] * t;
            q[i] = s;
            sum2 += s * s;
        }
    }
    {
        f32 inv = 1.0f / __builtin_sqrtf(sum2);
        for (i = 0; i < 4; i++)
            dst[i] = q[i] * inv;
    }
}
#endif

/*
 * Rotate stored data by angle $f12 about an axis selected by 'mode' ($5),
 * computing sin/cos through the VU sin/cos microprograms (func_00283B30/B48,
 * vcallms). The vcallms-based trig makes this hardware-tied.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284248);
#else
/* TODO(hle): uses VU0 sin/cos microprograms (vcallms via func_00283B30/B48); needs VU backend. */
void func_00284248(Vec4f dst, f32 angle, s32 mode);
#endif

/** Build the 3x3 rotation matrix (rows 0..2) for the quaternion at src; store at dst. VU0 quat->matrix. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284308);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_00284308(const Vec4f src, Vec4f dst) {
    f32 x = src[0], y = src[1], z = src[2], w = src[3];
    f32 xx = 2*x*x, yy = 2*y*y, zz = 2*z*z;
    f32 xy = 2*x*y, xz = 2*x*z, yz = 2*y*z;
    f32 wx = 2*w*x, wy = 2*w*y, wz = 2*w*z;
    Vec4f *r = (Vec4f *)dst;
    /* off-diagonal w-terms match the VU0 output (the opposite sign builds the transpose). */
    r[0][0] = 1.0f - (yy + zz); r[0][1] = xy - wz;          r[0][2] = xz + wy;          r[0][3] = 0.0f;
    r[1][0] = xy + wz;          r[1][1] = 1.0f - (xx + zz); r[1][2] = yz - wx;          r[1][3] = 0.0f;
    r[2][0] = xz - wy;          r[2][1] = yz + wx;          r[2][2] = 1.0f - (xx + yy); r[2][3] = 0.0f;
}
#endif

/** Build the 4x4 rotation matrix for the quaternion at src; store at dst (row 3 = identity). VU0 quat->matrix. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284380);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_00284380(const Vec4f src, Vec4f dst) {
    f32 x = src[0], y = src[1], z = src[2], w = src[3];
    f32 xx = 2*x*x, yy = 2*y*y, zz = 2*z*z;
    f32 xy = 2*x*y, xz = 2*x*z, yz = 2*y*z;
    f32 wx = 2*w*x, wy = 2*w*y, wz = 2*w*z;
    Vec4f *r = (Vec4f *)dst;
    /* off-diagonal w-terms match the VU0 output (the opposite sign builds the transpose). */
    r[0][0] = 1.0f - (yy + zz); r[0][1] = xy - wz;          r[0][2] = xz + wy;          r[0][3] = 0.0f;
    r[1][0] = xy + wz;          r[1][1] = 1.0f - (xx + zz); r[1][2] = yz - wx;          r[1][3] = 0.0f;
    r[2][0] = xz - wy;          r[2][1] = yz + wx;          r[2][2] = 1.0f - (xx + yy); r[2][3] = 0.0f;
    r[3][0] = 0.0f;             r[3][1] = 0.0f;             r[3][2] = 0.0f;             r[3][3] = 1.0f;
}
#endif

/**
 * Build a 4x4 transform from quaternion (src=$4), per-axis scale (scale=$5) and
 * translation (translation=$6), writing to out=$7. The rotation matrix is formed
 * from the quaternion, each row scaled by the corresponding scale component, and
 * the translation row set to (translation.x, translation.y, translation.z, 1)
 * (asm loads vf17 from a2, sets .w=1, stores to row3). VU0 quat->matrix +
 * vmulx/vmuly/vmulz per row.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284408);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_00284408(const Vec4f src, const Vec4f scale, const Vec4f translation, Vec4f out) {
    f32 x = src[0], y = src[1], z = src[2], w = src[3];
    f32 xx = 2*x*x, yy = 2*y*y, zz = 2*z*z;
    f32 xy = 2*x*y, xz = 2*x*z, yz = 2*y*z;
    f32 wx = 2*w*x, wy = 2*w*y, wz = 2*w*z;
    Vec4f *r = (Vec4f *)out;
    /* off-diagonal w-terms match the VU0 output (the opposite sign builds the transpose). */
    r[0][0] = 1.0f - (yy + zz); r[0][1] = xy - wz;          r[0][2] = xz + wy;
    r[1][0] = xy + wz;          r[1][1] = 1.0f - (xx + zz); r[1][2] = yz - wx;
    r[2][0] = xz - wy;          r[2][1] = yz + wx;          r[2][2] = 1.0f - (xx + yy);
    r[0][0] *= scale[0]; r[0][1] *= scale[0]; r[0][2] *= scale[0]; r[0][3] = 0.0f;
    r[1][0] *= scale[1]; r[1][1] *= scale[1]; r[1][2] *= scale[1]; r[1][3] = 0.0f;
    r[2][0] *= scale[2]; r[2][1] *= scale[2]; r[2][2] *= scale[2]; r[2][3] = 0.0f;
    r[3][0] = translation[0]; r[3][1] = translation[1]; r[3][2] = translation[2]; r[3][3] = 1.0f;
}
#endif

/**
 * Unpack a compact pose record at src ($4) into three float vectors at dst ($5):
 * row1 = signed shorts at 1/2^15 (vitof15.xyzw); row2 = UNSIGNED shorts at
 * 1/4096 (psrlw + vitof15.xyz) with its w-lane left as the raw un-converted int;
 * row3 = signed shorts at integer scale (vitof0.xyz). VU0 vitof.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002844A0);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_002844A0(const s16 *src, Vec4f dst) {
    int i;
    const f32 inv15 = 1.0f / 32768.0f;
    /* row1: signed shorts, vitof15 (/2^15) on all 4 lanes */
    for (i = 0; i < 4; i++) dst[i] = (f32)src[i] * inv15;
    /* row2: UNSIGNED shorts scaled by 1/4096 (asm psrlw + vitof15.xyz); the w
     * lane is left as the raw un-converted int (u16)src[7]<<3, not a float. */
    for (i = 0; i < 3; i++) dst[4 + i] = (f32)((unsigned short)src[4 + i]) * (1.0f / 4096.0f);
    *(u32 *)&dst[7] = (u32)((unsigned short)src[7] << 3);
    /* row3: signed shorts at integer scale (vitof0) */
    for (i = 0; i < 3; i++) dst[8 + i] = (f32)src[8 + i];
}
#endif

/**
 * Linearly interpolate two 3x3 matrices a ($5) and b ($6) by factor t ($f12)
 * into dst ($4): each row = a_row*(1-t) + b_row*t. VU0 vmulaw/vmaddx.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002844F8);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
void func_002844F8(Vec4f dst, f32 t, const Vec4f a, const Vec4f b) {
    f32 wa = 1.0f - t;
    int i;
    for (i = 0; i < 12; i++)
        dst[i] = a[i] * wa + b[i] * t;
}
#endif

/** Sum two angles (radians) and wrap the result into [-pi, pi]. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284548);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
f32 func_00284548(f32 a, f32 b) {
    f32 r = a + b;
    if (r >= PR_PI) r -= 2.0f * PR_PI;
    if (r < -PR_PI) r += 2.0f * PR_PI;
    return r;
}
#endif

/** Subtract two angles (radians) and wrap the difference into [-pi, pi]. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284590);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
f32 func_00284590(f32 a, f32 b) {
    f32 r = a - b;
    if (r >= PR_PI) r -= 2.0f * PR_PI;
    if (r < -PR_PI) r += 2.0f * PR_PI;
    return r;
}
#endif

#ifdef TARGET_NATIVE
/* Native canonical-name wrappers: other #else bodies (16E980/1FFBA0) call these
 * by their canonical names (symbol_addrs WrapAnglePiSum/Diff = 0x284548/0x284590,
 * sigs verified (f32,f32)); the matched path still calls them by func_ name and
 * the proper rename is gated on a splat re-split. A forwarding wrapper (not an
 * alias - alias attrs are unsupported on darwin/clang) resolves the native link
 * to the real bodies, portable across clang+gcc, TARGET_NATIVE-only. */
f32 WrapAnglePiSum(f32 a, f32 b)  { return func_00284548(a, b); }
f32 WrapAnglePiDiff(f32 a, f32 b) { return func_00284590(a, b); }
#endif

/**
 * Range-reduce an angle to the symmetric fundamental period: returns
 * (frac(x/(2pi) + 0.5) - 0.5) * 2pi, i.e. x wrapped into [-pi, pi]. Uses an
 * inline truncate-toward-floor (cvt.w.s + sign correction).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002845D8);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
f32 func_002845D8(f32 x) {
    f32 two_pi = 6.28318548202514648f;       /* 0x40C90FDB */
    f32 inv_two_pi = 0.159154936671257019f;  /* 0x3E22F983 */
    f32 t = x * inv_two_pi + 0.5f;
    s32 trunc = (s32)t;
    /* floor via sign-bit correction - the asm corrects with the sign of t
     * (mfc1 from the add.s result, sra 0x1f), NOT the sign of x. For x in
     * (-pi,0), sign(x)=-1 but t in (0,0.5) so sign(t)=0; using sign(x) here put
     * fl off by 1 -> frac off by 1 -> result off by a full 2pi (tester-caught). */
    s32 fl = ((*(s32 *)&t) >> 31) + trunc;
    f32 frac = t - (f32)fl;
    return (frac - 0.5f) * two_pi;
}
#endif

/** Shortest absolute angular distance between angles a and b (radians): result in [0, pi]. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284630);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
f32 func_00284630(f32 a, f32 b) {
    f32 d = a - b;
    d = (d < 0.0f) ? -d : d;
    if (d >= PR_PI) d = 2.0f * PR_PI - d;
    return d;
}
#endif

/** Fractional part of x: x - trunc(x) (VU0 cvt.w.s/cvt.s.w; truncates toward zero). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284668);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
f32 func_00284668(f32 x) {
    return x - (f32)(s32)x;
}
#endif

/** Split x into integer part (stored at *intPart) and return the fractional part. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284678);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
f32 func_00284678(f32 *intPart, f32 x) {
    f32 ip = (f32)(s32)x;
    *intPart = ip;
    return x - ip;
}
#endif
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
/** Floating-point remainder a mod b: a - trunc(a/b)*b (VU0 vdiv/vftoi0/vitof0/vsub/vmul). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002846B0);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
f32 func_002846B0(f32 a, f32 b) {
    f32 q = a / b;
    return (q - (f32)(s32)q) * b;
}
#endif

/**
 * Blend two packed colours c0 ($4) and c1 ($5) by factor t ($f12) and pack the
 * result back to a 4-byte colour: out = trunc(c0*(1-t) + c1*t) per channel.
 * VU0 unpack (vitof0) + vmulaw/vmaddx + vftoi0 + ppach/ppacb.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_002846E8);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
u32 func_002846E8(u32 c0, u32 c1, f32 t) {
    f32 w0 = 1.0f - t;
    u32 out = 0;
    int i;
    for (i = 0; i < 4; i++) {
        f32 a = (f32)((c0 >> (i * 8)) & 0xFF);
        f32 b = (f32)((c1 >> (i * 8)) & 0xFF);
        u32 v = (u32)(s32)(a * w0 + b * t) & 0xFF;
        out |= v << (i * 8);
    }
    return out;
}
#endif

/**
 * Signed-side test of point p ($4) against the plane through point q ($5) with
 * normal n ($6): returns 1 if dot(p - q, n) >= 0 else 0 (the asm computes
 * 1 - signbit(dot)). VU0 vsub/vmul/vmadd.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284730);
#else
/* TODO(match): functional equivalent (VU0 math) - not byte-exact; portable scalar form. */
s32 func_00284730(const Vec4f p, const Vec4f q, const Vec4f n) {
    f32 d = (p[0] - q[0]) * n[0] + (p[1] - q[1]) * n[1] + (p[2] - q[2]) * n[2];
    return (d < 0.0f) ? 0 : 1;
}
#endif

/*
 * Classify a point against a precomputed VU0 frustum/clip volume (constants
 * resident from g_fogColorRed+0x50). Returns -1/0/1 for which side / clip region
 * the transformed point lands in. Runs entirely in VU0 macro-mode against
 * resident VU constants - hardware-coupled, not a portable pure-math helper.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284768);
#else
/* TODO(hle): VU0 clip/frustum classifier using resident VU constants; needs VU backend. */
s32 func_00284768(const Vec4f p, f32 param);
#endif

/*
 * Screen-clip / RLE bit-stream codec helper that wraps the VU0 clip classifier
 * func_00284768 (it jal's into it at the head). Pure byte/bit manipulation but
 * tied to that VU0 classifier; left as asm until the VU backend exists.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284860);
#else
/* TODO(hle): wraps the VU0 clip classifier func_00284768; needs VU backend. */
s32 func_00284860(u8 *dst, u8 *dstEnd, const u8 *src, const u8 *table);
#endif

/*
 * Stack-frame epilogue fragment surfaced by the disassembler as a standalone
 * label (only `addiu $sp` ops, no real body). Not a callable function; kept as
 * asm so the bytes stay in place.
 */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284998);

/* SetVideoMode globals/callees (declared for the TARGET_NATIVE #else only). */
extern s32  g_bPalMode;         /* 0x1A7B98 PAL flag (USA build clears it -> NTSC) */
extern s32  g_bProgressiveScan; /* NTSC 480p progressive-scan flag */
extern void func_124418(void);  /* GS/DMA reset preamble */
extern void sceGsResetGraph(short mode, short inter, short omode, short ffmode);

/** SetVideoMode — reset the GS into the correct scan mode for the current
 *  region/user setting. PAL always forces interlaced (clears progressive), then:
 *  progressive -> sceGsResetGraph(0,0,0x50,1) (NTSC 480p); otherwise interlaced
 *  sceGsResetGraph(0,1,omode,0) with omode = NTSC(2) / PAL(3). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/183558", SetVideoMode);
#else
void SetVideoMode(void) {
    func_124418();
    if (g_bPalMode != 0) {
        g_bProgressiveScan = 0;
    }
    if (g_bProgressiveScan != 0) {
        sceGsResetGraph(0, 0, 0x50, 1);
    } else {
        sceGsResetGraph(0, 1, (g_bPalMode == 0) ? 2 : 3, 0);
    }
}
#endif
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/183558", func_00284A20);
