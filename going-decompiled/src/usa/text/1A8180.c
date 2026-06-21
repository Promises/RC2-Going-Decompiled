#include "common.h"

/*
 * text/1A8180 — the "game-state cluster" (carve-pipeline pick #1, 2026-06-12;
 * vaddr 0x2A8200..0x2B2267, 190 fns): moby helpers (group iteration, ground
 * probes, hit events, list maintenance), small math/easing helpers, and the
 * progress counters (CountPlatinumBolts / CountSkillPointsCompleted /
 * MarkLevelAvailable and friends).
 *
 * The matcher builds THIS unit at -O2 -G8 -fno-gcse (per-unit GFLAG/CC1EXTRA
 * override in tools/ee/objdiff_build.sh / diff.sh / build.sh) — the same
 * later-SN-cc1 TU model as the other gameplay-text units.
 *
 * -G8 extern-sizing rules (see text/1907F0 / text/198FA0), EXTENDED here
 * with the third symbol class proven by this unit's bytes:
 *   - size <= 8: true small data, %gp_rel everywhere;
 *   - size 16: cc1-small / assembler-absolute (lui/$at macro everywhere,
 *     hoisted out of delay slots with a nop);
 *   - size 9..15 (we use 12): gp-addressable / assembler-absolute — the
 *     absolute lui/$at macro in straight-line code, but the 1-insn %gp_rel
 *     form when the access sits in a branch delay slot (proven by
 *     func_002AC9E0 / func_002A9468; asm_unit.sh's delay-slot rule keys on
 *     this size band).
 *
 * SAVE-LAYOUT WALL (same as text/1907F0): this TU was built by the later SN
 * cc1 that packs callee-save slots 8-byte; the pinned cc1 reserves 16 bytes
 * per save. Every function below that saves two or more GPRs (incl. $ra)
 * is blocked on that wall and stays INCLUDE_ASM (not annotated per
 * function - check the prologue: two+ sd of s-regs/$ra at 8-byte spacing).
 *
 * PADDING/FRAGMENT PSEUDO-FUNCTIONS: 36 of the unit's 190 splat symbols are
 * pure inter-function fill (orphaned `addiu $sp,+N / nop` pairs, e.g.
 * func_002A8B00) or unreachable code fragments (e.g. func_002A8628,
 * func_002B1870) - not compiler output reachable from C; they keep their
 * INCLUDE_ASM permanently. The 17 fill runs that had swallowed real
 * prologues were split off via symbol_addrs.txt size pins (2026-06-12).
 */

/* gp-addressable / assembler-absolute symbols (size class 12, see header). */
__asm__(".extern g_mobyTableBase, 12");
__asm__(".extern g_mobyTableEnd, 12");
__asm__(".extern g_mobyGroupCount, 12");
__asm__(".extern g_pMobyGroupIterCursor, 12");
__asm__(".extern g_mobyGroupIterSlot, 12");
__asm__(".extern g_pMobyGroupIterMoby, 12");
__asm__(".extern D_1A8CA4, 12");
__asm__(".extern D_1A8CB0, 12");
__asm__(".extern D_001B1750, 12");
__asm__(".extern D_1A7A4F, 12");
__asm__(".extern D_1A8BD0, 12");
__asm__(".extern g_skillPointFlags, 12");
__asm__(".extern g_abLevelAvailableFlags, 12");

#ifdef TARGET_NATIVE
/* gcc -m32 cannot emulate mode(TI); copy-only here, so a 16-byte aligned struct
 * is an exact portable stand-in. Inert to the matching build. */
typedef struct { unsigned long long _q[2]; } __attribute__((aligned(16))) u_long128;
#else
typedef unsigned long u_long128 __attribute__((mode(TI)));
#endif
typedef struct Vec4 { f32 x, y, z, w; } Vec4;
typedef union QVec { u_long128 q; Vec4 v; } QVec;

/* Minimal moby view (0x100-stride table entries). Field meanings from the
 * Track-B passes; only fields this unit touches are declared. */
typedef struct Moby {
    /* 0x00 */ u8 pad0[0x20];
    /* 0x20 */ s8 state;          /* <0 = inactive/free */
    /* 0x21 */ u8 group;
    /* 0x22 */ u8 classSlot;
    /* 0x23 */ u8 pad23[0x11];
    /* 0x34 */ u16 modeBits;      /* 0x20 = has pvars/extra block (+0x68) */
    /* 0x36 */ u8 pad36[0x32];
    /* 0x68 */ s32 *pExtra;
    /* 0x6C */ u8 pad6C[0x3C];
    /* 0xA8 */ u8 hitEventSlot;   /* index into g_collHitEventRing, 0xFF = none */
    /* 0xA9 */ u8 padA9[1];
    /* 0xAA */ u16 oClass;
    /* 0xAC */ u8 padAC[0x10];
    /* 0xBC */ u8 lightMode;
    /* 0xBD */ u8 padBD[1];
    /* 0xBE */ u8 animFlags;
    /* 0xBF */ u8 padTail[0x100 - 0xBF]; /* pad this local field-view out to the
                                            canonical Moby SIZE (0x100, see moby.h)
                                            so the functional-equivalence tester can
                                            allocate + bind a full Moby. This keeps
                                            the TU's own field names (which diverge
                                            from moby.h's) - only the total size is
                                            unified. Byte-neutral: trailing only, no
                                            field above this moves. */
} Moby;
/* Verify the view spans the full canonical 0x100 record (ILP32 only — the
 * matching ee-gcc 2.9 lacks both _Static_assert and __SIZEOF_POINTER__, and a
 * 64-bit host widens pExtra; same guard as include/moby.h). */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(Moby) == 0x100, "Moby must be 0x100 under ILP32");
#endif

/* Moby animation header view for func_002A85B8 (anim id bookkeeping). */
typedef struct MobyAnim {
    /* 0x00 */ u8 pad0[0x20];
    /* 0x20 */ u8 animId;
    /* 0x21 */ u8 pad21[0x73];
    /* 0x94 */ u8 prevAnimId;
    /* 0x95 */ s8 subAnim;
    /* 0x96 */ u16 animTimer;
    /* 0x98 */ u8 pad98[0x26];
    /* 0xBE */ u8 animFlags;
} MobyAnim;

/* True small / gp-addressable globals (complete <=8-byte declarations). */
extern Moby *g_mobyTableBase;          /* moby table base (slot stride 0x100) */
extern Moby *g_mobyTableEnd;           /* moby table end (walk bound) */
extern s32 g_mobyGroupCount;           /* number of moby groups */
extern u16 *g_pMobyGroupIterCursor;    /* group iterator: current list entry */
extern u16 g_mobyGroupIterSlot;        /* group iterator: current slot index */
extern Moby *g_pMobyGroupIterMoby;     /* group iterator: current moby */
extern s32 D_1A8CA4;                   /* breath/oxygen HUD inversion flag */
extern f32 D_1A8CB0;                   /* inverted breath meter source value */
extern f32 D_001B1750;                 /* breath meter source value */
extern u8 D_1A7A4F;                    /* "freeze palette cycling" flag */
extern f32 D_1A8BD0;                   /* default hit-direction Vec4 for func_002A9F30 (declared as its first float so cc1 schedules the address materialisation as one insn) */

/* Large/absolute globals. */
extern u16 *g_mobyGroupLists[];        /* per-group i16 slot lists (negative-terminated) */
extern Moby *g_pHeroMoby[];            /* incomplete-array decl: keeps the 4-byte ptr out of cc1 small data (0x18C0B0 is outside the gp range) */
extern Vec4 g_collHitPoint;
extern Vec4 g_collHitPointNudged;
extern u8 g_skillPointFlags[8];   /* really u8[0x20]; declared 8 so cc1 schedules the address materialisation as one insn (gp-range, assembler-absolute) */
extern u8 g_platinumBoltFlags[];
extern u8 g_itemEquippedSlot[0x38];
typedef struct WeaponVariant {
    /* 0x00 */ s32 exists;
    /* 0x04 */ u8 upgradeLevel;
    /* 0x05 */ u8 pad[0xDB];
} WeaponVariant;
extern WeaponVariant g_weaponTable[];  /* 0xE0-stride weapon-variant table */
extern u8 g_abLevelAvailableFlags[8]; /* really u8[0x1C]; same cc1-small address model as g_skillPointFlags */
extern s32 g_anAvailableLevelOrder[0x1C];
extern u8 g_collHitEventRing[];        /* 64 x 0x40 hit-event records */
extern u8 D_139648[];                  /* palette-cycle counter base (incomplete-array decl: out of gp range) */
extern s32 D_258CF0[0x15];             /* per-level lookup table */

/* Callees (value-returning declarations keep cc1 from sibling-call
 * optimising forwarding tails — see text/198FA0). */
extern s32 CollLine(void *to, void *from, s32 mask, void *moby, void *out);
extern f32 ProbeGroundHeight(Vec4 *pos, f32 zOffset, s32 mask);
extern s32 CollSphere(void *center, s32 mask, void *moby);
extern s32 func_001163B0(void);        /* core random-state step */
extern s32 ProbeMobyGroundBelow(Moby *moby);
extern s32 func_002846E8(void *a, void *b, void *c);
/* func_002846E8's real prototype: blend vec4 dst from src by phase t (t in
 * $f12). The (void*,void*,void*) form above is only used by the pure
 * register-passthrough forwarder func_002AA3D0. */
