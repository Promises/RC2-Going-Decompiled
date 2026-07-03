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
    /* 0x00 */ u8 pad0[0x10];
    /* 0x10 */ Vec4 pos;          /* world position (xyz) — also the bsphere centre
                                     source (moby.h +0x10). RECOVERED: lwc1 at
                                     +0x10/+0x14 in func_002A8C70.s -> f32 x/y. */
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
    /* 0xBF */ u8 padBF[0xF8 - 0xBF];
    /* 0xF8 */ f32 facingAngle;   /* current facing/heading yaw (radians, wrapped
                                     into [-pi,pi]); driven toward a target angle by
                                     UpdateMobyFacingAngle (func_002A8B08). RECOVERED:
                                     lwc1/swc1 at +0xF8 in func_002A8B08.s -> f32. */
    /* 0xFC */ u8 padTail[0x100 - 0xFC]; /* pad this local field-view out to the
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
/* g_soundBankHandles+0x20 base (0x189E20); func_002B0E40 reads a reference
 * position Vec4 at +0xB0 and a sign-selector float at +0xBC. */
extern u8 g_soundBankHandlesBlk[];
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
extern void func_002B0E40(Vec4 *a, Vec4 *out, s32 flag);
extern void func_002B0F40(Vec4 *a, Vec4 *out, Vec4 *src, f32 t);
extern void func_002B0C40(s32 ctx, void *out, void *a, void *b);
extern void func_002B1270(void *a, Vec4 *b, void *c);
extern s32 func_002837D0(void *vec);
/* func_00283BF8 == Atan2fPoly (183558.c region): 2-arg arctangent (minimax poly
 * + quadrant offset, self-contained VU0 — no vcallms upload), returns the angle
 * as f32 in $f0. Native #else of func_002A8C70 needs the true f32 return so the
 * atan2 result feeds func_002A8B08's angle arg without a spurious int<->float
 * cvt (same f32-vs-s32 class as func_00284548 above). The two callers in this
 * unit (func_002A8C70, func_002B1348) are both #else-only, so the f32 form is
 * inert to the matched build; guarded per-build to mirror the func_00284548
 * convention. RECOVERED: $f0 return at jr ra in func_00283BF8.s -> f32. */
#ifdef TARGET_NATIVE
extern f32 func_00283BF8(f32 y, f32 x);
#else
extern s32 func_00283BF8(f32 x, f32 y);
#endif
extern s32 func_002A12C0(void *p, s32 r, s32 g, s32 b);
extern s32 func_00283638(Moby *moby);
extern s32 PostMobyHitEvent(Moby *moby, s32 a, s32 b, s32 c, Vec4 *dir);
extern s32 func_002B1C20(void);
/* func_002A9550: moby-group iterator advance/filter step. Takes the same
 * (out, moby, wantInactive, wantActive) 4-arg shape its only caller
 * func_002A9468 forwards ($a0..$a3 passthrough); the wantInactive/wantActive
 * filter args were dropped by an earlier 2-arg guess. Defined later in this
 * unit (#else); declared here for func_002A9468's #else call. */
extern s32 func_002A9550(Moby **out, Moby *moby, s32 wantInactive, s32 wantActive);
extern f32 func_002837F8(void *p, f32 *src);
/* func_002A0368: read a reference value from an object (1A00F0 unit); takes the
 * object pointer in $4 (not a float), returns the value as f32 in $f0. */
extern f32 func_002A0368(void *obj);
extern s32 func_002AFAB0(f32 step, f32 max);
extern f32 GetFloatAbs(f32 x);
extern s32 func_002835E0(s32 x);
extern f32 func_00284678(f32 *out, f32 angle);
extern f32 func_00283B30(f32 angle);  /* cosine */
extern f32 func_00283B48(f32 angle);  /* sine (0x18 after cosine in 183558.c) */
extern f32 func_00284590(f32 a, f32 b);
/* func_002AB000: spring-style scalar step. Clamps *p to +/-|v0|, integrates
 * *p = p*(1-v2) + v1*v0, clamps to +/-v3 (when v3>0), then re-clamps to +/-|v0|.
 * v0..v3 arrive in $f12..$f15. Return is void: the asm leaves an incidental $f0
 * but all callers discard it. Defined later in this unit (#else). */
extern void func_002AB000(f32 *p, f32 v0, f32 v1, f32 v2, f32 v3);
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
/* Canonical-name angle helpers (183558.c): the func_002A8B08 .s calls these by
 * the WrapAnglePi* glabels (== func_00284548 / func_00284590). Both genuinely
 * return f32 in $f0. Declared here only for the native #else of
 * func_002A8B08 / func_002A8C70; inert to the matched build (no matched fn in
 * this unit calls them by these names). */
extern f32 WrapAnglePiSum(f32 a, f32 b);    /* a+b wrapped into [-pi,pi] */
extern f32 WrapAnglePiDiff(f32 a, f32 b);   /* signed (a-b) wrapped into [-pi,pi] */
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
#ifdef TARGET_NATIVE
/* --- cohort-4 coverage-body deps (byte-neutral extern decls) --- */
extern u8 g_pCollWorldData[];        /* collision-world block; +0x14 = hit-event ring write cursor */
extern u8 g_proceduralAnimBounds[];  /* per-slot cached pose-bounds Vec4, stride 0x10 */
extern Moby *g_mobyFlagged1000List[];/* null-terminated array of flagged mobys */
extern void func_00283460(void *dst, void *src, s32 n);      /* byte copy */
extern f32 func_002835C0(f32 x);                             /* sqrtf */
extern void func_00283AA0(Vec4 *dst, u32 packed);
extern void func_00283DC0(Mat4x4 *dst, Vec4 *in);            /* build rotation matrix from a vec (VU0) */
extern void func_002840E8(void *dst, void *a, void *b);      /* 3x3 matrix multiply (a*b -> dst) */
extern void func_00284308(Vec4 *quat, void *outMtx);         /* quaternion -> 3x4 matrix */
extern s32 func_002A08C0(void *obj);                         /* acquire a procedural-anim slot; <0 = none */
extern void func_002A12F0(void *src, s32 *r, s32 *g, s32 *b);
extern void func_002A3288(void *obj, s32 flags);             /* bind the acquired procedural-anim slot */
extern f32 func_002B0150(Vec4 *query, Moby *moby, s32 *outFlag, f32 a, f32 b, f32 c, f32 d);
extern f32 IntToFloat(s32 x);
extern void MatrixMultiplyVu0(Mat4x4 *dst, Mat4x4 *a, Mat4x4 *b);
extern void MatrixToEulerAngles(Mat4x4 *mtx, void *outAngles);
extern void Vec3CrossVu0(Vec4 *dst, Vec4 *a, Vec4 *b);
#endif
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

/* Resolve the moby's current/next anim-frame data pointers (+0x58/+0x5C) from
 * its active sequence + frame indices. Defined in another unit. */
extern void ResolveMobyAnimFramePtrs(Moby *moby);