typedef void (*LerpByteVec4PackedFn)(f32 t, void *dst, void *src);
#define LerpByteVec4PackedVu0 ((LerpByteVec4PackedFn)func_002846E8)
extern s32 func_0029DA88(s32 a);
extern char *GetLocalizedString(s32 stringId);
extern s32 func_0029DAD0(char *text, s32 arg);
extern s32 func_002B0E40(void *p);
extern s32 func_002B0F40(void *p);
extern s32 func_002B0C40(s32 ctx, void *out, void *a, void *b);
extern s32 func_002837D0(void *vec);
extern s32 func_00283BF8(f32 x, f32 y);
extern s32 func_002A12C0(void *p, s32 r, s32 g, s32 b);
extern s32 func_00283638(Moby *moby);
extern s32 PostMobyHitEvent(Moby *moby, s32 a, s32 b, s32 c, Vec4 *dir);
extern s32 func_002B1C20(void);
extern s32 func_002A9550(Moby **out, Moby *moby);
extern f32 func_002837F8(void *p, f32 *src);
extern s32 func_002AFAB0(f32 step, f32 max);
extern f32 GetFloatAbs(f32 x);
extern s32 func_002835E0(s32 x);
extern f32 func_00284678(f32 *out, f32 angle);
extern f32 func_00283B30(f32 angle);  /* cosine */
extern f32 func_00283B48(f32 angle);  /* sine (0x18 after cosine in 183558.c) */
extern f32 func_00284590(f32 a, f32 b);
/* func_002AB000: spring-style scalar approach. Clamps an absolute step
 * (|v0|-bounded) applied to *p toward 0 by a velocity term, re-clamping into
 * +/-|v0| and returning the residual |distance| in $f0. v0..v3 arrive in
 * $f12..$f15. Defined later in this unit (still INCLUDE_ASM); declared here so
 * the #else bodies above can call it with the right f32 return. */
extern f32 func_002AB000(f32 *p, f32 v0, f32 v1, f32 v2, f32 v3);
/* func_00284548 == WrapAnglePiSum, which genuinely returns f32 in $f0. The
 * native #else of func_002AB668 needs the true f32 return (else ee-gcc inserts a
 * spurious int->float cvt that corrupts the *p out-param, else_divergences #19).
 * BUT the matching build's func_002AAFB8 (line ~768) byte-matches ONLY with the
 * s32 form: it returns s32 from this call, so the s32-callee + s32-return type
 * errors cancel into the exact $f0 passthrough the original emits (verified: a
 * plain f32 here regresses func_002AAFB8 from 100% to 88.24%). So guard per
 * build - matching keeps s32, native gets the correct f32. */
#ifdef TARGET_NATIVE
extern f32 func_00284548(f32 a, f32 b);
#else
extern s32 func_00284548(f32 a, f32 b);
#endif
extern void Vec4SubVu0(Vec4 *dst, Vec4 *a, Vec4 *b);
extern void Vec4ScaleVu0(Vec4 *dst, f32 s, const Vec4 *src);   /* sig: scale BEFORE src (matches the def in 183558.c) */
extern void Vec3RescaleToLenVu0(Vec4 *dst, f32 len, Vec4 *src);
extern void Vec4AddVu0(Vec4 *dst, Vec4 *a, Vec4 *b);           /* dst = a + b (xyz), via VU0 (183558.c) */
extern void SetVec4UnitZ(Vec4 *dst);                           /* writes a unit +Z vec4 via vmr32 of vf0 */
extern void ScaleVec4IncludingW(Vec4 *dst, f32 scale, Vec4 *src); /* vec4 scale incl. w; scale in $f12 */
/* VU0 reductions that genuinely return their scalar in $f0 — they MUST be
 * declared f32-returning for the native #else bodies below: an s32 return here
 * would make ee-gcc insert a spurious cvt.s.w that corrupts the value
 * (else_divergences #19 class). No matched function in this unit calls them, so
 * the f32 type is inert to the matched build. */
extern f32 Vec3DotVu0(Vec4 *a, Vec4 *b);                       /* 3-component dot product */
extern f32 Vec3LengthVu0(Vec4 *v);                             /* 3-component length (vsqrt) */
extern f32 DistXYVu0(Vec4 *a, Vec4 *b);                        /* horizontal xy distance */
extern s32 GetCollHitMaterial(void);                           /* low 5 bits of hit poly info, -1 if none */
/* SampleWaterHeightfield(x, y, z, &outHeight): bilinear-samples the dynamic wave
 * lattice; returns nonzero on a valid sample and writes the surface height to
 * *outHeight (the x/y/z are passed in $f12/$f13/$f14). */
extern s32 SampleWaterHeightfield(f32 x, f32 y, f32 z, f32 *outHeight);
/* Quaternion helpers (183558.c region). func_00284248 builds an axis-angle
 * quaternion (axis 0/1/2 = x/y/z) into *out; func_00284180 multiplies two
 * quaternions a*b into *out. */
extern void func_00284248(Vec4 *out, f32 angle, s32 axis);
extern void func_00284180(Vec4 *out, Vec4 *a, Vec4 *b);

/* 4x4 row-major matrix (4 Vec4 rows). func_00284048 builds a rotation matrix
 * from a quaternion (src Vec4 quat -> dst Mat4x4); func_00283A70 transforms a
 * Vec4 by a Mat4x4 (dst = mat * vec). Both are VU0 (183558.c region). */
typedef struct Mat4x4 { Vec4 row[4]; } Mat4x4;
extern void func_00284048(Mat4x4 *dst, const Vec4 *quat);
extern void func_00283A70(Vec4 *dst, const Vec4 *vec, const Mat4x4 *mat);
/* FloatToInt: truncate a float to an s32 (cvt.w.s style). */
extern s32 FloatToInt(f32 x);

/* Water surface globals (the g_waterPool Vec4 packs xy-centre, z-surface,
 * w-radius; see symbol_addrs). */
extern s32 g_bWaterWavesActive;        /* gate for the wave-heightfield path */
extern s32 g_bWaterPoolActive;         /* gate for the static water-pool test */
extern Vec4 g_waterPool;               /* xy centre, z surface height, w radius */

/* Blob-shadow drop queue (drawn under mobies that pass the ground probe). */
extern Vec4 g_collHitNormal;           /* face normal of the last collision hit */
extern s32 g_blobShadowCount;          /* live entries in g_blobShadowQueue (max 32) */
typedef struct BlobShadow {
    /* 0x00 */ Vec4 pos;               /* shadow centre (moby pos +0x10), z bumped to ground */
    /* 0x10 */ Vec4 normal;            /* ground normal (g_collHitNormal) */
} BlobShadow;                          /* 0x20 stride */
extern BlobShadow g_blobShadowQueue[]; /* 32-entry ring (count in g_blobShadowCount) */

/* Per-moby ground-probe mode flag: 0 = straight downward drop probe, nonzero =
 * probe along the moby's own axis vector (+0xE0). */
extern s32 D_1A8CA0;


INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8200);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A82D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8448);

/* func_002A85B8: begin a moby animation (stash prev anim id, reset the anim
 * timer, clear the done flags, optional sub-anim). Best attempt 58%: the
 * pinned cc1 schedules the animFlags store before the timer clear and
 * recomputes the second mask from the original flags where the later cc1
 * keeps an incremental masked value and a -1 compare register in a0 - the
 * register-coloring/store-scheduling wall. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A85B8);
#else
void func_002A85B8(MobyAnim *m, s32 animId, s32 subAnim) {
    u8 flags = m->animFlags & 0xFE;   /* clear "done" bit 0 */
    u8 oldId = m->animId;

    m->animId = animId;
    m->prevAnimId = oldId;
    m->animTimer = 0;
    m->animFlags = flags;
    if (subAnim != -1) {
        m->subAnim = subAnim;
        m->animFlags = flags & 0xFD; /* also clear bit 1 for the sub-anim path */
    }
}
#endif

/**
 * Quadratic ease-in: x squared.
 */
f32 func_002A85F8(f32 x) {
    return x * x;
}

/**
 * Quadratic ease-out: 1 - (1-x)^2.
 */
f32 func_002A8600(f32 x) {
    f32 t = 1.0f - x;

    return 1.0f - t * t;
}

/* unreachable code fragment (stray FP tail from splat over-split), not C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8628);

/**
 * Uniform random integer in [0, n): take a 15-bit random value from the core
 * LCG (func_001163B0() >> 16 & 0x7FFF) and reduce it modulo n.
 *
 * The asm shifts the 31-bit LCG output right by 16 (arithmetic, but bit 31 is
 * already clear so it is a logical shift in effect), masks to 15 bits, then
 * computes the remainder by n. n == 0 traps on hardware (break 0x7 in the div
 * delay slot); no caller passes 0, so the native shim leaves that path to the
 * platform divide. NATIVE SHIM (no byte target; matching build uses asm).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", GetRandomInt);
#else
s32 GetRandomInt(s32 n) {
    s32 r = (func_001163B0() >> 16) & 0x7FFF;
    return r % n;
}
#endif

/* func_002A8688: uniform random integer in [lo, hi] (inclusive) — take a 15-bit
 * random value from the core LCG (func_001163B0() >> 16 & 0x7FFF) and reduce it
 * modulo the span (hi - lo + 1), then bias by lo. Walled: saves $16/$17/$31
 * (save-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8688);
#else
s32 func_002A8688(s32 lo, s32 hi) {
    s32 span = hi - lo + 1;
    s32 r = (func_001163B0() >> 16) & 0x7FFF;

    return r % span + lo;
}
#endif

/**
 * Uniform random float in [lo, hi): scale a 15-bit random fraction
 * (rand>>16 & 0x7FFF) / 32768 across the (hi - lo) span.
 */
f32 func_002A86E0(f32 lo, f32 hi) {
    return lo + (f32)((func_001163B0() >> 16) & 0x7FFF) * (hi - lo) * 0.000030517578125f;
}

/**
 * Random signed offset in [lo, hi): a 12-bit random fraction (rand>>16 & 0xFFF)
 * scaled across the (hi - lo) span and added to lo, with its sign flipped when
 * the random value is odd.
 */
f32 func_002A8740(f32 lo, f32 hi) {
    s32 r = func_001163B0() >> 16;
    f32 v = lo + (f32)(r & 0xFFF) * (hi - lo) * 0.000244140625f;

    if (r & 1) {
        v = -v;
    }
    return v;
}

/**
 * Random small angle: uniform in [-0x800, 0x800) scaled by pi/2048 —
 * i.e. a random angle in [-pi, pi).
 */
f32 func_002A87A8(void) {
    return (f32)(((func_001163B0() >> 16) & 0xFFF) - 0x800) * 0.0015339808f;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A87F0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8868);

/**
 * Cubic blend through two value pairs: hermite-style interpolation of the
 * (a0,a1) -> (b0,b1) segment at parameter t.
 */
f32 func_002A8910(f32 a1, f32 a0, f32 b0, f32 b1, f32 t) {
    f32 c = (b1 - b0) - (a1 - a0);
    f32 t2 = t * t;
    f32 t3 = t2 * t;

    return c * t3 + ((a1 - a0) - c) * t2 + (b0 - a1) * t + a0;
}

/* func_002A8948: per-component cubic (Catmull-Rom-style) blend of four control
 * vectors p1..p4 into out at parameter t - for each of x/y/z it is the vector
 * form of func_002A8910(p3, p1, p2, p4, t), with w forced to 0; t==0 copies p1
 * and t==1 copies p2 verbatim (128-bit lq/sq). Best attempt 78.7%: structure
 * (the two endpoint shortcuts incl. the bc1fl with the hoisted p3.x delay-slot
 * load, the lq/sq quad copies, the three per-axis cubic evaluations) all
 * reproduce, but the per-component FP arithmetic colours its temporaries into
 * different physical registers than the original and the endpoint sq lands a
 * slot earlier - the register-coloring + store-scheduling wall. Re-derived from
 * func_002A8910 (the matched scalar twin); not byte-reachable with the pinned
 * cc1. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8948);
#else
void func_002A8948(Vec4 *out, Vec4 *p1, Vec4 *p2, Vec4 *p3, Vec4 *p4, f32 t) {
    if (t == 0.0f) {
        *out = *p1;
        return;
    }
    if (t == 1.0f) {
        *out = *p2;
        return;
    }
    /* per axis: scalar cubic of the (p3,p1)->(p2,p4) segment; w := 0 */
    out->x = func_002A8910(p3->x, p1->x, p2->x, p4->x, t);
    out->y = func_002A8910(p3->y, p1->y, p2->y, p4->y, t);
    out->w = 0.0f;
    out->z = func_002A8910(p3->z, p1->z, p2->z, p4->z, t);
}
#endif

/**
 * Cosine ("smootherstep"-style) ease between a and b by t in [0,1]: shortcut
 * the endpoints (t==0 -> a, t==1 -> b), otherwise blend by (1 - cos(t*pi))/2.
 */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - 95.7%. The two endpoint c.eq.s shortcuts
   and the (1 - cos(t*pi))*0.5 blend reproduce, but the later cc1 schedules the
   compare's zero/one constant materialisation differently from the pinned cc1
   (operand/const-scheduling wall). Revisit once the gameplay-TU compiler is
   available. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8A68);
#else
f32 func_002A8A68(f32 a, f32 b, f32 t) {
    if (t == 0.0f) {
        return a;
    }
    if (t == 1.0f) {
        return b;
    }
    return a + (b - a) * ((1.0f - func_00283B30(t * 3.14159274f)) * 0.5f);
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8B00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8B08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8C70);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8CF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8D08);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A90A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A90A8);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9348);

/* func_002A9370: count a group's active mobys filtered by anim state. When
 * state == -1 every active (state byte >= 0) moby in the group is counted;
 * otherwise the count is of the active mobys whose state byte differs from
 * `state` (the equal-state ones are skipped). group == -1 returns 0. Best
 * attempt 82%: byte-identical except the list pointer colours v1 (reusing
 * the address temp) where the original loads it into v0 with a later move
 * into a0 - the register-coloring wall. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9370);
#else
s32 func_002A9370(s32 group, s32 state) {
    u16 *list;
    s32 count;

    if (group == -1) {
        return 0;
    }
    list = g_mobyGroupLists[group];
    if (list == 0) {
        return 0;
    }
    count = 0;
    do {
        u16 entry = *list;
        Moby *moby = (Moby *)((u8 *)g_mobyTableBase + (entry & 0x7FFF) * 0x100);

        if (moby->state >= 0) {
            if (state == -1) {
                count = count + 1;
            } else if ((u8)moby->state != state) {
                count = count + 1;
            }
        }
        list = list + 1;
        if ((s16)entry < 0) {
            break;
        }
    } while (1);
    return count;
}
#endif

/**
 * Set the light-mode byte of every moby in a group.
 */
void func_002A9400(s32 group, s32 lightMode) {
    u16 *list = g_mobyGroupLists[group];

    if (list != 0) {
        u16 *p = list;
        s16 entry;

        do {
            ((Moby *)((u8 *)g_mobyTableBase + (*p & 0x7FFF) * 0x100))->lightMode = lightMode;
            entry = *p;
            p = p + 1;
        } while (entry >= 0);
    }
}

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9458);

/**
 * Begin iterating a moby group: validate the group index, point the iterator
 * globals at the group's slot list, fetch the first moby into *out, then apply
 * the active/inactive filter (wantInactive/wantActive) before chaining to the
 * iterator-advance helper. Returns -1 for an empty/invalid group, 0 when the
 * first moby is filtered out, else the advance helper's result.
 */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - 85%. The filter lattice and the
   size-12 %hi/%lo-vs-%gp_rel reload of g_pMobyGroupIterMoby reproduce, but the
   later cc1 schedules the wantActive-path reload straight-line (absolute
   lui/lw) with the $ra restore in the branch delay slot, while the pinned cc1
   fills that delay slot with the reload itself (forced %gp_rel form) and folds
   the two return paths - register-coloring + delay-slot wall (same family as
   func_002AC9E0). Revisit once the gameplay-TU compiler is available. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9468);
#else
s32 func_002A9468(Moby **out, s32 group, s32 wantInactive, s32 wantActive) {
    u16 *list;
    s32 slot;
    s32 sign;

    if (group < 0 || g_mobyGroupCount < group) {
        *out = 0;
        return -1;
    }
    *out = 0;
    g_pMobyGroupIterMoby = 0;
    list = g_mobyGroupLists[group];
    g_pMobyGroupIterCursor = list;
    if (list == 0) {
        return -1;
    }
    slot = *list & 0x7FFF;
    g_mobyGroupIterSlot = slot;
    g_pMobyGroupIterMoby = (Moby *)((u8 *)g_mobyTableBase + slot * 0x100);
    *out = g_pMobyGroupIterMoby;
    sign = (u32)(s32)g_pMobyGroupIterMoby->state >> 31;
    if (wantInactive == 0) {
        if (wantActive == 0 && sign == 0) {
            return 0;
        }
    } else {
        if (wantActive == 0) {
            return 0;
        }
        if (sign != 0) {
            return 0;
        }
    }
    return func_002A9550(out, g_pMobyGroupIterMoby);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9550);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A96B8);

/**
 * Decrement the 8-bit countdown packed in the top byte of *p (clamped at
 * zero); returns nonzero once it reaches zero.
 */
s32 func_002A96C8(u32 *p, s32 dec) {
    s32 t = ((s32)*p >> 24) - dec;

    if (t < 0) {
        t = 0;
    }
    *p = (*p & 0xFFFFFF) | (t << 24);
    return t == 0;
}

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9700);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9708);

/* ProbeGroundHeight: ground height under a point - CollLine from z=0.01 up
 * to pos.z + zOffset, returns the hit z or 0 (the 0.5f/0x20 defaults come
 * from func_002A9888). Best attempt 72% (volatile 128-bit copy pinning +
 * an asm scheduling barrier recover the copy/const order): the pinned cc1
 * still hoists the call-argument moves above the second source copy where
 * the later cc1 keeps them below - prologue-scheduling wall. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", ProbeGroundHeight);
#else
f32 ProbeGroundHeight(Vec4 *pos, f32 zOffset, s32 mask) {
    QVec from;
    QVec to;

    from.q = *(u_long128 *)pos;
    from.v.z = 0.00999999978f;          /* 0x3C23D70A == 0.01f */
    to.q = *(u_long128 *)pos;
    to.v.z = pos->z + zOffset;
    if (CollLine(&to, &from, mask | 2, 0, 0) == 0) {
        return 0.0f;
    }
    return g_collHitPoint.z;
}
#endif

/**
 * Ground probe with the default parameters: half a unit above the query
 * point, line mask 0x20.
 */
f32 func_002A9888(Vec4 *pos) {
    return ProbeGroundHeight(pos, 0.5f, 0x20);
}

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A98B0);

/* func_002A98B8: first polygon edge (0x10-stride xy verts) the point lies
 * left of, 1-based; 0 = inside. Best attempt 90%: the original emits the
 * div-by-zero check of the i%%n twice (one hoisted to the loop top) around
 * a single CSEd div - the div-expansion-scheduling wall (same family as
 * func_00351328). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A98B8);
#else
s32 func_002A98B8(f32 *pt, f32 *verts, s32 n) {
    f32 px = pt[0];
    f32 py = pt[1];
    s32 i;

    if (n <= 0) {
        return 0;
    }
    for (i = 0; i < n; i++) {
        s32 j = (i + 1) % n;
        f32 *cur = (f32 *)((u8 *)verts + i * 0x10);
        f32 *nxt = (f32 *)((u8 *)verts + j * 0x10);
        f32 ex = px - cur[0];
        f32 ey = py - cur[1];
        f32 dx = nxt[0] - cur[0];
        f32 dy = nxt[1] - cur[1];
        f32 cross = dx * ey - dy * ex;

        if (0.0f < cross) {
            return i + 1;
        }
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9958);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9A28);

/**
 * Re-target an active hit record: store the new owner/params and clear its
 * pending flag, then forward the moby to the hit-refresh helper.
 */
s32 func_002A9A38(f32 power, Moby *moby, s32 a, s32 b) {
    Moby *m = moby;

    *(s32 *)((u8 *)m + 0x10) = a;
    *(s32 *)((u8 *)m + 0x14) = b;
    *(f32 *)((u8 *)m + 0x1C) = power;
    *(s32 *)((u8 *)m + 0x20) = 0;
    return func_00283638(moby);
}