/**
 * Set a moby's active animation sequence `seq` and frame `frameIdx`, clamping
 * the frame to the sequence's frame count. The sequence descriptor is
 * pClass+0x48+seq*4; its byte at +0x10 is the frame count, +0x11 the loop-sound
 * index. animFrame is clamped to [0, frameCount-1]; animFrameNext = frame+1
 * clamped to frameCount-1 (and wrapped to 0 if it would still reach the count).
 * Then resolves the frame pointers, seeds animRate2 from the resolved frame,
 * clears the anim-event byte's bit 1, and caches the loop-sound index.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8200);
#else
void func_002A8200(Moby *moby, s32 seq, s32 frameIdx) {
    u8 *m = (u8 *)moby;
    u8 *pClass = *(u8 **)(m + 0x24);                     /* moby->pClass */
    u8 *seqEntry = *(u8 **)(pClass + 0x48 + seq * 4);
    s32 frameCount = *(u8 *)(seqEntry + 0x10);
    s32 cur, next;

    m[0x42] = (u8)seq;                                   /* animSeq */

    cur = (frameIdx < frameCount) ? frameIdx : (frameCount - 1);
    next = cur + 1;
    m[0x40] = (u8)cur;                                   /* animFrame */
    m[0x41] = (u8)next;                                  /* animFrameNext */
    if ((frameCount - 1) < next) {
        m[0x41] = (u8)(frameCount - 1);
    }

    m[0x43] = (u8)seq;                                   /* animSeqNext */
    if (m[0x41] >= frameCount) {
        m[0x41] = 0;
    }

    ResolveMobyAnimFramePtrs(moby);

    *(f32 *)(m + 0x4C) = *(f32 *)(*(u8 **)(m + 0x58));   /* animRate2 = *animFramePtr */
    m[0x60] &= 0xFD;                                     /* animEventByte: clear bit 1 */
    m[0x6C] = *(u8 *)(seqEntry + 0x11);                  /* loopSoundIdx */
}
#endif

/**
 * Start animation sequence `idx` at frame `arg3` on a moby, blended over `arg4`
 * frames. arg3 is clamped to the sequence's [.., frameCount-1]. When arg4 <= 0
 * this delegates to func_002A8200 (instant set). Otherwise it starts a timed
 * blend: when the moby is already mid-blend (animTime > 0.025, or either blend
 * word at +0x50/+0x54 set) it acquires a procedural-anim slot (func_002A08C0),
 * snapshots the current pose bounds into g_proceduralAnimBounds[slot], stashes
 * the current sequence (+0x42 → +0xA9) and marks it procedural (+0x42 = 0xFF).
 * Then it sets the target frame, resolves the frame pointers, and seeds the
 * blend: animRate=1, animRate2=1/arg4, animTime=0, clears animEventByte bit 1,
 * caches loopSoundIdx.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A82D8);
#else
void func_002A82D8(Moby *obj, s32 idx, s32 arg3, s32 arg4) {
    u8 *m = (u8 *)obj;
    u8 *pClass = *(u8 **)(m + 0x24);
    u8 *seqEntry = *(u8 **)(pClass + 0x48 + idx * 4);
    s32 frameCount = *(u8 *)(seqEntry + 0x10);
    s32 clampedFrame = (arg3 < frameCount) ? arg3 : (frameCount - 1);

    if (arg4 <= 0) {
        func_002A8200(obj, idx, clampedFrame);
        return;
    }

    if (0.025f < *(f32 *)(m + 0x44) ||
        *(s32 *)(m + 0x50) != 0 ||
        *(s32 *)(m + 0x54) != 0) {
        s32 slot = func_002A08C0(obj);
        if (slot < 0) {
            m[0x41] = (u8)clampedFrame;
        } else {
            func_002A3288(obj, slot | 0x300);
            *(Vec4 *)(g_proceduralAnimBounds + slot * 0x10) = *(Vec4 *)(m + 0x80);
            if (m[0x42] != 0xFF) {
                m[0xA9] = m[0x42];
            }
            m[0x42] = 0xFF;
            m[0x40] = (u8)slot;
            m[0x41] = (u8)clampedFrame;
        }
    } else {
        m[0x41] = (u8)clampedFrame;
    }

    m[0x43] = (u8)idx;
    ResolveMobyAnimFramePtrs(obj);
    *(f32 *)(m + 0x48) = 1.0f;
    *(f32 *)(m + 0x4C) = 1.0f / IntToFloat(arg4);
    *(f32 *)(m + 0x44) = 0.0f;
    m[0x60] &= 0xFD;
    m[0x6C] = *(u8 *)(seqEntry + 0x11);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8448);

/* func_002A85B8: begin a moby animation. Stash the current anim id into the
 * "prev anim id" slot, install the new anim id, reset the anim timer, and clear
 * the "done" flag (bit 0 of animFlags). If a sub-anim id is supplied (!= -1),
 * record it and additionally clear bit 1 of animFlags. */
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

/* func_002A8620: 8-byte zero pad between functions (splat drops all-zero
 * inter-function regions) — raw-word filler keeps the unit size==span byte-exact
 * (06333c5 precedent; NO re-split). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8620);

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
#if !defined(TARGET_NATIVE) && !defined(MATCH_GetRandomInt)
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", GetRandomInt);
#elif defined(MATCH_GetRandomInt)
/* engine-2.96 byte-match (verify_match.sh RAW: byte+reloc identical). rand @0x1163B0. */
s32 GetRandomInt(s32 n) {
    return ((rand() >> 16) & 0x7FFF) % n;
}
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
/* RandRangeInclusive: uniform random int in [lo, hi] inclusive.
 * PARKED (engine-2.96): correct C reaches 80.95% but the residual is a pure
 * fine-scheduling offset — the prologue `sd $17` callee-save and the `sra`
 * land one instruction earlier in the original; regalloc/structure are
 * identical. This is the 001003-vs-exact-2.96 scheduler gap (not C-controllable,
 * not post-pass-fixable — [[reference_register_coloring_wall]] scheduling class).
 * Best faithful body kept as the TARGET_NATIVE #else. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8688);
#else
s32 func_002A8688(s32 lo, s32 hi) {
    s32 r = (func_001163B0() >> 16) & 0x7FFF;
    s32 span = hi - lo + 1;

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

/**
 * Generate a random direction vector into `dst`: a random magnitude in [lo, hi]
 * (func_002A86E0) and two random angles (func_002A87A8), mapped to cartesian —
 * x = r·cos(a2)·sin(a1), y = r·sin(a2)·sin(a1), z = r·cos(a1). The lo/hi pass
 * straight through to the magnitude RNG. (func_00283B30 = cos, ...B48 = sin.)
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8868);
#else
void func_002A8868(Vec4 *dst, f32 lo, f32 hi) {
    f32 radius = func_002A86E0(lo, hi);
    f32 angle2 = func_002A87A8();
    f32 angle1 = func_002A87A8();
    dst->x = radius * func_00283B30(angle2) * func_00283B48(angle1);
    dst->y = radius * func_00283B48(angle2) * func_00283B48(angle1);
    dst->z = radius * func_00283B30(angle1);
}
#endif

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

/**
 * UpdateMobyFacingAngle — drive a moby's facing/heading yaw (moby->facingAngle,
 * +0xF8) toward a target angle by a damped, clamped angular step, returning
 * nothing (the moby field is updated in place; *driveOut holds the per-frame
 * angular delta).
 *
 *   d   = WrapAnglePiDiff(targetAngle, moby->facingAngle)   signed shortest turn
 *   s   = clamp(d * 6.366197, -1, 1)                        normalised turn drive
 *   v   = *driveOut + (gain*s - damp*v)                     damped step integrate
 *   if (clampLimit != 0)  v = clamp(v, -clampLimit, clampLimit)
 *   v   = clamp(v, -|d|, |d|)                               never overshoot target
 *   *driveOut = v
 *   moby->facingAngle = WrapAnglePiSum(moby->facingAngle, v)
 *
 * 6.366197 (0x40CBB7E3) = 20/pi: maps a half-pi-ish error to the [-1,1] drive
 * band. gain/damp/clampLimit arrive in $f12/$f13/$f14 (the moby ptr in $a0, the
 * drive-accumulator pointer in $a1).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8B08);
#else
void func_002A8B08(Moby *moby, f32 *driveOut, f32 targetAngle,
                   f32 gain, f32 damp, f32 clampLimit) {
    /* signed shortest turn from the current heading to the target */
    f32 d = WrapAnglePiDiff(targetAngle, moby->facingAngle);
    /* normalised turn drive, clamped to [-1, 1] (0x40CBB7E3 = 20/pi) */
    f32 s = d * 6.366197f;   /* 0x40CBB7E3 (exact bit pattern) */
    f32 v;
    f32 ad;

    if (s > 1.0f) {
        s = 1.0f;
    } else if (s < -1.0f) {
        s = -1.0f;
    }
    /* damped integrate: feed-forward gain*s minus damp*current */
    v = *driveOut;
    v = v + (gain * s - damp * v);
    *driveOut = v;
    /* optional symmetric clamp to the per-call rate limit (0 disables) */
    if (clampLimit != 0.0f) {
        if (clampLimit < v) {
            *driveOut = clampLimit;
        } else if (v < -clampLimit) {
            *driveOut = -clampLimit;
        }
    }
    /* never step past the target: clamp the residual into [-|d|, |d|] */
    ad = GetFloatAbs(d);
    if (ad < *driveOut) {
        *driveOut = GetFloatAbs(d);
    } else if (-GetFloatAbs(d) > *driveOut) {
        *driveOut = -GetFloatAbs(d);
    }
    /* advance and re-wrap the stored heading */
    moby->facingAngle = WrapAnglePiSum(moby->facingAngle, *driveOut);
}
#endif