/* func_002A9A68: fill a hit-event record (params + 128-bit source vector +
 * live flag). Best attempt 58%: the pinned cc1 materialises the li 1 after
 * the first store (original: first insn, in v1) and insists on filling the
 * jr delay slot with the volatile sq - register-coloring + slot-fill wall. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9A68);
#else
void func_002A9A68(void *rec, s32 a, s32 b, f32 power, void *src) {
    *(s32 *)((u8 *)rec + 0x10) = a;
    *(s32 *)((u8 *)rec + 0x14) = b;
    *(f32 *)((u8 *)rec + 0x1C) = power;
    *(s32 *)((u8 *)rec + 0x20) = 1;          /* live flag */
    *(u_long128 *)rec = *(u_long128 *)src;    /* 128-bit source vector at +0x00 */
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9A90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9BD8);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9C80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9C88);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", PostMobyHitEvent);

/**
 * Post a moby hit event with the default hit direction vector.
 */
s32 func_002A9F30(Moby *moby, s32 a, s32 b, s32 c) {
    return PostMobyHitEvent(moby, a, b, c, (Vec4 *)&D_1A8BD0);
}

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9F58);

/* GetWaterSurfaceHeight: water surface z (up axis) under a point. First tries
 * the dynamic wave heightfield (when g_bWaterWavesActive); otherwise tests the
 * static water-pool disc g_waterPool (xy centre, z surface, w radius) — the
 * point is "in the pool" when |pos.z - pool.z| < 0.5 and its xy distance to the
 * pool centre is within the radius. Returns the pool surface z when inside,
 * else the point's own z. When outNormal is non-null it gets a unit +Z normal.
 *
 * Walled: saves $16/$17/$31 (save-layout wall, see unit header). The VU0
 * reductions DistXYVu0/GetFloatAbs and SampleWaterHeightfield all return f32 in
 * $f0 / via the out-pointer; declared accordingly so this body is faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", GetWaterSurfaceHeight);
#else
f32 GetWaterSurfaceHeight(Vec4 *pos, Vec4 *outNormal) {
    f32 height;

    if (g_bWaterWavesActive != 0) {
        if (SampleWaterHeightfield(pos->x, pos->y, pos->z, &height) != 0) {
            return height;
        }
    }
    if (g_bWaterPoolActive != 0 &&
        GetFloatAbs(pos->z - g_waterPool.z) < 0.5f &&
        DistXYVu0(pos, &g_waterPool) < g_waterPool.w) {
        if (outNormal != 0) {
            SetVec4UnitZ(outNormal);
        }
        return g_waterPool.z;
    }
    if (outNormal != 0) {
        SetVec4UnitZ(outNormal);
    }
    return pos->z;
}
#endif

/* func_002AA058: euler-angles → quaternion. Builds three axis-angle quaternions
 * from eulerAngles.x/.y/.z about axes 0/1/2 and concatenates them
 * (out = qx * qy, then out = out * qz) via the VU0 quaternion helpers.
 *
 * Walled: saves $16-$19/$31 (save-layout wall). The three scratch quaternions
 * live on the stack; func_00284248 takes its angle in $f12. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AA058);
#else
void func_002AA058(Vec4 *outQuat, Vec4 *eulerAngles) {
    Vec4 qx;
    Vec4 qy;
    Vec4 qz;

    func_00284248(&qx, eulerAngles->x, 0);
    func_00284248(&qy, eulerAngles->y, 1);
    func_00284248(&qz, eulerAngles->z, 2);
    func_00284180(outQuat, &qx, &qy);
    func_00284180(outQuat, outQuat, &qz);
}
#endif

/* QueueMobyBlobShadow: enqueue a ground blob shadow under a moby (drop-shadow
 * render pass). No-op unless the moby's shadow-enable byte (+0x31) is set and
 * the queue has room (< 32). Copies the moby position (+0x10) into the next slot, probes
 * the ground straight down (ProbeGroundHeight, zOffset 0.5, mask 0); on a real
 * surface (material != none) lifts the shadow 0.025 above the ground, stores the
 * hit normal, and computes an alpha from the moby's height above the ground
 * (written to slot +0xC): alpha = baseAlpha * max(0.125*(8 - |moby.z - groundZ|), 0.25).
 *
 * Walled: saves $16/$17/$31 + $f20/$f21 (save-layout wall). baseAlpha arrives
 * in $f12 (preserved across the probe in $f21); groundZ is kept in $f20. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", QueueMobyBlobShadow);
#else
void QueueMobyBlobShadow(Moby *moby, f32 baseAlpha) {
    BlobShadow *slot;
    f32 groundZ;
    f32 lift;
    f32 alpha;

    if (*(u8 *)((u8 *)moby + 0x31) == 0) {
        return;
    }
    if (g_blobShadowCount >= 0x20) {
        return;
    }
    slot = &g_blobShadowQueue[g_blobShadowCount];
    slot->pos = *(Vec4 *)((u8 *)moby + 0x10);
    groundZ = ProbeGroundHeight((Vec4 *)((u8 *)moby + 0x10), 0.5f, 0);
    if (GetCollHitMaterial() == 0) {
        return;
    }
    slot = &g_blobShadowQueue[g_blobShadowCount];
    slot->pos.z = groundZ + 0.0250000004f;     /* 0x3CCCCCCD == 0.025f */
    slot->normal = g_collHitNormal;
    lift = (8.0f - GetFloatAbs(*(f32 *)((u8 *)moby + 0x18) - groundZ)) * 0.125f;
    if (lift < 0.25f) {
        lift = 0.25f;
    }
    alpha = baseAlpha * lift;
    slot = &g_blobShadowQueue[g_blobShadowCount];
    g_blobShadowCount = g_blobShadowCount + 1;
    slot->pos.w = alpha;
}
#endif

/* ProbeMobyGroundBelow: ground-fit probe for a moby's drop shadow / ground snap.
 * Two modes selected by D_1A8CA0: in the default (0) mode, a downward CollLine
 * (mask 0x22, skipping water material 0) is cast from the moby position
 * (+0x10) over a 16-unit drop — but only when the moby's "on-ground" factor
 * (+0xE8) is at least 0.9; the resulting ground z-delta (g_collHitPoint.z minus
 * moby.z) goes to moby+0x70 and a scale (moby+0xC * 1/4096) to moby+0x74. In
 * the axis mode (nonzero), the ray is cast along the moby's scaled axis vector
 * (+0xE0): on a hit it stores the dot of (hitPoint - moby.pos) with the axis to
 * moby+0x70, a fixed 0.2 to +0x74, and flags moby+0xBD = 0xFF. No hit (or the
 * default-mode height gate) zeroes moby+0x70/+0x74.
 *
 * Walled: saves $16-$18/$31 (save-layout wall). The 1/1024, -8.0, 1/4096 and
 * 0.2 constants come straight from the asm immediates. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", ProbeMobyGroundBelow);
#else
/* Returns s32 only to match the forwarder func_002AA3B0's prototype; the asm
 * leaves $2 holding store-scratch (no meaningful result), so callers ignore it.
 * All real output is written into the moby record (+0x70/+0x74/+0xBD). */
s32 ProbeMobyGroundBelow(Moby *moby) {
    Vec4 from;
    Vec4 to;
    Vec4 hitDelta;
    f32 zDelta;
    f32 scale;

    if (D_1A8CA0 == 0) {
        if (*(f32 *)((u8 *)moby + 0xE8) < 0.9f) {
            *(s32 *)((u8 *)moby + 0x74) = 0;
            *(s32 *)((u8 *)moby + 0x70) = 0;
            return 0;
        }
        from = *(Vec4 *)((u8 *)moby + 0x10);
        from.z = *(f32 *)((u8 *)moby + 0x18) - 16.0f;
        if (from.z < 0.5f) {
            from.z = 0.5f;
        }
        to = *(Vec4 *)((u8 *)moby + 0x10);
        to.z = *(f32 *)((u8 *)moby + 0x18) + 0.5f;
        if (CollLine(&to, &from, 0x22, 0, 0) != 0) {
            zDelta = g_collHitPoint.z - *(f32 *)((u8 *)moby + 0x18);
            scale = *(f32 *)((u8 *)moby + 0xC) * 0.000244140625f;   /* 1/4096 */
            *(f32 *)((u8 *)moby + 0x70) = zDelta;
            *(f32 *)((u8 *)moby + 0x74) = scale;
            return 0;
        }
    } else {
        /* `to` = 1/1024 * moby (a near-zero query point); `from` = that point
         * minus 8 along the moby axis (+0xE0). CollLine casts to<-from. */
        ScaleVec4IncludingW(&to, 0.0009765625f, (Vec4 *)moby);   /* 1/1024 */
        Vec4ScaleVu0(&from, -8.0f, (Vec4 *)((u8 *)moby + 0xE0));
        Vec4AddVu0(&from, &to, &from);
        if (CollLine(&to, &from, 0x22, 0, 0) != 0) {
            Vec4SubVu0(&hitDelta, &g_collHitPoint, (Vec4 *)((u8 *)moby + 0x10));
            zDelta = Vec3DotVu0(&hitDelta, (Vec4 *)((u8 *)moby + 0xE0));
            scale = 0.200000003f;
            *(u8 *)((u8 *)moby + 0xBD) = 0xFF;
            *(f32 *)((u8 *)moby + 0x70) = zDelta;
            *(f32 *)((u8 *)moby + 0x74) = scale;
            return 0;
        }
    }
    *(s32 *)((u8 *)moby + 0x74) = 0;
    *(s32 *)((u8 *)moby + 0x70) = 0;
    return 0;
}
#endif

/**
 * Forward to the moby ground-fit probe (drop-shadow placement).
 */
s32 func_002AA3B0(Moby *moby) {
    return ProbeMobyGroundBelow(moby);
}