/**
 * UpdateMobyFacingTowardPoint — point a moby at a target position: compute the
 * heading from the planar (x,y) delta and feed it to UpdateMobyFacingAngle.
 *
 *   angle = atan2(target->pos.x - moby->pos.x, target->pos.y - moby->pos.y)
 *   UpdateMobyFacingAngle(moby, driveOut, angle, gain, damp)  [clampLimit param4]
 *
 * Pure forwarder; the three trailing floats (gain/damp/clampLimit) pass through
 * in $f12/$f13/$f14.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8C70);
#else
void func_002A8C70(Moby *moby, Moby *target, f32 *driveOut,
                   f32 gain, f32 damp, f32 clampLimit) {
    f32 dx = target->pos.x - moby->pos.x;
    f32 dy = target->pos.y - moby->pos.y;
    f32 angle = func_00283BF8(dx, dy);
    func_002A8B08(moby, driveOut, angle, gain, damp, clampLimit);
}
#endif

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

/* func_002A9450: 8-byte zero pad between functions (splat drops all-zero
 * inter-function regions) — raw-word filler keeps the unit size==span byte-exact
 * (06333c5 precedent; NO re-split). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9450);

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
    return func_002A9550(out, g_pMobyGroupIterMoby, wantInactive, wantActive);
}
#endif

/**
 * func_002A9550 — moby-group iterator ADVANCE/filter step (the worker behind
 * func_002A9468's iteration). Walks the per-group slot list (g_mobyGroupLists)
 * via the iterator globals and returns the next moby that passes the
 * active/inactive filter through *out.
 *
 *   - If `moby` is the iterator's current moby (g_pMobyGroupIterMoby), resume
 *     scanning from the saved cursor (g_pMobyGroupIterCursor).
 *   - Otherwise re-seed: validate moby->group against g_mobyGroupCount, point the
 *     cursor at that group's slot list, and scan forward until `moby` itself is
 *     found (so the next step starts right after it).
 *   Each list entry is a u16 { slot:15 | terminatorSignBit }; the moby is
 *   g_mobyTableBase + slot*0x100. A negative entry (s16 < 0) terminates the list.
 *
 * Filter (matches func_002A9468's lattice), keyed on the moby state sign bit
 * (sign = 1 when state byte +0x20 is negative, i.e. inactive):
 *   wantInactive==0, wantActive==0 -> accept the ACTIVE (sign==0) moby
 *   wantInactive==0, wantActive!=0 -> accept none (always advance)
 *   wantInactive!=0, wantActive==0 -> accept the next moby unconditionally
 *   wantInactive!=0, wantActive!=0 -> accept the INACTIVE (sign==1) moby
 * Returns 0 with the accepted moby in *out, or -1 (with *out cleared) at
 * end-of-list / invalid group.
 *
 * Type-recovery oracle lane (cmp_1A8180c.c). RECOVERED: list entries are u16
 * read via lhu/lh (+0x7FFF slot mask, sign-bit terminator) -> u16 *cursor;
 * moby->group is lbu at +0x21 -> u8 group; moby->state is lb at +0x20 -> s8 state
 * (the .s sign-extends then srl 31). All iterator state lives in the named
 * globals; no float, no vcallms -> standalone integer/pointer oracle.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9550);
#else
s32 func_002A9550(Moby **out, Moby *moby, s32 wantInactive, s32 wantActive) {
    u16 *cursor;
    s32 slot;
    s32 sign;
    s32 accept;

    *out = 0;
    if (moby == g_pMobyGroupIterMoby) {
        /* resume from the saved cursor */
        cursor = g_pMobyGroupIterCursor;
        if ((s16)*cursor < 0) {
            return -1;
        }
    } else {
        /* re-seed the iterator for this moby's own group */
        if (g_mobyGroupCount < moby->group) {
            return -1;
        }
        g_pMobyGroupIterMoby = 0;
        cursor = g_mobyGroupLists[moby->group];
        g_pMobyGroupIterCursor = cursor;
        if (cursor == 0) {
            return -1;
        }
        /* one entry before the head; the seek loop pre-increments */
        cursor = cursor - 1;
        g_pMobyGroupIterCursor = cursor;
        for (;;) {
            Moby *m;

            cursor = cursor + 1;
            g_pMobyGroupIterCursor = cursor;
            slot = *cursor & 0x7FFF;
            g_mobyGroupIterSlot = slot;
            m = (Moby *)((u8 *)g_mobyTableBase + slot * 0x100);
            g_pMobyGroupIterMoby = m;
            if ((s16)*cursor < 0) {
                return -1;
            }
            if (moby == m) {
                break;   /* found it; the filter loop starts on the NEXT entry */
            }
            cursor = g_pMobyGroupIterCursor;
        }
    }

    /* advance one entry at a time, applying the active/inactive filter */
    for (;;) {
        Moby *m;

        cursor = cursor + 1;
        g_pMobyGroupIterCursor = cursor;
        slot = *cursor & 0x7FFF;
        g_mobyGroupIterSlot = slot;
        m = (Moby *)((u8 *)g_mobyTableBase + slot * 0x100);
        g_pMobyGroupIterMoby = m;
        *out = m;
        sign = (u32)(s32)g_pMobyGroupIterMoby->state >> 31;

        if (wantInactive == 0) {
            if (wantActive != 0) {
                accept = 0;                 /* always advance */
            } else {
                accept = (sign == 0);       /* accept active */
            }
        } else {
            if (wantActive == 0) {
                accept = 1;                 /* accept unconditionally */
            } else {
                accept = (sign != 0);       /* accept inactive */
            }
        }
        if (accept) {
            return 0;
        }
        /* rejected: keep scanning unless the current entry terminates the list */
        cursor = g_pMobyGroupIterCursor;
        if ((s16)*cursor < 0) {
            *out = 0;
            return -1;
        }
    }
}
#endif

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

/**
 * Record a moby collision hit into the 64-entry hit-event ring, deduped per
 * moby. If the moby already owns a ring slot (hitEventSlot != 0xFF) that still
 * belongs to it: when the new hit is closer (hitInfo depth +0x1C < the slot's
 * +0x2C) just OR the new flags into it and return; otherwise carry the slot's
 * existing flags forward into a fresh entry. A new entry is written at the ring
 * write cursor (g_pCollWorldData+0x14, wrapping mod 64): the hit vec, flags
 * (|carried), material/normal fields, depth, and the owning moby; the moby's
 * hitEventSlot is pointed at it and the cursor advances.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9C88);
#else
void func_002A9C88(Moby *moby, void *hitInfo) {
    u8 *m = (u8 *)moby;
    u8 *hi = (u8 *)hitInfo;
    s32 ringCursor = *(s32 *)(g_pCollWorldData + 0x14);
    s32 carriedFlags = 0;
    s32 slot = *(u8 *)(m + 0xA8);
    u8 *entry;

    if (slot != 0xFF) {
        u8 *existing = g_collHitEventRing + slot * 0x40;
        if (*(Moby **)(existing + 0x38) == moby) {
            if (*(f32 *)(hi + 0x1C) < *(f32 *)(existing + 0x2C)) {
                *(s32 *)(existing + 0x24) |= *(s32 *)(hi + 0x14);
                return;
            }
            carriedFlags = *(s32 *)(existing + 0x24);
        }
        /* else: slot was reused by another moby — fall through to a new entry */
    }

    entry = g_collHitEventRing + ringCursor * 0x40;
    func_00283638((Moby *)entry);                       /* zero entry[0..0x10] */
    *(Vec4 *)(entry + 0x10) = *(Vec4 *)hi;
    *(s32 *)(entry + 0x24) = *(s32 *)(hi + 0x14) | carriedFlags;
    *(s32 *)(entry + 0x20) = *(s32 *)(hi + 0x10);
    entry[0x28] = hi[0x18];
    entry[0x29] = hi[0x19];
    *(u16 *)(entry + 0x2A) = *(u16 *)(hi + 0x1A);
    *(f32 *)(entry + 0x2C) = *(f32 *)(hi + 0x1C);
    *(f32 *)(entry + 0x34) = *(f32 *)(hi + 0x24);
    *(s32 *)(entry + 0x3C) = 0;
    *(s32 *)(entry + 0x30) = *(s32 *)(hi + 0x20);
    *(Moby **)(entry + 0x38) = moby;
    m[0xA8] = (u8)ringCursor;
    *(s32 *)(g_pCollWorldData + 0x14) = (ringCursor + 1) & 0x3F;
}
#endif

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
 * live on the stack; func_00284248 takes its angle in $f12.
 *
 * ORACLE STATUS = routed-to-tester-EE. func_00284248 (axis-angle quat build)
 * uploads a vcallms VU0 sin/cos microprogram (func_00283B48 @ 0xC90 /
 * func_00283B30 @ 0xC80), which is NOT present in the standalone headless cmp
 * harness, so this body CANNOT be ULP/bit-oracled there. Verification is via the
 * tester's full-game EE effect-diff (the sin/cos microprograms ARE loaded in the
 * full-game context), same path as func_002AFCD8/func_002AFD90. */
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

/**
 * Rate-limited integrator with symmetric clamps on *state. First clamps *state to
 * [-|maxDelta|, |maxDelta|]; then integrates *state = v + (b*maxDelta - c*v)
 * (i.e. v*(1-c) + b*maxDelta) and clamps to [-bound, bound] when bound > 0;
 * finally re-clamps to [-|maxDelta|, |maxDelta|].
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB000);
#else
void func_002AB000(f32 *state, f32 maxDelta, f32 b, f32 c, f32 bound) {
    f32 lim = GetFloatAbs(maxDelta);
    f32 v;

    if (*state > lim) {
        *state = lim;
    } else if (*state < -lim) {
        *state = -lim;
    }

    v = *state;
    *state = v + (b * maxDelta - c * v);
    if (0.0f < bound) {
        if (*state > bound) {
            *state = bound;
        } else if (*state < -bound) {
            *state = -bound;
        }
    }

    if (*state > lim) {
        *state = lim;
    } else if (*state < -lim) {
        *state = -lim;
    }
}
#endif

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
/* PARKED (engine-2.96): 65% — regalloc/structural gap (frame -48 vs -64, s0/s1
 * hold p/vel swapped, commutative operand order); not save-layout. NOT crackable
 * with 001003 vs exact-2.96. Faithful body kept as #else. */
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
/* PARKED (engine-2.96): Vec4 spring twin of func_002AB210 — same regalloc/
 * structural wall class. Faithful body kept as #else. */
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
    if (GetFloatAbs(d) <= 0.00174532947f) {   /* 0x3AE4C38A = 0.1 deg */
        return 0.0f;
    }
    if (0.0f < d) {
        return d - 6.28318596f;               /* 0x40C90FDC = 2*pi */
    }
    return d + 6.28318596f;
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
/* PARKED (engine-2.96): correct C, but the 4-channel mul/FloatToInt/mask
 * sequence + fp-save ordering schedules differently than the original
 * (fine-scheduling, 001003-vs-exact-2.96 gap). Faithful body kept as #else. */
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
extern void func_002AC1E0(void *out, void *m);

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
/* PARKED (engine-2.96): correct C, but frame is -0x30 vs my -0x20 + $ra/save
 * ordering differs (frame/regalloc, 001003-vs-exact-2.96 gap; possibly a missing
 * stack temp in the source). Faithful body kept as #else. */
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

/* Matrix helpers for MatrixToEulerAngles (also declared later for the AE-family). */
extern void func_00283DC0(Mat4x4 *dst, Vec4 *in);   /* build a rotation matrix from a vec (VU0) */
extern void func_00283DE0(Mat4x4 *dst, Vec4 *in);   /* sibling rotation builder (0xD18 variant) */
extern void MatrixMultiplyVu0(Mat4x4 *dst, Mat4x4 *a, Mat4x4 *b);

/**
 * Extract ZYX-style euler angles from a rotation matrix `mtx` into out[0..2].
 * Copies the 3x3 into a scratch matrix (translation row zeroed to {0,0,0,1}),
 * then peels the angles with three atan2 (func_00283BF8) + Givens rotations that
 * successively zero the off-axis terms: a1=atan2(row0.x,row0.y) about -Z, then
 * a2=atan2(row0.x,-row0.z) about -Y, then a3=atan2(row1.y,row1.z) on the residual.
 * Writes out[0]=a3, out[1]=a2, out[2]=a1.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", MatrixToEulerAngles);
#else
void MatrixToEulerAngles(Mat4x4 *mtx, void *out) {
    Mat4x4 m;
    Mat4x4 rot;
    Vec4 axis;
    f32 *e = (f32 *)out;
    f32 a1, a2, a3;

    m = *mtx;
    func_00283638((Moby *)((u8 *)&m + 0x30));   /* zero the translation row */
    *(f32 *)((u8 *)&m + 0x3C) = 1.0f;

    a1 = func_00283BF8(*(f32 *)((u8 *)&m + 0x00), *(f32 *)((u8 *)&m + 0x04));
    axis.x = 0.0f;
    axis.y = 0.0f;
    axis.z = -a1;
    func_00283DC0(&rot, &axis);
    MatrixMultiplyVu0(&m, &rot, &m);

    a2 = func_00283BF8(*(f32 *)((u8 *)&m + 0x00), -*(f32 *)((u8 *)&m + 0x08));
    axis.x = 0.0f;
    axis.y = -a2;
    axis.z = 0.0f;
    func_00283DE0(&rot, &axis);
    MatrixMultiplyVu0(&m, &rot, &m);

    a3 = func_00283BF8(*(f32 *)((u8 *)&m + 0x14), *(f32 *)((u8 *)&m + 0x18));
    e[2] = a1;
    e[1] = a2;
    e[0] = a3;
}
#endif