/**
 * Forward to the shared 2-colour blend helper func_002846E8.
 *
 * TYPE NOTE: a/b/c stay void* on purpose. func_002846E8 reads its GPR args by
 * VALUE via pextlb/pextlh (they ARE packed 32-bit RGBA colour words, not
 * pointers) and never dereferences them; c is unused by the callee. So these
 * are not Vec4* (the earlier "vector op" hypothesis is disproven by the asm).
 */
s32 func_002AA3D0(void *a, void *b, void *c) {
    return func_002846E8(a, b, c);
}

/* Two free-running phase counters for the ping-pong colour ramp; the selector
 * arg picks which one to advance (gp-addressable small data). */
extern s32 D_1A9E94;
extern s32 D_1A9E98;

/**
 * Advance a ping-pong colour ramp: bump (or reset) one of two free-running
 * phase counters, fold it into a triangle wave over [0,2*period) to a 0..1
 * blend, and blend between the two packed colours by t.
 *
 * TYPE NOTE: dst/src stay void* on purpose. They flow straight into
 * func_002846E8, which consumes its arg registers by VALUE as packed 32-bit
 * RGBA colour words (pextlb/pextlh, no memory load) and returns the blended
 * word; they are not Vec4* pointers.
 */
void func_002AA3F0(void *dst, void *src, s32 period, s32 useSecond, s32 reset) {
    s32 *phase = &D_1A9E98;
    s32 pos;
    f32 t;

    if (!useSecond) {
        phase = &D_1A9E94;
    }

    *phase = *phase + 1;
    if (reset != 0) {
        *phase = 0;
    }
    pos = *phase % (period * 2);
    if (pos >= period) {
        t = 1.0f - (f32)(pos - period) / (f32)period;
    } else {
        t = (f32)pos / (f32)period;
    }
    LerpByteVec4PackedVu0(t, src, dst);
}

/**
 * Query a moby's pending hit-event record: returns the record when it
 * carries any of the requested event bits, otherwise 0 (releasing the
 * record unless keep is set).
 */
s32 func_002AA4A8(Moby *m, u32 mask, s32 keep) {
    u8 slot = m->hitEventSlot;
    u8 *rec;
    Moby *owner;

    if (slot == 0xFF) {
        return 0;
    }
    rec = &g_collHitEventRing[slot << 6];
    owner = *(Moby **)(rec + 0x38);
    if (owner != m) {
        return 0;
    }
    if ((*(u32 *)(rec + 0x24) & mask) != 0) {
        return (s32)rec;
    }
    if (keep == 0) {
        owner->hitEventSlot = 0xFF;
    }
    return 0;
}

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AA500);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AA508);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AA6B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AA808);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AAF98);

/**
 * Linear interpolation: a + (b - a) * t.
 */
f32 func_002AAFA8(f32 a, f32 b, f32 t) {
    return a + (b - a) * t;
}

/**
 * Two-stage scalar transform: feed (b, a) through func_00284590, scale the
 * result by c, and forward (a, scaled) to func_00284548.
 */
s32 func_002AAFB8(f32 a, f32 b, f32 c) {
    return func_00284548(a, func_00284590(b, a) * c);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB000);

/**
 * Step a float toward a target by at most rate, returning how far from the
 * target the stepped value still is (absolute).
 */
f32 func_002AB150(f32 target, f32 rate, f32 *p) {
    f32 d = target - *p;
    f32 nv;

    if (rate < d) {
        d = rate;
    } else {
        rate = -rate;
        if (d < rate) {
            d = rate;
        }
    }
    nv = *p + d;
    *p = nv;
    return GetFloatAbs(target - nv);
}

/**
 * Integer approach-by-rate step (int twin of func_002AB150): move *p toward
 * target by at most rate, store it back, and return the remaining signed
 * delta mapped to a float through func_002835E0.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB1A8);
#else
f32 func_002AB1A8(s32 *p, s32 target, s32 rate) {
    s32 d = target - *p;
    s32 nv;

    if (rate < d) {
        d = rate;
    } else {
        rate = -rate;
        if (d < rate) {
            d = rate;
        }
    }
    nv = *p + d;
    *p = nv;
    return (f32)func_002835E0(target - nv);
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB208);

/* func_002AB210: critically-damped scalar approach. Integrates a spring step
 * toward `target` into the velocity *vel (func_002AB000 with the stiffness/
 * damping/dt triple b/c/d), advances *p by the new velocity, and returns the
 * residual signed distance to the target. When that residual falls below a
 * dt-scaled epsilon (d * 0.01) the value snaps exactly to target and the
 * velocity is zeroed (returning 0). Scalar twin of the Vec4 func_002AB2C0 just
 * below. Walled: $f20-$f22 + $16/$17/$31 saves (save-layout wall). b/c arrive
 * in $f13/$f14, d in $f15. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB210);
#else
f32 func_002AB210(f32 *p, f32 *vel, f32 target, f32 b, f32 c, f32 d) {
    f32 residual;

    func_002AB000(vel, target - *p, b, c, d);
    *p = *p + *vel;
    residual = target - *p;
    if (GetFloatAbs(residual) < d * 0.009999999776f) {   /* 0x3C23D70A */
        *p = target;
        *vel = 0.0f;
        return *vel;
    }
    return residual;
}
#endif

/* func_002AB2C0: critically-damped Vec4 approach. Drives the position `cur`
 * toward `target` along the connecting direction, integrating the scalar
 * approach speed into *vel via the spring step (func_002AB000 over the current
 * separation distance, with stiffness/damping/dt b/c/eps). The direction vector
 * is rescaled to the new speed and added back onto `cur`. Returns the residual
 * separation (distance - speed); when that drops below a dt-scaled epsilon
 * (eps * 0.01) the position snaps to `target` (full Vec4 copy) and the velocity
 * is zeroed (returning 0). Vec4 twin of the scalar func_002AB210 above. Walled:
 * $f20-$f22 + $16/$17/$18/$31 saves (save-layout wall). b/eps arrive in
 * $f12/$f14, c in $f13. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB2C0);
#else
f32 func_002AB2C0(Vec4 *cur, Vec4 *target, f32 *vel, f32 b, f32 c, f32 eps) {
    Vec4 dir;
    f32 dist;
    f32 residual;

    Vec4SubVu0(&dir, target, cur);
    dist = Vec3LengthVu0(&dir);
    func_002AB000(vel, dist, b, c, eps);
    Vec3RescaleToLenVu0(&dir, *vel, &dir);
    Vec4AddVu0(cur, cur, &dir);
    residual = dist - *vel;
    if (GetFloatAbs(residual) < eps * 0.009999999776f) {   /* 0x3C23D70A */
        *cur = *target;
        *vel = 0.0f;
        return *vel;
    }
    return residual;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB3B0);

/* func_002AB5A0: wrap an angle into (-pi, pi] (via WrapAnglePiDiff), then when a
 * direction sign is supplied bias the result onto the requested rotation side:
 * if the wrapped delta already agrees with the sign (delta*sign > 0) keep it; an
 * almost-zero delta collapses to 0; otherwise add/subtract a full 2*pi turn so
 * the result rotates the requested way. Walled: $f20/$f21 + $16/$31 saves. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB5A0);
#else
f32 func_002AB5A0(f32 a, f32 b, s32 sign) {
    f32 d = func_00284590(a, b);   /* func_00284590 == WrapAnglePiDiff(a - b) */

    if (sign == 0) {
        return d;
    }
    if (0.0f < d * (f32)sign) {
        return d;
    }
    if (GetFloatAbs(d) <= 0.00174532062f) {   /* 0x3AE4C38A */
        return 0.0f;
    }
    if (0.0f < d) {
        return d - 6.28318548f;               /* 0x40C90FDC = 2*pi */
    }
    return d + 6.28318548f;
}
#endif

/* func_002AB668: step a stored angle (*p) toward target `a` by at most `maxStep`
 * (clamped both ways), wrapping the sum into (-pi, pi], and return the residual
 * signed angle difference after the step. `sign` forces the rotation side (see
 * func_002AB5A0). Walled: $f20/$f21 + $16/$17/$31 saves (save-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB668);
#else
f32 func_002AB668(f32 a, f32 maxStep, f32 *p, s32 sign) {
    f32 delta = func_002AB5A0(a, p[0], sign);

    if (maxStep < delta) {
        delta = maxStep;
    } else if (delta < -maxStep) {
        delta = -maxStep;
    }
    p[0] = func_00284548(p[0], delta);   /* func_00284548 == WrapAnglePiSum */
    return func_002AB5A0(a, p[0], sign);
}
#endif

/* func_002AB700: critically-damped scalar angle driver. Eases the stored angle
 * *p toward `target` while tracking its angular velocity in *vel.
 *
 *   - When mode == 2 a rotation-side `sign` is derived from the relative signs
 *     of target and the current angle so the spring takes the shorter wrap arc
 *     (sign = +1 when target>0 & *p<0, -1 when target<0 & *p>0, else 0); for any
 *     other mode the caller's mode value is used directly as that sign.
 *   - func_002AB5A0 gives the signed wrapped delta (cur->target on the chosen
 *     side); func_002AB000 integrates the spring step into *vel (stiffness/
 *     damping/dt in b/c/d); *p is re-wrapped against *vel and the residual delta
 *     recomputed. When the residual is below a dt-scaled epsilon the angle snaps
 *     exactly to target and the velocity is zeroed.
 *
 * Returns the residual signed angle error (0 when snapped). Walled: saves
 * $16-$18/$31 + $f20-$f23 (save-layout wall). b/c/d are $f13/$f14/$f15. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB700);
#else
f32 func_002AB700(f32 *p, f32 *vel, s32 mode, f32 target, f32 b, f32 c, f32 d) {
    s32 sign;
    f32 delta;
    f32 r;

    if (mode == 2) {
        if (target > 0.0f && *p < 0.0f) {
            sign = 1;
        } else if (target < 0.0f && *p > 0.0f) {
            sign = -1;
        } else {
            sign = 0;
        }
    } else {
        sign = mode;
    }

    delta = func_002AB5A0(target, *p, sign);
    func_002AB000(vel, delta, b, c, d);
    *p = func_00284548(*p, *vel);            /* func_00284548 == WrapAnglePiSum */
    r = func_002AB5A0(target, *p, sign);
    if (GetFloatAbs(r) < d * 0.009999999776f) {   /* 0x3C23D70A */
        *p = target;
        *vel = 0.0f;
        return *vel;
    }
    return r;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB868);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ABAE8);