/**
 * Advance a countdown/fade field pair on `obj`. When the counter (+0x0) is
 * running (!=0) but its active flag (+0x2) is clear, does nothing. Otherwise
 * clears the flag, reloads the counter from its reset value (+0xC), and either:
 *   - counter was 0: refreshes the RGB bytes (+0x4/+0x5/+0x6) from `src`'s packed
 *     colour (func_002A12F0), or
 *   - counter was running: rescales it to resetValue * counter / divisor(+0xE),
 *     clamped to at least 1.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC668);
#else
void func_002AC668(void *src, u8 *obj) {
    s16 counter = *(s16 *)(obj + 0x0);

    if (counter != 0 && *(s16 *)(obj + 0x2) == 0) {
        return;
    }

    *(s16 *)(obj + 0x2) = 0;
    *(s16 *)(obj + 0x0) = (s16)*(u16 *)(obj + 0xC);

    if (counter == 0) {
        s32 r, g, b;
        func_002A12F0(src, &r, &g, &b);
        obj[0x4] = (u8)r;
        obj[0x6] = (u8)b;
        obj[0x5] = (u8)g;
    } else {
        f32 ratio = (f32)counter / IntToFloat(*(s16 *)(obj + 0xE));
        s32 scaled = FloatToInt((f32)*(s16 *)(obj + 0xC) * ratio);
        *(s16 *)(obj + 0x0) = (s16)scaled;
        if ((s16)scaled <= 0) {
            *(s16 *)(obj + 0x0) = 1;
        }
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC728);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC978);

/**
 * Wrap an angle into [-pi, pi) via the shared frac helper: take the fractional
 * part of (angle + pi) * (1/2pi), scale it back by 2pi and recentre by -pi.
 *
 * LITERAL NOTE (do NOT "fix"): this body's 2pi is 6.28318548f (0x40C90FDB) -
 * that is the byte-correct value its own .s loads. It is DISTINCT from
 * func_002AB5A0's 2pi (6.28318596f / 0x40C90FDC, 1 ULP higher) - the two
 * functions legitimately use different roundings. cmp-oracle-confirmed.
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
 * 0x80-stride, origin at +0x30), rotate the offset into the segment's local
 * frame by the segment's 3x3 rotation matrix (at seg+0x40) via func_00283A48
 * (out = m * local), and return 1 only if all of x/y/z land in [-1, 1].
 * segIdx == -1 returns 0. Walled: $16/$31 saves (save-layout wall). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADA30);
#else
/* out = m * v (3x3 rotate; see the canonical def in text/183558.c). The asm
 * passes the segment's rotation matrix (seg+0x40) as the 3rd arg - dropping it
 * silently leaves `out` undefined, so it must be forwarded. */
extern void func_00283A48(Vec4 *out, Vec4 *v, Vec4 *m);
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
    func_00283A48(&out, &local, (Vec4 *)(seg + 0x40));
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

/**
 * Test whether `pos` lies within unit distance of segment `segIdx`'s local
 * frame. The segment table base is *(g_deferredSegment2Tag+0xD4); each entry is
 * 0x80 bytes with its origin at +0x30 and a 3x3 rotation at +0x40. Transforms
 * (pos - origin) into the segment's local frame and returns 1 if the resulting
 * length is < 1.0, else 0. A negative segIdx returns 0.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADB10);
#else
s32 func_002ADB10(Vec4 *pos, s32 segIdx) {
    u8 *seg;
    Vec4 delta;
    Vec4 local;
    f32 dist;

    if (segIdx < 0) {
        return 0;
    }
    seg = *(u8 **)(g_deferredSegment2Tag + 0xD4) + segIdx * 0x80;
    Vec4SubVu0(&delta, pos, (Vec4 *)(seg + 0x30));
    delta.w = 0.0f;
    func_00283A48(&local, &delta, (Vec4 *)(seg + 0x40));
    dist = Vec3LengthVu0(&local);
    return dist < 1.0f;
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADB98);

/* func_002ADBA0(subject): scan the point-light manager block
 * (g_pointLights+0x2400) back-to-front — for each active light entry
 * (g_pointLights+0x2420, 0x10 stride, count at mgr+0xC) call the predicate
 * func_00284730(subject, &entry, &entry+0x20); on the first hit record the
 * 1-based index and stop. Returns 1 iff that hit index equals the manager's
 * head field (mgr+0x0) — i.e. the top-most entry was the match. */
extern u8 g_pointLights[];
extern s32 func_00284730(void *subject, void *entry, void *entryHi);
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADBA0);
#else
s32 func_002ADBA0(void *subject) {
    u8 *mgr = g_pointLights + 0x2400;
    s32 found = 0;
    s32 i;

    for (i = *(s32 *)(mgr + 0xC) - 1; i >= 0; i--) {
        u8 *entry = g_pointLights + 0x2420 + i * 0x10;
        if (func_00284730(subject, entry, entry + 0x20) != 0) {
            found = i + 1;
            break;
        }
    }
    return found == *(s32 *)mgr;
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADC30);

/**
 * Rotate vector `v` by quaternion `q`, writing the result to `out`:
 * out = q * (v as a pure quaternion, w=0) * conjugate(q). The conjugate negates
 * the xyz of q and keeps its w.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADC50);
#else
void func_002ADC50(Vec4 *out, Vec4 *v, Vec4 *q) {
    Vec4 conj, vpure, rotated;

    Vec4ScaleVu0(&conj, -1.0f, q);
    conj.w = q->w;
    vpure = *v;
    vpure.w = 0.0f;
    func_00284180(&rotated, q, &vpure);
    func_00284180(out, &rotated, &conj);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADCE0);

/**
 * Rotate `src` about `axis` by `angle` into `dst`. For a negligible angle
 * (|angle| < 1e-5) it just copies src to dst; otherwise it normalises the axis,
 * builds the axis-angle quaternion (func_002AC4D0), and rotates src by it
 * (func_002ADC50).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADD28);
#else
void func_002ADD28(Vec4 *dst, Vec4 *src, Vec4 *axis, f32 angle) {
    if (GetFloatAbs(angle) < 1e-5f) {
        *dst = *src;
    } else {
        Vec4 quat;
        Vec3RescaleToLenVu0(&quat, 1.0f, axis);
        func_002AC4D0(&quat, &quat, angle);
        func_002ADC50(dst, src, &quat);
    }
}
#endif

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

extern void func_00283DC0(Mat4x4 *dst, Vec4 *in);   /* build rotation matrix from a vec (VU0) */

/**
 * Transform `in` into `out` by the orientation of obj's resolved sub-source
 * (func_002ADF18 = pvar word 2). If there is no sub-source, out = in (copy) and
 * returns 0. Otherwise builds its rotation matrix; when the source's flag (+0x3C)
 * has bit 0x2, it composes an extra rotation from the source's +0x20 vec and
 * re-applies obj's own matrix at +0xC0. Returns 1 when transformed.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE0B8);
#else
s32 func_002AE0B8(void *self, void *obj, Vec4 *in, Vec4 *out) {
    s32 src = func_002ADF18((Moby *)obj);
    Mat4x4 rot;

    (void)self;
    if (src == 0) {
        *out = *in;
        return 0;
    }
    func_00283DC0(&rot, (Vec4 *)src);
    if (*(s32 *)(src + 0x3C) & 0x2) {
        Mat4x4 rot2;
        Mat4x4 mtx;
        func_00283DC0(&rot2, (Vec4 *)(src + 0x20));
        func_00284048(&mtx, (const Vec4 *)&rot2);
        func_00283A48(out, in, (Vec4 *)&mtx);
        func_00283A48(out, out, (Vec4 *)((u8 *)obj + 0xC0));
    } else {
        func_00283A48(out, in, (Vec4 *)&rot);
    }
    return 1;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE198);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE2D8);

extern void MatrixMultiplyVu0(Mat4x4 *dst, Mat4x4 *a, Mat4x4 *b);
extern void MatrixToEulerAngles(Mat4x4 *mtx, void *outAngles);

/**
 * Compute a moby's local-frame offset + composed orientation from its resolved
 * sub-source. No source (func_002ADF18==0) → returns 0. Otherwise builds the
 * source's base rotation matrix — from obj+0xF0 when the source flag +0x3C bit
 * 0x40 is set, else obj+0xC0 — transforms (arg3 - obj pos+0x10) into that frame
 * (out arg5), then composes with arg4's rotation (MatrixMultiplyVu0) and writes
 * the euler angles to arg6. Returns 1.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE460);
#else
s32 func_002AE460(void *self, Moby *obj, Vec4 *arg3, Vec4 *arg4, void *arg5, void *arg6) {
    s32 src = func_002ADF18(obj);
    Mat4x4 rot;
    Vec4 delta;
    Mat4x4 composed;
    (void)self;

    if (src == 0) {
        return 0;
    }
    if (*(s32 *)(src + 0x3C) & 0x40) {
        func_00283DC0(&rot, (Vec4 *)((u8 *)obj + 0xF0));
        func_00284048(&rot, (const Vec4 *)&rot);
    } else {
        func_00284048(&rot, (const Vec4 *)((u8 *)obj + 0xC0));
    }
    Vec4SubVu0(&delta, arg3, (Vec4 *)((u8 *)obj + 0x10));
    func_00283A70((Vec4 *)arg5, &delta, &rot);
    func_00283DC0(&composed, arg4);
    MatrixMultiplyVu0(&composed, &rot, &composed);
    MatrixToEulerAngles(&composed, arg6);
    return 1;
}
#endif

/**
 * Compose two orientation matrices (from m1 and m2), convert the product to euler
 * angles written into `out`, then stash the source vectors: out+0x10 always gets
 * arg2, and out+0x20 gets m1 when out's flag word (+0x3C) has bit 0x2 set.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE558);
#else
void func_002AE558(void *out, Vec4 *arg2, Vec4 *m1, Vec4 *m2) {
    Mat4x4 rot1;
    Mat4x4 mtx1;
    Mat4x4 rot2;
    Mat4x4 product;
    u8 *o = (u8 *)out;

    func_00283DC0(&rot1, m1);
    func_00284048(&mtx1, (const Vec4 *)&rot1);
    func_00283DC0(&rot2, m2);
    MatrixMultiplyVu0(&product, &mtx1, &rot2);
    MatrixToEulerAngles(&product, out);
    if (*(s32 *)(o + 0x3C) & 0x2) {
        *(Vec4 *)(o + 0x20) = *m1;
    }
    *(Vec4 *)(o + 0x10) = *arg2;
}
#endif

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

extern s32 g_activeGadgetItem[];   /* [0]=active gadget item; [1]/[2]/[3] = per-mode slots */
extern s16 g_fileLoadState;
extern s32 func_00294EE0(s32 fileId);   /* nonzero if the item's resource is already resident */
extern s32 func_00294CD0(s32 fileId);   /* kick the item's resource load */

/**
 * Equip gadget `itemId`. Looks up its weapon-variant entry (g_weaponTable indexed
 * by g_itemEquippedSlot[itemId], 0xE0 stride) and dispatches on its mode (+0xC):
 *   - mode 0: needs a resource. If not resident (func_00294EE0==0) and a file load
 *     is already in progress (g_fileLoadState!=0), abort (return 0). Otherwise set
 *     it active and, if still not resident, kick its load (func_00294CD0).
 *   - mode 1/2/3: record itemId into the matching g_activeGadgetItem slot (1/2/3).
 * Returns 1 when equipped/queued, 0 when aborted.
 *
 * NOTE: the field passed to func_00294EE0/func_00294CD0 is the weapon-variant
 * entry's +0x14, which symbol_addrs currently labels "boltPrice". Either that
 * label is context-dependent or slightly off — the #else is faithful to the asm
 * (it forwards +0x14 to those two calls regardless); flagged for a Ghidra recheck.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE6C8);
#else
s32 func_002AE6C8(s32 itemId) {
    u8 *w = (u8 *)&g_weaponTable[g_itemEquippedSlot[itemId]];
    s32 mode = *(s32 *)(w + 0xC);

    if (mode == 0) {
        if (func_00294EE0(*(s32 *)(w + 0x14)) == 0 && g_fileLoadState != 0) {
            return 0;
        }
        g_activeGadgetItem[0] = itemId;
        if (func_00294EE0(*(s32 *)(w + 0x14)) != 0) {
            return 1;
        }
        func_00294CD0(*(s32 *)(w + 0x14));
        return 1;
    }
    if (mode == 3) {
        g_activeGadgetItem[3] = itemId;
        return 1;
    }
    if (mode == 2) {
        g_activeGadgetItem[2] = itemId;
        return 1;
    }
    if (mode == 1) {
        g_activeGadgetItem[1] = itemId;
        return 1;
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE7E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE9E0);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF590);

extern s32 func_00283AB8(Vec4 *v);   /* pack a float {x,y,z,scale} vec into a word */

/**
 * Quantise a vector into a packed word written to *out. Scales the vector so its
 * largest-magnitude component maps to a chosen precision: takes maxAbs of x/y/z,
 * derives an integer scale = clamp(trunc(maxAbs * 158.73), 1, 255) (FloatToInt
 * truncates toward zero, not rounds), normalises
 * each component by 1/(scale*1e-4) and biases by 127, then packs {x',y',z',scale}
 * via func_00283AB8.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF598);
#else
void func_002AF598(Vec4 *vec, s32 *out) {
    f32 ax = GetFloatAbs(vec->x);
    f32 ay = GetFloatAbs(vec->y);
    f32 az = GetFloatAbs(vec->z);
    f32 maxAbs;
    s32 scale;
    f32 inv;
    Vec4 packed;

    maxAbs = (ay > ax) ? ay : ax;
    maxAbs = (az > maxAbs) ? az : maxAbs;
    scale = FloatToInt(maxAbs * 0x1.3d75d8p7f);   /* * 158.73016 */
    if (scale >= 0x100) {
        scale = 0xFF;
    }
    if (scale <= 0) {
        scale = 1;
    }
    inv = 1.0f / ((f32)scale * 1e-4f);
    packed.x = vec->x * inv + 127.0f;
    packed.y = vec->y * inv + 127.0f;
    packed.z = vec->z * inv + 127.0f;
    packed.w = (f32)scale;
    *out = func_00283AB8(&packed);
}
#endif

/**
 * Decode a packed RGBA colour pointed to by `colorPtr` into a signed direction/
 * offset vector scaled by its alpha: unpack to floats, recentre RGB around 127
 * (so 0x80 -> 0), and scale the whole vector by alpha * 1e-4. Writes to `out`.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF6A0);
#else
void func_002AF6A0(Vec4 *out, u32 *colorPtr) {
    Vec4 color;
    Vec4 offset;
    f32 alpha;

    offset.x = 127.0f;
    offset.y = 127.0f;
    offset.z = 127.0f;
    offset.w = 0.0f;
    func_00283AA0(&color, *colorPtr);
    alpha = color.w * 0.0001f;
    Vec4SubVu0(&color, &color, &offset);
    Vec4ScaleVu0(out, alpha, &color);
}
#endif

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

/**
 * Test whether `x` sits just below the object's reference value: returns 1 when
 * x <= round(refValue, 4) AND the gap (rounded - x) is smaller than
 * round(obj->field48 * 0.5, 4); otherwise 0. Both quantities are rounded to 4
 * decimal places via func_002AF948. refValue = func_002A0368(obj).
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF9C8);
#else
s32 func_002AF9C8(void *obj, f32 x) {
    f32 rounded = func_002AF948(4, func_002A0368(obj));
    f32 delta = rounded - x;
    f32 half = func_002AF948(4, *(f32 *)((u8 *)obj + 0x48) * 0.5f);
    return (x <= rounded) && (delta < half);
}
#endif

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
 * ORACLE STATUS: VALIDATED via the tester's full-game EE effect-diff (PASS). Not
 * cmp-oracle-able by the standalone asm-vs-C harness - its sin helper
 * func_00283B48 is a VU0 *microprogram* (vcallms 0xC90) that cannot run headless
 * - but the tester runs it for real (the microprograms ARE loaded in full-game
 * context). Covered by the tester-EE path, not the standalone cmp-oracle. */
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
 * ORACLE STATUS: VALIDATED via the tester's full-game EE effect-diff (PASS). Not
 * cmp-oracle-able by the standalone asm-vs-C harness - its sin/cos helpers
 * func_00283B48/func_00283B30 are VU0 *microprograms* (vcallms 0xC80/0xC90) that
 * cannot run headless - but the tester runs them for real (microprograms loaded
 * in full-game context). Covered by the tester-EE path, not the cmp-oracle. */
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