/* func_002ABD00: pack four [0,1] float colour components into a 0xAABBGGRR u32.
 * Each of r/g/b/a (passed in $f12/$f13/$f14/$f15) is scaled by 255.0, truncated
 * to an int (FloatToInt), masked to a byte and shifted into its channel:
 *   r = bits 0-7, g = bits 8-15, b = bits 16-23, a = bits 24-31.
 * Walled: $f20-$f23 + $31 saves (save-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ABD00);
#else
s32 func_002ABD00(f32 r, f32 g, f32 b, f32 a) {
    s32 ri = FloatToInt(r * 255.0f) & 0xFF;
    s32 gi = FloatToInt(g * 255.0f) & 0xFF;
    s32 bi = FloatToInt(b * 255.0f) & 0xFF;
    s32 ai = FloatToInt(a * 255.0f);
    return ri | (gi << 8) | (bi << 16) | (ai << 24);
}
#endif

/**
 * Conditionally exchange three values: bit0 swaps a/b, bit1 swaps b/c,
 * bit2 swaps a/c.
 */
void func_002ABDA8(s32 *a, s32 *b, s32 *c, s32 bits) {
    s32 t;
    s32 u;

    if (bits & 1) {
        t = *b;
        u = *a;
        *a = t;
        *b = u;
    }
    if (bits & 2) {
        t = *c;
        u = *b;
        *b = t;
        *c = u;
    }
    if (bits & 4) {
        t = *a;
        u = *c;
        *c = t;
        *a = u;
    }
}

/* func_002ABE08: conditionally permute the low three colour channels of a packed
 * 0xAABBGGRR word. When `bits` is 0 the word is returned unchanged; otherwise the
 * R/G/B bytes are unpacked and swapped per func_002ABDA8's bit mask (bit0 R<->G,
 * bit1 G<->B, bit2 R<->B), with the alpha (top) byte preserved. Walled: $16/$31
 * saves (save-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ABE08);
#else
u32 func_002ABE08(u32 word, s32 bits) {
    s32 r;
    s32 g;
    s32 b;
    s32 a;

    if (bits == 0) {
        return word;
    }
    r = word & 0xFF;
    g = (word & 0xFF00) >> 8;
    b = (word >> 16) & 0xFF;
    a = word >> 24;
    func_002ABDA8(&r, &g, &b, bits);
    return (a << 24) | (b << 16) | (g << 8) | r;
}
#endif

/* func_002ABE90: orthonormalise the 3 columns of the rotation 3x3 of a Mat4x4.
 * For each column i (0..2): gather the column (the i-th element of rows 0,1,2,
 * stride 0x10) into a scratch Vec3 (w zeroed), normalise it to unit length via
 * Vec3RescaleToLenVu0(len 1.0), then scatter it back into column i.
 * Walled: $16/$17/$18 + $31 saves (save-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ABE90);
#else
void func_002ABE90(Mat4x4 *mat) {
    f32 *base = (f32 *)mat;
    s32 i;

    for (i = 0; i < 3; i++) {
        Vec4 col;
        f32 *src = base + i;        /* &mat[row0][col i] */
        s32 j;

        col.w = 0.0f;
        for (j = 0; j < 3; j++) {
            ((f32 *)&col)[j] = *src;
            src += 4;               /* next row (stride 0x10 bytes) */
        }
        Vec3RescaleToLenVu0(&col, 1.0f, &col);
        {
            f32 *dst = base + i;
            for (j = 0; j < 3; j++) {
                *dst = ((f32 *)&col)[j];
                dst += 4;
            }
        }
    }
}
#endif

/* func_002ABF50: transform the delta (c - a) into the local frame of quaternion
 * q and return component `idx` of the result. Builds the rotation matrix from q
 * (func_00284048), transforms the delta vector by it (func_00283A70), and reads
 * out result[idx]. Args: a, q, c, idx. Walled: $16-$19 + $31 saves. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ABF50);
#else
f32 func_002ABF50(const Vec4 *a, const Vec4 *q, const Vec4 *c, s32 idx) {
    Vec4 delta;
    Vec4 result;
    Mat4x4 mat;

    Vec4SubVu0(&delta, (Vec4 *)c, (Vec4 *)a);
    func_00284048(&mat, q);
    func_00283A70(&result, &delta, &mat);
    return ((f32 *)&result)[idx];
}
#endif

/* func_002ABFD0: remove the component of `vec` along `axis`, scaled by `scale`.
 * Normalises axis (Vec3RescaleToLenVu0, len 1.0), projects vec onto it
 * (Vec3DotVu0), scales the projection by `scale`, scales the unit axis by that
 * amount and subtracts it from vec, writing the result into *out.
 *   out = vec - scale * dot(vec, unit(axis)) * unit(axis)
 * Args: out, vec, axis, scale ($f12). Walled: $f20 + $16-$18 + $31 saves. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ABFD0);
#else
void func_002ABFD0(Vec4 *out, Vec4 *vec, Vec4 *axis, f32 scale) {
    Vec4 unit;
    Vec4 proj;
    f32 amount;

    Vec3RescaleToLenVu0(&unit, 1.0f, axis);
    amount = Vec3DotVu0(&unit, vec) * scale;
    Vec4ScaleVu0(&proj, amount, &unit);
    Vec4SubVu0(out, vec, &proj);
}
#endif

/* func_002AC058: read word 0 of a moby's extra/pvar block (mode bit 0x20
 * gates the block). Best attempt 66%: structure identical (bnezl + shared
 * return-0) but the later cc1 emits two scheduler nops between the andi
 * and its beqz that the pinned cc1 never produces (same wall as
 * func_002AC088/func_002AC9E0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC058);
#else
s32 func_002AC058(Moby *moby) {
    if (moby == 0) {
        return 0;
    }
    if ((moby->modeBits & 0x20) != 0) {
        return moby->pExtra[0];
    }
    return 0;
}
#endif

/* func_002AC088: read word 4 of a moby's extra/pvar block. Same two-
 * scheduler-nops wall as func_002AC058 (best attempt 66%). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC088);
#else
s32 func_002AC088(Moby *moby) {
    if (moby == 0) {
        return 0;
    }
    if ((moby->modeBits & 0x20) != 0) {
        return moby->pExtra[4];
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC0B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC1E0);

/* func_002AC468: build a scratch transform/matrix (func_00284008), feed it
 * through func_002AC1E0 with `out`, then resolve `in` against it (func_00284028).
 * The 0x40-byte scratch is a 4x4 matrix shared by all three helpers. Walled:
 * $16/$17/$31 saves (save-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC468);
#else
extern void func_00284008(void *m);
extern void func_00284028(void *src, void *m);

void func_002AC468(void *out, void *in) {
    u8 scratch[0x40];   /* 4x4 matrix */

    func_00284008(scratch);
    func_002AC1E0(out, scratch);
    func_00284028(in, scratch);
}
#endif

/**
 * Copy the hero's velocity pair (+0x38, 64-bit) onto another moby.
 */
void func_002AC4B8(Moby *moby) {
    *(u64 *)((u8 *)moby + 0x38) = *(u64 *)((u8 *)g_pHeroMoby[0] + 0x38);
}

/* func_002AC4D0: axis-angle -> quaternion. out.xyz = axis(src) * sin(angle/2),
 * out.w = cos(angle/2). Walled: $f20 + $16/$17/$31 saves (save-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC4D0);
#else
extern f32 func_00283B48(f32 x);   /* sine */

void func_002AC4D0(Vec4 *out, const Vec4 *src, f32 angle) {
    f32 half = angle * 0.5f;

    Vec4ScaleVu0(out, func_00283B48(half), src);
    out->w = func_00283B30(half);   /* +0xC = w (quaternion-style) */
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC538);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", MatrixToEulerAngles);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC668);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC728);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC978);

/**
 * Wrap an angle into [-pi, pi) via the shared frac helper: take the fractional
 * part of (angle + pi) * (1/2pi), scale it back by 2pi and recentre by -pi.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC980);
#else
f32 func_002AC980(f32 angle) {
    f32 frac;
    f32 scratch;

    frac = func_00284678(&scratch, (angle + 3.14159274f) * 0.159154937f);
    return frac * 6.28318548f - 3.14159274f;
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC9D8);

/* func_002AC9E0: true when m is a valid moby-table entry with class id in
 * [500, 540]. Best attempt 46%: the %gp_rel delay-slot loads of
 * g_mobyTableBase/End reproduce (size-12 class), but the later cc1 lays the
 * shared return-0 out early with backward branches and pads the first
 * compare with two scheduler nops (same wall as func_002AC058). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC9E0);
#else
s32 func_002AC9E0(Moby *m) {
    if (m == 0) {
        return 0;
    }
    if ((u32)m < (u32)g_mobyTableBase) {
        return 0;
    }
    if ((u32)g_mobyTableEnd < (u32)m) {
        return 0;
    }
    return (u32)(m->oClass - 500) < 0x29;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ACA20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AD590);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AD858);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AD860);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AD8B0);

/* func_002AD8B8: append a moby's table slot to an i16 count-prefixed list
 * if absent and below cap. Best attempt 69%: the later cc1 derives the
 * loop bound from a register copy of the count and pads the scan loop -
 * scan-loop scheduling wall (sibling of func_002AD938). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AD8B8);
#else
void func_002AD8B8(Moby *moby, s16 *list, s32 cap) {
    s16 slot = (s16)(((u8 *)moby - (u8 *)g_mobyTableBase) >> 8);
    s16 count = list[0];
    s16 i;

    if (count > 0) {
        for (i = 1; i <= count; i++) {
            if (list[i] == slot) {
                return;   /* already present */
            }
        }
    }
    if ((s32)list[0] < (s32)(s16)cap) {
        s16 n = (s16)((u16)list[0] + 1);

        list[0] = n;
        list[n] = slot;   /* append after the live entries */
    }
}
#endif

/* func_002AD938: remove a moby from an i16 count-prefixed list by swapping
 * the last entry into its slot. Best attempt 57%: the original keeps the
 * raw count in a register across the scan with branch-likely reloads the
 * pinned cc1 will not produce - scan-loop scheduling wall. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AD938);
#else
void func_002AD938(Moby *moby, s16 *list) {
    s16 count = list[0];
    s16 i;

    if (count <= 0) {
        return;
    }
    for (i = 1; i <= count; i++) {
        Moby *entry = (Moby *)((u8 *)g_mobyTableBase + (s16)list[i] * 0x100);

        if (entry == moby) {
            list[i] = list[(u16)list[0]];   /* swap last entry into this slot */
            list[0] = (u16)list[0] - 1;
            return;
        }
    }
}
#endif

/* func_002AD9B0: jitter a Vec3 in place — add an independent uniform random
 * offset in [-amt, amt) to each of x/y/z. Walled: $f20/$f21 + $16/$31 saves
 * (save-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AD9B0);
#else
void func_002AD9B0(Vec4 *p, f32 amt) {
    p->x = p->x + func_002A86E0(-amt, amt);   /* func_002A86E0 == GetRandomFloatRange */
    p->y = p->y + func_002A86E0(-amt, amt);
    p->z = p->z + func_002A86E0(-amt, amt);
}
#endif

/* func_002ADA30: test whether `pos` falls inside segment `segIdx`'s unit-cube
 * bounds. Subtract the segment origin (table at *(g_deferredSegment2Tag+0xCC),
 * 0x80-stride, origin at +0x30), transform the offset into the segment's local
 * frame (func_00283A48), and return 1 only if all of x/y/z land in [-1, 1].
 * segIdx == -1 returns 0. Walled: $16/$31 saves (save-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADA30);
#else
extern void func_00283A48(Vec4 *out, Vec4 *local);
extern u8 g_deferredSegment2Tag[];   /* +0xCC holds the segment-table base ptr */

s32 func_002ADA30(Vec4 *pos, s32 segIdx) {
    u8 *seg;
    Vec4 local;
    Vec4 out;

    if (segIdx == -1) {
        return 0;
    }
    seg = *(u8 **)(g_deferredSegment2Tag + 0xCC) + segIdx * 0x80;
    Vec4SubVu0(&local, pos, (Vec4 *)(seg + 0x30));
    local.w = 0.0f;
    func_00283A48(&out, &local);
    if (-1.0f <= out.x && out.x <= 1.0f &&
        -1.0f <= out.y && out.y <= 1.0f &&
        -1.0f <= out.z && out.z <= 1.0f) {
        return 1;
    }
    return 0;
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADB08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADB10);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADB98);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADBA0);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADC30);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADC50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADCE0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADD28);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADDD0);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADF10);

/**
 * Read word 2 of a moby's extra/pvar block.
 */
s32 func_002ADF18(Moby *moby) {
    if (moby == 0) {
        return 0;
    }
    if ((moby->modeBits & 0x20) != 0) {
        return moby->pExtra[2];
    }
    return 0;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADF48);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE0B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE198);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE2D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE460);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE558);

/* MarkLevelAvailable: set a level's available flag and append it to the
 * ordered level list (regular levels < 0x15, plus level 0x18). Best attempt
 * 75%: the original contains an EMPTY 28-iteration delay loop padded with
 * four scheduler nops per iteration (later-cc1 emission that the pinned cc1
 * collapses), plus the trailing free-slot scan - not reproducible. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", MarkLevelAvailable);
#else
void MarkLevelAvailable(s32 level) {
    s32 i;
    s32 idx;

    if (g_abLevelAvailableFlags[level] != 0) {
        return;   /* already available */
    }
    /* original spins an empty 28-iteration delay loop here (no state effect) */
    g_abLevelAvailableFlags[level] = 1;
    if (level >= 0x15 && level != 0x18) {
        return;   /* only regular levels (< 0x15) and level 0x18 are ordered */
    }
    idx = 0;
    for (i = 0; i < 0x1C; i++) {
        if (g_anAvailableLevelOrder[i] != 0) {
            idx = idx + 1;
        }
    }
    g_anAvailableLevelOrder[idx] = level;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE6C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE7E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE9E0);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF590);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF598);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF6A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF728);

/* func_002AF948: round a float to `digits` decimal places. Builds the scale
 * 10^digits (digits<=0 -> 1), adds the half-ulp rounding bias 1/(2*scale),
 * truncates (FloatToInt) the scaled value, and divides back. Walled: $f20 +
 * $31 saves (save-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF948);
#else
extern s32 FloatToInt(f32 x);

f32 func_002AF948(s32 digits, f32 x) {
    s32 pow10 = 1;
    f32 scale;

    while (digits > 0) {
        pow10 = pow10 * 10;
        digits = digits - 1;
    }
    scale = (f32)pow10;
    return (f32)FloatToInt((x + 1.0f / (scale + scale)) * scale) / scale;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF9C8);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFA58);

/**
 * Unpack a 0xBBGGRR colour word and forward to the colour setter.
 *
 * TYPE NOTE: p stays void*. The callee func_002A12C0 writes the packed RGB into
 * the low 24 bits of the 8-byte field at p+0x38 (ld/sd, high dword preserved).
 * That +0x38 colour field does not match the recovered Moby layout (0x38 falls
 * in Moby's unnamed pad36 block) and no confirmed caller is available, so the
 * container struct is unidentified - left void* rather than guessed as Moby*.
 */
s32 func_002AFA80(void *p, s32 rgb) {
    s32 b = rgb >> 16;
    s32 g = rgb >> 8;

    __asm__ __volatile__("");
    return func_002A12C0(p, rgb & 0xFF, g & 0xFF, b & 0xFF);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFAB0);

/* func_002AFCD8: advance one oscillating angle channel and project it to a
 * scalar offset stored at out+0x18.
 *
 *   - The phase *p1 is always stepped by `b` and wrapped into (-pi,pi].
 *   - When the amplitude `c` is positive the offset is recomputed from scratch:
 *     out->0x18 = sin(phase)*amp + c  (amp = `a`).
 *   - When `c` is non-positive the channel runs incrementally: the previous
 *     per-frame contribution *p2 is first subtracted out of out->0x18, the new
 *     contribution sin(phase)*a is stored into *p2, and that is added back —
 *     i.e. out->0x18 is edited in place by the delta of this channel.
 *
 * Walled: saves $16-$18/$31 + $f20/$f21. func_00283B48 is sine; a/b/c are the
 * $f12/$f13/$f14 args. Returns void.
 *
 * ORACLE STATUS: trace-only. NOT cmp-oracle-able by the asm-vs-C harness - its
 * sin helper func_00283B48 is a VU0 *microprogram* (vcallms 0xC90) that cannot
 * run standalone headless. Routed to the TESTER's full-game EE effect-diff
 * (PCSX2/PINE, real microprograms loaded). If the tester cannot reach it
 * in-context either, it is a KNOWN un-oracle-able body (trace-only + VU0-
 * microprogram limit) - do NOT count it as oracle-covered. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFCD8);
#else
void func_002AFCD8(void *out, f32 *p1, f32 *p2, f32 a, f32 b, f32 c) {
    f32 *offset = (f32 *)((u8 *)out + 0x18);

    if (c > 0.0f) {
        *p1 = func_00284548(*p1, b);          /* WrapAnglePiSum */
        *offset = func_00283B48(*p1) * a + c;
    } else {
        *p1 = func_00284548(*p1, b);
        *offset = *offset - *p2;
        *p2 = func_00283B48(*p1) * a;
        *offset = *offset + *p2;
    }
}
#endif

/* func_002AFD90: build the XY components of a spherical-swing direction into
 * out+0xF0 / out+0xF4 from two phase angles, then advance both phases.
 *
 *   out->0xF0 = scale * sin(*p1) * sin(*p2);
 *   out->0xF4 = scale * sin(*p1) * cos(*p2);
 *   *p1 = WrapAnglePiSum(*p1, b);   (b = $f13)
 *   *p2 = WrapAnglePiSum(*p2, c);   (c = $f14)
 *
 * The sin/cos sample the phases BEFORE they are advanced. func_00283B48 is sine,
 * func_00283B30 cosine. Walled: saves $16-$18/$31 + $f20-$f23; scale is $f12.
 * Returns void.
 *
 * ORACLE STATUS: trace-only. NOT cmp-oracle-able by the asm-vs-C harness - its
 * sin/cos helpers func_00283B48/func_00283B30 are VU0 *microprograms* (vcallms
 * 0xC80/0xC90) that cannot run standalone headless. Routed to the TESTER's
 * full-game EE effect-diff (PCSX2/PINE, real microprograms loaded). If the tester
 * cannot reach it in-context either, it is a KNOWN un-oracle-able body (trace-only
 * + VU0-microprogram limit) - do NOT count it as oracle-covered. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFD90);
#else
void func_002AFD90(void *out, f32 *p1, f32 *p2, f32 scale, f32 b, f32 c) {
    f32 sinP1 = func_00283B48(*p1);

    *(f32 *)((u8 *)out + 0xF0) = scale * sinP1 * func_00283B48(*p2);
    *(f32 *)((u8 *)out + 0xF4) = scale * func_00283B48(*p1) * func_00283B30(*p2);
    *p1 = func_00284548(*p1, b);              /* WrapAnglePiSum */
    *p2 = func_00284548(*p2, c);
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFE58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFE68);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFF08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFF10);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0038);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0150);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B02C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B03E8);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0BD8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0BF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0C40);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0CA8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0CC0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0D20);