/**
 * Convert spherical coordinates (radius, azimuth, elevation) to a cartesian
 * vector in `dst`: x = r·cos(az)·cos(el), y = r·sin(az)·cos(el), z = r·sin(el).
 * (func_00283B30 = cosine, func_00283B48 = sine.)
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFE68);
#else
void func_002AFE68(void *handle, f32 value, f32 angle1, f32 angle2) {
    Vec4 *dst = (Vec4 *)handle;
    dst->x = value * func_00283B30(angle1) * func_00283B30(angle2);
    dst->y = value * func_00283B48(angle1) * func_00283B30(angle2);
    dst->z = value * func_00283B48(angle2);
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFF08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFF10);

/* Callees for func_002B0038 (sigs traced from call registers / symbol_addrs). */
extern void func_002A0A58(Moby *parent, void *srcTransform, Mat4x4 *out);
extern void UpdateMobyAnimation(Moby *moby);
extern void UpdateMobyBSphereAndGrid(Moby *moby);
extern void func_002A1F20(Moby *moby);

/**
 * Rebuild a child moby's transform from its parent + a source transform, then
 * re-run its per-frame updates. Composes parent/srcTransform into a scratch
 * matrix (func_002A0A58), copies the translation row (matrix+0x30) to the child
 * pos (+0x10), optionally mirrors basis rows 0/1/2 per the flags bits 1/2/4,
 * ticks the animation, refreshes the bounding sphere/grid (unless mode bit 4),
 * installs the matrix at child+0xC0 (func_00284028/func_002ABE90) and runs
 * func_002A1F20. Finally mirrors parent's mode bit 0 into the child's flags
 * (set 0x40|0x01 / clear 0x41) and forces the 0x6 dirty bits.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0038);
#else
void func_002B0038(Moby *parent, Moby *child, void *srcTransform, s32 flags) {
    u8 *c = (u8 *)child;
    Mat4x4 m;

    func_002A0A58(parent, srcTransform, &m);
    *(Vec4 *)(c + 0x10) = *(Vec4 *)((u8 *)&m + 0x30);
    if (flags & 1) {
        Vec4ScaleVu0((Vec4 *)&m, -1.0f, (Vec4 *)&m);
    }
    if (flags & 2) {
        Vec4ScaleVu0((Vec4 *)((u8 *)&m + 0x10), -1.0f, (Vec4 *)((u8 *)&m + 0x10));
    }
    if (flags & 4) {
        Vec4ScaleVu0((Vec4 *)((u8 *)&m + 0x20), -1.0f, (Vec4 *)((u8 *)&m + 0x20));
    }
    UpdateMobyAnimation(child);
    if ((*(u16 *)(c + 0x34) & 4) == 0) {
        UpdateMobyBSphereAndGrid(child);
    }
    func_00284028(c + 0xC0, &m);
    func_002ABE90((Mat4x4 *)(c + 0xC0));
    func_002A1F20(child);
    if (*(u16 *)((u8 *)parent + 0x34) & 1) {
        *(u16 *)(c + 0x34) = (u16)(*(u16 *)(c + 0x34) | 0x41);
    } else {
        *(u16 *)(c + 0x34) = (u16)(*(u16 *)(c + 0x34) & 0xFFBE);
    }
    *(u16 *)(c + 0x34) = (u16)(*(u16 *)(c + 0x34) | 0x6);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0150);

/**
 * Pick the nearest valid moby to `queryVec` from the flagged-moby list. For each
 * entry: skip if it has no pvar block (func_002AC058 == 0) or its word0 float is
 * 0, then score it with func_002B0150 (skip on its reject flag). Track the moby
 * with the smallest score. The list cursor only advances on a skip — a scored
 * entry re-reads the same slot (func_002B0150 consumes/compacts it), mirroring
 * the original's loop exactly. Returns the best moby, or NULL if none.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B02C8);
#else
Moby *func_002B02C8(Vec4 *queryVec, f32 a, f32 b, f32 c, f32 d) {
    Moby **cursor = g_mobyFlagged1000List;
    Moby *moby = *cursor;
    Moby *best = 0;
    f32 bestScore = 99999008.0f;   /* 0x4CBEBC20 */
    Vec4 query;

    if (moby == 0) {
        return 0;
    }
    query = *queryVec;

    do {
        s32 pvar = func_002AC058(moby);
        if (pvar == 0) {
            cursor++;
        } else if (*(f32 *)pvar == 0.0f) {
            cursor++;
        } else {
            s32 reject;
            f32 score = func_002B0150(&query, moby, &reject, a, b, c, d);
            if (reject != 0) {
                cursor++;
            } else if (score < bestScore) {
                bestScore = score;
                best = moby;
            }
        }
        moby = *cursor;
    } while (moby != 0);

    return best;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B03E8);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0BD8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0BF0);

/**
 * Transform vector `a` into `out` by a rotation. When a matrix `b` is supplied
 * (b != 0) use it directly; otherwise build one from the quaternion at ctx+0xC0
 * into a scratch matrix and transform through that.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0C40);
#else
void func_002B0C40(s32 ctx, void *out, void *a, void *b) {
    if (b != 0) {
        func_00283A48((Vec4 *)out, (Vec4 *)a, (Vec4 *)b);
    } else {
        Mat4x4 mat;
        func_00284048(&mat, (const Vec4 *)(ctx + 0xC0));
        func_00283A48((Vec4 *)out, (Vec4 *)a, (Vec4 *)&mat);
    }
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0CA8);

/**
 * Resolve `a` into `out` via func_002B0C40, rescale out's XY to horizontal
 * length `len`, then transform out in place by the object's matrix at ctx+0xC0.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0CC0);
#else
extern void func_00283920(Vec4 *dst, Vec4 *src, f32 len);   /* rescale xy to length len (VU0) */
void func_002B0CC0(s32 ctx, Vec4 *out, void *a, void *b, f32 len) {
    func_002B0C40(ctx, out, a, b);
    func_00283920(out, out, len);
    func_00283A48(out, out, (Vec4 *)(ctx + 0xC0));
}
#endif

/**
 * Resolve `a` into `out` via func_002B0C40, override out.z with the supplied
 * height t, then transform out in place by the object's matrix at ctx+0xC0.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0D20);
#else
void func_002B0D20(s32 ctx, Vec4 *out, void *a, void *b, f32 t) {
    func_002B0C40(ctx, out, a, b);
    out->z = t;
    func_00283A48(out, out, (Vec4 *)(ctx + 0xC0));
}
#endif

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

/**
 * Compute a direction vector into `out`, selected by two mode flags:
 *  - D_1A8CA0 == 0: out = world +Z (SetVec4UnitZ).
 *  - else D_1A8CA4 != 0: out = normalised flattened to-camera dir from `a`
 *    (func_002B0DC8 with keepSign=1).
 *  - else: out = (reference point g_soundBankHandlesBlk+0xB0) - a, rescaled to
 *    length ±1 (negative when the +0xBC selector is positive).
 * Finally, when flag == 0 the result is negated in place.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0E40);
#else
void func_002B0E40(Vec4 *a, Vec4 *out, s32 flag) {
    if (D_1A8CA0 == 0) {
        SetVec4UnitZ(out);
    } else if (D_1A8CA4 != 0) {
        func_002B0DC8(a, out, 1);
    } else {
        f32 sign = 1.0f;
        if (0.0f < *(f32 *)(g_soundBankHandlesBlk + 0xBC)) {
            sign = -1.0f;
        }
        Vec4SubVu0(out, (Vec4 *)(g_soundBankHandlesBlk + 0xB0), a);
        Vec3RescaleToLenVu0(out, sign, out);
    }
    if (flag == 0) {
        Vec4ScaleVu0(out, -1.0f, out);
    }
}
#endif

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
    return ((s32 (*)(u8 *))func_002B0E40)(p + 0x10);
}

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0F38);

/**
 * Offset a base position `src` into `out` by a direction of magnitude t:
 *  - D_1A8CA0 == 0 (simple mode): out.z = src.z - t (straight vertical drop).
 *  - else: build a direction from `a` (func_002B0E40 with flag=0), rescale it to
 *    length t, and add it to src: out = src + t*dir.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0F40);
#else
void func_002B0F40(Vec4 *a, Vec4 *out, Vec4 *src, f32 t) {
    if (D_1A8CA0 == 0) {
        out->z = src->z - t;
    } else {
        Vec4 dir;
        func_002B0E40(a, &dir, 0);
        Vec3RescaleToLenVu0(&dir, t, &dir);
        Vec4AddVu0(out, src, &dir);
    }
}
#endif

/**
 * Forward to func_002B0F40 on the sub-object at +0x10.
 *
 * TYPE NOTE: p stays u8*. func_002B0F40 consumes p+0x10 (its a0) as a Vec4*
 * (Vec3RescaleToLenVu0 / Vec4AddVu0), so p+0x10 is a Vec4. As with func_002B0F18
 * the container is plausibly a Moby (pos@0x10) but unconfirmed (caller passes a
 * stack-local pointer, StepMobySpringFollow @ 0x2B5844), so p is left u8*.
 */
s32 func_002B0FC0(u8 *p) {
    return ((s32 (*)(u8 *))func_002B0F40)(p + 0x10);
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

/**
 * func_002B1220 = OrientMatrixToGravityInverted(mtx3x4, gravDir, outMtxOpt):
 * orient mtx3x4 toward "up" by negating the gravity direction and delegating to
 * func_002B1270 (OrientMatrixToGravity, which orients toward gravDir).
 *
 * gravDir (arg2/$5) is scaled by -1 into a scratch vector; outMtxOpt (arg3/$6),
 * the optional delta-matrix output, is passed through unchanged.
 *
 * NOTE: the negated source is arg2/gravDir, NOT arg3. The body never explicitly
 * moves $5 because it forwards it straight into Vec4ScaleVu0, whose src operand
 * IS $5 — `n`/Vec4ScaleVu0 @0x2836E0 does `lqc2 $vf1,0(a1)` (a1=$5). func_002B1270's
 * gravDir param is likewise $5. Arg3/$6 is outMtxOpt (an output pointer); negating
 * it would be meaningless.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1220);
#else
void func_002B1220(void *mtx3x4, Vec4 *gravDir, void *outMtxOpt) {
    Vec4 invGravDir;
    Vec4ScaleVu0(&invGravDir, -1.0f, gravDir);
    func_002B1270(mtx3x4, &invGravDir, outMtxOpt);
}
#endif

/**
 * func_002B1270 = OrientMatrixToGravity: rotate the 3x4 matrix `mtx3x4` so its
 * +0x20 axis aligns toward the gravity direction, in place. Builds the shortest-
 * arc half-angle quaternion from cross(normalize(gravDir), mtx+0x20): the xyz is
 * that cross scaled by 0.5, w = -sqrt(1 - |xyz|^2). Converts the quaternion to a
 * 3x4 matrix (func_00284308) and multiplies it into mtx3x4 (func_002840E8). If
 * outMtxOpt != 0, the 0x30-byte delta matrix is also copied out.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1270);
#else
void func_002B1270(void *mtx3x4, Vec4 *gravDir, void *outMtxOpt) {
    u8 quatMtx[0x30];
    Vec4 normGrav;
    Vec4 quat;
    f32 lenSq;

    Vec3RescaleToLenVu0(&normGrav, 1.0f, gravDir);
    Vec3CrossVu0(&quat, &normGrav, (Vec4 *)((char *)mtx3x4 + 0x20));
    Vec4ScaleVu0(&quat, 0.5f, &quat);
    lenSq = Vec3LengthVu0(&quat);
    lenSq = lenSq * lenSq;
    quat.w = -func_002835C0(1.0f - lenSq);
    func_00284308(&quat, quatMtx);
    func_002840E8(mtx3x4, quatMtx, mtx3x4);
    if (outMtxOpt != 0) {
        func_00283460(outMtxOpt, quatMtx, 0x30);
    }
}
#endif

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

/* Callee of func_002B1778's #else arm (func_002B1710 itself stays INCLUDE_ASM,
 * its own #else is out of this cohort — this prototype is needed only so the
 * carried func_002B1778 #else body compiles under TARGET_NATIVE). */
extern f32 func_002B1710(s32 a, s32 b);

/** func_002B1778 — sample the sine of the (a mod b) index angle, remap it from
 *  [-1,1] to [0,1], and drive the packed-vec4 2-colour blend by that weight:
 *  LerpByteVec4Packed(sin(func_002B1710(a,b))*0.5+0.5, dst, src). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1778);
#else
void func_002B1778(s32 a, s32 b, void *dst, void *src) {
    f32 s = func_00283B48(func_002B1710(a, b));
    LerpByteVec4PackedVu0(s * 0.5f + 0.5f, dst, src);
}
#endif

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

/* shared by func_002B1A90 + func_002B1B48 (the notice-slot pair) — declared
 * above both so the TARGET_NATIVE #else bodies compile. */
extern s32   D_1A8C60;  /* pending-notice arg2 latch */
extern void *D_1A8C64;  /* pending-notice subject latch */
extern void  func_0029DB10(char *text, s32 arg);

/* func_002B1A90(subject, stringId, arg2): claim the single global notice slot for
 * `subject`. If this subject already holds it (D_1A8C64 == subject): refresh the
 * text and return 2. If a DIFFERENT subject holds it (D_1A8C64 != 0): reject,
 * return 0. Otherwise claim it (latch D_1A8C64 = subject) and return 1. In the
 * claim/refresh cases it localizes stringId (when set) through func_0029DB10,
 * writes D_1A8C60 = 2, and records the stringId at &g_pMobyGroupIterMoby+0x8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1A90);
#else
s32 func_002B1A90(void *subject, s32 stringId, s32 arg2) {
    if (D_1A8C64 == subject) {
        if (stringId != 0) {
            func_0029DB10(GetLocalizedString(stringId), arg2);
        }
        *(s32 *)((u8 *)&g_pMobyGroupIterMoby + 0x8) = stringId;
        D_1A8C60 = 2;
        return 2;
    }
    if (D_1A8C64 != 0) {
        return 0;
    }
    if (stringId != 0) {
        func_0029DB10(GetLocalizedString(stringId), arg2);
    }
    D_1A8C64 = subject;
    D_1A8C60 = 2;
    *(s32 *)((u8 *)&g_pMobyGroupIterMoby + 0x8) = stringId;
    return 1;
}
#endif

/* func_002B1B48(subject, stringId, arg2): guarded one-shot notice/prompt setup.
 * First runs func_002B1A90(subject) as a gate — if it returns non-zero, abort
 * with that code. Otherwise, when stringId is set, localize it and hand the
 * text + arg2 to func_0029DB10 (the display/queue helper). Latches the pending
 * notice state (g_pendingNoticeArg2 = arg2, g_pendingNoticeSubject = subject,
 * and the subject-slot's stringId at &g_pMobyGroupIterMoby+0x8) and returns 3.
 * (D_1A8C60/D_1A8C64/func_0029DB10 declared above the pair; func_002B1A90 is
 * defined just above so needs no forward decl in the TARGET_NATIVE build.) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1B48);
#else
s32 func_002B1B48(void *subject, s32 stringId, s32 arg2) {
    s32 gate = func_002B1A90(subject, stringId, arg2);
    if (gate != 0) {
        return gate;
    }
    if (stringId != 0) {
        func_0029DB10(GetLocalizedString(stringId), arg2);
    }
    D_1A8C60 = arg2;
    D_1A8C64 = subject;
    *(s32 *)((u8 *)&g_pMobyGroupIterMoby + 0x8) = stringId;
    return 3;
}
#endif

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