/**
 * Resolve a tracked position into a local vector, then forward it to the
 * shared vector consumer.
 */
void func_002B0D70(s32 ctx, void *a, void *b) {
    Vec4 out;

    func_002B0C40(ctx, &out, a, b);
    func_002837D0(&out);
}

/**
 * Resolve a tracked position into a local vector and return its z.
 */
f32 func_002B0DA0(s32 ctx, void *a, void *b) {
    Vec4 out;

    func_002B0C40(ctx, &out, a, b);
    return out.z;
}

/**
 * Build a unit "to-camera" direction in out: out = D_1A8CB0 - src, flatten z to
 * 0, normalise to length 1, and flip it when the flag is clear.
 */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - save-layout wall ($16/$17/$31 packed
   8-byte by the later cc1 vs 16-byte by the pinned cc1). The Sub/flatten/
   rescale/conditional-negate sequence is otherwise straightforward. Revisit
   once the gameplay-TU compiler is available. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0DC8);
#else
void func_002B0DC8(Vec4 *src, Vec4 *out, s32 keepSign) {
    Vec4SubVu0(out, (Vec4 *)&D_1A8CB0, src);
    out->z = 0.0f;
    Vec3RescaleToLenVu0(out, 1.0f, out);
    if (keepSign == 0) {
        Vec4ScaleVu0(out, -1.0f, out);   /* sig is (dst, f32 scale, src); negate in place */
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0E40);

/**
 * Forward to func_002B0E40 on the sub-object at +0x10.
 *
 * TYPE NOTE: p stays u8*. func_002B0E40 consumes p+0x10 as a Vec4* (it runs
 * SetVec4UnitZ / Vec4SubVu0 / Vec3RescaleToLenVu0 / Vec4ScaleVu0 on it), so
 * p+0x10 is a Vec4 (a direction/position vec). The container is consistent with
 * a Moby (pos@0x10) but that single-field match is not conclusive and the
 * caller hands in a stack-local pointer, so p is left u8* rather than Moby*.
 */
s32 func_002B0F18(u8 *p) {
    return func_002B0E40(p + 0x10);
}

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0F38);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0F40);

/**
 * Forward to func_002B0F40 on the sub-object at +0x10.
 *
 * TYPE NOTE: p stays u8*. func_002B0F40 consumes p+0x10 (its a0) as a Vec4*
 * (Vec3RescaleToLenVu0 / Vec4AddVu0), so p+0x10 is a Vec4. As with func_002B0F18
 * the container is plausibly a Moby (pos@0x10) but unconfirmed (caller passes a
 * stack-local pointer, StepMobySpringFollow @ 0x2B5844), so p is left u8*.
 */
s32 func_002B0FC0(u8 *p) {
    return func_002B0F40(p + 0x10);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0FE0);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B11C0);

/**
 * Sample the breath/oxygen meter value; when the HUD inversion flag is set
 * the meter counts down from 100 instead.
 *
 * `p` is read directly as an offset-0 16-byte vector by func_002837F8 (lqc2),
 * so it is a Vec4* (a world position point); callers pass moby+0x10, i.e. the
 * moby's position vec (verified in CheckMobyOverWater @ 0x2B7334).
 */
f32 func_002B11C8(Vec4 *p) {
    if (D_1A8CA4 == 0) {
        return func_002837F8(p, &D_001B1750);
    }
    return 100.0f - func_002837F8(p, &D_1A8CB0);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1220);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1270);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1340);

/**
 * Resolve a tracked position from a caller vector (128-bit local copy) and
 * forward its x/y to the 2D consumer.
 */
void func_002B1348(s32 ctx, Vec4 *vec, void *b) {
    QVec t;

    t.q = *(u_long128 *)vec;
    func_002B0C40(ctx, (void *)&t, (void *)&t, b);
    func_00283BF8(t.v.x, t.v.y);
}

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1380);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", SampleRainHeightmap);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", SpawnRaindropImpactFx);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1708);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1710);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1778);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B17D0);

/**
 * Set a moby's "freeze fade" weight at +0x70: while the palette-cycle freeze
 * flag is set it tracks the supplied value, otherwise it snaps to 1.0. When
 * the caller asks to settle (settle != 0) and the weight has not yet reached
 * the moby's base value at +0x20, kick the shared eased-approach helper.
 */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - 99.67%, a single c.eq.s operand-order
   instruction. The later cc1 loads the +0x70 weight first AND uses it as the
   compare's fs; the pinned cc1 ties fs to the != LHS while loading the RHS
   first, so it can never produce both at once (operand-scheduling wall).
   Revisit once the gameplay-TU compiler is available. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B17F8);
#else
void func_002B17F8(f32 value, s32 a1, u8 *p, s32 a3, s32 settle) {
    if (D_1A7A4F != 0) {
        *(f32 *)(p + 0x70) = value;
    } else {
        *(f32 *)(p + 0x70) = 1.0f;
    }
    if (settle != 0 && *(f32 *)(p + 0x70) != *(f32 *)(p + 0x20)) {
        func_002AFAB0(0.03f, 0.3f);
    }
}
#endif

/* unreachable code fragment (stray sh + $sp tail from splat over-split), not C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1870);

/**
 * Look up a localized string by id and hand it (with the caller's second arg)
 * to the GUI text helper func_0029DAD0.
 */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - save-layout wall ($16/$31 packed
   8-byte by the later cc1 vs 16-byte by the pinned cc1). Body is a plain
   two-call forward. Revisit once the gameplay-TU compiler is available. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1880);
#else
s32 func_002B1880(s32 stringId, s32 arg) {
    return func_0029DAD0(GetLocalizedString(stringId), arg);
}
#endif

/**
 * Forward to func_0029DA88 (GUI text helper).
 */
s32 func_002B18B0(s32 a) {
    return func_0029DA88(a);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B18D0);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1A80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1A90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1B48);

/**
 * Read the palette-cycle counter base byte.
 */
s32 func_002B1BC8(void) {
    return D_139648[0];
}

/**
 * Remaining platinum bolts: total collected (func_002B1C20) minus the
 * palette-cycle base counter (func_002B1BC8), clamped to [0, 40].
 */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - save-layout wall ($16/$31 packed 8-byte
   by the later cc1 vs 16-byte by the pinned cc1); the subtract and [0,40]
   movz/movn clamp are otherwise exact. Revisit once the gameplay-TU compiler
   is available. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1BD8);
#else
s32 func_002B1BD8(void) {
    s32 remaining = func_002B1C20() - func_002B1BC8();

    if (remaining < 0) {
        remaining = 0;
    }
    if (remaining > 0x28) {
        remaining = 0x28;
    }
    return remaining;
}
#endif

/**
 * Total collected platinum bolts across all levels (sum of CountPlatinumBolts
 * over levels 0..0x1B, skipping the unused level 0x1A), clamped to [0, 40].
 */
#ifndef TARGET_NATIVE
/* TODO(match): functional equivalent - save-layout wall ($16/$17/$18/$31, the
   later cc1 packs the four callee-save slots 8-byte where the pinned cc1
   reserves 16). The level-skip beql loop and the [0,40] movz/movn clamp are
   otherwise exact. Revisit once the gameplay-TU compiler is available. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1C20);
#else
extern s32 CountPlatinumBolts(s32 level);

s32 func_002B1C20(void) {
    s32 total = 0;
    s32 level;

    for (level = 0; level < 0x1C; level++) {
        if (level != 0x1A) {
            total += CountPlatinumBolts(level);
        }
    }
    if (total < 0) {
        total = 0;
    }
    if (total > 0x28) {
        total = 0x28;
    }
    return total;
}
#endif

/* CountPlatinumBolts: count one level's collected platinum bolts (4 flags
 * at level*4, plus the 4 extra flags at +0x68 for level 2), clamped to
 * [0, 40]. Best attempt 94%: byte-identical except a single later-cc1
 * scheduler nop before each counting loop's bottom branch (the movn-in-
 * delay-slot loops themselves reproduce). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", CountPlatinumBolts);
#else
s32 CountPlatinumBolts(s32 level) {
    u8 *flags = &g_platinumBoltFlags[level * 4];
    s32 count = 0;
    s32 i;

    for (i = 0; i < 4; i++) {
        if (flags[i] != 0) {
            count = count + 1;
        }
    }
    if (level == 2) {
        for (i = 0; i < 4; i++) {
            if (g_platinumBoltFlags[0x68 + i] != 0) {
                count = count + 1;
            }
        }
    }
    if (count < 0) {
        count = 0;
    }
    if (count > 0x28) {
        count = 0x28;
    }
    return count;
}
#endif

/**
 * Bounds-checked read of the per-level lookup table (21 levels).
 */
s32 func_002B1D18(u32 level) {
    if (level >= 0x15) {
        return 0;
    }
    return D_258CF0[level];
}

/**
 * Count the inventory items whose active weapon variant has reached at
 * least upgrade level 2, clamped to [0, 20].
 */
s32 func_002B1D40(void) {
    u8 *slots = g_itemEquippedSlot;
    WeaponVariant *table = g_weaponTable;
    s32 count = 0;
    s32 i = 0;

    do {
        s32 level = table[*(u8 *)(i + (s32)slots)].upgradeLevel;

        if (level >= 2) {
            count = count + 1;
        }
        i = i + 1;
    } while (i < 0x38);
    if (count < 0) {
        count = 0;
    }
    if (count > 0x14) {
        count = 0x14;
    }
    return count;
}

/**
 * Count the earned skill points (set bytes in g_skillPointFlags, 0x20
 * scanned), clamped to [0, 30].
 */
s32 CountSkillPointsCompleted(void) {
    s32 count = 0;
    s32 i = 0;

    do {
        if (g_skillPointFlags[i]) {
            count = count + 1;
        }
        i = i + 1;
    } while (i < 0x20);
    if (count < 0) {
        count = 0;
    }
    if (count > 0x1E) {
        count = 0x1E;
    }
    return count;
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1DF0);
