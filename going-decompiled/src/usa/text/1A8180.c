#include "common.h"

/*
 * text/1A8180 — the "game-state cluster" (carve-pipeline pick #1, 2026-06-12;
 * vaddr 0x2A8200..0x2B2267, 190 fns): moby helpers (group iteration, ground
 * probes, hit events, list maintenance), small math/easing helpers, and the
 * progress counters (CountPlatinumBolts / CountSkillPointsCompleted /
 * MarkLevelAvailable and friends).
 *
 * The matcher builds THIS unit at -O2 -G8 -fno-gcse -fno-strict-aliasing
 * (per-unit GFLAG/CC1EXTRA override in tools/ee/objdiff_build.sh / diff.sh /
 * build.sh / unit_flags.sh) — the same later-SN-cc1 TU model as the other
 * gameplay-text units. -fno-strict-aliasing (as text/235FE8): cc1 2.9 turns
 * type-based aliasing on at -O2, so a u16 load is scheduled above a store to
 * a pointer-typed global and CSE'd across it; the ROM orders func_002A9550's
 * slot-list reads after the g_pMobyGroupIterCursor store and re-reads them
 * after the slot/moby stores, which only the no-TBAA model reproduces. Adding
 * the flag left every function already at 100.00% in this unit unchanged
 * (task #1100).
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
 * ENGINE-2.96 ARM SWEEP (task #467, 2026-09-19): every `#else` arm below was
 * promoted once through the MATCH_ engine96 arm (objdiff_build.sh: cc1
 * 2.96-ee-001003-1, -O2 -G8 -fno-schedule-insns -fno-strict-aliasing) —
 * 116 measured, 0 reached 100.00%, all reverted to `#else`; each arm carries
 * a `t467 engine96 arm` line with its % and residual class. Three unit-wide
 * facts from that sweep, so nobody re-derives them per function:
 *   1. cc1 2.96 SIBLING-CALLS every tail call and the ROM never does; the
 *      `__asm__ __volatile__("")` guard (apply_tailcall_guards.py) removes it
 *      only as the function-scope LAST statement of a VOID path — a
 *      `return f(...)` needs `T r = f(...); __asm__ __volatile__(""); return r;`.
 *      The guards are now in place in the arms that needed them.
 *   2. cc1 2.96-001003-1 emits every float literal 1 ULP BELOW nearest
 *      (3.1415927f -> 0x40490FDA, even 3.14159274101257324f -> ...FDA); the
 *      ROM bits come out only from the +1 ULP spelling (3.1415929794311523f
 *      -> ...FDB, measured on func_002B1710 / func_002B17F8). Such a
 *      spelling is WRONG for the native arm, so it cannot live in a shared
 *      `#else` body — an engine promotion needs its own literal.
 *   3. The dominant residual is SCHED-TIEBREAK: instruction-identical code
 *      whose same-cycle-ready instructions are emitted in the opposite order
 *      (the ROM keeps RTL/luid order — prologue `sd` before the body's first
 *      `move`, `sra` before `subu` after a call — cc1 001003-1 inverts it).
 *      8 arms are ORDER-ONLY (same instruction multiset). Not C-controllable:
 *      statement/temp/expression re-phrasings RUN on 5 of them changed
 *      nothing; sched-ON and -fno-schedule-insns2 both make it worse.
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
    /* 0x05 */ u8 pad05[0x0F];
    /* 0x14 */ s32 mobyClass;     /* moby class id this variant spawns/answers to
                                     (matched against Moby.oClass by func_002AE7E8) */
    /* 0x18 */ u8 pad18[0xC8];
} WeaponVariant;
extern WeaponVariant g_weaponTable[];  /* 0xE0-stride weapon-variant table */
extern u8 g_abLevelAvailableFlags[8]; /* really u8[0x1C]; same cc1-small address model as g_skillPointFlags */
extern s32 g_anAvailableLevelOrder[0x1C];
extern u8 g_collHitEventRing[];        /* 64 x 0x40 hit-event records */
extern u8 g_pCollWorldData[];          /* collision-world block; +0x14 = hit-event ring write cursor */
extern u8 D_139648[];                  /* palette-cycle counter base (incomplete-array decl: out of gp range) */
extern s32 D_258CF0[0x15];             /* per-level lookup table */

/* Callees (value-returning declarations keep cc1 from sibling-call
 * optimising forwarding tails — see text/198FA0). */
extern s32 CollLine(void *to, void *from, s32 mask, void *moby, void *out);
extern f32 ProbeGroundHeight(Vec4 *pos, f32 zOffset, s32 mask);
extern s32 CollSphere(void *center, s32 mask, void *moby, f32 radius); /* radius in $f12, clamped to 10.0 (CONFIRMED per symbol_addrs + func_002A90A8 asm) */
extern s32 func_001163B0(void);        /* core random-state step */
extern s32 ProbeMobyGroundBelow(Moby *moby);
extern s32 ColorLerpPacked(void *a, void *b, void *c);
/* ColorLerpPacked's real prototype: blend vec4 dst from src by phase t (t in
 * $f12). The (void*,void*,void*) form above is only used by the pure
 * register-passthrough forwarder func_002AA3D0. */
typedef void (*LerpByteVec4PackedFn)(f32 t, void *dst, void *src);
#define LerpByteVec4PackedVu0 ((LerpByteVec4PackedFn)ColorLerpPacked)
extern s32 func_0029DA88(s32 a);
extern char *GetLocalizedString(s32 stringId);
extern s32 func_0029DAD0(char *text, s32 arg);
extern void func_002B0E40(Vec4 *a, Vec4 *out, s32 flag);
extern void func_002B0F40(Vec4 *a, Vec4 *out, Vec4 *src, f32 t);
extern void func_002B0C40(s32 ctx, void *out, void *a, void *b);
extern void func_002B1270(void *a, Vec4 *b, void *c);
/* Vec2LengthXyVu0: planar (xy) magnitude of a Vec4, returned as f32 in $f0. The
 * native #else scanners (func_002A8D08/func_002A90A8) feed it into Atan2fPoly
 * (atan2) and float compares, so they need the true f32 return (an s32 decl would
 * make ee-gcc insert a spurious cvt.s.w). The sole 2.9-matched reference
 * (func_002B0D70) discards the value, so the f32 form is inert to that arm and
 * the engine (MATCH_) arms need it — one declaration for every build. */
extern f32 Vec2LengthXyVu0(void *vec);
/* Atan2fPoly (183558.c region): 2-arg arctangent (minimax poly
 * + quadrant offset, self-contained VU0 — no vcallms upload), returns the angle
 * as f32 in $f0. Native #else of func_002A8C70 needs the true f32 return so the
 * atan2 result feeds func_002A8B08's angle arg without a spurious int<->float
 * cvt (same f32-vs-s32 class as WrapAnglePiSum above). The two callers in this
 * unit (func_002A8C70, func_002B1348) are both #else-only, so the f32 form is
 * inert to the matched build; guarded per-build to mirror the WrapAnglePiSum
 * convention. RECOVERED: $f0 return at jr ra in Atan2fPoly.s -> f32. */
#ifdef TARGET_NATIVE
extern f32 Atan2fPoly(f32 y, f32 x);
#else
extern s32 Atan2fPoly(f32 x, f32 y);
#endif
extern s32 func_002A12C0(void *p, s32 r, s32 g, s32 b);
extern s32 func_00283638(Moby *moby);
/* PostMobyHitEvent(moby, a1, flags, vecA, vecB, dist): true ABI recovered from
 * the .s — 5 int/ptr args + f32 dist in $f12, TWO Vec4* (the old 5-arg
 * (…,s32 c,Vec4 *dir) decl mistyped vecA as s32 and dropped the float). Aligns
 * with the forwarder func_002A9F30 (1B4218.c already calls that 5-arg-with-float). */
extern s32 PostMobyHitEvent(Moby *moby, s32 a1, s32 flags, Vec4 *vecA, Vec4 *vecB, f32 dist);
extern s32 func_002B1C20(void);
/* func_002A9550: moby-group iterator advance/filter step. Takes the same
 * (out, moby, wantInactive, wantActive) 4-arg shape its only caller
 * func_002A9468 forwards ($a0..$a3 passthrough); the wantInactive/wantActive
 * filter args were dropped by an earlier 2-arg guess. Defined later in this
 * unit (#else); declared here for func_002A9468's #else call. */
extern s32 func_002A9550(Moby **out, Moby *moby, s32 wantInactive, s32 wantActive);
extern f32 Vec3DistVu0(void *p, f32 *src);
/* func_002A0368: read a reference value from an object (1A00F0 unit); takes the
 * object pointer in $4 (not a float), returns the value as f32 in $f0. */
extern f32 func_002A0368(void *obj);
extern f32 GetFloatAbs(f32 x);
extern s32 func_002835E0(s32 x);
extern f32 func_00284678(f32 *out, f32 angle);
extern f32 func_00283B30(f32 angle);  /* cosine */
extern f32 func_00283B48(f32 angle);  /* sine (0x18 after cosine in 183558.c) */
extern f32 WrapAnglePiDiff(f32 a, f32 b);
/* func_002AB000: spring-style scalar step. Clamps *p to +/-|v0|, integrates
 * *p = p*(1-v2) + v1*v0, clamps to +/-v3 (when v3>0), then re-clamps to +/-|v0|.
 * v0..v3 arrive in $f12..$f15. Return is void: the asm leaves an incidental $f0
 * but all callers discard it. Defined later in this unit (#else). */
extern void func_002AB000(f32 *p, f32 v0, f32 v1, f32 v2, f32 v3);
/* WrapAnglePiSum, which genuinely returns f32 in $f0. The
 * native #else of func_002AB668 needs the true f32 return (else ee-gcc inserts a
 * spurious int->float cvt that corrupts the *p out-param, else_divergences #19).
 * BUT the matching build's func_002AAFB8 (line ~768) byte-matches ONLY with the
 * s32 form: it returns s32 from this call, so the s32-callee + s32-return type
 * errors cancel into the exact $f0 passthrough the original emits (verified: a
 * plain f32 here regresses func_002AAFB8 from 100% to 88.24%). So guard per
 * build - matching keeps s32, native gets the correct f32. */
#ifdef TARGET_NATIVE
extern f32 WrapAnglePiSum(f32 a, f32 b);
#else
extern s32 WrapAnglePiSum(f32 a, f32 b);
#endif
/* WrapAnglePiDiff (183558.c) genuinely returns f32 in $f0; WrapAnglePiSum's
 * per-build declaration is above (s32 on the matched arm, see func_002AAFB8). */
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 36.40%
   -> UNKNOWN-@1: ROM `daddu a3,a1,zero` vs `sll v0,a1,0x2` */
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

/* Procedural/blend-anim helpers (other units) used by func_002A82D8's #else. */
extern s32 func_002A08C0(void *obj);              /* acquire a procedural-anim slot; <0 = none */
extern void func_002A3288(void *obj, s32 flags);  /* bind the acquired procedural-anim slot */
extern u8 g_proceduralAnimBounds[];               /* per-slot cached pose-bounds Vec4, stride 0x10 */
extern f32 IntToFloat(s32 x);

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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 54.11%
   -> UNKNOWN-@2: ROM `(none)` vs `daddu s0,a0,zero` */
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
        __asm__ __volatile__(""); /* cc1 2.96 sibling-call suppression (the ROM never sibcalls) */
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

/**
 * Start animation sequence `idx` at frame `frame` on a moby, blended over `arg4`
 * frames, with an explicit `flags` word (the parameterized sibling of
 * func_002A82D8 — which is this with flags fixed to the 0x300 bind + no bit-4
 * force). When arg4 <= 0 this delegates to func_002A8200 (instant set). Otherwise,
 * if the moby is already mid-blend (animTime > 0.025, or either blend word at
 * +0x50/+0x54 set) OR flags bit 2 (0x4) forces it, a procedural-anim slot is
 * acquired (func_002A08C0) and bound (func_002A3288) with bind = slot | (flags&1
 * ? 0x100 : 0) | (flags&2 ? 0x200 : 0); the current pose bounds snapshot into
 * g_proceduralAnimBounds[slot] and the current sequence is stashed (+0x42 → +0xA9,
 * +0x42 = 0xFF procedural marker, +0x40 = slot). Then the target frame, frame
 * pointers, and blend seed (animRate=1, animRate2=1/arg4, animTime=0, clear
 * animEventByte bit 1, cache loopSoundIdx) are set — identical to func_002A82D8.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 56.03%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-80` vs `addiu sp,sp,-64` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8448);
#else
void func_002A8448(Moby *obj, s32 idx, s32 frame, s32 arg4, s32 flags) {
    u8 *m = (u8 *)obj;
    u8 *seqEntry;

    if (arg4 <= 0) {
        func_002A8200(obj, idx, frame);
        __asm__ __volatile__(""); /* cc1 2.96 sibling-call suppression (the ROM never sibcalls) */
        return;
    }

    if (0.025f < *(f32 *)(m + 0x44) ||
        *(s32 *)(m + 0x50) != 0 ||
        *(s32 *)(m + 0x54) != 0 ||
        (flags & 4) != 0) {
        s32 slot = func_002A08C0(obj);
        if (slot >= 0) {
            s32 bind = (flags & 1) ? (slot | 0x100) : slot;
            if (flags & 2) {
                bind |= 0x200;
            }
            func_002A3288(obj, bind);
            *(Vec4 *)(g_proceduralAnimBounds + slot * 0x10) = *(Vec4 *)(m + 0x80);
            if (m[0x42] != 0xFF) {
                m[0xA9] = m[0x42];
            }
            m[0x42] = 0xFF;
            m[0x40] = (u8)slot;
        }
    }
    m[0x41] = (u8)frame;

    m[0x43] = (u8)idx;
    ResolveMobyAnimFramePtrs(obj);
    *(f32 *)(m + 0x48) = 1.0f;
    *(f32 *)(m + 0x4C) = 1.0f / IntToFloat(arg4);
    *(f32 *)(m + 0x44) = 0.0f;
    m[0x60] &= 0xFD;
    seqEntry = *(u8 **)(*(u8 **)(m + 0x24) + 0x48 + idx * 4);
    m[0x6C] = *(u8 *)(seqEntry + 0x11);
}
#endif

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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8620);

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

/* RandRangeInclusive: uniform random integer in [lo, hi] (inclusive) — take a 15-bit
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 80.95%
   -> SCHED-TIEBREAK (prologue interleave + sra/subu order after the call), ORDER-ONLY */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", RandRangeInclusive);
#else
s32 RandRangeInclusive(s32 lo, s32 hi) {
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

extern f32  GetRandomAngle(void);
extern f32  GetRandomFloatRange(f32 lo, f32 hi);
extern void func_002AFE68(void *handle, f32 value, f32 angle1, f32 angle2);

/** func_002A87F0 — spawn/place helper: draw two random angles and a random value
 *  in [lo, hi], then hand them to func_002AFE68 for `handle`. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 64.60%
   -> SCHED-TIEBREAK, ORDER-ONLY (same instruction multiset, 18 rows displaced) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A87F0);
#else
void func_002A87F0(void *handle, f32 lo, f32 hi) {
    f32 angle1 = GetRandomAngle();
    f32 angle2 = GetRandomAngle();
    f32 value = GetRandomFloatRange(lo, hi);
    func_002AFE68(handle, value, angle1, angle2);
    __asm__ __volatile__("");
}
#endif

/**
 * Generate a random direction vector into `dst`: a random magnitude in [lo, hi]
 * (func_002A86E0) and two random angles (func_002A87A8), mapped to cartesian —
 * x = r·cos(a2)·sin(a1), y = r·sin(a2)·sin(a1), z = r·cos(a1). The lo/hi pass
 * straight through to the magnitude RNG. (func_00283B30 = cos, ...B48 = sin.)
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 92.95%
   -> 90.33% with ROM-order muls (cos*sin*r), then ORDER-ONLY: SCHED-TIEBREAK in the epilogue
   restores */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", GetRandomVectorInSphere);
#else
void GetRandomVectorInSphere(Vec4 *dst, f32 lo, f32 hi) {
    f32 radius = func_002A86E0(lo, hi);
    f32 angle2 = func_002A87A8();
    f32 angle1 = func_002A87A8();
    dst->x = func_00283B30(angle2) * func_00283B48(angle1) * radius;
    dst->y = func_00283B48(angle2) * func_00283B48(angle1) * radius;
    dst->z = func_00283B30(angle1) * radius;
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
 * cc1.
 * Task #946 narrows this. Two of the colouring rows are source shape:
 *  - `c` (db - da) is ONE variable shared by the three axes. cc1 then gives
 *    it its own register ($f7) instead of tying it to db, as the ROM does.
 *  - `out->w = 0` is stored after the z axis's loads, and the zero from the
 *    t == 0 test stays live in $f8, as in the ROM.
 * With both (body in task #946's NOTE), 65 = 65 non-nop words with the ROM's
 * instruction order in the x block. What differs is FP register numbering:
 * t*t / t*t*t / the p1 component get $f6/$f3/$f5 against the ROM's
 * $f5/$f2/$f6, the per-axis temporaries shift with them, and the y/z blocks
 * issue their loads in a different order (6 rows ignoring register numbers).
 * 358 generated source variants over t2/t3 placement, `register`, declaration
 * order and expression shape all gave the same allocation. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 7.06% ->
   UNKNOWN-@0: ROM `mtc1 zero,$f8` vs `addiu sp,sp,-64` */
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
/* TODO(match): functional equivalent - 95.7%. The two endpoint c.eq.s shortcuts
   and the (1 - cos(t*pi))*0.5 blend reproduce, but the later cc1 schedules the
   compare's zero/one constant materialisation differently from the pinned cc1
   (operand/const-scheduling wall). Revisit once the gameplay-TU compiler is
   available. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 78.84%
   -> UNKNOWN-@0: ROM `mtc1 zero,$f0` vs `mtc1 zero,$f1` */
#ifndef TARGET_NATIVE
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8B00);

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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 80.69%
   -> UNKNOWN-@2: ROM `(none)` vs `daddu s0,a1,zero` */
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 78.35%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-64` vs `addiu sp,sp,-48` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8C70);
#else
void func_002A8C70(Moby *moby, Moby *target, f32 *driveOut,
                   f32 gain, f32 damp, f32 clampLimit) {
    f32 dx = target->pos.x - moby->pos.x;
    f32 dy = target->pos.y - moby->pos.y;
    f32 angle = Atan2fPoly(dx, dy);
    func_002A8B08(moby, driveOut, angle, gain, damp, clampLimit);
    __asm__ __volatile__("");
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8CF8);

/* func_002A8D08 — vertical collision-sweep + resolve of a moby position (MODEL,
 * UNCONFIRMED). Structurally traced; kept INCLUDE_ASM pending a runtime oracle
 * (see caveat) rather than shipping a large un-verifiable VU0 body.
 *
 *   s32 func_002A8D08(void *ent, Vec4 *ref, Vec4 *pos, s32 flags,
 *                     f32 stepZ, f32 radius, f32 snapEps, f32 hitEps)
 *     ent   ($4)  collision-world / entity handle (forwarded to CollLine/CollSphere)
 *     ref   ($5)  reference position; its xy is overwritten with pos.xy on return
 *     pos   ($6)  swept position (Vec4, +0x8 = z) — read + written in place
 *     flags ($7)  bit0 = clamp direction select, bit1 = skip the sphere-nudge loop
 *     stepZ ($f12) z probe/advance delta      radius ($f13) rescale len + sphere radius
 *     snapEps ($f14) surface-snap threshold   hitEps ($f15) hit-accept threshold
 *   returns ($22): 1 normally, 0 once a snap/hit resolution moved `pos`.
 *
 * Flow: (1) probe the surface height at pos+stepZ via func_002A9888, restore z,
 * and if |surface - ref.z| > snapEps run a min/max clamp (bit0 + sign select)
 * that may snap pos<-ref. (2) Build a rescaled step dir (normalize(pos-ref) *
 * radius*1.2) and a lifted origin (ref + (0,0,stepZ)); CollLine along it, and on
 * a hit whose planar angle (Vec2LengthXyVu0/Atan2fPoly) passes hitEps, scale the
 * hit delta (func_001290E0/Vec4ScaleVu0) into pos. (3) Unless bit1 is set, iterate
 * up to 6x: CollSphere(radius) at pos, and on a hit inside hitEps snap pos to
 * g_collHitPointNudged and step z by -(stepZ+radius). (4) Re-probe + mirror the
 * step-1 clamp, then write pos.xy back into ref.
 *
 * Helper sigs (recovered): f32 func_002A9888(Vec4 *pos) [surface sample, UNCONFIRMED];
 * void func_001290E0(void *dst, void *src) [hit-delta build, UNCONFIRMED];
 * f32 Vec2LengthXyVu0(Vec4 *v) [planar magnitude]; f32 Atan2fPoly(f32,f32) [atan2];
 * Vec4ScaleVu0(dst, scale, src). Delay-slot notes: the bnel @0x2A8F14 and the two
 * bc1fl @0x2A8F98/0x2A9030 are LIKELY (delay runs only when taken) — the sphere
 * loop's i++ lives in a bc1fl delay slot.
 *
 * ORACLE CAVEAT: engine region (no byte-match) AND VU0-microprogram-dependent
 * (Vec3RescaleToLenVu0/etc), so — like func_002AA058 — it is NOT ULP/bit-oracleable
 * in the headless cmp harness. The #else below is FAITHFUL-STRUCTURE (traced
 * store-for-store from the .s) but NOT oracle-verified; bit-trust needs the
 * tester full-game effect-diff. Sibling func_002A90A8 is the same class. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 44.54%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-224` vs `addiu sp,sp,-240` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8D08);
#else
extern f32 func_002A9888(Vec4 *pos);            /* surface-height sample (defined below) */
extern void func_001290E0(void *dst, void *src);  /* 0x1290E0 hit-delta build (UNCONFIRMED) */

/* Shared by steps 1 and 4: if the sampled surface is more than snapEps off ref->z,
 * snap pos to ref. The direction test is gated by flags bit0 (one-sided when set,
 * either-side when clear). Returns 1 if it snapped. Mirrors the inlined asm clamp. */
static s32 SnapPosToRefByZ(Vec4 *pos, Vec4 *ref, f32 surface, f32 snapEps, s32 flags) {
    if (snapEps < GetFloatAbs(surface - ref->z)) {
        s32 snap;
        if (flags & 1) {
            snap = (ref->z < surface);
        } else if (surface < ref->z) {
            snap = 1;
        } else {
            snap = (ref->z < surface);
        }
        if (snap) {
            *pos = *ref;
            return 1;
        }
    }
    return 0;
}

s32 func_002A8D08(void *ent, Vec4 *ref, Vec4 *pos, s32 flags,
                  f32 stepZ, f32 radius, f32 snapEps, f32 hitEps) {
    u8  *cw = (u8 *)g_pCollWorldData;   /* collision-result block base */
    Vec4 diff;        /* pos - ref (sweep vector + hit accumulator) */
    Vec4 stepDir;     /* diff rescaled to radius*1.2 */
    Vec4 origin;      /* lifted line origin (ref + up) */
    Vec4 endpoint;    /* line endpoint (origin + stepDir) */
    Vec4 up;          /* (0, 0, stepZ, 0) */
    Vec4 sphereFrom;  /* per-iteration sphere query point */
    f32  surface;
    s32  ret = 1;
    s32  i;

    /* Step 1: surface-probe just above pos, then optional z-snap to ref. */
    pos->z += stepZ;
    surface = func_002A9888(pos);
    pos->z -= stepZ;
    if (SnapPosToRefByZ(pos, ref, surface, snapEps, flags)) {
        ret = 0;
    }

    /* Build the rescaled sweep direction and a lifted line from ref. */
    Vec4SubVu0(&diff, pos, ref);
    Vec3RescaleToLenVu0(&stepDir, radius * 1.2f, &diff);
    up.x = 0.0f; up.y = 0.0f; up.z = stepZ; up.w = 0.0f;
    Vec4AddVu0(&origin, ref, &up);
    Vec4AddVu0(&endpoint, &origin, &stepDir);

    /* Step 2: unless step 1 already snapped, line-cast along the sweep and, on a
     * hit whose planar angle passes hitEps and whose cross term is positive, fold
     * the scaled hit delta into pos. */
    if (ret != 0) {
        if (CollLine(&origin, &endpoint, (flags & 2) | 0x24, ent, (void *)0)) {
            if (*(s32 *)(cw + 0x1C) > 0) {
                f32 ang = Atan2fPoly(*(f32 *)(cw + 0x48),
                                        Vec2LengthXyVu0((Vec4 *)(cw + 0x40)));
                if (hitEps <= ang) {
                    f32 det;
                    *(s32 *)(cw + 0x48) = 0;
                    func_001290E0((Vec4 *)(cw + 0x40), (Vec4 *)(cw + 0x40));
                    det = -diff.x * *(f32 *)(cw + 0x40)
                        -  diff.y * *(f32 *)(cw + 0x44);
                    if (0.0f < det) {
                        Vec4ScaleVu0((Vec4 *)(cw + 0x40), det, (Vec4 *)(cw + 0x40));
                        ret = 0;
                        Vec4AddVu0(&diff, &diff, (Vec4 *)(cw + 0x40));
                    }
                }
                Vec4AddVu0(pos, ref, &diff);   /* pos = ref + accumulated diff */
            }
        }
    }

    /* Step 3: unless bit1 is set, up to 6 sphere nudges toward the pushout point. */
    if ((flags & 2) == 0) {
        f32 dropZ = stepZ + radius;
        for (i = 0; i < 6; i++) {
            f32 ang;
            sphereFrom = *pos;
            sphereFrom.z += dropZ;
            if (CollSphere(&sphereFrom, 0x24, ent, radius) == 0) {
                break;
            }
            ang = Atan2fPoly(DistXYVu0(pos, &g_collHitPoint),
                                g_collHitPoint.z - pos->z);
            if (*(s32 *)(cw + 0x18) == 0 && !(hitEps < ang)) {
                continue;   /* grazing hit: keep scanning without snapping */
            }
            *pos = g_collHitPointNudged;
            ret = 0;
            pos->z -= dropZ;
        }
    }

    /* Step 4: re-probe the surface and mirror the step-1 clamp. */
    pos->z += stepZ;
    surface = func_002A9888(pos);
    pos->z -= stepZ;
    if (SnapPosToRefByZ(pos, ref, surface, snapEps, flags)) {
        ret = 0;
    }

    /* Write the resolved xy back into ref. */
    ref->x = pos->x;
    ref->y = pos->y;
    return ret;
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A90A0);

/* func_002A90A8 — directional collision sweep of a moby, accumulating hit flags
 * (MODEL, UNCONFIRMED). Same VU0-collision class as func_002A8D08; kept
 * INCLUDE_ASM pending a runtime oracle (see that function's caveat).
 *
 *   s32 func_002A90A8(void *moby, Vec4 *dir, s32 mask,
 *                     f32 stepZ, f32 minLen, f32 sphereZOfs)
 *     moby  ($4)  entity; +0x10 = Vec4 probe point, +0x18 = its z
 *     dir   ($5)  sweep direction Vec4 (added to the probe point; length-tested)
 *     mask  ($6)  collision mask: <<1 then bit 0x20 kept, OR'd with 0x2 (line) /
 *                 0x4 (sphere) per phase for the Coll* calls
 *     stepZ ($f12) z advance per phase   minLen ($f13) min sweep length + rescale len
 *     sphereZOfs ($f14) z offset applied after the sphere loop
 *   returns ($23): accumulated hit-flag bitmask (bit0 sphere-hit, bit1 line-hit,
 *                  bit2 grazing/angle flag — OR'd as each phase reports).
 *
 * Flow: probe = moby+0x10 + dir; if |dir| > minLen, rescale dir to (minLen) and
 * CollLine along it (on hit: step z by -stepZ, flag). Then up to 6x: CollSphere
 * at the probe, snapping to g_collHitPointNudged and stepping z by -stepZ on each
 * hit. Then a final CollLine; on a hit passing the Vec2LengthXyVu0/Atan2fPoly
 * planar-angle test (>0.5) and z test, nudge z by 0.3*overshoot. Ends with a
 * Vec4SubVu0 and returns the flag word. Likely branch: bc1tl @0x2A92B8 (its
 * `ori $23,0x4` delay slot runs only when taken). g_collHitPoint /
 * g_collHitPointNudged / g_pCollWorldData as in func_002A8D08.
 *
 * ORACLE CAVEAT: VU0-microprogram-dependent, not headless ULP-oracleable — the
 * #else below is FAITHFUL-STRUCTURE (traced store-for-store from the .s) but NOT
 * oracle-verified; bit-trust needs the tester full-game effect-diff. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 34.31%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-192` vs `addiu sp,sp,-176` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A90A8);
#else
s32 func_002A90A8(void *moby, Vec4 *dir, s32 mask, f32 stepZ, f32 minLen, f32 sphereZOfs) {
    Vec4 *probe = (Vec4 *)((u8 *)moby + 0x10);  /* moby +0x10 = current probe point */
    u8  *cw     = (u8 *)g_pCollWorldData;        /* collision-result block base */
    s32  m20    = (mask << 1) & 0x20;            /* base collision mask (mask<<1, bit 0x20) */
    Vec4 orig;                                   /* probe point before this sweep */
    Vec4 from, to, rescaled;
    s32  flags = 0;
    s32  i;

    orig = *probe;
    Vec4AddVu0(probe, probe, dir);               /* advance probe by dir */

    /* Phase 1: if the sweep is long enough, line-probe along dir. */
    if (minLen < Vec3LengthVu0(dir)) {
        from = orig;
        from.z += stepZ;
        Vec3RescaleToLenVu0(&rescaled, minLen, dir);
        Vec4AddVu0(&to, &rescaled, probe);
        to.z += stepZ;
        if (CollLine(&from, &to, m20, moby, (void *)0)) {
            Vec4SubVu0(probe, &g_collHitPoint, &rescaled);
            flags = 1;
            probe->z -= stepZ;
        }
    }

    /* Phase 2: up to 6 sphere pushes, snapping to the nudged hit point each time. */
    for (i = 0; i < 6; i++) {
        from = *probe;
        from.z += stepZ;
        if (CollSphere(&from, m20 | 0x4, moby, minLen) == 0) {
            break;
        }
        *probe = g_collHitPointNudged;
        flags |= 1;
        probe->z -= stepZ;
    }

    /* Phase 3: final downward line probe; flag + optional z nudge toward the hit. */
    {
        f32 lowZ = probe->z - sphereZOfs;
        from = *probe;
        to   = *probe;
        from.z += stepZ;
        to.z    = lowZ - 0.05f;
        if (CollLine(&from, &to, m20 | 0x2, moby, (void *)0)) {
            if (*(s32 *)(cw + 0x1C) > 0) {         /* hit-count field */
                flags |= 2;
                if (0.5f < Atan2fPoly(*(f32 *)(cw + 0x48),
                                         Vec2LengthXyVu0((Vec4 *)(cw + 0x40)))) {
                    flags |= 4;                    /* grazing / angle flag */
                }
                if (lowZ < *(f32 *)(cw + 0x28)) {
                    probe->z += 0.3f * (*(f32 *)(cw + 0x28) - lowZ);
                }
            }
        }
    }

    Vec4SubVu0(dir, probe, &orig);               /* dir <- net displacement */
    return flags;
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9348);

/**
 * func_002A9370 — count a group's active mobys, optionally filtered by state.
 *
 * Params: group — moby group index; -1 returns 0.
 *         state — -1 counts every active moby (state byte >= 0); otherwise
 *         only the active mobys whose state byte DIFFERS from `state` are
 *         counted (FACT #5781: the plate's "equals" has the polarity inverted).
 * Returns the count; 0 for group -1 or an empty (NULL) group list. The group
 * list is a run of u16 slot indices whose last entry has bit 15 set.
 *
 * MATCHED on the sdk29 arm (plain C; unit objdiff report via objdiff_build.sh +
 * unit_report.sh, 100.00%; task #758). The bracket is the row with ONLY that
 * lever reverted, everything else as written (same instrument, task #758):
 *  - one `||` condition feeding a single increment; nested if / else-if lets
 *    cc1 if-convert the second test into a movn [85.29];
 *  - the loaded list head copied into `list` (the ROM loads it into v0 and
 *    moves it to a0 after the table-base load) [91.29 walking head itself];
 *  - the entry read into `raw` for the index and copied into `entry` for the
 *    end-of-list test AFTER the moby address is formed, so the two stay in
 *    separate registers (v0/a2) as in the ROM [95.86 with the copy first];
 *  - the moby address summed as integers: pointer + int is canonicalised
 *    base-first, and the ROM's addu has the shifted index first [99.71];
 *  - the head bound to $2 (empty on native): cc1 otherwise loads it into the
 *    dying address temp v1 [99.57].
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 54.29%
   -> UNKNOWN-@0: ROM `addiu v0,zero,-1` vs `addiu v1,zero,-1` (measured on the earlier body) */
#ifndef TARGET_NATIVE
#define A9370_IN_V0 __asm__("$2")
#else
#define A9370_IN_V0
#endif
s32 func_002A9370(s32 group, s32 state) {
    register u16 *head A9370_IN_V0;
    u16 *list;
    s32 count;
    u16 entry;

    if (group == -1) {
        return 0;
    }
    head = g_mobyGroupLists[group];
    count = 0;
    if (head == 0) {
        return 0;
    }
    list = head;
    do {
        Moby *moby;
        u16 raw = *list;

        moby = (Moby *)((raw & 0x7FFF) * 0x100 + (u32)g_mobyTableBase);
        entry = raw;
        if (moby->state >= 0 && (state == -1 || (u8)moby->state != state)) {
            count = count + 1;
        }
        list = list + 1;
    } while ((s16)entry >= 0);
    return count;
}
#undef A9370_IN_V0

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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9450);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9458);

/**
 * Begin iterating a moby group: validate the group index, point the iterator
 * globals at the group's slot list, fetch the first moby into *out, then apply
 * the active/inactive filter (wantInactive/wantActive) before chaining to the
 * iterator-advance helper. Returns -1 for an empty/invalid group, 0 when the
 * first moby is filtered out, else the advance helper's result.
 */
/* TODO(match): functional equivalent - 85%. The filter lattice and the
   size-12 %hi/%lo-vs-%gp_rel reload of g_pMobyGroupIterMoby reproduce, but the
   later cc1 schedules the wantActive-path reload straight-line (absolute
   lui/lw) with the $ra restore in the branch delay slot, while the pinned cc1
   fills that delay slot with the reload itself (forced %gp_rel form) and folds
   the two return paths - register-coloring + delay-slot wall (same family as
   func_002AC9E0). Revisit once the gameplay-TU compiler is available. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 54.83%
   -> UNKNOWN-@1: ROM `daddu t0,a0,zero` vs `(none)` */
#ifndef TARGET_NATIVE
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
    {
        s32 r = func_002A9550(out, g_pMobyGroupIterMoby, wantInactive, wantActive);
        __asm__ __volatile__(""); /* cc1 2.96 sibling-call suppression (the ROM never sibcalls) */
        return r;
    }
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 36.93%
   -> UNKNOWN-@0: ROM `daddu t2,a0,zero` vs `sw zero,0(a0)` */
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A96B8);

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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9700);

extern f32 func_002835C0(f32 x); /* sqrtf */

/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 73.05%
   -> UNKNOWN-@3: ROM `swc1 $f21,40(sp)` vs `swc1 $f21,32(sp)` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9708);
#else
/**
 * Solve the quadratic a*x^2 + b*x + c = 0 for real roots.
 *
 * Returns the number of real roots and writes them to *out1 / *out2:
 *   2  two distinct real roots (discriminant > 0), ordered so *out1 >= *out2
 *   1  a single repeated root -b/(2a) (discriminant == 0), written to *out1
 *   0  no real roots (discriminant < 0); *out1 / *out2 still receive the
 *      formal values (-b +/- sqrt(|disc|)) / (2a) that the caller may ignore
 */
s32 func_002A9708(f32 a, f32 b, f32 c, f32 *out1, f32 *out2)
{
    f32 disc = b * b - a * (c * 4.0f);

    if (disc == 0.0f) {
        *out1 = -b / (a + a);
        return 1;
    } else {
        f32 root = func_002835C0(GetFloatAbs(disc)); /* sqrt(|disc|) */
        f32 hi = (-b + root) / (a + a);
        f32 lo = (-b - root) / (a + a);

        *out1 = hi;
        *out2 = lo;
        if (*out1 < lo) {
            *out2 = *out1;
            *out1 = lo;
        }
        if (disc > 0.0f) {
            return 2;
        }
        return 0;
    }
}
#endif

/* ProbeGroundHeight: ground height under a point - CollLine from z=0.01 up
 * to pos.z + zOffset, returns the hit z or 0 (the 0.5f/0x20 defaults come
 * from func_002A9888). Best attempt 72% (volatile 128-bit copy pinning +
 * an asm scheduling barrier recover the copy/const order): the pinned cc1
 * still hoists the call-argument moves above the second source copy where
 * the later cc1 keeps them below - prologue-scheduling wall. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 33.52%
   -> UNKNOWN-@1: ROM `daddu v1,a0,zero` vs `(none)` */
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A98B0);

/* func_002A98B8: first polygon edge (0x10-stride xy verts) the point lies
 * left of, 1-based; 0 = inside. Best attempt 90%: the original emits the
 * div-by-zero check of the i%%n twice (one hoisted to the loop top) around
 * a single CSEd div - the div-expansion-scheduling wall (same family as
 * func_00351328). */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 44.77%
   -> UNKNOWN-@0: ROM `(none)` vs `lwc1 $f7,0(a0)` */
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

/* func_002A9958(point, poly, count): even-odd point-in-polygon test in the XY
 * plane. For each polygon edge (adjacent vertices poly[i], poly[(i+1)%count])
 * that straddles the horizontal line y == point->y, it computes the edge's X
 * intersection with that line and toggles an inside flag when the crossing lies
 * left of point->x. Returns 1 if the point is inside (odd crossings), else 0.
 * Vertices are 16-byte Vec4 records; only .x/.y participate. Pure leaf.
 *
 * Matching build stays INCLUDE_ASM: the branch-likely toggle idioms
 * (bc1tl/bc1fl nullified delay slots) around the crossing test are an
 * engine-2.96 schedule this C won't reproduce. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 54.12%
   -> UNKNOWN-@0: ROM `daddu t0,zero,zero` vs `daddu t2,zero,zero` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9958);
#else
s32 func_002A9958(Vec4 *point, Vec4 *poly, s32 count) {
    s32 inside = 0;
    s32 i;

    if (count <= 0) {
        return 0;
    }
    for (i = 0; i < count; i++) {
        Vec4 *vi = &poly[i];
        Vec4 *vj = &poly[(i + 1 == count) ? 0 : i + 1];
        s32 crosses;

        if (vi->y < point->y) {
            crosses = point->y <= vj->y;
        } else {
            crosses = vj->y < point->y && point->y <= vi->y;
        }
        if (crosses) {
            f32 t  = (point->y - vi->y) / (vj->y - vi->y);
            f32 ix = vi->x + t * (vj->x - vi->x);
            if (ix < point->x) {
                inside = !inside;
            }
        }
    }
    return inside;
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9A28);

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

/*
 * func_002A9A68(packet, a, b, power, dir): fill a 0x28-byte damage packet
 * WITH a direction — the directional twin of func_002A9A38 (FACT #5761).
 *   packet +0x10 = a, +0x14 = b (s32 parameters), +0x1C = power (f32),
 *   +0x20 = 1 (hasDirection), +0x00..0x0F = the 128-bit direction vector
 *   copied from *dir with one lq/sq pair.
 * No return value.
 *
 * Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_002A9A68,
 * task #632): unit objdiff 100.00% (objdiff_build.sh + unit_report.sh). The
 * ROM leaves the return's delay slot EMPTY after the sq (`sq; jr $31; nop`).
 * Two things reproduce that, and both are needed:
 *   - the trailing empty asm stops cc1 2.96's reorg from filling the return
 *     slot with the sq itself (without it: `jr $31; sq` in noreorder);
 *   - asm_unit.sh's lq/sq return-slot pin stops GNU as from then swapping the
 *     sq into the reorder-mode `j $31` the way SN ee-as never did.
 * The INCLUDE_ASM below still feeds the 2.9 link in build.sh, which defines
 * no MATCH_.
 */
#if defined(MATCH_func_002A9A68) || defined(TARGET_NATIVE)
void func_002A9A68(void *packet, s32 a, s32 b, f32 power, void *dir) {
    *(s32 *)((u8 *)packet + 0x10) = a;
    *(s32 *)((u8 *)packet + 0x14) = b;
    *(f32 *)((u8 *)packet + 0x1C) = power;
    *(s32 *)((u8 *)packet + 0x20) = 1;              /* hasDirection */
    *(u_long128 *)packet = *(u_long128 *)dir;
    __asm__ __volatile__("");
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9A68);
#endif

/* Ghidra alias CollMobysSphere / QueryMobysInSphere @ 0x00277F58: moby-only
 * sphere gather + damage-event broadcast into g_collHitEventRing (see
 * collision.h). Returns the hit count (list in g_collMobyHitList). */
extern s32 CollMobysSphere(void *targetList, void *filter, Moby *self,
                           void *hitEvent, f32 radius);

/*
 * func_002A9A90: issue a directional moby sphere-collision / damage query.
 *
 * Builds a spread direction from the moby's orientation — for an oriented moby
 * (modeBits & 0x100) its motion vector (+0xE0) rescaled to 0.25 then offset by
 * the matrix column at +0xC0; otherwise a unit heading from the facing yaw
 * (cos, sin, z=1). The direction is scaled by `scale` and stamped with a fixed
 * .w magnitude (0x45AFDF66), then folded into a hit-event record together with
 * the source moby, class id, the two byte tags (arg4/arg5) and the impact
 * `power`. CollMobysSphere then gathers/broadcasts against that record within
 * `radius` and returns the hit count.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 75.22%
   -> UNKNOWN-@2: ROM `(none)` vs `daddu s1,a0,zero` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9A90);
#else
s32 func_002A9A90(Moby *self, void *targetList, s32 arg3, s32 arg4, s32 arg5,
                  void *filter, f32 radius, f32 power, f32 scale) {
    Vec4 dir;
    u8   hitEvent[0x30];

    if (self->modeBits & 0x100) {
        Vec3RescaleToLenVu0(&dir, 0.25f, (Vec4 *)((u8 *)self + 0xE0));
        Vec4AddVu0(&dir, &dir, (Vec4 *)((u8 *)self + 0xC0));
    } else {
        dir.x = func_00283B30(self->facingAngle);   /* cos */
        dir.y = func_00283B48(self->facingAngle);   /* sin */
        dir.z = 1.0f;
    }
    Vec4ScaleVu0(&dir, scale, &dir);
    dir.w = 5627.9248f;   /* 0x45AFDF66 */

    func_002A9A68(hitEvent, (s32)self, arg3, power, &dir);
    *(u8  *)(hitEvent + 0x18) = (u8)arg4;
    *(u8  *)(hitEvent + 0x19) = (u8)arg5;
    *(u16 *)(hitEvent + 0x1A) = self->oClass;

    return CollMobysSphere(targetList, filter, self, hitEvent, radius);
}
#endif

/* func_002A0AF8 @ 0x002A0AF8 (text/1A00F0): computes a moby attach-point world
 * position — scales the local offset by the moby's radius (+0x2C), transforms it
 * by the orientation matrix (+0xC0) and adds the world position (+0x10) — writing
 * the resulting Vec4 to `outPoint`. */
extern void func_002A0AF8(Moby *self, s32 attachId, void *outPoint);

/*
 * func_002A9BD8: attach-point variant of the directional sphere query.
 *
 * Resolves a world-space query origin at moby attach point `attachId`
 * (func_002A0AF8), then runs the directional moby sphere-collision query
 * (func_002A9A90) from that origin — forwarding the class/tag params and the
 * radius/power/scale floats unchanged. Returns the collision hit count.
 * (func_002A9A90's second parameter is this computed origin point.)
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 61.71%
   -> UNKNOWN-@1: ROM `(none)` vs `sd s1,24(sp)` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9BD8);
#else
s32 func_002A9BD8(Moby *self, s32 attachId, s32 arg3, s32 arg4, s32 arg5,
                  void *filter, f32 radius, f32 power, f32 scale) {
    Vec4 origin;
    func_002A0AF8(self, attachId, &origin);
    return func_002A9A90(self, &origin, arg3, arg4, arg5, filter,
                         radius, power, scale);
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9C80);

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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 58.64%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-80` vs `addiu sp,sp,-64` */
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

/* PostMobyHitEvent: record a 0x40-byte hit/damage event for `moby` into the
 * 64-slot ring g_collHitEventRing (write cursor at g_pCollWorldData+0x14, wraps
 * mod 64). Dedupe: moby+0xA8 holds this moby's last event slot (0xFF = none); if
 * that slot still belongs to `moby` and the new hit is CLOSER (dist < stored
 * dist), just OR the new flags into the existing record and return — otherwise a
 * fresh slot is allocated, carrying the old flags forward when the prior record
 * was this moby's. Record layout: +0x00 vecA, +0x10 vecB, +0x20 a1, +0x24 flags,
 * +0x2C dist, +0x30 hasDir (|vecB| > 1e-4), +0x34 dist, +0x38 moby, +0x3C 0.
 * Return is incidental in the original (merge path leaves the merged flags, alloc
 * path leaves &g_pCollWorldData); callers discard it — we return the record flags. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 59.79%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-80` vs `addiu sp,sp,-64` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", PostMobyHitEvent);
#else
s32 PostMobyHitEvent(Moby *moby, s32 a1, s32 flags, Vec4 *vecA, Vec4 *vecB, f32 dist) {
    s32 ringCursor = *(s32 *)(g_pCollWorldData + 0x14);
    s32 prevIdx    = ((u8 *)moby)[0xA8];   /* this moby's last event slot, 0xFF = none */
    s32 extraFlags = 0;
    u8 *rec;

    /* Merge into this moby's existing event if the new hit is closer. */
    if (prevIdx != 0xFF) {
        u8 *prev = g_collHitEventRing + prevIdx * 0x40;
        if (*(Moby **)(prev + 0x38) == moby) {
            if (dist < *(f32 *)(prev + 0x2C)) {
                *(s32 *)(prev + 0x24) |= flags;
                return *(s32 *)(prev + 0x24);
            }
            extraFlags = *(s32 *)(prev + 0x24);   /* carry old flags into the new record */
        }
    }

    /* Allocate a fresh record at the ring write cursor. */
    rec = g_collHitEventRing + ringCursor * 0x40;
    *(s32 *)(rec + 0x24)   = flags | extraFlags;
    *(s32 *)(rec + 0x20)   = a1;
    *(f32 *)(rec + 0x34)   = dist;
    *(Moby **)(rec + 0x38) = moby;
    *(f32 *)(rec + 0x2C)   = dist;
    *(s32 *)(rec + 0x3C)   = 0;
    *(s32 *)(rec + 0x30)   = (0.0001f < Vec3LengthVu0(vecB)) ? 1 : 0; /* hasDir */
    *(Vec4 *)(rec + 0x00)  = *vecA;
    *(Vec4 *)(rec + 0x10)  = *vecB;
    ((u8 *)moby)[0xA8]     = (u8)ringCursor;
    *(s32 *)(g_pCollWorldData + 0x14) = (ringCursor + 1) & 0x3F;
    return *(s32 *)(rec + 0x24);
}
#endif

/**
 * Post a moby hit event with the default hit-direction vector (vecB = D_1A8BD0).
 * Recovered ABI: 5 args incl. f32 dist in $f12 (the old 4-arg (…,s32 c) form
 * mistyped vecA and dropped the float); matches 1B4218.c's caller.
 */
s32 func_002A9F30(Moby *moby, s32 a1, s32 flags, Vec4 *vecA, f32 dist) {
    return PostMobyHitEvent(moby, a1, flags, vecA, (Vec4 *)&D_1A8BD0, dist);
}

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9F58);

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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 65.23%
   -> UNKNOWN-@0: ROM `(none)` vs `lw v0,0(gp)  [GPREL16 0x001B2258]` */
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 63.54%
   -> UNKNOWN-@2: ROM `sd s1,56(sp)` vs `(none)` */
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 50.22%
   -> UNKNOWN-@2: ROM `swc1 $f21,40(sp)` vs `(none)` */
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 50.42%
   -> UNKNOWN-@0: ROM `(none)` vs `lw v0,0(gp)  [GPREL16 D_1A8CA0]` */
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
 * Forward to the shared 2-colour blend helper ColorLerpPacked.
 *
 * TYPE NOTE: a/b/c stay void* on purpose. ColorLerpPacked reads its GPR args by
 * VALUE via pextlb/pextlh (they ARE packed 32-bit RGBA colour words, not
 * pointers) and never dereferences them; c is unused by the callee. So these
 * are not Vec4* (the earlier "vector op" hypothesis is disproven by the asm).
 */
s32 func_002AA3D0(void *a, void *b, void *c) {
    return ColorLerpPacked(a, b, c);
}

/* Two free-running phase counters for the ping-pong colour ramp; the selector
 * arg picks which one to advance (gp-addressable small data). */
extern s32 D_1A9E94;
extern s32 D_1A9E98;

/* ColorLerpPacked as a packed-colour lerp that RETURNS the blended colour word:
 * t in $f12, the two colour words by value in $4/$5, result in $2. */
typedef u32 (*LerpColorPackedFn)(f32 t, u32 colorA, u32 colorB);

/**
 * PulseColorBlend (func_002AA3F0): advance a ping-pong colour pulse and return
 * the current blended colour. Bumps (or resets) one of two free-running phase
 * counters — counterSel picks D_1A9E94 (0) vs D_1A9E98 — folds it into a
 * triangle wave over [0, 2*period) giving a 0..1 blend factor, and returns
 * color1/color2 blended by that factor (ColorLerpPacked).
 *
 * SIGNATURE (verified from asm, corrects an earlier void ptr-copier guess):
 *   returns u32 (packed RGBA in $2); color1=$4, color2=$5 are packed colour
 *   words passed BY VALUE (ColorLerpPacked consumes them via pextlb/pextlh, no
 *   memory load); period=$6, counterSel=$7, reset=$8 are s32.
 *
 * reset is the 5th arg in $8 — that is normal EABI (-mabi=eabi passes integer
 * args 1-8 in $4-$11), NOT a custom ABI, so plain C compiles byte-exact and this
 * is a genuine match (no INCLUDE_ASM). The u32-return signature is the accurate
 * one: d2's 235FE8 callers consume the returned packed colour.
 */
u32 func_002AA3F0(u32 color1, u32 color2, s32 period, s32 counterSel, s32 reset) {
    s32 *phase = &D_1A9E98;
    s32 pos;
    f32 t;

    if (counterSel == 0) {
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
    return ((LerpColorPackedFn)ColorLerpPacked)(t, color2, color1);
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AA500);

/* func_002AE7E8 @ 0x002AE7E8: map a moby's class id (+0xAA) to an announcer /
 * sound-cue id via a class-value switch. Returns the id byte. */
extern s32 func_002AE7E8(Moby *moby);
/* pvar-block word readers (defined later in this unit): word 0 / word 4. */
extern s32 func_002AC058(Moby *moby);
extern s32 func_002AC088(Moby *moby);

/*
 * func_002AA508: advance the per-owner "focus target" announcer state and return
 * the emphasis weight for this frame.
 *
 * `owner`s pvar word 0 holds a small state block (`st`): current focus target
 * moby (+0x18), a hold/cooldown tick counter (+0x1C), the resolved announcer id
 * (+0x17) and a priority level (+0x04). Each call: (1) drops the focus target if
 * its moby has despawned (state 0xFE/0xFD); (2) advances the hold counter,
 * resetting target+counter after 0x79 ticks. Then, for the candidate event
 * `event`: latches a new focus target (resolving its announcer id) or, on the
 * same target, suppresses the emphasis weight (returns 0) while inside the
 * per-kind re-trigger window (kind 9 -> 0x3C ticks, else 0xA ticks). The weight
 * (event +0x2C) is clamped up to 1.0 for low-priority owners, and a companion
 * state flag (owners pvar word 4, +0x18) is toggled by event flag 0x100000.
 *
 * UNCONFIRMED: state/event field meanings inferred from access pattern; owner is
 * a Moby, target/candidate are Mobys (func_002AE7E8 reads their class).
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 90.11%
   -> UNKNOWN-@1: ROM `(none)` vs `sd s0,0(sp)` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AA508);
#else
f32 func_002AA508(Moby *owner, void *event) {
    u8 *st = (u8 *)func_002AC058(owner);   /* focus-target state (pvar word 0) */
    f32 weight = 0.0f;
    s16 counter;

    if (st == NULL) {
        return 0.0f;
    }

    /* drop the focus target if its moby has despawned (state 0xFE/0xFD) */
    {
        Moby *cur = *(Moby **)(st + 0x18);
        if (cur != NULL) {
            u8 state = *(u8 *)((u8 *)cur + 0x20);
            if (state == 0xFE || state == 0xFD) {
                *(s16 *)(st + 0x1C) = 0;
                *(s32 *)(st + 0x18) = 0;
            }
        }
    }

    /* advance the hold counter; expire target + counter after 0x79 ticks */
    counter = *(s16 *)(st + 0x1C);
    if (counter != 0) {
        *(s16 *)(st + 0x1C) = (s16)(*(u16 *)(st + 0x1C) + 1);
        if (counter >= 0x79) {
            *(s32 *)(st + 0x18) = 0;
            *(s16 *)(st + 0x1C) = 0;
        }
    }

    if (event == NULL) {
        return 0.0f;
    }

    weight = *(f32 *)((u8 *)event + 0x2C);
    {
        Moby *cand = *(Moby **)((u8 *)event + 0x20);
        if (cand != NULL) {
            if (*(Moby **)(st + 0x18) != cand) {
                /* new focus target: latch it + resolve its announcer id */
                *(s32 *)(st + 0x18) = (s32)cand;
                *(u8 *)(st + 0x17) = (u8)func_002AE7E8(cand);
                *(s16 *)(st + 0x1C) = 1;
            } else {
                /* same target: gate the re-trigger window by event kind */
                u8  kind = *(u8 *)((u8 *)event + 0x28);
                s16 c    = *(s16 *)(st + 0x1C);
                if (kind == 9) {
                    if (c < 0x3C) {
                        weight = 0.0f;
                    } else {
                        *(s16 *)(st + 0x1C) = 1;
                    }
                } else {
                    if (c < 0xA) {
                        weight = 0.0f;
                    }
                    *(s16 *)(st + 0x1C) = 1;
                }
            }
        }
    }

    /* low-priority owners get their emphasis floored to 1.0 */
    if ((f32)*(s16 *)(st + 0x4) <= 1.0f && weight > 0.0f && weight < 1.0f) {
        weight = 1.0f;
    }

    /* toggle the companion state flag (owner pvar word 4, +0x18) */
    if (*(s32 *)((u8 *)event + 0x24) & 0x100000) {
        u8 *st2 = (u8 *)func_002AC088(owner);
        if (st2 != NULL) {
            *(u8 *)(st2 + 0x18) = 1;
        }
    } else {
        u8 *st2 = (u8 *)func_002AC088(owner);
        if (st2 != NULL) {
            *(u8 *)(st2 + 0x18) = 0;
        }
    }

    return weight;
}
#endif

/*
 * func_002AA6B8: broadcast a per-target directional damage packet to a moby list.
 *
 * For each of the `count` mobys in `list` (skipping `skip`), computes the bearing
 * from the reference position `refPos` to that moby (Atan2fPoly on the xy delta),
 * builds a hit-event packet — a `magnitude`-scaled direction (cos/sin of the
 * bearing) in xy, `zComp` in z, the fixed 0x45AFDF66 .w magnitude, plus the
 * source moby / its class / the tag words (arg6) and bytes (arg7/arg8) / the
 * `power` float (fa0) — and posts it to that moby via PostMobyDamagePacket
 * (func_002A9C88). Same packet layout as func_002A9A90's sphere-query record.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 24.20%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-192` vs `addiu sp,sp,-176` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AA6B8);
#else
void func_002AA6B8(Moby *self, const Vec4 *refPos, Moby **list, s32 count,
                   Moby *skip, s32 arg6, s32 arg7, s32 arg8,
                   f32 power, f32 magnitude, f32 zComp) {
    Vec4 ref = *refPos;   /* 128-bit copy of the reference position */
    s32  i;

    for (i = 0; i < count; i++) {
        Moby *entry = list[i];
        u8   packet[0x30];
        f32  bearing;

        if (entry == skip) {
            continue;
        }
        bearing = Atan2fPoly(entry->pos.x - ref.x, entry->pos.y - ref.y);
        *(f32 *)(packet + 0x00) = func_00283B30(bearing) * magnitude;  /* cos */
        *(f32 *)(packet + 0x04) = func_00283B48(bearing) * magnitude;  /* sin */
        *(f32 *)(packet + 0x08) = zComp;
        *(f32 *)(packet + 0x0C) = 5627.9248f;   /* 0x45AFDF66 */
        *(s32 *)(packet + 0x10) = (s32)self;
        *(s32 *)(packet + 0x14) = arg6;
        *(u8  *)(packet + 0x18) = (u8)arg7;
        *(u8  *)(packet + 0x19) = (u8)arg8;
        *(u16 *)(packet + 0x1A) = self->oClass;
        *(f32 *)(packet + 0x1C) = power;
        *(s32 *)(packet + 0x20) = arg6;
        func_002A9C88(entry, packet);
    }
}
#endif

/**
 * func_002AA808 — scatter up to `maxPoints` random world points through a moby's
 * collision volume, weighted by each primitive's volume.
 *
 * Used by the FX spawners (sole caller func_002AC0B8) to distribute particles
 * evenly *inside* a moby's body rather than at its origin.
 *
 * Pass 1 walks the moby's collision primitive list (class header +0x24, the
 * primitive set at header+0x10; header word +0x2 = skin-mesh element count,
 * +0x4 = primitive count, records start at +0x10 with stride 0x20). Primitives
 * are kept only when bit `i` of `primMask` is set AND the record's flag word has
 * bit 0x20000 set; for each kept primitive it computes an approximate volume
 * into a weight table and accumulates the total. If the set carries a skinned
 * mesh it is first posed into the SPR cache (SkinMobyCollisionMesh) since the
 * type-2/4 records index scratchpad vectors at 0x70000000.
 *
 * The point count is then `min(maxPoints, (s32)(density * totalVolume + 0.5f))`,
 * i.e. `density` is points-per-unit-volume, not a radius. Returns that count
 * (0 when the moby has no primitive set / no primitives / a non-positive count).
 *
 * Pass 2 draws each point by roulette-wheel selection over the weight table
 * (a primitive is picked with probability proportional to its volume), then
 * samples a uniform point inside that primitive and writes it as a Vec4 into
 * `outPoints[i]`. Sphere-ish primitives (types 0/1/2/4) sample by rejection —
 * redraw a cube point until it lies inside the sphere — while the type-3
 * cylinder samples analytically in polar coordinates (sqrt for a uniform disc).
 *
 * NON-OBVIOUS: the primitive-list walk is terminated by the sign bit of each
 * record's flag word (last-record marker), NOT by the +0x4 count, and a
 * primitive whose type is outside 0..4 is still counted and still consumes a
 * weight slot — the slot is left holding whatever the previous iteration wrote
 * (a compiled `switch` with an empty default arm). Both are transcribed as-is.
 * NON-OBVIOUS: the roulette walk can land on index == acceptedCount when
 * floating-point drift makes the draw exceed the running sum; reproduced
 * verbatim so a tester sees the same (out-of-range) pick the ROM makes.
 *
 * Faithful coverage body — the matching build keeps the asm (save-layout wall).
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 33.92%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-416` vs `addiu sp,sp,-384` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AA808);
#else
extern void SkinMobyCollisionMesh(void *moby, s32 count, u32 flags);
/* dst = src / d on x,y,z, dst.w = src.w (VU0 vdiv Q; canonical def in 183558.c). */
extern void func_00283740(Vec4 *dst, f32 d, const Vec4 *src);
/* dst = src * scale on ALL FOUR lanes (alternate entry inside func_00129120,
 * cod/015180: lqc2/vmulx.xyzw/sqc2). Scale arrives in $f12. */
extern void func_00129148(Vec4 *dst, const Vec4 *src, f32 scale);
/* out = m * v (3x3 rotate; canonical def in text/183558.c). */
extern void func_00283A48(Vec4 *out, Vec4 *v, Vec4 *m);

/* Collision-primitive record (stride 0x20) as this function reads it. Only the
 * lanes each primitive type actually uses are named; the rest stays raw. */
typedef struct CollPrim {
    /* 0x00 */ u32 flags;      /* low byte = type; bit 0x20000 = spawnable;
                                  sign bit = last record in the list */
    /* 0x04 */ union {
                   s32 sprIndex;    /* type 2: scratchpad vector index      */
                   f32 height;      /* type 3: cylinder height (1/1024 u)   */
                   s16 endIndex[2]; /* type 4: scratchpad indices of the
                                       capsule's two endpoints             */
               } a;
    /* 0x08 */ u32 unk08;
    /* 0x0C */ f32 radius;     /* types 2/4 only (1/1024 units) */
    /* 0x10 */ Vec4 centre;    /* local centre; .w = radius for types 0/1/3 */
} CollPrim;

/* Scratchpad (SPR) vector array the skinned collision mesh is posed into. */
#define COLL_SPR_VECS  ((const Vec4 *)0x70000000)

/* Exact ROM encodings — 0x40860A92 is 4*pi/3 but 0x408601FE is a DISTINCT
 * constant ~0.99975 of it, so neither may be re-derived from a decimal literal
 * (see the unit's float-constant rule). 0x3FC90FDB is pi/2. */
static const union { u32 u; f32 f; } kSphereVolFactor  = { 0x40860A92 };
static const union { u32 u; f32 f; } kCapsuleVolFactor = { 0x408601FE };
static const union { u32 u; f32 f; } kHalfPi           = { 0x3FC90FDB };

/* Collision primitive units are 1/1024 of a moby-space unit.
 * The ROM holds these as TWO distinct constants doing two different jobs, and the C
 * keeps them apart for that reason: 0x3A800000 is loaded into a callee-saved FP
 * register and MULTIPLIED by, while 0x44800000 is passed to the divide helper
 * (func_00283740). Numerically 1/1024 either way - an exact power of two - but they
 * are separate values in the binary.
 * Spelled as a single literal rather than `1.0f / 1024.0f`: a compile-time division
 * is two literals to a reader and to the float screen, which reported 0x3A800000 as
 * an unaccounted ROM constant until this was written as one number. It also matches
 * the spelling this unit already uses at its other two 1/1024 sites. */
#define COLL_UNIT_SCALE   0.0009765625f   /* 0x3A800000 */
#define COLL_UNITS_PER_M  1024.0f         /* 0x44800000 */

/* Draw a uniform point inside the sphere of radius `radius` by rejection. */
static void ScatterPointInSphere(Vec4 *out, f32 radius) {
    do {
        out->x = GetRandomFloatRange(-radius, radius);
        out->y = GetRandomFloatRange(-radius, radius);
        out->z = GetRandomFloatRange(-radius, radius);
    } while (!(Vec3LengthVu0(out) <= radius));
}

s32 func_002AA808(Moby *moby, s32 maxPoints, Vec4 *outPoints, s32 primMask,
                  f32 density) {
    u8 *primSet = *(u8 **)((u8 *)*(u8 **)((u8 *)moby + 0x24) + 0x10);
    f32 mobyScale;
    f32 weights[20];
    CollPrim *picked[20];
    Vec4 tmpA;
    Vec4 tmpB;
    Vec4 tmpC;
    Vec4 tmpD;
    Vec4 sample;
    CollPrim *prim;
    s32 acceptedCount;
    s32 primIndex;
    f32 totalVolume;
    s32 pointCount;
    s32 i;

    if (primSet == 0) {
        return 0;
    }
    /* Pose the skinned collision mesh into the SPR cache — types 2 and 4 index
     * it through COLL_SPR_VECS below. */
    if (*(s16 *)(primSet + 2) != 0) {
        SkinMobyCollisionMesh(moby, *(s16 *)(primSet + 2), 0);
    }
    if (*(s32 *)(primSet + 4) <= 0) {
        return 0;
    }

    mobyScale = *(f32 *)((u8 *)moby + 0x2C);
    prim = (CollPrim *)(primSet + 0x10);
    totalVolume = 0.0f;
    acceptedCount = 0;
    primIndex = 0;

    /* Pass 1 — per-primitive volume weights. */
    do {
        u32 flags = prim->flags;
        s32 type = (s32)(flags & 0xFF);

        if (((primMask >> primIndex) & 1) != 0 && (flags & 0x20000) != 0) {
            f32 radius;

            picked[acceptedCount] = prim;
            switch (type) {
            case 0:
            case 1:
                /* Sphere: centre.w carries the radius in collision units. */
                tmpA = prim->centre;
                ScaleVec4IncludingW(&tmpA, mobyScale * COLL_UNIT_SCALE, &tmpA);
                radius = tmpA.w;
                weights[acceptedCount] =
                    radius * (radius * (radius * kSphereVolFactor.f));
                break;

            case 2:
                /* Sphere attached to a skinned vertex: centre is an offset from
                 * the posed scratchpad vector, radius lives at +0xC. */
                tmpA = COLL_SPR_VECS[prim->a.sprIndex];
                tmpB = prim->centre;
                Vec4AddVu0(&tmpA, &tmpA, &tmpB);
                Vec4ScaleVu0(&tmpA, mobyScale * COLL_UNIT_SCALE, &tmpA);
                radius = prim->radius * mobyScale * COLL_UNIT_SCALE;
                weights[acceptedCount] =
                    radius * (radius * (radius * kSphereVolFactor.f));
                break;

            case 3: {
                /* Cylinder: centre.w = radius, +0x4 = height. */
                f32 height;

                tmpA = prim->centre;
                Vec4ScaleVu0(&tmpA, mobyScale, &tmpA);
                radius = tmpA.w * mobyScale * COLL_UNIT_SCALE;
                height = prim->a.height * COLL_UNIT_SCALE * mobyScale;
                weights[acceptedCount] =
                    radius * (radius * (radius * kCapsuleVolFactor.f)) +
                    radius * (radius * kHalfPi.f) * height;
                break;
            }

            case 4: {
                /* Capsule between two posed scratchpad vertices. */
                f32 length;

                func_00129148(&tmpA, &COLL_SPR_VECS[prim->a.endIndex[0]],
                            mobyScale * COLL_UNIT_SCALE);
                func_00129148(&tmpB, &COLL_SPR_VECS[prim->a.endIndex[1]],
                            mobyScale * COLL_UNIT_SCALE);
                Vec4SubVu0(&tmpC, &tmpA, &tmpB);
                length = Vec3LengthVu0(&tmpC);
                radius = prim->radius * mobyScale * COLL_UNIT_SCALE;
                weights[acceptedCount] =
                    radius * (radius * kHalfPi.f) * length;
                break;
            }

            default:
                /* No weight written — the slot keeps its previous contents and
                 * the primitive is still counted (verbatim from the asm). */
                break;
            }
            totalVolume += weights[acceptedCount];
            acceptedCount++;
        }
        primIndex++;
        prim = (CollPrim *)((u8 *)prim + 0x20);
    } while ((((CollPrim *)((u8 *)prim - 0x20))->flags >> 31) == 0);

    pointCount = (s32)(density * totalVolume + 0.5f);
    if (maxPoints < pointCount) {
        pointCount = maxPoints;
    }
    if (pointCount <= 0) {
        return pointCount;
    }

    /* Pass 2 — one scattered point per output slot. */
    for (i = 0; i < pointCount; i++) {
        f32 draw = GetRandomFloatRange(0.0f, totalVolume);
        CollPrim *sel;
        s32 k = 0;
        f32 radius;

        while (k < acceptedCount && !(draw < weights[k])) {
            draw -= weights[k];
            k++;
        }
        sel = picked[k];

        switch ((s32)(sel->flags & 0xFF)) {
        case 0:
        case 1:
            /* Sphere at the primitive's local centre. */
            tmpA = sel->centre;
            Vec4ScaleVu0(&tmpA, mobyScale, &tmpA);
            radius = tmpA.w * mobyScale * COLL_UNIT_SCALE;
            func_00283740(&tmpA, COLL_UNITS_PER_M, &tmpA);
            ScatterPointInSphere(&tmpB, radius);
            Vec4AddVu0(&tmpB, &tmpB, &tmpA);
            func_00283A48(&tmpB, &tmpB, (Vec4 *)((u8 *)moby + 0xC0));
            Vec4AddVu0(&outPoints[i], &tmpB, (Vec4 *)((u8 *)moby + 0x10));
            break;

        case 2:
            /* Sphere offset from a posed scratchpad vertex. */
            tmpA = COLL_SPR_VECS[sel->a.sprIndex];
            tmpC = sel->centre;
            Vec4AddVu0(&tmpA, &tmpA, &tmpC);
            Vec4ScaleVu0(&tmpA, mobyScale, &tmpA);
            radius = sel->radius * mobyScale * COLL_UNIT_SCALE;
            func_00283740(&tmpA, COLL_UNITS_PER_M, &tmpA);
            ScatterPointInSphere(&tmpC, radius);
            Vec4AddVu0(&tmpC, &tmpC, &tmpA);
            func_00283A48(&tmpC, &tmpC, (Vec4 *)((u8 *)moby + 0xC0));
            Vec4AddVu0(&outPoints[i], &tmpC, (Vec4 *)((u8 *)moby + 0x10));
            break;

        case 3: {
            /* Cylinder: polar sample (sqrt of a uniform r^2 gives a uniform
             * disc) plus a uniform height offset. NOTE this arm applies the
             * moby's translation but NOT its rotation matrix — as in the ROM. */
            f32 height;
            f32 angle;
            f32 sampleRadius;

            tmpA = sel->centre;
            Vec4ScaleVu0(&tmpA, mobyScale, &tmpA);
            radius = tmpA.w * mobyScale * COLL_UNIT_SCALE;
            height = sel->a.height * mobyScale * COLL_UNIT_SCALE;
            func_00283740(&tmpA, COLL_UNITS_PER_M, &tmpA);

            sampleRadius = func_002835C0(
                GetRandomFloatRange(0.0f, radius * radius));
            tmpB.z = GetRandomFloatRange(0.0f, height);
            angle = GetRandomAngle();
            tmpB.x = func_00283B30(angle) * sampleRadius;
            tmpB.y = func_00283B48(angle) * sampleRadius;
            Vec4AddVu0(&tmpB, &tmpB, (Vec4 *)((u8 *)moby + 0x10));
            Vec4AddVu0(&outPoints[i], &tmpA, &tmpB);
            break;
        }

        case 4: {
            /* Capsule: pick a uniform point along the axis, then a uniform
             * point in the sphere of the capsule's radius around it. */
            f32 t;

            func_00129148(&tmpA, &COLL_SPR_VECS[sel->a.endIndex[0]], mobyScale);
            func_00129148(&tmpB, &COLL_SPR_VECS[sel->a.endIndex[1]], mobyScale);
            Vec4SubVu0(&tmpC, &tmpB, &tmpA);
            t = GetRandomFloatRange(0.0f, 1.0f);
            Vec4ScaleVu0(&tmpC, t, &tmpC);
            Vec4AddVu0(&tmpD, &tmpC, &tmpA);
            func_00283740(&tmpD, COLL_UNITS_PER_M, &tmpD);
            radius = sel->radius * mobyScale * COLL_UNIT_SCALE;

            ScatterPointInSphere(&sample, radius);
            Vec4AddVu0(&sample, &sample, &tmpD);
            func_00283A48(&sample, &sample, (Vec4 *)((u8 *)moby + 0xC0));
            Vec4AddVu0(&outPoints[i], &sample, (Vec4 *)((u8 *)moby + 0x10));
            break;
        }

        default:
            /* Slot left untouched (verbatim from the asm). */
            break;
        }
    }
    return pointCount;
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AAF98);

/**
 * Linear interpolation: a + (b - a) * t.
 */
f32 func_002AAFA8(f32 a, f32 b, f32 t) {
    return a + (b - a) * t;
}

/**
 * Two-stage scalar transform: feed (b, a) through WrapAnglePiDiff, scale the
 * result by c, and forward (a, scaled) to WrapAnglePiSum.
 */
/* Return type guarded like Vec2LengthXyVu0: the matching build byte-matches ONLY
 * with the s32 form (the s32 callee/return type-errors cancel into the exact $f0
 * passthrough — 100% vs 88.24% with plain f32), while the native #else needs the
 * true f32 return so callers (func_002AF728) get the untruncated angle. */
#ifndef TARGET_NATIVE
s32 func_002AAFB8(f32 a, f32 b, f32 c) {
    return WrapAnglePiSum(a, WrapAnglePiDiff(b, a) * c);
}
#else
f32 func_002AAFB8(f32 a, f32 b, f32 c) {
    return WrapAnglePiSum(a, WrapAnglePiDiff(b, a) * c);
}
#endif

/**
 * Rate-limited integrator with symmetric clamps on *state. First clamps *state to
 * [-|maxDelta|, |maxDelta|]; then integrates *state = v + (b*maxDelta - c*v)
 * (i.e. v*(1-c) + b*maxDelta) and clamps to [-bound, bound] when bound > 0;
 * finally re-clamps to [-|maxDelta|, |maxDelta|].
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 53.13%
   -> UNKNOWN-@1: ROM `(none)` vs `sd s0,0(sp)` */
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
 * func_002AB1A8 — integer approach-by-rate step (the int twin of
 * func_002AB150).
 *
 * Params: p — the stored value, stepped in place toward `target`.
 *         target — the value to approach.
 *         rate — the largest step allowed in either direction.
 * Returns the remaining signed distance (target - *p after the step), converted
 * to float by func_002835E0.
 *
 * MATCHED on the sdk29 arm (plain C; unit objdiff report via objdiff_build.sh +
 * unit_report.sh, 100.00%; task #758). The lower bound -rate lives in its own
 * local: the ROM negates it into a spare register in the plain `beq` delay
 * slot, so `rate` survives for the upper-bound arm. Writing `rate = -rate`
 * instead reuses rate's register, which forces a branch-likely (`beql`) and
 * shifts the whole allocation [94.57].
 */
f32 func_002AB1A8(s32 *p, s32 target, s32 rate) {
    s32 step = target - *p;
    s32 stepped;

    if (rate < step) {
        step = rate;
    } else {
        s32 minStep = -rate;
        if (step < minStep) {
            step = minStep;
        }
    }
    stepped = *p + step;
    *p = stepped;
    return (f32)func_002835E0(target - stepped);
}

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB208);

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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 65.77%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-64` vs `addiu sp,sp,-48` */
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 69.80%
   -> UNKNOWN-@1: ROM `(none)` vs `sd s1,24(sp)` */
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

/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 31.89%
   -> UNKNOWN-@1: ROM `mtc1 zero,$f2` vs `sd s1,8(sp)` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB3B0);
#else
/**
 * One-dimensional acceleration-limited "arrival" controller: advance a position
 * *pPos toward `target` by integrating a velocity *pVel that is itself ramped up
 * and braked so the motion decelerates to a stop at the target.
 *
 *   target   goal position
 *   velRate  max change applied to the velocity while spinning up (per step)
 *   accel    braking/deceleration rate; also the kinematic constant used for the
 *            stopping-distance test and the sqrt(2*accel*d) arrival speed
 *   maxSpeed velocity magnitude cap
 *   pPos     position accumulator (read + written)
 *   pVel     velocity state (read + written)
 *
 * Returns the velocity applied this step, or the residual distance when the
 * step would reach/overshoot the target (in which case *pPos is snapped to it).
 *
 * Behaviour by case:
 *  - already stopped exactly on target -> return 0.
 *  - velocity opposing the target (product < 0): bleed |*pVel| by accel, then
 *    integrate (raw, unclamped) -- kills momentum that points the wrong way.
 *  - moving toward target, inside braking distance 0.5*vel^2/accel: step the
 *    velocity toward 0 (by accel, or accel*1.1 when nearly stopped short).
 *  - moving toward target, outside braking distance: ramp the velocity (by
 *    velRate) toward the arrival speed sqrt(2*accel*diff), capped at maxSpeed.
 *  Then integrate: if |newVel| < |diff| add it to *pPos and return it, else
 *  snap *pPos to target and return diff.
 */
f32 func_002AB3B0(f32 target, f32 velRate, f32 accel, f32 maxSpeed, f32 *pPos, f32 *pVel)
{
    f32 diff = target - *pPos;
    f32 absDiff;
    f32 absVel;

    if (*pVel == 0.0f && diff == 0.0f) {
        return 0.0f;
    }

    if (*pVel * diff >= 0.0f) {
        /* velocity points toward the target (or is stationary) */
        f32 brakeDist = 0.5f * (*pVel * *pVel) / accel;

        absDiff = GetFloatAbs(diff);
        if (absDiff < brakeDist) {
            /* within stopping distance: decelerate toward a halt */
            absVel = GetFloatAbs(*pVel);
            if (brakeDist < absDiff + absVel) {
                func_002AB150(0.0f, accel, pVel);
            } else {
                func_002AB150(0.0f, accel * 1.1f, pVel);
            }
        } else {
            /* still approaching: ramp velocity toward the arrival speed */
            f32 speed = func_002835C0(2.0f * accel * diff);
            if (maxSpeed < speed) {
                speed = maxSpeed;
            }
            if (diff < 0.0f) {
                func_002AB150(-speed, velRate, pVel);
            } else {
                func_002AB150(speed, velRate, pVel);
            }
        }

        absDiff = GetFloatAbs(diff);
        absVel = GetFloatAbs(*pVel);
        if (absVel < absDiff) {
            *pPos = *pPos + *pVel;
            return *pVel;
        }
        *pPos = target;
        return diff;
    } else {
        /* velocity opposes the target: bleed it off by accel, then integrate */
        if (*pVel >= 0.0f) {
            *pVel = *pVel - accel;
        } else {
            *pVel = *pVel + accel;
        }
        *pPos = *pPos + *pVel;
        return *pVel;
    }
}
#endif

/* func_002AB5A0: wrap an angle into (-pi, pi] (via WrapAnglePiDiff), then when a
 * direction sign is supplied bias the result onto the requested rotation side:
 * if the wrapped delta already agrees with the sign (delta*sign > 0) keep it; an
 * almost-zero delta collapses to 0; otherwise add/subtract a full 2*pi turn so
 * the result rotates the requested way. Walled: $f20/$f21 + $16/$31 saves. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 69.68%
   -> UNKNOWN-@2: ROM `(none)` vs `daddu s0,a0,zero` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB5A0);
#else
f32 func_002AB5A0(f32 a, f32 b, s32 sign) {
    f32 d = WrapAnglePiDiff(a, b);   /* WrapAnglePiDiff(a - b) */

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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 65.71%
   -> UNKNOWN-@2: ROM `(none)` vs `daddu s0,a0,zero` */
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
    p[0] = WrapAnglePiSum(p[0], delta);   /* WrapAnglePiSum */
    {
        f32 r = func_002AB5A0(a, p[0], sign);
        __asm__ __volatile__(""); /* cc1 2.96 sibling-call suppression (the ROM never sibcalls) */
        return r;
    }
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 65.75%
   -> UNKNOWN-@3: ROM `(none)` vs `daddu s0,a0,zero` */
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
    *p = WrapAnglePiSum(*p, *vel);            /* WrapAnglePiSum */
    r = func_002AB5A0(target, *p, sign);
    if (GetFloatAbs(r) < d * 0.009999999776f) {   /* 0x3C23D70A */
        *p = target;
        *vel = 0.0f;
        return *vel;
    }
    return r;
}
#endif

/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 64.64%
   -> UNKNOWN-@3: ROM `(none)` vs `daddu s1,a0,zero` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB868);
#else
/**
 * Angular counterpart of func_002AB3B0: drive an angle *pAngle toward `target`
 * with an acceleration-limited angular velocity *pVel, honouring shortest-arc
 * (-pi, pi] wrapping. Deltas go through func_002AB5A0 (signed wrapped diff on
 * the chosen rotation side), the accelerating velocity step goes through
 * func_002AB668 (clamped, wrapping), and the angle integrates via WrapAnglePiSum.
 *
 *   target   goal angle
 *   maxStep  max angle the velocity may move per accelerate step (func_002AB668)
 *   accel    braking/deceleration rate + kinematic constant
 *   maxSpeed angular-speed cap
 *   pAngle   angle accumulator (read + written, kept wrapped)
 *   pVel     angular velocity (read + written)
 *   mode     rotation-side selector: 2 => derive the shorter-arc sign from the
 *            signs of target and *pAngle (as func_002AB700 does); any other value
 *            is used directly as the sign fed to func_002AB5A0.
 *
 * Returns the residual signed angle error: 0 when already stopped on target,
 * *pVel after an integrating step, or the fresh delta when the step reaches/
 * overshoots and *pAngle snaps to target.
 */
f32 func_002AB868(f32 target, f32 maxStep, f32 accel, f32 maxSpeed, f32 *pAngle, f32 *pVel, s32 mode)
{
    s32 sign;
    f32 delta;
    f32 absDelta;
    f32 absVel;

    if (mode == 2) {
        if (0.0f < target && *pAngle < 0.0f) {
            sign = 1;
        } else if (target < 0.0f && 0.0f < *pAngle) {
            sign = -1;
        } else {
            sign = 0;
        }
    } else {
        sign = mode;
    }

    delta = func_002AB5A0(target, *pAngle, sign);

    if (*pVel == 0.0f && delta == 0.0f) {
        return 0.0f;
    }

    if (*pVel * delta >= 0.0f) {
        /* velocity points toward the target */
        f32 brakeDist = 0.5f * (*pVel * *pVel) / accel;

        absDelta = GetFloatAbs(delta);
        if (absDelta < brakeDist) {
            /* within stopping distance: decelerate the velocity toward a halt */
            absVel = GetFloatAbs(*pVel);
            if (brakeDist < absDelta + absVel) {
                func_002AB150(0.0f, accel, pVel);
            } else {
                func_002AB150(0.0f, accel * 1.1f, pVel);
            }
        } else {
            /* still approaching: ramp velocity toward the arrival speed */
            f32 speed = func_002835C0(2.0f * accel * delta);
            if (maxSpeed < speed) {
                speed = maxSpeed;
            }
            if (delta < 0.0f) {
                func_002AB668(-speed, maxStep, pVel, 0);
            } else {
                func_002AB668(speed, maxStep, pVel, 0);
            }
        }

        absDelta = GetFloatAbs(delta);
        absVel = GetFloatAbs(*pVel);
        if (absVel < absDelta) {
            *pAngle = WrapAnglePiSum(*pVel, *pAngle);
            return *pVel;
        }
        *pAngle = target;
        return delta;
    } else {
        /* velocity opposes the target: bleed it off by accel, then integrate */
        if (*pVel >= 0.0f) {
            *pVel = *pVel - accel;
        } else {
            *pVel = *pVel + accel;
        }
        *pAngle = WrapAnglePiSum(*pVel, *pAngle);
        return *pVel;
    }
}
#endif

/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 66.83%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-64` vs `addiu sp,sp,-80` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ABAE8);
#else
/**
 * Single-state variant of func_002AB3B0: ramp/brake a velocity *pVel so it
 * arrives at a target `dist` units away, WITHOUT integrating a position (the
 * caller adds *pVel to its own position each step). `dist` is the signed
 * remaining distance, passed directly rather than derived from a position.
 *
 *   dist     signed distance still to cover
 *   velRate  max change applied to the velocity while spinning up
 *   accel    braking/deceleration rate + kinematic constant
 *   maxSpeed velocity magnitude cap
 *   pVel     velocity state (read + written)
 *
 * Same three cases as func_002AB3B0 (brake inside the stopping distance
 * 0.5*vel^2/accel; else ramp toward the arrival speed sqrt(2*accel*dist) capped
 * at maxSpeed; velocity opposing the target is bled off by accel), all via the
 * clamped-approach step func_002AB150. The finalize clamps *pVel so its
 * magnitude never exceeds |dist|, so one integration step lands on the target
 * rather than overshooting. The two callers discard the result, so this is void.
 */
void func_002ABAE8(f32 dist, f32 velRate, f32 accel, f32 maxSpeed, f32 *pVel)
{
    f32 absDist;
    f32 absVel;

    if (dist == 0.0f && *pVel == 0.0f) {
        return;
    }

    if (*pVel * dist >= 0.0f && dist != 0.0f) {
        /* velocity points toward the target */
        f32 brakeDist = 0.5f * (*pVel * *pVel) / accel;

        absDist = GetFloatAbs(dist);
        if (absDist < brakeDist) {
            /* within stopping distance: decelerate toward a halt */
            absVel = GetFloatAbs(*pVel);
            if (brakeDist < absDist + absVel) {
                func_002AB150(0.0f, accel, pVel);
            } else {
                func_002AB150(0.0f, accel * 1.1f, pVel);
            }
        } else {
            /* still approaching: ramp velocity toward the arrival speed */
            f32 speed = func_002835C0(2.0f * accel * dist);
            if (maxSpeed < speed) {
                speed = maxSpeed;
            }
            if (dist < 0.0f) {
                func_002AB150(-speed, velRate, pVel);
            } else {
                func_002AB150(speed, velRate, pVel);
            }
        }

        /* don't let the velocity carry past the target in one step */
        absDist = GetFloatAbs(dist);
        absVel = GetFloatAbs(*pVel);
        if (absVel >= absDist) {
            *pVel = dist;
        }
    } else {
        /* velocity opposes the target (or dist == 0): bleed it off by accel */
        if (*pVel >= 0.0f) {
            *pVel = *pVel - accel;
        } else {
            *pVel = *pVel + accel;
        }
        if (dist < 0.0f) {
            if (*pVel < dist) {
                *pVel = dist;
            }
        } else if (dist > 0.0f) {
            if (*pVel > dist) {
                *pVel = dist;
            }
        }
    }
}
#endif

/* func_002ABD00: pack four [0,1] float colour components into a 0xAABBGGRR u32.
 * Each of r/g/b/a (passed in $f12/$f13/$f14/$f15) is scaled by 255.0, truncated
 * to an int (FloatToInt), masked to a byte and shifted into its channel:
 *   r = bits 0-7, g = bits 8-15, b = bits 16-23, a = bits 24-31.
 * Walled: $f20-$f23 + $31 saves (save-layout wall). */
/* PARKED (engine-2.96): correct C, but the 4-channel mul/FloatToInt/mask
 * sequence + fp-save ordering schedules differently than the original
 * (fine-scheduling, 001003-vs-exact-2.96 gap). Faithful body kept as #else. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 66.63%
   -> SCHED-TIEBREAK, ORDER-ONLY (same instruction multiset, 23 rows displaced) */
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 52.82%
   -> UNKNOWN-@3: ROM `(none)` vs `daddu v0,a0,zero` */
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 76.32%
   -> UNKNOWN-@1: ROM `daddu v1,zero,zero` vs `sd s0,16(sp)` */
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 55.10%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-144` vs `addiu sp,sp,-128` */
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 62.39%
   -> UNKNOWN-@2: ROM `sd s0,32(sp)` vs `(none)` */
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

/*
 * R5900 SHORT-LOOP PAD. The ROM's assembler padded every backward branch
 * whose loop (branch target .. branch, inclusive) is shorter than 6
 * instructions with `nop`s up to 6 - the R5900 short-loop erratum
 * workaround. Across the engine-region USA asm no backward branch closes a
 * loop shorter than 6 (task #659 census). cc1 never emits the pad and
 * neither assembler we run inserts it (GNU as 2.40 does not, even with
 * -mfix-r5900; SN's bundled as.exe does not either), so it is written here,
 * directly before the branch it pads:
 *   - `.set noreorder` stops GNU as from swapping the last pad `nop` into
 *     the branch delay slot (without it one pad word becomes the slot
 *     filler and the function comes out one word short);
 *   - the "+r" operand is the value the branch tests, which pins the pad
 *     between that value's computation and the branch;
 *   - PAD1's `next` input is the register the ROM updates in the delay
 *     slot: reading it here keeps that update after the pad, where reorg
 *     can move it into the slot (without it the scheduler hoists the update
 *     above the pad and reorg fills the slot from the loop head instead).
 * The pad is a no-op on the native build.
 */
#ifndef TARGET_NATIVE
#define R5900_SHORT_LOOP_PAD1(v, next) \
    __asm__(".set noreorder\n\tnop\n\t.set reorder" : "+r"(v) : "r"(next))
#define R5900_SHORT_LOOP_PAD2(v) \
    __asm__(".set noreorder\n\tnop\n\tnop\n\t.set reorder" : "+r"(v))
#else
#define R5900_SHORT_LOOP_PAD1(v, next) ((void)0)
#define R5900_SHORT_LOOP_PAD2(v) ((void)0)
#endif

/**
 * func_002AC058 — return word 0 of a moby's extra/pvar block.
 * Track-B identity GetMobyCombatState (CONFIRMED, symbol_addrs.txt comment
 * block): word 0 is the moby's damageable-state block (+0x00 health f32,
 * +0x04 typeCategory s16). Kept under its splat name, which objdiff
 * matches against the frozen glabel.
 *
 * @param moby  moby to read; may be NULL.
 * @return      moby->pExtra[0] when the moby exists and has an extra block
 *              (modeBits & 0x20), else 0.
 *
 * The NULL test and the missing-block test share one `return 0` block that
 * the ROM places first: `bnezl` over it, then a BACKWARD `beqz` into it.
 * Only the early-return phrasing gives that layout under cc1 2.9 (-O2 -G8
 * -fno-gcse); `if (bits) return x; return 0;` and its &&/goto/nested
 * spellings all give a forward branch and a duplicated tail (FACT #7911).
 * The backward branch closes a 4-instruction loop, so the two ROM `nop`s
 * before it are the short-loop pad above, not scheduler output.
 */
s32 func_002AC058(Moby *moby) {
    u32 hasExtra;

    if (moby == 0) {
        return 0;
    }
    hasExtra = moby->modeBits & 0x20;
    R5900_SHORT_LOOP_PAD2(hasExtra);
    if (hasExtra == 0) {
        return 0;
    }
    return moby->pExtra[0];
}

/**
 * func_002AC088 — return word 4 (byte offset 0x10) of a moby's extra/pvar
 * block. Twin of func_002AC058; same layout and the same short-loop pad.
 *
 * @param moby  moby to read; may be NULL.
 * @return      moby->pExtra[4] when the moby has an extra block, else 0.
 */
s32 func_002AC088(Moby *moby) {
    u32 hasExtra;

    if (moby == 0) {
        return 0;
    }
    hasExtra = moby->modeBits & 0x20;
    R5900_SHORT_LOOP_PAD2(hasExtra);
    if (hasExtra == 0) {
        return 0;
    }
    return moby->pExtra[4];
}

/* SpawnParticleType04 (0x2BBD70): emit one type-04 particle whose spawn point is
 * `origin` and whose target/offset position is `pos`. Trailing ints are
 * lifetime/size/blend params. */
extern void SpawnParticleType04(Vec4 *origin, Vec4 *pos, u32 color, s32 a, s32 b,
                                s32 c, s32 d, s32 e);
/* Defined later in this unit (its own #else arm); declared here so the f32 arg
 * is not default-promoted to double when the callee is still INCLUDE_ASM. */
s32 func_002AA808(Moby *moby, s32 maxPoints, Vec4 *outPoints, s32 primMask, f32 density);

/**
 * func_002AC0B8 — emit a small burst of type-04 particles around a point.
 *
 * Scatters up to 20 points through the collision volume of `owner` at density
 * 14 (func_002AA808, honouring the `primMask` primitive filter), then for each
 * point builds a jittered offset direction from two random angles
 * (func_002AFE68, magnitude 0.03), adds it to `basePos`, nudges the result up in
 * Z by 0.015, and spawns a type-04 particle from that scatter point with two
 * randomised lifetime parameters (RandRangeInclusive 20..35 and 40..60).
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 83.52%
   -> UNKNOWN-@1: ROM `daddu a3,a2,zero` vs `(none)` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC0B8);
#else
void func_002AC0B8(Moby *owner, Vec4 *basePos, s32 primMask) {
    Vec4 scatterPoints[20];    /* func_002AA808 fills up to 20 Vec4 spawn points */
    Vec4 dir;
    s32 count;
    s32 i;

    count = func_002AA808(owner, 20, scatterPoints, primMask, 14.0f);
    if (count <= 0) {
        return;
    }
    for (i = 0; i < count; i++) {
        f32 angle1 = GetRandomAngle();
        f32 angle2 = GetRandomAngle();
        s32 r1;
        s32 r2;

        func_002AFE68(&dir, 0.030000001f, angle1, angle2);
        Vec4AddVu0(&dir, &dir, basePos);
        dir.z += 0.015000001f;

        r1 = RandRangeInclusive(20, 35);   /* RandRangeInclusive */
        r2 = RandRangeInclusive(40, 60);
        SpawnParticleType04(&scatterPoints[i], &dir, 0x7000A0FFu, 0xFF, r1, 30, r2, 1);
    }
}
#endif

/* func_002AC1E0: convert a 3x3 rotation matrix `m` (row stride 0x10 = 3 floats +
 * pad) into a quaternion `out` (x,y,z,w at +0x0/+0x4/+0x8/+0xC) — Shepperd's
 * method. If the trace is positive, use the direct w-largest form; otherwise pick
 * the largest diagonal element as the pivot p (with j,k the cyclic successors
 * from the {1,2,0} table D_1A9EA0) and build the quaternion around out[p].
 * func_002835C0 = sqrtf. (Identity MatrixToQuaternion — UNCONFIRMED name.) */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 49.45%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-144` vs `addiu sp,sp,-80` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC1E0);
#else
extern s32 D_1A9EA0[];   /* cyclic next-index table {1, 2, 0} */

void func_002AC1E0(void *out, void *matrix) {
    f32 *q = (f32 *)out;
    f32 *m = (f32 *)matrix;
#define M(i, j) m[(i) * 4 + (j)]     /* M[i][j]; rows are 0x10 bytes (4 floats) apart */
    f32 trace = M(0, 0) + M(1, 1) + M(2, 2);

    if (0.0f < trace) {
        f32 s = func_002835C0(trace + 1.0f);
        f32 scale = 0.5f / s;
        q[3] = 0.5f * s;                        /* w */
        q[0] = (M(2, 1) - M(1, 2)) * scale;     /* x */
        q[1] = (M(0, 2) - M(2, 0)) * scale;     /* y */
        q[2] = (M(1, 0) - M(0, 1)) * scale;     /* z */
    } else {
        s32 p = 0;
        s32 j, k;
        f32 s, scale;

        if (M(0, 0) < M(1, 1)) {
            p = 1;
        }
        if (M(p, p) < M(2, 2)) {
            p = 2;
        }
        j = D_1A9EA0[p];
        k = D_1A9EA0[j];

        s = func_002835C0(M(p, p) - M(j, j) - M(k, k) + 1.0f);
        q[p] = 0.5f * s;
        scale = (s == 0.0f) ? s : (0.5f / s);
        q[3] = (M(k, j) - M(j, k)) * scale;     /* w */
        q[j] = (M(j, p) + M(p, j)) * scale;
        q[k] = (M(k, p) + M(p, k)) * scale;
    }
#undef M
}
#endif

/* func_002AC468: build a scratch transform/matrix from `in` (func_00284008),
 * feed it through func_002AC1E0 with `out`, then resolve `in` against it
 * (func_00284028). The 0x40-byte scratch is a 4x4 matrix shared by all three
 * helpers. Walled: $16/$17/$31 saves (save-layout wall). */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 87.32%
   -> REGNUM-COLORING (s0/s1 swapped) + SCHED-TIEBREAK (prologue interleave) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC468);
#else
/* func_00284008 takes (dst, src): both call sites (here and func_002AE2D8)
 * keep the source pointer live in $5. */
extern void func_00284008(void *dst, void *src);
extern void func_00284028(void *src, void *m);
extern void func_002AC1E0(void *out, void *m);

void func_002AC468(void *out, void *in) {
    u8 scratch[0x40];   /* 4x4 matrix */

    func_00284008(scratch, in);
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 90.28%
   -> UNKNOWN-frame: ROM frame -48 with $f20 at 32, cc1 -32 with $f20 at 24 (a missing 8-16 B
   stack object), rest SCHED-TIEBREAK */
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC538);

/* Matrix helpers for MatrixToEulerAngles (also declared later for the AE-family). */
extern void func_00283DC0(Mat4x4 *dst, Vec4 *in);   /* build a rotation matrix from a vec (VU0) */
extern void func_00283DE0(Mat4x4 *dst, Vec4 *in);   /* sibling rotation builder (0xD18 variant) */
extern void MatrixMultiplyVu0(Mat4x4 *dst, Mat4x4 *a, Mat4x4 *b);

/**
 * Extract ZYX-style euler angles from a rotation matrix `mtx` into out[0..2].
 * Copies the 3x3 into a scratch matrix (translation row zeroed to {0,0,0,1}),
 * then peels the angles with three atan2 (Atan2fPoly) + Givens rotations that
 * successively zero the off-axis terms: a1=atan2(row0.x,row0.y) about -Z, then
 * a2=atan2(row0.x,-row0.z) about -Y, then a3=atan2(row1.y,row1.z) on the residual.
 * Writes out[0]=a3, out[1]=a2, out[2]=a1.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 20.34%
   -> UNKNOWN-@1: ROM `daddu v0,a0,zero` vs `(none)` */
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

    a1 = Atan2fPoly(*(f32 *)((u8 *)&m + 0x00), *(f32 *)((u8 *)&m + 0x04));
    axis.x = 0.0f;
    axis.y = 0.0f;
    axis.z = -a1;
    func_00283DC0(&rot, &axis);
    MatrixMultiplyVu0(&m, &rot, &m);

    a2 = Atan2fPoly(*(f32 *)((u8 *)&m + 0x00), -*(f32 *)((u8 *)&m + 0x08));
    axis.x = 0.0f;
    axis.y = -a2;
    axis.z = 0.0f;
    func_00283DE0(&rot, &axis);
    MatrixMultiplyVu0(&m, &rot, &m);

    a3 = Atan2fPoly(*(f32 *)((u8 *)&m + 0x14), *(f32 *)((u8 *)&m + 0x18));
    e[2] = a1;
    e[1] = a2;
    e[0] = a3;
}
#endif

/* Unpack RGB bytes (bytes 4/5/6 of the u64 colour at src+0x38) into 3 out words. */
extern void func_002A12F0(void *src, s32 *r, s32 *g, s32 *b);

/**
 * Advance a countdown/fade field pair on `obj`. When the counter (+0x0) is
 * running (!=0) but its active flag (+0x2) is clear, does nothing. Otherwise
 * clears the flag, reloads the counter from its reset value (+0xC), and either:
 *   - counter was 0: refreshes the RGB bytes (+0x4/+0x5/+0x6) from `src`'s packed
 *     colour (func_002A12F0), or
 *   - counter was running: rescales it to resetValue * counter / divisor(+0xE),
 *     clamped to at least 1.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 8.19% ->
   UNKNOWN-@2: ROM `sd ra,32(sp)` vs `(none)` */
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

extern s32 func_00283328(void *state); /* tick countdown: ret 0=counting, 1=already 0, 2=just hit 0; decrements state[0] */

/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 77.58%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-64` vs `addiu sp,sp,-48` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC728);
#else
/**
 * Per-frame RGB colour fade / ping-pong applied to a target object.
 *
 * `s` is the fade state block (byte offsets):
 *   s[0x0] (s16) frames remaining, ticked down each call via func_00283328
 *   s[0x2] (s16) direction flag: 0 = fade current->dest, nonzero = dest->current
 *   s[0x4..0x6] (u8) current RGB
 *   s[0x7..0x9] (u8) per-channel destination RGB; a 0 byte leaves that channel
 *                    at its current value (channel not animated)
 *   s[0xC]  (s16) forward-fade duration (fraction denominator)
 *   s[0xE]  (s16) reverse-fade duration, also the reload value on restart
 *
 * Each frame: while the counter is nonzero, tick it down; when it reaches 0
 * either latch the destination colour and stop (dir set) or restart the fade
 * reversed (dir clear, counter reloaded from s[0xE]). Otherwise interpolate
 * each enabled channel by frac = (duration - counter)/duration and push the
 * colour to `target` via func_002A12C0, whose result is returned (0 when the
 * fade is inactive).
 */
s32 func_002AC728(void *target, u8 *s)
{
    s32 counter;
    s32 dir;
    f32 frac;
    s32 r, g, b;

    if (*(s16 *)(s + 0x0) == 0) {
        return 0;
    }

    if (func_00283328(s) != 0) {
        /* countdown expired this frame */
        if (*(s16 *)(s + 0x2) != 0) {
            /* latch the destination colour and stop */
            {
                s32 r = func_002A12C0(target, s[0x4], s[0x5], s[0x6]);
                __asm__ __volatile__(""); /* cc1 2.96 sibling-call suppression (the ROM never sibcalls) */
                return r;
            }
        }
        /* restart the fade running in reverse */
        *(s16 *)(s + 0x2) = 1;
        *(s16 *)(s + 0x0) = *(s16 *)(s + 0xE);
    }

    dir = *(s16 *)(s + 0x2);
    counter = *(s16 *)(s + 0x0);

    if (dir == 0) {
        frac = ((f32)*(s16 *)(s + 0xC) - (f32)counter) / (f32)*(s16 *)(s + 0xC);
        r = s[0x7] ? FloatToInt((f32)s[0x4] + (f32)((s32)s[0x7] - (s32)s[0x4]) * frac) : s[0x4];
        g = s[0x8] ? FloatToInt((f32)s[0x5] + (f32)((s32)s[0x8] - (s32)s[0x5]) * frac) : s[0x5];
        b = s[0x9] ? FloatToInt((f32)s[0x6] + (f32)((s32)s[0x9] - (s32)s[0x6]) * frac) : s[0x6];
    } else {
        frac = ((f32)*(s16 *)(s + 0xE) - (f32)counter) / (f32)*(s16 *)(s + 0xE);
        r = s[0x7] ? FloatToInt((f32)s[0x7] + (f32)((s32)s[0x4] - (s32)s[0x7]) * frac) : s[0x4];
        g = s[0x8] ? FloatToInt((f32)s[0x8] + (f32)((s32)s[0x5] - (s32)s[0x8]) * frac) : s[0x5];
        b = s[0x9] ? FloatToInt((f32)s[0x9] + (f32)((s32)s[0x6] - (s32)s[0x9]) * frac) : s[0x6];
    }
    {
        s32 result = func_002A12C0(target, r, g, b);
        __asm__ __volatile__(""); /* cc1 2.96 sibling-call suppression (the ROM never sibcalls) */
        return result;
    }
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC978);

/**
 * func_002AC980 — wrap an angle toward [-pi, pi) via the shared frac helper;
 * the wrap holds only for angle >= -pi (see RANGE).
 *
 * Params: angle — radians.
 * Returns frac((angle + pi) / 2pi) * 2pi - pi, where func_00284678(out, x)
 * stores the truncated integer part (float)(int)x to *out and returns
 * x - *out (the scratch out-parameter is unused here).
 *
 * RANGE (do NOT "fix" by editing the range or the body): func_00284678
 * TRUNCATES toward zero - its ROM body is cvt.w.s / cvt.s.w / sub.s, and
 * the EE FPU's cvt.w.s rounds toward zero - it does not floor. So its
 * fraction has the sign of x:
 *  - angle >= -pi: x >= 0, frac in [0, 1), result in [-pi, pi) - wrapped;
 *  - angle <  -pi: x <  0, frac in (-1, 0], result in (-3pi, -pi] - NOT
 *    wrapped; it can sit up to 2pi below the range.
 * A caller needing [-pi, pi) for arbitrary input cannot rely on this alone.
 * ROM callers (jal sites, tree-wide over asm/usa; a jalr cannot be seen
 * this way): UpdateLightningBeamArcAndDamage x3, BuildGadgetBeamArcPolyline
 * x3 - 2 functions, 6 sites, all in the bulk asm/usa/text/208010.s.
 *
 * LITERAL NOTE (do NOT "fix"): this body's 2pi is 6.28318548f (0x40C90FDB) -
 * that is the byte-correct value its own .s loads. It is DISTINCT from
 * func_002AB5A0's 2pi (6.28318596f / 0x40C90FDC, 1 ULP higher) - the two
 * functions legitimately use different roundings. cmp-oracle-confirmed.
 *
 * MATCHED on the sdk29 arm (plain C; unit objdiff report via objdiff_build.sh +
 * unit_report.sh, 100.00%; task #758). The ROM loads 2pi into $f1 and puts the
 * product in $f12 (the dead argument register); the pinned cc1 colours the
 * product into $f0 and nothing in the expression moves it (task #597 ran three
 * phrasings, and reusing `angle` for the product is a fourth [99.55]). Both
 * registers are therefore bound explicitly (empty on native):
 *  - `turn` in $f12 alone makes cc1 load the constant into $f12 too [99.55];
 *  - `twoPi` in $f1 with it gives the ROM's pair.
 */
#ifndef TARGET_NATIVE
#define AC980_IN_F1 __asm__("$f1")
#define AC980_IN_F12 __asm__("$f12")
#else
#define AC980_IN_F1
#define AC980_IN_F12
#endif
f32 func_002AC980(f32 angle) {
    f32 frac;
    f32 scratch;
    register f32 twoPi AC980_IN_F1;
    register f32 turn AC980_IN_F12;

    frac = func_00284678(&scratch, (angle + 3.14159274f) * 0.159154937f);
    twoPi = 6.28318548f;
    turn = frac * twoPi;
    return turn - 3.14159274f;
}
#undef AC980_IN_F1
#undef AC980_IN_F12

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC9D8);

/**
 * func_002AC9E0 — test whether a pointer is a live moby-table entry whose
 * class id is in [500, 540].
 *
 * @param m  candidate moby; may be NULL or outside the table.
 * @return   1 when g_mobyTableBase <= m <= g_mobyTableEnd and
 *           500 <= m->oClass <= 540, else 0.
 *
 * All three rejections branch BACKWARD into one `return 0` block that the
 * ROM places right after the NULL test; the `goto` into that block is the
 * phrasing that gives cc1 2.9 this layout (plain early returns put the
 * block last). The first backward branch closes a 4-instruction loop, so the
 * ROM pads it with two `nop`s (R5900_SHORT_LOOP_PAD2); the second closes 9
 * and is not padded. The table-bound loads are the size-12 %gp_rel
 * delay-slot class (unit header).
 */
s32 func_002AC9E0(Moby *m) {
    u32 belowTable;

    if (m == 0) {
invalid:
        return 0;
    }
    belowTable = (u32)m < (u32)g_mobyTableBase;
    R5900_SHORT_LOOP_PAD2(belowTable);
    if (belowTable != 0) {
        goto invalid;
    }
    if ((u32)g_mobyTableEnd < (u32)m) {
        goto invalid;
    }
    return (u32)(m->oClass - 500) < 0x29;
}

/**
 * func_002ACA20 — surface-impact FX dispatcher. Emits the full effect for a
 * projectile/explosion hit: an optional moby splash-damage sphere query, a spray
 * of Type-0F spark particles, optional debris-streak mobys, bursts of Type-0B
 * "dust" and Type-08 "mist" particles (tinted from two random-indexed color
 * palettes), up to four expanding shockwave rings, a camera shake, an impact
 * sound, and an optional screen-space camera FX. The heavier sibling of
 * func_002AD590, with which it shares the palette + Type-0B tint idiom. Faithful
 * #else transcription — the matching build uses the INCLUDE_ASM arm above.
 *
 * Note: Ghidra types the 2nd arg as an integer, but the .s prologue preserves
 * seven incoming FP arg registers ($f12-$f18), so it is really an f32 (the query
 * power stamped into the hit-event record) — reflected in the signature here.
 *
 * @param queryRadius   splash-damage sphere-query radius; <=0 skips the query.
 * @param queryPower    impact power stamped into the hit-event record.
 * @param ringRadius    base radius of the first shockwave-ring set; <=0 skips them.
 * @param whiteRingRad  radius of the final white shockwave ring; <=0 skips it.
 * @param ringDistGate  min camera distance before the first two rings spawn.
 * @param scale         master FX scale (drives every particle spawn speed).
 * @param cameraFxMag   screen-space camera FX magnitude; 0 (or lod != 0) skips it.
 * @param moby          source moby (rings/sound/query attach here); 0 -> pos only.
 * @param spawnCtx9     unused spawn context (kept for ABI).
 * @param pos           impact world position (Vec4); defaults to moby+0x10 if null.
 * @param sparkCount    number of Type-0F spark particles.
 * @param dustCount     base count of Type-0B "dust" particles (scaled by distance).
 * @param mistCount     number of Type-08 "mist" particles.
 * @param soundIdx      impact sound index; -1 skips the sound.
 * @param shakeEnable   non-zero enables the on-screen camera shake.
 * @param debrisCount   number of debris-streak mobys.
 * @param lod           detail level; biases particle lifetimes / ring alpha down.
 * @param matFlag       material flag selecting particle tints + ring colors.
 * @param queryTag      tag stamped into the hit-event record.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 56.31%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-368` vs `addiu sp,sp,-432` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ACA20);
#else
/* callees / globals used only by this #else body that have no file-scope decl
 * above this point (some are re-declared in func_002AD590's #else arm below with
 * identical signatures — duplicate file-scope externs are legal). */
extern s32  RandRangeInclusive(s32 lo, s32 hi);
extern s32  GetRandomInt(s32 n);
extern void SpawnParticleType0F(f32 speed, void *spawnCtx, Vec4 *dir, u32 c1,
                                u32 c2, s32 life, s32 a7, s64 a8, s64 a9);
extern void SpawnParticleType0B(f32 speed, f32 life, u64 spawnCtx, void *pos,
                                u32 c1, u32 c2, s32 a7, s32 a8, s32 a9, s32 a10);
extern void SpawnParticleType08(f32 speed, u64 spawnCtx, Vec4 *dir, u32 c1,
                                u32 c2, s32 life);
extern void SpawnShockwaveRingMoby(f32 radius, long moby, u64 spawnCtx,
                                   void *pos, s32 a4, s32 a5, s32 a6,
                                   s32 a7, s32 a8);
extern void func_003093F8(void *spawnCtx, Vec4 *dir, s32 life, s32 flag, s32 a4); /* SpawnImpactDebrisStreakMoby */
extern void func_00283968(f32 minLen, Vec4 *dst, Vec4 *src);                      /* Vec3NormalizeCheckedVu0 */
extern long func_00284768(f32 range, Vec4 *pt);                                  /* ClassifyPointVsScreenBoxVu0 */
extern void func_00313968(void *shakeBlock, u64 spawnCtx, s32 a2, s32 a3);
extern s32  PlayMobySound(s32 soundIdx, s32 flags, Moby *owner);
extern u64  D_1A9F20[3];     /* particle color palette A (6 u32, 24 bytes) */
extern u64  D_1A9F38[3];     /* particle color palette B (6 u32, 24 bytes) */
extern u8   D_258C00[];      /* camera-FX request block (matFlag == 0) */
extern u8   D_258C50[];      /* camera-FX request block (matFlag != 0) */
extern u8   g_cameraState[]; /* g_cameraShakeUp lives at +0x160 */
extern f32  g_frameGpuTime;  /* last-frame GPU time */
extern f32  g_gameTime[];    /* g_gameTime[1] (+0x4) = last-frame CPU/step time */
extern Vec4 g_cameraPos;     /* 0x1B52C0: camera world position */

void func_002ACA20(f32 queryRadius, f32 queryPower, f32 ringRadius, f32 whiteRingRad,
                   f32 ringDistGate, f32 scale, f32 cameraFxMag, long moby, u64 spawnCtx9,
                   u64 *pos, s32 sparkCount, s32 dustCount, s32 mistCount, s32 soundIdx,
                   s32 shakeEnable, s32 debrisCount, s32 lod, u8 matFlag, u32 queryTag) {
    const f32 invFrameRate = 0.016666668f;   /* 0x3C888889 = 1/60 */
    Vec4 spawnPos;                            /* {0,0, scale*0.13333334, 0} anchor for rings/dust/mist */
    Vec4 dir;                                 /* per-particle direction scratch */
    Vec4 camDelta;                            /* g_cameraPos - pos */
    u8   hitRec[0x60];                        /* CollMobysSphere hit-event record */
    f32  camDist;

    spawnPos.x = 0.0f;
    spawnPos.y = 0.0f;
    spawnPos.z = scale * 0.13333334f;         /* 0x3E088889 */
    spawnPos.w = 0.0f;

    /* high GPU+step time forces the reduced-detail path */
    if (lod == -1 && 1.7f < g_frameGpuTime + g_gameTime[1]) {
        lod = 1;
    }

    if (pos == NULL) {
        pos = (u64 *)(moby + 0x10);
        if (moby == 0) {
            return;
        }
    }

    /* --- splash-damage moby sphere query --- */
    if (0.0f < queryRadius) {
        func_002A9A38(queryPower, (Moby *)hitRec, (s32)moby, (s32)queryTag);
        hitRec[0x18] = 2;
        hitRec[0x19] = 1;
        *(u16 *)(hitRec + 0x1A) = *(u16 *)((u8 *)moby + 0xAA);
        CollMobysSphere(pos, (void *)0x10, (Moby *)moby, hitRec, queryRadius);
    }

    /* --- Type-0F spark spray --- */
    if (0 < sparkCount) {
        s32 sparkLifeBias = lod * 10;
        do {
            f32 a2 = GetRandomFloatRange(0.26179942f, 1.3962636f);
            f32 a1 = GetRandomAngle();
            f32 mag = GetRandomFloatRange(7.0f, 10.5f);
            u32 c1, c2;
            s32 life;

            sparkCount--;
            func_002AFE68(&dir, scale * invFrameRate * mag, a1, a2);  /* SphericalAnglesToVec3 */
            dir.z += 0.050000004f;                                   /* 0x3D4CCCCE */
            c1 = func_002ABE08(0x4F007FFF, matFlag);
            c2 = func_002ABE08(0x1F00007F, matFlag);
            life = RandRangeInclusive(0x3C, 0x78);
            SpawnParticleType0F(scale * 40000.0f, pos, &dir, c1, c2,
                                life - sparkLifeBias, 1, -1, -1);
        } while (sparkCount != 0);
    }

    /* camera distance to the impact point */
    Vec4SubVu0(&camDelta, &g_cameraPos, (Vec4 *)pos);
    camDist = Vec3LengthVu0(&camDelta);

    /* --- debris-streak mobys --- */
    if (debrisCount != 0) {
        Vec4 streak;

        streak.x = GetRandomFloatRange(-1.0f, 1.0f);
        streak.y = GetRandomFloatRange(-1.0f, 1.0f);
        streak.z = GetRandomFloatRange(-1.0f, 1.0f);
        streak.w = 0.0f;

        if (camDist < 14.0f) {
            s32 life;

            Vec3RescaleToLenVu0(&streak, camDist * 0.0033333336f, &streak);  /* 0x3B5A740F */
            camDelta.z += camDist * 0.5f;
            Vec3RescaleToLenVu0(&camDelta, (camDist + camDist) * invFrameRate, &camDelta);
            Vec4AddVu0(&streak, &streak, &camDelta);
            func_00283968(0.16666667f, &streak, &streak);                    /* 0x3E2AAAAB */
            life = RandRangeInclusive(0x3C, 0x5A);
            func_003093F8(pos, &streak, life, 0, 0);
        }
        {
            s32 i = 0;
            if (0 < debrisCount - 1) {
                do {
                    f32 a2 = GetRandomFloatRange(0.26179942f, 1.308997f);
                    f32 a1 = GetRandomAngle();
                    f32 mag = GetRandomFloatRange(7.0f, 10.5f);
                    s32 stagger = i % 3;
                    s32 life;

                    func_002AFE68(&streak, scale * invFrameRate * mag, a1, a2);
                    i++;
                    streak.z += 0.033333335f;                                /* 0x3D088889 */
                    life = RandRangeInclusive(0x3C, 0x5A);
                    func_003093F8(pos, &streak, life, stagger == 0, 0);
                } while (i < debrisCount - 1);
            }
        }
    }

    /* --- Type-0B "dust" burst (count falls off with distance) --- */
    {
        f32 dc = IntToFloat(dustCount);
        s32 nDust = dustCount;
        f32 lifeBoost;

        if (camDist < dc + dc) {
            nDust = FloatToInt(camDist) / 2;
        }
        lifeBoost = (camDist < 7.0f) ? (7.0f - camDist) : 0.0f;

        if (0 < nDust) {
            s32 dustLifeBias = lod * 5;
            do {
                f32 speed = GetRandomFloatRange(8.0f, 10.0f);
                u64 palA[3];
                u64 palB[3];
                s32 idx;
                u32 c1, c2;
                s32 life1, life2;

                nDust--;
                palA[0] = D_1A9F20[0];
                palA[1] = D_1A9F20[1];
                palA[2] = D_1A9F20[2];
                palB[0] = D_1A9F38[0];
                palB[1] = D_1A9F38[1];
                palB[2] = D_1A9F38[2];

                speed = (speed * invFrameRate - lifeBoost * invFrameRate) * scale;
                idx = GetRandomInt(6);
                c1 = func_002ABE08(*(u32 *)((u8 *)palA + idx * 4), matFlag);
                idx = GetRandomInt(6);
                c2 = func_002ABE08(*(u32 *)((u8 *)palB + idx * 4), matFlag);
                life1 = RandRangeInclusive(0xF, 0x14);
                life2 = RandRangeInclusive(0x1E, 0x2D);
                SpawnParticleType0B(scale * 400000.0f, speed, (u64)(u32)pos, &spawnPos,
                                    c1, c2, life1 + lod * -3, life2 - dustLifeBias, 0, 0);
                life1 = RandRangeInclusive(5, 0xA);
                life2 = RandRangeInclusive(0xF, 0x14);
                SpawnParticleType0B(scale * 400000.0f, speed * 0.5f, (u64)(u32)pos, &spawnPos,
                                    0x7FFFFFFF, 0xFFFFFF, life1 + lod * -2, life2 + lod * -3, 0, 0);
            } while (nDust != 0);
        }
    }

    /* --- Type-08 "mist" burst --- */
    if (0 < mistCount) {
        s32 mistLifeBias = lod * 5;
        s32 n = mistCount;
        do {
            u64  palA[3];
            u64  palB[3];
            Vec4 mist;
            f32  mag;
            s32  idx;
            u32  c1, c2;
            s32  life;

            palA[0] = D_1A9F20[0];
            palA[1] = D_1A9F20[1];
            palA[2] = D_1A9F20[2];
            palB[0] = D_1A9F38[0];
            palB[1] = D_1A9F38[1];
            palB[2] = D_1A9F38[2];

            mist.x = GetRandomFloatRange(-1.0f, 1.0f);
            n--;
            mist.y = GetRandomFloatRange(-1.0f, 1.0f);
            mist.z = GetRandomFloatRange(-1.0f, 1.0f);
            mist.w = 0.0f;
            mag = GetRandomFloatRange(0.0f, 3.0f);
            Vec3RescaleToLenVu0(&mist, scale * invFrameRate * mag, &mist);
            idx = GetRandomInt(6);
            c1 = func_002ABE08(*(u32 *)((u8 *)palA + idx * 4), matFlag);
            idx = GetRandomInt(6);
            c2 = func_002ABE08(*(u32 *)((u8 *)palB + idx * 4), matFlag);
            life = RandRangeInclusive(0x1E, 0x2D);
            SpawnParticleType08(200000.0f, (u64)(u32)pos, &mist, c1, c2, life - mistLifeBias);
        } while (n != 0);
    }

    /* --- expanding shockwave rings --- */
    if (moby != 0) {
        if (0.0f < ringRadius) {
            s32 r5 = 0x96, g5 = 0x96, b5 = 0x96;  /* first-set color */
            s32 r6 = 0x7F, g6 = 0x7F, extra = 0;  /* second/third-ring color + alpha bump */

            if (matFlag != 0) {
                r5 = 0x23;
                if ((matFlag & 4) == 0) {
                    r5 = 0x46;
                    b5 = 0x46;
                    r6 = 0x3C;
                } else {
                    g5 = 0;
                    b5 = 0xFA;
                    r6 = 0x20;
                    g6 = 0;
                    extra = 100;
                }
            }

            if (g_gameTime[1] < 0.95f && ringDistGate < camDist) {
                SpawnShockwaveRingMoby(ringRadius, moby, (u64)(u32)pos, &spawnPos,
                                       0x10, r5, g5, b5, 0x20);
                SpawnShockwaveRingMoby(ringRadius, moby, (u64)(u32)pos, &spawnPos,
                                       0x16, r6, g6, extra + 0x50, 0x20);
            }
            SpawnShockwaveRingMoby(ringRadius, moby, (u64)(u32)pos, &spawnPos,
                                   0x1E, r6, g6, extra, 0x30);
        }
        if (moby != 0 && 0.0f < whiteRingRad) {
            SpawnShockwaveRingMoby(whiteRingRad, moby, (u64)(u32)pos, &spawnPos,
                                   0x1B, 0xFF, 0xFF, 0xFF, 0x20);
        }
    }

    /* --- on-screen camera shake --- */
    if (shakeEnable != 0) {
        Vec4 pt;

        pt = *(Vec4 *)pos;
        pt.w = 2.0f;                                    /* 0x40000000 */
        if (func_00284768(10.0f, &pt) != -1) {          /* ClassifyPointVsScreenBoxVu0 */
            if (camDist < 20.0f) {
                *(f32 *)(g_cameraState + 0x160) = 0.4f - camDist * 0.0175f; /* amplitude */
            } else {
                *(f32 *)(g_cameraState + 0x160) = 0.050000012f;
            }
            *(s32 *)(g_cameraState + 0x168) = 0x19;     /* timer */
            *(s32 *)(g_cameraState + 0x16C) = 0;        /* duration */
        }
    }

    /* --- impact sound --- */
    if (moby != 0 && soundIdx != -1) {
        PlayMobySound(soundIdx, 0, (Moby *)moby);
    }

    /* --- optional screen-space camera FX --- */
    if (cameraFxMag != 0.0f && lod == 0) {
        Vec4 pt;

        pt = *(Vec4 *)pos;
        pt.w = cameraFxMag;
        if (func_00284768(100.0f, &pt) != -1) {         /* 0x42C80000 = 100.0 */
            f32 mag = (cameraFxMag <= 0.0f) ? 15.0f : cameraFxMag;

            *(f32 *)(D_258C00 + 0x20) = mag;
            *(f32 *)(D_258C00 + 0x24) = mag;
            *(f32 *)(D_258C00 + 0x28) = mag;
            if (matFlag == 0) {
                func_00313968(D_258C00, (u64)(u32)pos, 0, 0);
            } else {
                func_00313968(D_258C50, (u64)(u32)pos, 0, 0);
            }
        }
    }
}
#endif

/**
 * func_002AD590 — explosion / impact FX spawner. Emits a 3-particle sparkle
 * burst, optional pair of expanding shockwave rings around `moby`, an optional
 * impact sound, and an optional camera shake.
 *
 * @param a          intensity/scale (drives particle spawn speed and ring size).
 * @param b          camera-shake magnitude; 0 skips the shake, values <= 0 are
 *                   clamped up to 13.0.
 * @param moby       moby the rings attach to and the impact sound plays from;
 *                   0 skips rings + sound.
 * @param spawnCtx   opaque spawn/handle context forwarded to every FX call.
 * @param sound      impact sound index; -1 skips the sound.
 *
 * Each of the 3 particles copies two 6-entry u32 color palettes (D_1A9F20 and
 * D_1A9F38) onto the stack, then picks one random color from each palette
 * (GetRandomInt(6) index) for the SpawnParticleType0B tint pair. The rings are
 * only spawned when the moby's +0x20 class byte is not 0xFE/0xFD (those two
 * class ids suppress the impact sound). Faithful #else transcription — the
 * matching build uses the INCLUDE_ASM arm above.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 54.53%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-224` vs `addiu sp,sp,-208` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AD590);
#else
/* callees not yet declared above this point (RandRangeInclusive is block-scoped
 * in a later #else arm; the FX spawners have no file-scope decl). */
extern s32  RandRangeInclusive(s32 lo, s32 hi);         /* 0x2A8688 */
extern void SpawnParticleType0B(f32 speed, f32 life, u64 spawnCtx, void *pos,
                                u32 color1, u32 color2, s32 arg7, s32 arg8,
                                s32 arg9, s32 arg10);    /* 0x2BCBD0 */
extern void SpawnShockwaveRingMoby(f32 radius, long moby, u64 spawnCtx,
                                   void *pos, s32 a4, s32 a5, s32 a6,
                                   s32 a7, s32 a8);       /* 0x309200 */
extern void func_00313968(void *shakeBlock, u64 spawnCtx, s32 a2, s32 a3);
extern s32  PlayMobySound(s32 soundIdx, s32 flags, Moby *owner);   /* 0x2E6B30 */

/* two 6-entry u32 color palettes copied wholesale onto the stack per particle */
extern u64 D_1A9F20[3];   /* 0x1A9F20 particle color palette A (24 bytes) */
extern u64 D_1A9F38[3];   /* 0x1A9F38 particle color palette B (24 bytes) */
extern u8  D_258CA0[];    /* 0x258CA0 camera-shake request block */

void func_002AD590(f32 a, f32 b, long moby, u64 spawnCtx, s32 sound) {
    const f32 invFrameRate = 0.016666668f;   /* 0x3C888889 = 1/60 */
    u8 pos[16];                              /* zeroed spawn position (Vec4) */
    u64 palA[3];                             /* stack copy of D_1A9F20 */
    u64 palB[3];                             /* stack copy of D_1A9F38 */
    s32 soundSlot = sound;
    s32 i = 2;

    func_00283638((Moby *)pos);              /* Vec4ZeroInt: clear the position */

    do {
        f32 speed = a * 400000.0f;           /* 0x48C35000 */
        f32 rand;
        s32 idxA;
        s32 idxB;
        s32 life7;
        s32 life8;

        i = i - 1;
        rand = GetRandomFloatRange(8.0f, 10.0f) * invFrameRate;

        palA[0] = D_1A9F20[0];
        palA[1] = D_1A9F20[1];
        palA[2] = D_1A9F20[2];
        palB[0] = D_1A9F38[0];
        palB[1] = D_1A9F38[1];
        palB[2] = D_1A9F38[2];

        idxA  = GetRandomInt(6);
        idxB  = GetRandomInt(6);
        life7 = RandRangeInclusive(0xF, 0x14);
        life8 = RandRangeInclusive(0x19, 0x1E);
        SpawnParticleType0B(speed, rand * a, spawnCtx, pos,
                            *(u32 *)((u8 *)palA + idxA * 4),
                            *(u32 *)((u8 *)palB + idxB * 4),
                            life7, life8, 0, 0);
    } while (-1 < i);

    if (moby != 0) {
        SpawnShockwaveRingMoby(a * 4.0f, moby, spawnCtx, pos,
                               0x14, 0x7F, 0x40, 0, 0x30);   /* 0x40800000 */
        SpawnShockwaveRingMoby(a * 3.0f, moby, spawnCtx, pos,
                               0x1D, 0x60, 0x20, 0, 0x20);   /* 0x40400000 */
        {
            u8 classByte = *(u8 *)((u8 *)moby + 0x20);
            if (classByte != 0xFE && classByte != 0xFD && soundSlot != -1) {
                PlayMobySound(soundSlot, 0, (Moby *)moby);
            }
        }
    }

    if (b != 0.0f) {
        f32 shake = (b <= 0.0f) ? 13.0f : b;   /* 0x41500000 */
        *(f32 *)(D_258CA0 + 0x20) = shake;
        *(f32 *)(D_258CA0 + 0x24) = shake;
        *(f32 *)(D_258CA0 + 0x28) = shake;
        func_00313968(D_258CA0, spawnCtx, 0, 0);
    }
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AD858);

/** func_002AD860 — clamp a vector's length: if `vec`'s 3-component length exceeds
 *  `maxLen`, rescale it in place down to maxLen (else leave it unchanged). */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 89.47%
   -> SCHED-TIEBREAK (move s0,a0 vs swc1 $f20 order in the prologue), ORDER-ONLY */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AD860);
#else
void func_002AD860(Vec4 *vec, f32 maxLen) {
    if (maxLen < Vec3LengthVu0(vec)) {
        Vec3RescaleToLenVu0(vec, maxLen, vec);
    }
    __asm__ __volatile__("");
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AD8B0);

/**
 * func_002AD8B8 — append a moby's table slot to an s16 count-prefixed list
 * (list[0] = count, list[1..count] = slots) unless it is already present or
 * the list is full.
 *
 * @param moby  moby whose slot index ((moby - g_mobyTableBase) / 0x100) is
 *              appended.
 * @param list  count-prefixed slot list.
 * @param cap   maximum entry count; the append happens only while
 *              list[0] < cap.
 *
 * The scan loop is the ROM's: a pointer walk with the bound in a copy of
 * the count (only a loop-local copy gives the `daddu $8,$3`), `i++` in the
 * match-branch slot and `p++` in the loop-branch slot. The loop is 5
 * instructions, so the ROM pads it with one `nop` (R5900_SHORT_LOOP_PAD1).
 * `++list[0]` gives the ROM's single lhu/addiu feeding both the count store
 * and the (s16) index.
 */
void func_002AD8B8(Moby *moby, s16 *list, s16 cap) {
    s16 slot = (s16)(((u8 *)moby - (u8 *)g_mobyTableBase) >> 8);
    s32 i = 1;
    s16 *p;

    if (list[0] > 0) {
        s32 count = list[0];
        s32 scanned;

        p = &list[1];
        do {
            if (*p == slot) {
                return;
            }
            i++;
            scanned = count < i;
            R5900_SHORT_LOOP_PAD1(scanned, p);
            p++;
        } while (scanned == 0);
    }
    if (list[0] < cap) {
        s16 n = ++list[0];

        list[n] = slot;
    }
}

/* func_002AD938: remove a moby from an s16 count-prefixed slot list.
 * list[0] is the count and list[1..count] are moby-table slot indices. Scans
 * for the entry whose moby (g_mobyTableBase + slot * 0x100) is `moby`, moves
 * the last entry into its place and decrements the count. No-op if absent.
 * No return.
 * The ROM re-reads list[0] and g_mobyTableBase on every iteration (the base
 * through the assembler's absolute `lui; lw` macro, from `.extern ..., 12`
 * above). The volatile read of the base keeps loop.c from hoisting it, and
 * re-reading list[0] in the loop test gives the branch-likely count reload.
 * The earlier "scan-loop scheduling wall" note was wrong on both counts.
 * The volatile is a codegen device, not a claim that g_mobyTableBase changes
 * asynchronously under this loop (FACT #8380; RULING #8404, retired by a
 * non-volatile byte-exact spelling or by a concurrent writer). Its job is the
 * per-iteration reload. #8380 found three
 * non-volatile spellings that all hoist the load and do not match; it does not
 * establish why the ROM compiler did not hoist. The object is genuinely
 * mutable: SwapMobyTableContext (188858.c) stores the HUD base into it through
 * SWAP_VOLATILE (ROM 0x28BDD8, `sw $7, %lo(g_mobyTableBase)($1)`).
 * MATCHED (task #946): sdk29 (-O2 -G8 -fno-gcse), solo. */
void func_002AD938(Moby *moby, s16 *list) {
    s32 i;

    for (i = 1; i <= list[0]; i++) {
        u8 *base = *(u8 *volatile *)&g_mobyTableBase;

        if ((Moby *)(base + list[i] * 0x100) == moby) {
            list[i] = list[(s16)(u16)list[0]];   /* last entry into this slot */
            list[0] = (u16)list[0] - 1;
            return;
        }
    }
}

/* func_002AD9B0: jitter a Vec3 in place — add an independent uniform random
 * offset in [-amt, amt) to each of x/y/z. Walled: $f20/$f21 + $16/$31 saves
 * (save-layout wall). */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 92.12%
   -> REGNUM-COLORING ($f20/$f21 swapped) + SCHED-TIEBREAK; hoisting -amt RUN, no change */
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 56.28%
   -> UNKNOWN-@1: ROM `addiu v0,zero,-1` vs `addiu v1,zero,-1` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADA30);
#else
/* out = m * v (3x3 rotate; see the canonical def in text/183558.c). The asm
 * passes the segment's rotation matrix (seg+0x40) as the 3rd arg - dropping it
 * silently leaves `out` undefined, so it must be forwarded. */
extern void func_00283A48(Vec4 *out, Vec4 *v, Vec4 *m);
/* func_00283920: rescale a vector's XY to horizontal length `len`, preserving
 * z/w (VU0 vrsqrt; snaps to zero when the xy magnitude is 0). */
extern void func_00283920(Vec4 *dst, Vec4 *src, f32 len);
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADB08);

/**
 * Test whether `pos` lies within unit distance of segment `segIdx`'s local
 * frame. The segment table base is *(g_deferredSegment2Tag+0xD4); each entry is
 * 0x80 bytes with its origin at +0x30 and a 3x3 rotation at +0x40. Transforms
 * (pos - origin) into the segment's local frame and returns 1 if the resulting
 * length is < 1.0, else 0. A negative segIdx returns 0.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 63.18%
   -> UNKNOWN-@1: ROM `(none)` vs `daddu a2,a0,zero` */
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADB98);

/* func_002ADBA0(subject): scan the point-light manager block
 * (g_pointLights+0x2400) back-to-front — for each active light entry
 * (g_pointLights+0x2420, 0x10 stride, count at mgr+0xC) call the predicate
 * func_00284730(subject, &entry, &entry+0x20); on the first hit record the
 * 1-based index and stop. Returns 1 iff that hit index equals the manager's
 * head field (mgr+0x0) — i.e. the top-most entry was the match. */
extern u8 g_pointLights[];
extern s32 func_00284730(void *subject, void *entry, void *entryHi);
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 35.61%
   -> UNKNOWN-@3: ROM `(none)` vs `addiu s2,v0,9216  [LO16 0x001C2AC0]` */
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 65.57%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-96` vs `addiu sp,sp,-80` */
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

extern f32 func_002835C0(f32 x);   /* sqrtf */

/** func_002ADCE0 — rescale `src`'s vec3 to length `t` into `dst`, then set dst->w
 *  to sqrt(1 - t*t) (the w that keeps a unit quaternion / normal). */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 49.33%
   -> UNKNOWN-@1: ROM `(none)` vs `swc1 $f20,16(sp)` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADCE0);
#else
void func_002ADCE0(Vec4 *dst, f32 t, Vec4 *src) {
    Vec3RescaleToLenVu0(dst, t, src);
    dst->w = func_002835C0(1.0f - t * t);
}
#endif

/**
 * Rotate `src` about `axis` by `angle` into `dst`. For a negligible angle
 * (|angle| < 1e-5) it just copies src to dst; otherwise it normalises the axis,
 * builds the axis-angle quaternion (func_002AC4D0), and rotates src by it
 * (func_002ADC50).
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 40.21%
   -> UNKNOWN-@1: ROM `(none)` vs `sd s0,16(sp)` */
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

extern void Vec3CrossVu0(Vec4 *dst, Vec4 *a, Vec4 *b);   /* fwd (also declared later) */
extern f32 func_00283B60(f32 x);                          /* arccos */

/**
 * Rotate vector `a` a fraction `t` of the way toward `b`, into `out` (slerp-like).
 * Axis = normalize(cross(b, a)); cosθ = dot(a, b) — divided by |a|·|b| unless
 * `useRaw` != 0 (and if that product is 0, out = a and returns). Builds a rotation
 * quaternion by ((π/2 − acos(cosθ))·t·0.5) about the axis and rotates a by it
 * (func_002ADC50).
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 61.26%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-96` vs `addiu sp,sp,-80` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADDD0);
#else
void func_002ADDD0(Vec4 *out, Vec4 *a, Vec4 *b, s32 useRaw, f32 t) {
    Vec4 quat;
    f32 cosA;
    f32 angle;
    f32 rotAngle;

    Vec3CrossVu0(&quat, b, a);
    Vec3RescaleToLenVu0(&quat, 1.0f, &quat);
    cosA = Vec3DotVu0(a, b);

    if (useRaw == 0) {
        f32 lenAB = Vec3LengthVu0(a) * Vec3LengthVu0(b);
        if (lenAB == 0.0f) {
            *out = *a;
            return;
        }
        cosA = cosA / lenAB;
    }

    angle = func_00283B60(cosA);
    rotAngle = (1.5707964f - angle) * (t * 0.5f);
    Vec4ScaleVu0(&quat, func_00283B48(rotAngle), &quat);
    quat.w = func_00283B30(rotAngle);
    func_002ADC50(out, a, &quat);
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADF10);

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

/**
 * AE-family combined: map a point AND compose an orientation through obj's
 * sub-source. No sub-source (func_002ADF18==0) → arg5=arg3, arg6=arg4, return 0.
 * Otherwise: bring (arg3 + src pos − obj pos) into local space and rotate into
 * arg5 (src+0x20 folded with obj matrix +0xC0 when src flag +0x3C bit 0x2, else
 * base matrix), re-add obj pos; and compose arg4's rotation with the base matrix
 * → euler into arg6. Returns 1.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 67.07%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-272` vs `addiu sp,sp,-256` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADF48);
#else
s32 func_002ADF48(void *self, Moby *obj, Vec4 *arg3, Vec4 *arg4, Vec4 *arg5, void *arg6) {
    s32 src = func_002ADF18(obj);
    Mat4x4 base;
    Mat4x4 composed;
    (void)self;

    if (src == 0) {
        *arg5 = *arg3;
        *(Vec4 *)arg6 = *arg4;
        return 0;
    }
    func_00283DC0(&base, (Vec4 *)src);
    Vec4AddVu0(arg5, arg3, (Vec4 *)(src + 0x10));
    Vec4SubVu0(arg5, arg5, (Vec4 *)((u8 *)obj + 0x10));
    if (*(s32 *)(src + 0x3C) & 0x2) {
        Mat4x4 rot;
        func_00283DC0(&rot, (Vec4 *)(src + 0x20));
        func_00284048(&composed, (const Vec4 *)&rot);
        func_00283A48(arg5, arg5, (Vec4 *)&composed);
        func_00283A48(arg5, arg5, (Vec4 *)((u8 *)obj + 0xC0));
    } else {
        func_00283A48(arg5, arg5, (Vec4 *)&base);
    }
    Vec4AddVu0(arg5, arg5, (Vec4 *)((u8 *)obj + 0x10));
    func_00283DC0(&composed, arg4);
    MatrixMultiplyVu0(&composed, &base, &composed);
    MatrixToEulerAngles(&composed, arg6);
    return 1;
}
#endif

extern void func_00283DC0(Mat4x4 *dst, Vec4 *in);   /* build rotation matrix from a vec (VU0) */

/**
 * Transform `in` into `out` by the orientation of obj's resolved sub-source
 * (func_002ADF18 = pvar word 2). If there is no sub-source, out = in (copy) and
 * returns 0. Otherwise builds its rotation matrix; when the source's flag (+0x3C)
 * has bit 0x2, it composes an extra rotation from the source's +0x20 vec and
 * re-applies obj's own matrix at +0xC0. Returns 1 when transformed.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 73.45%
   -> UNKNOWN-@2: ROM `(none)` vs `daddu s4,a1,zero` */
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

/**
 * Map a point `arg3` (given in the sub-source's space) back through obj into
 * `arg4`, relative to arg3. No sub-source (func_002ADF18==0) → arg4 = 0, return 0.
 * Otherwise: bring (arg3 + src pos − obj pos) into local space, rotate it by the
 * source orientation (from src+0x20 folded with obj's matrix +0xC0 when src flag
 * +0x3C bit 0x2 is set, else the base matrix), add obj pos back, and write
 * (result − arg3) to arg4. Returns 1.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 62.64%
   -> UNKNOWN-@1: ROM `sd s4,240(sp)` vs `sd s3,248(sp)` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE198);
#else
s32 func_002AE198(void *self, Moby *obj, Vec4 *arg3, Vec4 *arg4) {
    s32 src = func_002ADF18(obj);
    Mat4x4 base;
    Vec4 p;
    (void)self;

    if (src == 0) {
        Vec4 zero = {0.0f, 0.0f, 0.0f, 0.0f};
        *arg4 = zero;
        return 0;
    }
    func_00283DC0(&base, (Vec4 *)src);
    Vec4AddVu0(&p, arg3, (Vec4 *)(src + 0x10));
    Vec4SubVu0(&p, &p, (Vec4 *)((u8 *)obj + 0x10));
    if (*(s32 *)(src + 0x3C) & 0x2) {
        Mat4x4 rot;
        Mat4x4 mtx;
        func_00283DC0(&rot, (Vec4 *)(src + 0x20));
        func_00284048(&mtx, (const Vec4 *)&rot);
        func_00283A48(&p, &p, (Vec4 *)&mtx);
        func_00283A48(&p, &p, (Vec4 *)((u8 *)obj + 0xC0));
    } else {
        func_00283A48(&p, &p, (Vec4 *)&base);
    }
    Vec4AddVu0(&p, &p, (Vec4 *)((u8 *)obj + 0x10));
    Vec4SubVu0(arg4, &p, arg3);
    return 1;
}
#endif

extern void func_00284008(void *dst, void *src);   /* build/convert matrix dst from src */
extern void MatrixToEulerAngles(Mat4x4 *mtx, void *outAngles);
extern s32  func_002AE460(void *self, Moby *obj, Vec4 *arg3, Vec4 *arg4,
                          void *arg5, void *arg6);

/**
 * func_002AE2D8 — AE-family placement with a hero/priority fallback.
 *
 * Resolves `obj`'s sub-source (func_002ADF18); no source → returns 0. Builds the
 * source's base rotation matrix — from obj facing (+0xF0) via func_00283DC0 when
 * the source flag (+0x3C) bit 0x40 is set, else loaded from obj's matrix rows
 * (+0xC0) via func_00284008 — transforms `point` into that frame (out `outPos`)
 * and re-adds obj's world position (+0x10); then composes `rotIn`'s rotation with
 * the base matrix and writes euler angles to `outAngles`.
 *
 * Then, on source flag bit 0x4, re-adds the source's own offset (+0x10) into
 * `outPos` and defers to func_002AE460 for the full compose. Otherwise, unless
 * `self` is the hero, and only when `self` sorts below `obj` by class slot
 * (+0x22), adds the source offset clamped to unit length (func_002AD860). Always
 * returns 1 once a source exists.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 76.16%
   -> UNKNOWN-@1: ROM `(none)` vs `sd s2,160(sp)` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE2D8);
#else
s32 func_002AE2D8(Moby *self, Moby *obj, Vec4 *point, Vec4 *rotIn,
                  Vec4 *outPos, void *outAngles) {
    s32 src = func_002ADF18(obj);
    Mat4x4 base;
    Mat4x4 composed;

    if (src == 0) {
        return 0;
    }
    if (*(s32 *)(src + 0x3C) & 0x40) {
        func_00283DC0(&base, (Vec4 *)((u8 *)obj + 0xF0));
    } else {
        func_00284008(&base, (u8 *)obj + 0xC0);
    }
    func_00283A48(outPos, point, (Vec4 *)&base);
    Vec4AddVu0(outPos, outPos, (Vec4 *)((u8 *)obj + 0x10));

    func_00283DC0(&composed, rotIn);
    MatrixMultiplyVu0(&composed, &base, &composed);
    MatrixToEulerAngles(&composed, outAngles);

    if (*(s32 *)(src + 0x3C) & 0x4) {
        Vec4AddVu0(outPos, outPos, (Vec4 *)(src + 0x10));
        func_002AE460(self, obj, outPos, outAngles, point, rotIn);
        return 1;
    }
    if (self == g_pHeroMoby[0]) {
        return 1;
    }
    if (self->classSlot < obj->classSlot) {
        Vec4 srcOffset = *(Vec4 *)(src + 0x10);

        func_002AD860(&srcOffset, 1.0f);
        Vec4AddVu0(outPos, outPos, &srcOffset);
    }
    return 1;
}
#endif

/**
 * Compute a moby's local-frame offset + composed orientation from its resolved
 * sub-source. No source (func_002ADF18==0) → returns 0. Otherwise builds the
 * source's base rotation matrix — from obj+0xF0 when the source flag +0x3C bit
 * 0x40 is set, else obj+0xC0 — transforms (arg3 - obj pos+0x10) into that frame
 * (out arg5), then composes with arg4's rotation (MatrixMultiplyVu0) and writes
 * the euler angles to arg6. Returns 1.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 84.51%
   -> UNKNOWN-@3: ROM `(none)` vs `daddu a0,s1,zero` */
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

/* Matrix helpers (sigs traced from func_002AE558's call registers). */
extern void func_00283DC0(Mat4x4 *dst, Vec4 *in);          /* build a rotation matrix from a vec (VU0) */
extern void MatrixMultiplyVu0(Mat4x4 *dst, Mat4x4 *a, Mat4x4 *b);
extern void MatrixToEulerAngles(Mat4x4 *mtx, void *outAngles);

/**
 * Compose two orientation matrices (from m1 and m2), convert the product to euler
 * angles written into `out`, then stash the source vectors: out+0x10 always gets
 * arg2, and out+0x20 gets m1 when out's flag word (+0x3C) has bit 0x2 set.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 51.55%
   -> UNKNOWN-@1: ROM `(none)` vs `sd s4,288(sp)` */
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
 * 75%: the original contains an EMPTY 28-iteration delay loop. Its four
 * nops are the R5900 short-loop pad (a 2-instruction loop padded to 6), not
 * scheduler output. A pad asm with a "+r" operand keeps the empty loop alive
 * under cc1 2.9, but reorg then does not move the loop's i++ into the bnez
 * slot the way the ROM does. The scan loop is the CountPlatinumBolts shape
 * (5 + 1 pad, movn in the slot). Task #659 best: 63.29% (sdk29, unit
 * objdiff report, VM b). Not landed. FACT #7937. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 52.76%
   -> UNKNOWN-@0: ROM `lui v0,0x0  [HI16 0x001A7BD0]` vs `lui v1,0x0  [HI16 0x001A7BD0]` */
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 38.18%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-48` vs `addiu sp,sp,-32` */
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

/**
 * func_002AE7E8 — map a moby to an announcer/category code by its class id.
 *
 * Params: moby — the moby to classify (reads oClass at +0xAA and the linked
 *         moby pointer at +0xB8).
 * Returns: a fixed code for the known class ids in the switch; otherwise the
 * equipped-weapon code (g_soundBankHandlesBlk +0x1248) when the moby, or its
 * linked moby, is the equipped-weapon moby (+0x1220); otherwise the first slot
 * i (0..0x37) whose weapon variant g_weaponTable[g_itemEquippedSlot[i]] has a
 * mobyClass equal to the moby's class or the linked moby's class; else 0xFF.
 *
 * MATCHED on the sdk29 arm (plain C; unit objdiff report via objdiff_build.sh +
 * unit_report.sh, 100.00%; task #758). The bracket is the row with ONLY that
 * lever reverted, everything else as written (same instrument, task #758):
 *  - the loop re-reads moby->oClass instead of reusing a cached class, so the
 *    switch value dies after the switch rather than living in a1 across the
 *    loop [96.88];
 *  - cases in the ROM's return-block order (0x29, 0x2A, 0x16, ... 0xE) [99.27
 *    in the earlier body's case order, 0xB2E first];
 *  - the switch on `(s16)moby->oClass`: the ROM sign-extends (lh, not lhu)
 *    [99.52];
 *  - the linked pointer is tested through its own copy (linkedTest) and read
 *    through the original: the ROM keeps both, a3 for the test and a2 for the
 *    read [98.08 with no copy; 99.92 with the roles reversed];
 *  - the 0xE0 stride is an explicit local set in the loop init ahead of that
 *    copy, the one spelling found that orders the stride load before the copy
 *    [98.40 as `&g_weaponTable[slot]`].
 * The mobyClass field is for readability only: reading +0x14 through a raw
 * offset is also 100.00.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 48.22%
   -> UNKNOWN-@0: ROM `lh v1,170(a0)` vs `lh a1,170(a0)` (measured on the earlier body) */
s32 func_002AE7E8(Moby *moby) {
    u8 *blk;
    void *equipped;
    Moby *linked;
    Moby *linkedTest;
    s32 stride;
    s32 i;

    switch ((s16)moby->oClass) {
    case 0xA66:
    case 0xA82: return 0x29;
    case 0xC07:
    case 0xCE4:
    case 0xCE5: return 0x2A;
    case 0x87A: return 0x16;
    case 0xD01: return 0x18;
    case 0xB2E: return 0x1D;
    case 0xE64:
    case 0xECE: return 0x25;
    case 0x5DA: return 0x1B;
    case 0x9A9:
    case 0xA9B: return 0x1C;
    case 0xB58:
    case 0xE79: return 0x20;
    case 0x79:  return 0xC;
    case 0xAC:  return 0xE;
    }

    blk = g_soundBankHandlesBlk;
    equipped = *(void **)(blk + 0x1220);
    if ((void *)moby == equipped) {
        return *(s32 *)(blk + 0x1248);
    }
    linked = *(Moby **)((u8 *)moby + 0xB8);
    if (linked != 0 && (void *)linked == equipped) {
        return *(s32 *)(blk + 0x1248);
    }

    for (i = 0, stride = 0xE0, linkedTest = linked; i < 0x38; i++) {
        WeaponVariant *variant =
            (WeaponVariant *)((u8 *)g_weaponTable + g_itemEquippedSlot[i] * stride);
        s32 weaponClass = variant->mobyClass;

        if ((s16)moby->oClass == weaponClass) {
            return i;
        }
        if (linkedTest != 0 && (s16)linked->oClass == weaponClass) {
            return i;
        }
    }
    return 0xFF;
}

/* t467 engine96 arm: NOT MEASURED — the #else body is named SpawnBoltShower while the
   asm/symbol is func_002AE9E0 (the bb754675 name-skew class, FACT #6402 defect 1), so a
   MATCH_func_002AE9E0 guard fails objdiff_build.sh's check 3 (no `.ent func_002AE9E0`).
   Rename the body (not the asm) before promoting. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE9E0);
#else
/* --- SpawnBoltShower (func_002AE9E0) faithful #else externs (guarded) ---------
 * Engine (ee-gcc 2.96) coverage body — functional, NOT a byte-match. The
 * matching INCLUDE_ASM arm above stays intact. These externs exist only for the
 * #else body, so they are guarded per the unit's #else-extern rule. */
extern u32  D_1A7A10;                  /* PlayerStats double-bolts word: (& 0xffff0000) != 0 -> 2x */
extern u8   D_1A7A13;                  /* cap-select byte: 0 -> cap 0x1d4fd396, else -0x1ab5ae28 */
extern u8   D_1A7A3A;                  /* capped-path remainder divisor byte (DAT_001a7a38._2_1_) */
extern u32  D_1A8C70;                  /* scatter-mode flag (moby+0xb0 -> (g_pSkyShellSpinRates+0x10)[b] >> 7) */
extern u8   g_pSkyShellSpinRates[];    /* +0x10 is the per-class scatter-mode table (DAT_001b1920) */
extern s32  g_health;                  /* base for the two per-planet economy tables below */
extern s32  g_playerProgress;          /* current-planet index (gp-rel), indexes the tables */
extern s32  g_pendingBoltCredit;       /* deferred bolt-credit sink for the capped remainder */
extern u32  D_001F0000[];              /* +0x1680 = nav-moby pointer table, indexed navTargetIdx*4 */

extern void SpawnBoltDenominationPickup(u64 moby, Vec4 *vel, Vec4 *pos, u32 flags,
                                        s32 denom, u32 mode);
extern void func_002CA3E8(s32 navTargetIdx, Vec4 *srcPos, Vec4 *dstPos, Vec4 *out);
extern s32  func_001234F0(f32 x);      /* float-format helper for DebugPrintStub */
extern void DebugPrintStub(s32 fmt, ...);

/**
 * SpawnBoltShower — the bolt-payout scatterer.
 *
 * Rolls a random bolt total in [minBolts, maxBolts], optionally doubled when the
 * PlayerStats double-bolts word (D_1A7A10 upper half) is set, then applies the
 * dynamic per-planet bolt-economy throttle: rate = earned[planet] * 3600 /
 * threshold[planet] (halved when double-bolts is on); a low rate multiplies the
 * total x5, a high rate scales it down toward /10. The (possibly throttled)
 * total is split into 1000/500/100/50/20/5/1 denomination counters — greedily
 * unless flag 8 is set, in which case the fill is capped at coinCap (per
 * denomination row) and the leftover is deferred into g_pendingBoltCredit.
 * Finally each counted coin is spawned with a scattered (and, when a nav target
 * is given, ballistically-aimed) velocity via SpawnBoltDenominationPickup.
 *
 * @param zStep1      per-coin world-Z step added to the spawn position
 * @param sourceMoby2 source moby (spawn origin; +0x10 world pos, +0xaa/+0xb0 class bytes)
 * @param minBolts3   lower bound of the rolled total (clamped to >= 1)
 * @param maxBolts4   upper bound of the rolled total
 * @param flags5      spawn flags: bit 3 = capped variant, bit 5 = suppress double-scatter
 * @param navTargetIdx6 nav-moby table index for ballistic aim (< 0 = no aim)
 * @param coinCap7    per-denomination fill cap for the capped variant
 */
void SpawnBoltShower(f32 zStep1, u64 sourceMoby2, long minBolts3, long maxBolts4,
                     u32 flags5, int navTargetIdx6, int coinCap7)
{
    Moby *moby = (Moby *)(long)(u32)sourceMoby2;
    int  navTargetIdx = navTargetIdx6;
    int  coinCap = coinCap7;
    u32  scatterMode;
    int  total;
    int  rate;

    /* the seven denomination counters (1000s .. 1s) */
    int n1000, n500, n100, n50, n20, n5, n1;

    /* scatter working buffers — kept distinct so the vel/pos pair computed early
     * survives the intervening writes and reaches SpawnBoltDenominationPickup. */
    Vec4 scatterAccum;   /* sp+0x00 : Vec4Add accumulator (nav-aim base) */
    Vec4 vel;            /* sp+0x10 : the coin velocity handed to the spawner */
    Vec4 step;           /* sp+0x20 : the pos/step vec handed to the spawner */
    Vec4 step2;          /* sp+0x30 : rescaled scatter offset added into vel */
    Vec4 ballisticPos;   /* sp+0x50 : ballistic aim work vector */
    Vec4 navWorldPos;    /* sp+0x60 : nav target world position */
    Vec4 navRescaled;    /* sp+0x70 : rescaled nav direction */
    f32  arc;            /* sp+0xb0 : ballistic-arc root out-param */
    f32  arcOut2;        /* sp+0xb4 : ballistic solver 2nd out-param (unused) */

    func_00283638((Moby *)&scatterAccum);   /* Vec4ZeroInt(scatterAccum) */
    n1000 = n500 = n100 = n50 = n20 = n5 = n1 = 0;

    /* --- scatter-mode select ------------------------------------------------ */
    if ((flags5 & 0x20) == 0 && *(u8 *)((u8 *)moby + 0xb0) != 0xff &&
        (u32)(*(u16 *)((u8 *)moby + 0xaa) - 500) > 0x28) {
        D_1A8C70 = (u32)((u8)(&g_pSkyShellSpinRates[0x10])[*(u8 *)((u8 *)moby + 0xb0)] >> 7);
    }
    scatterMode = 0;
    if (D_1A8C70 != 0) {
        scatterMode = (((s32)flags5 >> 5) ^ 1U) & 1;
    }

    /* --- guards ------------------------------------------------------------- */
    if (flags5 == 0) {
        return;
    }
    if (minBolts3 == 0 && maxBolts4 == 0) {
        return;
    }
    if (minBolts3 < 1) {
        minBolts3 = 1;
    }

    /* --- roll the total ----------------------------------------------------- */
    total = GetRandomInt(((int)maxBolts4 - (int)minBolts3) + 1) + (int)minBolts3;
    if ((D_1A7A10 & 0xffff0000) != 0) {
        total = total * 2;
    }

    /* --- economy throttle (small payouts only, when scatter-mode active) ----- */
    if (total < 500 && scatterMode != 0) {
        int threshold = *(int *)((u8 *)&g_health + 0xEAC + g_playerProgress * 4);
        if (threshold != 0) {
            rate = (*(int *)((u8 *)&g_health + 0xF1C + g_playerProgress * 4) * 0xE10) / threshold;
            if ((D_1A7A10 & 0xffff0000) != 0) {
                rate = rate / 2;
            }
            if (rate < 0x3BFE5913) {
                if (rate < 0xA6568A6) {
                    DebugPrintStub(0x1A9F58);
                    total = total * 5;
                } else {
                    DebugPrintStub(0x1A9F68, (-0xCEE480 - rate) / 0x7FFC79C);
                    total = (total * (-0xCEE480 - rate)) / 0x7FFC79C;
                }
                if (D_1A7A13 == 0) {
                    if (total >= 0x1D4FD397) {
                        total = 0x1D4FD396;
                    }
                } else {
                    if (total >= -0x1AB5AE27) {
                        total = -0x1AB5AE28;
                    }
                }
            } else if ((u32)rate > 0x77FCB227) {
                if (rate < 0xBFC252E) {
                    f32 shown = IntToFloat((-0x99D909E - rate) / -0x22303B1);
                    DebugPrintStub(0x1A9F78, func_001234F0(shown));
                    total = (total * (-0x99D909E - rate)) / -0x22303B1;
                } else {
                    /* format arg is the bit pattern 0x3fb99999a0000000 (double 0.1) */
                    DebugPrintStub(0x1A9F78, 0x3FB99999A0000000LL);
                    total = total / 10;
                }
                if (total < 1) {
                    total = 1;
                }
            }
        }
    }

    /* --- denomination split ------------------------------------------------- */
    if ((flags5 & 8) == 0) {
        /* greedy split */
        if (total > 999) { n1000 = total / 1000; total = total % 1000; }
        if (total > 499) { n500  = total / 500;  total = total % 500;  }
        if (total > 99)  { n100  = total / 100;  total = total % 100;  }
        if (total > 0x31){ n50   = total / 0x32; total = total % 0x32; }
        if (total > 0x13){ n20   = total / 0x14; total = total % 0x14; }
        if (total > 4)   { n5    = total / 5;    total = total % 5;    }
        n1 = total;
        /* negative-clamp guard (only entered if any counter went negative) */
        if (n1000 < 0 || n500 < 0 || n100 < 0 || n50 < 0 || n20 < 0 || n5 < 0 || total < 0) {
            DebugPrintStub(0x1A9F88);
            if (total < 0) total = 0;
            if (n1000 < 0) n1000 = 0;
            if (n500  < 0) n500  = 0;
            if (n100  < 0) n100  = 0;
            if (n5    < 0) n5    = 0;
            if (n50   < 0) n50   = 0;
            n1 = total;
            if (n20 < 0) n20 = 0;
        }
    } else {
        /* capped split: fill high->low but never exceed the remaining cap */
        int cap = coinCap;
        n1000 = cap;
        if (total / 1000 <= cap) { n1000 = total / 1000; }
        cap   = cap - n1000;
        total = total + n1000 * -1000;
        if (cap > 0) {
            n500 = total / 500;
            if (cap < total / 500) { n500 = cap; }
            cap   = cap - n500;
            total = total + n500 * -500;
            if (cap > 0) {
                n100 = total / 100;
                if (cap < total / 100) { n100 = cap; }
                cap   = cap - n100;
                total = total + n100 * -100;
                if (cap > 0) {
                    n50 = total / 0x32;
                    if (cap < total / 0x32) { n50 = cap; }
                    cap   = cap - n50;
                    total = total + n50 * -0x32;
                    if (cap > 0) {
                        n20 = total / 0x14;
                        if (cap < total / 0x14) { n20 = cap; }
                        cap   = cap - n20;
                        total = total + n20 * -0x14;
                        if (cap > 0) {
                            n5 = total / 5;
                            if (cap < total / 5) { n5 = cap; }
                            cap   = cap - n5;
                            total = total + n5 * -5;
                            if (cap > 0) {
                                n1 = total;
                                if (cap < total) { n1 = cap; }
                            }
                        }
                    }
                }
            }
        }
        if (D_1A7A3A != 0) {
            total = total / (int)(u32)D_1A7A3A;
        }
        g_pendingBoltCredit = g_pendingBoltCredit + total;
    }

    /* --- spawn loop: emit each counted coin with a scattered velocity ------- */
    while (n1 != 0 || n5 != 0 || n20 != 0 || n50 != 0 || n100 != 0 || n500 != 0 || n1000 != 0) {
        f32 angle;
        f32 rnd;

        Vec4ScaleVu0(&vel, 0.0009765625f, (const Vec4 *)moby); /* 0x3A800000 = 1/1024; src = moby base ($5=$22 @ .L002AF16C) */
        angle = GetRandomAngle();

        rnd = GetRandomFloatRange(0.0f, 3.0f);
        step.x = func_00283B30(angle) * 0.016666668f * rnd;

        rnd = GetRandomFloatRange(0.0f, 3.0f);
        step.y = func_00283B48(angle) * 0.016666668f * rnd;

        step.z = GetRandomFloatRange(3.7f, 6.0f) * 0.016666668f;

        step2.x = step.x;
        step2.y = step.y;
        step2.z = 0.0f;   /* w-lane hole zeroed like the .s (uStack_e8 = 0) */
        Vec3RescaleToLenVu0(&step2, 0.5f, &step2);       /* 0x3F000000 = rescale to length 0.5 */
        Vec4AddVu0(&vel, &vel, &step2);

        vel.z = vel.z + zStep1;    /* vel.z (sp+0x18) += the per-coin z step */

        if (navTargetIdx >= 0) {
            /* ballistic aim: solve the arc, clamp to [.. ,120] */
            int nRoots;
            f32 flightTime = 120.0f;
            union { u32 u; f32 f; } gravity;
            gravity.u = 0xBAC49BA6;   /* bit-exact f32 == -0.0015 (the gravity coeff) */
            /* .s uses the SAME 0xBAC49BA6 for arg a AND for the step.z subtraction
             * (Ghidra renders it once as raw hex, once as "-0.0015"). */
            nRoots = func_002A9708(gravity.f, step.z - gravity.f,
                                   vel.z - *(f32 *)(D_001F0000[0x1680 / 4 + navTargetIdx] + 0x18),
                                   &arc, &arcOut2);
            if ((f32)nRoots < 1.0f || (0.0f < arc && (flightTime = arc, 120.0f <= arc))) {
                flightTime = 120.0f;
            }
            Vec4ScaleVu0(&ballisticPos, flightTime, &step);
            Vec4AddVu0(&ballisticPos, &ballisticPos, (Vec4 *)((u8 *)moby + 0x10));
            func_002CA3E8(navTargetIdx, (Vec4 *)((u8 *)moby + 0x10), &ballisticPos, &navWorldPos);
            Vec4SubVu0(&ballisticPos, &navWorldPos, &vel);
            ballisticPos.z = 0.0f;   /* sw $0,0x48($sp): zero z of rescale source before the Vec3 rescale */
            Vec3RescaleToLenVu0(&navRescaled, 0.25f, &ballisticPos);   /* 0x3E800000 */
            Vec4SubVu0(&ballisticPos, &ballisticPos, &navRescaled);
            Vec4ScaleVu0(&ballisticPos, 1.0f / flightTime, &ballisticPos);
            step.x = ballisticPos.x;
            step.y = ballisticPos.y;
        }

        Vec4AddVu0(&step, &step, &scatterAccum);

        /* highest nonzero denomination first */
        if (n1000 != 0) {
            SpawnBoltDenominationPickup(sourceMoby2, &vel, &step, flags5, 1000, scatterMode);
            n1000 = n1000 - 1;
        } else if (n500 != 0) {
            SpawnBoltDenominationPickup(sourceMoby2, &vel, &step, flags5, 500, scatterMode);
            n500 = n500 - 1;
        } else if (n100 != 0) {
            SpawnBoltDenominationPickup(sourceMoby2, &vel, &step, flags5, 100, scatterMode);
            n100 = n100 - 1;
        } else if (n50 != 0) {
            SpawnBoltDenominationPickup(sourceMoby2, &vel, &step, flags5, 0x32, scatterMode);
            n50 = n50 - 1;
        } else if (n20 != 0) {
            SpawnBoltDenominationPickup(sourceMoby2, &vel, &step, flags5, 0x14, scatterMode);
            n20 = n20 - 1;
        } else if (n5 != 0) {
            SpawnBoltDenominationPickup(sourceMoby2, &vel, &step, flags5, 5, scatterMode);
            n5 = n5 - 1;
        } else {
            SpawnBoltDenominationPickup(sourceMoby2, &vel, &step, flags5, 1, scatterMode);
            n1 = n1 - 1;
        }
    }
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF590);

extern s32 func_00283AB8(Vec4 *v);   /* pack a float {x,y,z,scale} vec into a word */

/**
 * Quantise a vector into a packed word written to *out. Scales the vector so its
 * largest-magnitude component maps to a chosen precision: takes maxAbs of x/y/z,
 * derives an integer scale = clamp(trunc(maxAbs * 158.73), 1, 255) (FloatToInt
 * truncates toward zero, not rounds), normalises
 * each component by 1/(scale*1e-4) and biases by 127, then packs {x',y',z',scale}
 * via func_00283AB8.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 52.27%
   -> UNKNOWN-@2: ROM `(none)` vs `daddu s0,a0,zero` */
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

/* Unpack a packed 32-bit RGBA colour to a Vec4 of floats (pextlb/pextlh + itof). */
extern void func_00283AA0(Vec4 *dst, u32 packed);

/**
 * Decode a packed RGBA colour pointed to by `colorPtr` into a signed direction/
 * offset vector scaled by its alpha: unpack to floats, recentre RGB around 127
 * (so 0x80 -> 0), and scale the whole vector by alpha * 1e-4. Writes to `out`.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 76.27%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-64` vs `(none)` */
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

/* func_002AF728: per-frame wander/approach AI step for a moby with a control
 * block `ctrl`. When the wander phase (ctrl+0x28) is idle, pick a fresh heading
 * jitter (ctrl+0x24) and a random dwell timer (ctrl+0x2A). Otherwise step the
 * heading toward the target (func_002AB668) and tick the dwell timer
 * (func_00283328), clearing the phase when it expires. Each frame: advance a
 * probe point from the moby position along the current yaw (ctrl+0xF8) by
 * ctrl+0x14, resolve it against the world (func_002A8D08 vertical sweep), snap
 * the moby ground height (ProbeGroundHeight). If unobstructed OR out of leash
 * range (ctrl+0x1C), re-aim at the target (ctrl+0x0/+0x4) and re-roll the timer;
 * else, when within hero range (ctrl+0x20), steer the heading toward the hero
 * (func_002AAFB8 over the hero bearing, weighted by the normalized hero
 * distance). Uses func_002AAFB8's guarded f32 return (see above). */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 60.01%
   -> UNKNOWN-@2: ROM `(none)` vs `daddu s1,a1,zero` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF728);
#else
extern f32 GetRandomFloatSigned(f32 lo, f32 hi);   /* 0x2A8740 signed random magnitude */
extern s32 RandRangeInclusive(s32 lo, s32 hi);     /* 0x2A8688 random int in [lo,hi] */
extern Vec4 g_heroPos;                             /* 0x189EA0 hero world position */

void func_002AF728(Moby *moby, void *ctrlPtr, f32 stepZ, f32 snapEps) {
    u8   *m = (u8 *)moby;
    u8   *c = (u8 *)ctrlPtr;
    Vec4 *mpos = (Vec4 *)(m + 0x10);
    Vec4  probe;
    s32   blocked;
    f32   dist;

    if (*(s16 *)(c + 0x28) == 0) {
        /* idle: choose a new heading jitter + dwell timer */
        /* Both spelled from the ROM bits: the .s materialises 0x3F490FDC and
           0x40278D37. Tidier decimals land one ULP low on the first, and the
           second was wrong by ~1647 ULP -- its EU twin already carried the
           correct value. */
        *(f32 *)(c + 0x24) += GetRandomFloatSigned(0.78539824f, 2.6179941f);
        *(s16 *)(c + 0x2A) = (s16)RandRangeInclusive(*(s16 *)(c + 0x2C), *(s16 *)(c + 0x2E));
        *(s16 *)(c + 0x28) = 1;
    } else {
        /* active: converge the heading + tick the dwell timer */
        func_002AB668(*(f32 *)(c + 0x24), *(f32 *)(c + 0x18), (f32 *)(m + 0xF8), 0);
        if (func_00283328(c + 0x2A) != 0) {
            *(s16 *)(c + 0x28) = 0;
        }
    }

    /* advance the probe point along the current yaw and resolve it */
    probe = *mpos;
    probe.x += func_00283B30(*(f32 *)(m + 0xF8)) * *(f32 *)(c + 0x14);
    probe.y += func_00283B48(*(f32 *)(m + 0xF8)) * *(f32 *)(c + 0x14);
    /* 0.52359885f is 0x3F060A93, the value the .s materialises; the tidier
       pi/6 spelling lands one ULP low. */
    blocked = func_002A8D08(moby, mpos, &probe, 0, stepZ, *(f32 *)(c + 0x10), snapEps, 0.52359885f);
    *(f32 *)(m + 0x18) = ProbeGroundHeight(mpos, 0.5f, 0);
    dist = DistXYVu0(mpos, (Vec4 *)c);

    if (blocked == 0 || *(f32 *)(c + 0x1C) < dist) {
        /* clear path or beyond leash: re-aim at the target and re-roll the timer */
        *(f32 *)(c + 0x24) = Atan2fPoly(*(f32 *)(c + 0x0) - *(f32 *)(m + 0x10),
                                           *(f32 *)(c + 0x4) - *(f32 *)(m + 0x14));
        *(s16 *)(c + 0x2A) = (s16)RandRangeInclusive(0x1E, 0x5A);
        *(s16 *)(c + 0x28) = 1;
    } else {
        /* blocked and within leash: steer toward the hero when close enough */
        f32 heroDist = DistXYVu0(mpos, &g_heroPos);
        if (heroDist < *(f32 *)(c + 0x20)) {
            f32 bearing = Atan2fPoly(*(f32 *)(m + 0x10) - g_heroPos.x,
                                        *(f32 *)(m + 0x14) - g_heroPos.y);
            *(f32 *)(c + 0x24) = func_002AAFB8(*(f32 *)(c + 0x24), bearing,
                                              heroDist / *(f32 *)(c + 0x20));
        }
    }
}
#endif

/* func_002AF948: round a float to `digits` decimal places. Builds the scale
 * 10^digits (digits<=0 -> 1), adds the half-ulp rounding bias 1/(2*scale),
 * truncates (FloatToInt) the scaled value, and divides back.
 *   digits - decimal places (<= 0 means round to an integer)
 *   x      - value to round
 *   returns trunc((x + 1/(2*scale)) * scale) / scale
 * Not a save-layout wall: cc1 2.9 emits the ROM's ra + $f20 frame word for
 * word (FACT #8260). Best sdk29 body before task #1026 was t894's 85.62%
 * (NOTE #8262); task #946 left two residuals, both now closed:
 *  - Loop: the ROM's `addiu; nop x4; bnez; mult` is the R5900 short-loop pad
 *    with `mult` in the slot. cc1 2.9 pads a short loop itself and those
 *    pads block slot filling; written as four counted pad statements plus an
 *    EMPTY operand-tied fence (emits nothing, RULING #8483) the loop crosses
 *    cc1's short-loop threshold, cc1 adds no pad of its own and reorg puts
 *    the `mult` in the slot (FACT #8384). Each pad reads pow10, so the
 *    multiply stays after the pads.
 *  - Tail: the ROM's nop between `ld $31` and `div.s` is written as
 *    AF948_FPU_PAD, an explicit `noreorder` nop tied to the converted value
 *    (a SCHEDULING DEVICE under RULING #8435: EE arm only, empty natively).
 * The two mtc1 -> cvt.s.w nops are asm_unit.sh's SN-as mtc1 hazard rule.
 * Reusing `x` for the truncated value puts it in $f12 as the ROM has it, and
 * the literal 10 lets loop.c hoist it into the guarded preheader. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 73.00%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-32` vs `addiu sp,sp,-16` */
#ifndef TARGET_NATIVE
#define AF948_FPU_PAD(v) __asm__(".set noreorder\n\tnop\n\t.set reorder" : "+f"(v))
#else
#define AF948_FPU_PAD(v) ((void)0)
#endif
extern s32 FloatToInt(f32 x);

f32 func_002AF948(s32 digits, f32 x) {
    s32 pow10 = 1;
    f32 scale;

    if (digits > 0) {
        do {
            digits = digits - 1;
            __asm__("" : "+r"(digits));
            R5900_SHORT_LOOP_PAD1(digits, pow10);
            R5900_SHORT_LOOP_PAD1(digits, pow10);
            R5900_SHORT_LOOP_PAD1(digits, pow10);
            R5900_SHORT_LOOP_PAD1(digits, pow10);
            pow10 = pow10 * 10;
        } while (digits != 0);
    }
    scale = (f32)pow10;
    x = x + 1.0f / (scale + scale);
    x = (f32)FloatToInt(x * scale);
    AF948_FPU_PAD(x);
    return x / scale;
}

/**
 * Test whether `x` sits just below the object's reference value: returns 1 when
 * x <= round(refValue, 4) AND the gap (rounded - x) is smaller than
 * round(obj->field48 * 0.5, 4); otherwise 0. Both quantities are rounded to 4
 * decimal places via func_002AF948. refValue = func_002A0368(obj).
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 78.42%
   -> UNKNOWN-@2: ROM `swc1 $f21,24(sp)` vs `(none)` */
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

/**
 * Advance an object's eased-rotation driver one integration step and (re)bind it
 * to its owning controller.
 *
 * `state` is the driver block embedded in the animated object:
 *   +0x01  u8    registered flag (nonzero once bound into a controller list)
 *   +0x10  Vec4  orientation quaternion (rebuilt from the stepped eulers)
 *   +0x20  f32x3 per-axis blend weight (fanned from the current +0x70 weight)
 *   +0x40  f32x3 eased euler angles      (the integrator's accumulators)
 *   +0x50  f32x3 per-axis angular velocity
 *   +0x60  f32x3 target euler angles     (consumed: the quad is zeroed after use)
 *   +0x70  f32   blend weight (latched to 1.0 once a step runs)
 *   +0x78  void* current owning controller
 *
 * The block is first re-keyed to `owner`: if it was bound to a different live
 * controller it is detached (func_002A0828) when still flagged and that owner is
 * accepting (its +0x20 byte < 0x7F). If the target orientation is already the
 * identity (zero targets, unit weight, sub-0.005 residual eulers) the driver is
 * released and the call returns. Otherwise each euler axis is stepped toward its
 * target by the critically-damped driver func_002AB700 (rate/cap in $f12/$f13),
 * the block is registered (func_002A07B0) if it was not already, the orientation
 * quaternion at +0x10 is rebuilt from the stepped eulers, the blend weight is
 * fanned into +0x20/+0x24/+0x28, the target quad is cleared, and the weight is
 * latched to 1.0.
 *
 * owner/arg3 are opaque controller handles (UNCONFIRMED — passed straight to the
 * register/deregister helpers in the 1A00F0 unit).
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 76.91%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-80` vs `addiu sp,sp,-64` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFAB0);
#else
/* Controller driver-list bind/unbind helpers (1A00F0 unit). UNCONFIRMED names:
 * func_002A0828 detaches `state` from an owner; func_002A07B0 attaches it. */
extern void func_002A0828(void *owner, void *state);
extern void func_002A07B0(void *owner, void *arg3, void *state);

void func_002AFAB0(void *owner, u8 *state, void *arg3, f32 rate, f32 cap) {
    void *curOwner = *(void **)(state + 0x78);
    f32 weight;

    /* Re-key the driver to `owner`, detaching from a live prior controller. */
    if (curOwner != owner) {
        if (curOwner != 0) {
            if (state[1] != 0 && *(u8 *)((u8 *)curOwner + 0x20) < 0x7F) {
                func_002A0828(curOwner, state);
            }
            state[1] = 0;
        }
        *(void **)(state + 0x78) = owner;
    }

    /* Already at the identity target: release and bail. */
    if (*(f32 *)(state + 0x60) == 0.0f &&
        *(f32 *)(state + 0x64) == 0.0f &&
        *(f32 *)(state + 0x68) == 0.0f &&
        *(f32 *)(state + 0x70) == 1.0f &&
        GetFloatAbs(*(f32 *)(state + 0x40)) < 0.005f &&
        GetFloatAbs(*(f32 *)(state + 0x44)) < 0.005f &&
        GetFloatAbs(*(f32 *)(state + 0x48)) < 0.005f) {
        if (state[1] != 0) {
            func_002A0828(owner, state);
        }
        __asm__ __volatile__(""); /* cc1 2.96 sibling-call suppression (the ROM never sibcalls) */
        return;
    }

    /* Step each euler axis toward its target with the damped angle driver. */
    func_002AB700((f32 *)(state + 0x40), (f32 *)(state + 0x50), 0, *(f32 *)(state + 0x60), rate, cap, 0.0f);
    func_002AB700((f32 *)(state + 0x44), (f32 *)(state + 0x54), 0, *(f32 *)(state + 0x64), rate, cap, 0.0f);
    func_002AB700((f32 *)(state + 0x48), (f32 *)(state + 0x58), 0, *(f32 *)(state + 0x68), rate, cap, 0.0f);

    if (state[1] == 0) {
        func_002A07B0(owner, arg3, state);
    }

    /* Rebuild the orientation quaternion from the stepped eulers, fan the blend
     * weight across +0x20/+0x24/+0x28, clear the consumed target quad, latch. */
    func_002AA058((Vec4 *)(state + 0x10), (Vec4 *)(state + 0x40));

    weight = *(f32 *)(state + 0x70);
    *(f32 *)(state + 0x20) = weight;
    *(f32 *)(state + 0x24) = weight;
    *(f32 *)(state + 0x28) = weight;

    func_00283638((Moby *)(state + 0x60));   /* zero the 16-byte target quad */
    *(f32 *)(state + 0x70) = 1.0f;
}
#endif

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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 68.33%
   -> UNKNOWN-@4: ROM `(none)` vs `c.lt.s $f0,$f20` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFCD8);
#else
void func_002AFCD8(void *out, f32 *p1, f32 *p2, f32 a, f32 b, f32 c) {
    f32 *offset = (f32 *)((u8 *)out + 0x18);

    if (c > 0.0f) {
        *p1 = WrapAnglePiSum(*p1, b);          /* WrapAnglePiSum */
        *offset = func_00283B48(*p1) * a + c;
    } else {
        *p1 = WrapAnglePiSum(*p1, b);
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 52.78%
   -> UNKNOWN-@2: ROM `(none)` vs `daddu s0,a1,zero` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFD90);
#else
void func_002AFD90(void *out, f32 *p1, f32 *p2, f32 scale, f32 b, f32 c) {
    f32 sinP1 = func_00283B48(*p1);

    *(f32 *)((u8 *)out + 0xF0) = scale * sinP1 * func_00283B48(*p2);
    *(f32 *)((u8 *)out + 0xF4) = scale * func_00283B48(*p1) * func_00283B30(*p2);
    *p1 = WrapAnglePiSum(*p1, b);              /* WrapAnglePiSum */
    *p2 = WrapAnglePiSum(*p2, c);
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFE58);

/**
 * Convert spherical coordinates (radius, azimuth, elevation) to a cartesian
 * vector in `dst`: x = r·cos(az)·cos(el), y = r·sin(az)·cos(el), z = r·sin(el).
 * (func_00283B30 = cosine, func_00283B48 = sine.)
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 73.79%
   -> UNKNOWN-@2: ROM `(none)` vs `mov.s $f23,$f13` */
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
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFF08);

extern u8 D_26CB10[];   /* 0x54-byte config table snapshotted per call */
extern s32 func_002B1880(s32 stringId, s32 arg);              /* defined later this unit */
extern s32 func_002B1B48(void *subject, s32 stringId, s32 arg2);

/**
 * Snapshot the D_26CB10 config table (0x54 bytes) into a local, clamp the index
 * `sel` to [0, 0x14], then dispatch: sel==3 shows the localized string at
 * table+0xC (func_002B1880), otherwise runs the notice/prompt setup with the
 * string id at table+sel*4 (func_002B1B48). Returns the dispatched call's result.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 67.55%
   -> UNKNOWN-@0: ROM `lui v0,0x0  [HI16 D_26CB10]` vs `lui v1,0x0  [HI16 D_26CB10]` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFF10);
#else
s32 func_002AFF10(s32 sel) {
    u8 table[0x54];
    s32 idx;

    memcpy(table, D_26CB10, 0x54);
    idx = (sel > -1) ? sel : 0;
    if (idx >= 0x15) {
        idx = 0x14;
    }
    if (idx == 3) {
        return func_002B1880(*(s32 *)(table + 0xC), 0xF0);
    }
    return func_002B1B48((void *)8, *(s32 *)(table + idx * 4), 0xB4);
}
#endif

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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 81.33%
   -> UNKNOWN-@2: ROM `(none)` vs `daddu s1,a1,zero` */
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

/* func_002B0150 (this unit): score a candidate `moby` (position at +0x10) against
 * a query point. Samples Vec3DistVu0(query, mobyPos); flags the candidate
 * (*outFlag=1) when b exceeds that sample. Then builds two drive-heading angles
 * (AngleAbsDiffPi over the planar bearing Atan2fPoly(dx,dy) and over the
 * XY-distance-vs-dz bearing) and flags again when 0<c<heading1 or 0<d<heading2.
 * Returns sample*(1+heading1), plus 8.0 when the moby is a valid class-filtered
 * entry (func_002AC9E0). (Un-parked: AngleAbsDiffPi CONFIRMED f32(f32,f32)
 * drive-heading helper — decompiled in 183558.c, used by 1B4218.c.) */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 78.04%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-96` vs `addiu sp,sp,-80` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0150);
#else
extern f32 AngleAbsDiffPi(f32 a, f32 b);   /* 0x284630 drive-heading angle helper (CONFIRMED) */

f32 func_002B0150(Vec4 *query, Moby *moby, s32 *outFlag, f32 a, f32 b, f32 c, f32 d) {
    f32 *mpos = (f32 *)((u8 *)moby + 0x10);   /* moby position Vec4 */
    f32 sample = Vec3DistVu0(query, mpos);
    f32 heading1, heading2, base;

    *outFlag = 0;
    if (b < sample) {
        *outFlag = 1;
    }

    heading1 = AngleAbsDiffPi(Atan2fPoly(mpos[0] - query->x, mpos[1] - query->y), a);
    heading2 = AngleAbsDiffPi(Atan2fPoly(DistXYVu0(query, (Vec4 *)mpos),
                                           mpos[2] - query->z), 0.0f);

    if (0.0f < c && c < heading1) {
        *outFlag = 1;
    }
    if (0.0f < d && d < heading2) {
        *outFlag = 1;
    }

    base = sample + heading1 * sample;
    if (func_002AC9E0(moby) != 0) {
        base += 8.0f;
    }
    return base;
}
#endif
extern f32 func_002B0150(Vec4 *query, Moby *moby, s32 *outFlag, f32 a, f32 b, f32 c, f32 d);
extern Moby *g_mobyFlagged1000List[];   /* null-terminated array of flagged mobys */

/**
 * Pick the nearest valid moby to `queryVec` from the flagged-moby list. For each
 * entry: skip if it has no pvar block (func_002AC058 == 0) or its word0 float is
 * 0, then score it with func_002B0150 (skip on its reject flag). Track the moby
 * with the smallest score. The list cursor only advances on a skip — a scored
 * entry re-reads the same slot (func_002B0150 consumes/compacts it), mirroring
 * the original's loop exactly. Returns the best moby, or NULL if none.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 62.45%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-128` vs `addiu sp,sp,-112` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B02C8);
#else
Moby *func_002B02C8(Vec4 *queryVec, f32 a, f32 b, f32 c, f32 d) {
    Moby **cursor = g_mobyFlagged1000List;
    Moby *moby = *cursor;
    Moby *best = 0;
    f32 bestScore = 100000000.0f;  /* 0x4CBEBC20 = 1e8 EXACTLY. NOT 99999008.0f:
                                    * that is the decimal a float32 PRINTS as,
                                    * and re-encoding it lands 124 ULP LOW at
                                    * 0x4CBEBBA4. Spell the round number; never
                                    * transcribe a printed approximation. */
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

/**
 * SelectLockOnTarget — the moby lock-on / auto-aim target selector.
 *
 * Walks the per-frame flagged-moby list (g_mobyFlagged1000List), and for every
 * candidate that (a) is combat-alive, (b) survives the gate12 predicate
 * (func_002AC9E0), (c) is not on either exclusion list (by moby ptr or by class
 * id) and (d) sits inside an aim cone, computes an angular+distance cost. The
 * candidate is confirmed by a line-of-sight ray (CollLine) — with a special
 * class-0xC20 / class-filter carve-out when the player is in progress-state 0xB.
 * Every survivor is appended to the out-list (*pOutCount/pOutList, capped at
 * maxOut). Finally the lowest-cost survivor is moved to the front of the
 * out-list. Returns the best target moby (0 if none).
 *
 * Params (5 floats then 8 int/ptr, per the EABI $f12-$f16 / $4-$11 split):
 *   enable1     master enable + the "reference cone half-angle" scalar; 0 -> no-op.
 *   coneYaw2    yaw cone half-angle (radians); 0 -> no-op.
 *   range3      max target distance; 0 -> no-op.
 *   conePitch4  pitch cone half-angle (radians).
 *   innerAngle5 inner (tight) cone half-angle used near-field.
 *   searchOrigin  aim ray origin Vec4 (eye/muzzle).
 *   aimDir      aim direction Vec4.
 *   pOutCount   out: number of survivors written to pOutList.
 *   pOutList    out: survivor moby-ptr array.
 *   maxOut      capacity of pOutList.
 *   excludeMobyList  0-terminated moby-ptr blacklist (or 0).
 *   gate12      passed to func_002AC9E0 (predicate mode); also selects the two
 *               scoring methods below.
 *   excludeClassList  (-1)-terminated class-id blacklist (or 0).
 *
 * Two scoring methods, selected by D_1A8CA0 (alt-gravity / pause flag):
 *   Method A (D_1A8CA0 == 0): planar-bearing method — Atan2fPoly on the xy/z
 *       deltas of candidate-vs-aim, differenced by AngleShortestDiff.
 *   Method B (D_1A8CA0 != 0): plane-projection method — build a gravity plane
 *       basis (func_002B0E40), project candToOrigin and aimDir into it, and take
 *       the between-vector angle via acos (func_00283B60, as pi/2 - acos = asin).
 *   Both methods are only entered when enable1 < pi OR coneYaw2 < pi; otherwise
 *   the angle terms stay 0 and only the distance term scores (LAB_002b09a0).
 *
 * NOTE: not byte-matched (engine ee-gcc 2.96 region) — this is the faithful
 * op-for-op #else coverage body; the matching arm above keeps its asm include.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 57.22%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-384` vs `(none)` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B03E8);
#else
extern f32 func_00283B60(f32 x);               /* acos */
extern f32 AngleAbsDiffPi(f32 a, f32 b);        /* AngleShortestDiff */
extern s32 func_002AC9E0(Moby *m);             /* gate predicate (class filter) */
extern s32 func_00301370(s32 classId);         /* class-id accept filter */
extern s32 g_playerProgress;                   /* persistent progress counter */
extern Moby *g_pCollHitMoby;                   /* moby hit by the last CollLine */
extern s16 D_0018a168;                         /* first-person flag (g_heroFacingDir + 0x88) */

int func_002B03E8(f32 enable1, f32 coneYaw2, f32 range3, f32 conePitch4,
                  f32 innerAngle5, void *searchOrigin, void *aimDir,
                  int *pOutCount, int *pOutList, int maxOut,
                  int *excludeMobyList, int gate12, int *excludeClassList) {
    Vec4 candPos;          /* sp+0x00 (uStack_180): candidate world position, adjusted */
    Vec4 scratchA;         /* sp+0x10 (auStack_170): Vec4Scale scratch (method B / axis nudge) */
    Vec4 candToOrigin;     /* sp+0x20 (auStack_160): candPos - searchOrigin */
    Vec4 planeNormal;      /* sp+0x30 (auStack_150): method-B gravity-plane normal */
    Vec4 scratchB;         /* sp+0x40 (auStack_140): Vec4Scale scratch (method B) */
    Vec4 tanCand;          /* sp+0x50 (auStack_130): candToOrigin projected into plane */
    Vec4 scratchC;         /* sp+0x60 (auStack_120): normal * dot(normal, aimDir) */
    Vec4 tanAim;           /* sp+0x70 (auStack_110): aimDir projected into plane */
    Vec4 scratchD;         /* sp+0x80 (auStack_100): reconstructed aim-in-plane vector */

    Moby *moby;
    Moby *best = 0;
    f32 bestScore = 999999.0f;   /* 0x497423F0 */
    s32 index = 0;               /* iVar7 (alive-candidate counter) */
    s32 nextIdx = 0;             /* iStack_c0 (0xC0): list index of the NEXT candidate */
    s32 gatePass;                /* func_002AC9E0 result (lVar4) */
    f32 *combat;                 /* GetMobyCombatState block (pfVar8) */

    *pOutCount = 0;
    if (enable1 == 0.0f) {
        return 0;
    }
    if (coneYaw2 == 0.0f) {
        return 0;
    }
    if (range3 == 0.0f) {
        return 0;
    }

    moby = (Moby *)g_mobyFlagged1000List[0];
    if (moby != 0) {
        char alive = *(char *)((u8 *)moby + 0x31);

        do {
            nextIdx = index + 1;
            if (alive != '\0') {
                s32 state;

                nextIdx = index + 1;
                gatePass = func_002AC9E0(moby);
                if ((gatePass != 0) && (gate12 == 0)) {
                    goto advance;
                }

                nextIdx = index + 1;
                state = func_002AC058(moby);
                if (state == 0) {
                    goto advance;
                }
                combat = (f32 *)state;
                if ((*combat <= 0.0f) && (gatePass == 0)) {
                    goto advance;
                }

                /* class-5 candidates skip the gate-null reject; both accepted
                 * paths advance the list index by one (via nextIdx = iStack_c0). */
                if ((moby != 0) && (*(int *)((u8 *)moby + 0x24) != 0) &&
                    (*(short *)(*(int *)((u8 *)moby + 0x24) + 0x46) == 5)) {
                    nextIdx = index + 1;
                } else {
                    nextIdx = index + 1;
                    if (gatePass == 0) {
                        goto advance;
                    }
                    nextIdx = index + 1;
                }

                /* exclusion list #1: moby pointer */
                if (excludeMobyList != 0) {
                    s32 slot = 0;
                    int *cursor = excludeMobyList;
                    int ent = *cursor;
                    while (ent != 0) {
                        if ((int)moby == ent) {
                            if (excludeMobyList[slot] != 0) {
                                goto advance;
                            }
                            break;
                        }
                        cursor++;
                        slot++;
                        ent = *cursor;
                    }
                }

                /* exclusion list #2: class id */
                if (excludeClassList != 0) {
                    s32 want = *excludeClassList;
                    s32 slot = 0;
                    if (want != -1) {
                        int *cursor = excludeClassList;
                        do {
                            if (*(short *)((u8 *)moby + 0xAA) == want) {
                                if (excludeClassList[slot] != -1) {
                                    goto advance;
                                }
                                break;
                            }
                            cursor++;
                            want = *cursor;
                            slot++;
                        } while (want != -1);
                    }
                }

                /* candidate world position (moby +0x10), z-adjusted by the combat
                 * state's +0x10 height along the moby axis (or straight up). */
                candPos = *(Vec4 *)((u8 *)moby + 0x10);
                if ((D_1A8CA0 == 0) && (D_0018a168 == 0)) {
                    candPos.z = candPos.z + combat[4];
                } else {
                    Vec4ScaleVu0(&scratchA, combat[4], (Vec4 *)((u8 *)moby + 0xE0));
                    Vec4AddVu0(&candPos, &candPos, &scratchA);
                }

                Vec4SubVu0(&candToOrigin, &candPos, (Vec4 *)searchOrigin);
                {
                    f32 dist = Vec3LengthVu0(&candToOrigin);
                    if ((dist <= range3) && (0.1f <= dist)) {
                        f32 angleH = 0.0f;      /* fVar19 / $f22 */
                        f32 angleV = 0.0f;      /* fVar18 / $f23 */
                        f32 conePitchN = 0.0f;  /* fVar20 / $f27-seed */
                        f32 t = func_002A8910(-2.0f, 0.0f, 1.0f, 0.0f, dist / range3);
                        f32 coneYawBase;        /* $f1 carried into the join */
                        int pitchWide;          /* bVar2 carried across the goto */

                        coneYawBase = coneYaw2;
                        if ((enable1 < 3.141593f) || (coneYaw2 < 3.141593f)) {
                            if (D_1A8CA0 == 0) {
                                /* METHOD A: planar bearing via atan2 + shortest-diff */
                                f32 a0 = Atan2fPoly(candToOrigin.x, candToOrigin.y);
                                f32 a1 = Atan2fPoly(((f32 *)aimDir)[0], ((f32 *)aimDir)[1]);
                                f32 innerN;
                                angleH = AngleAbsDiffPi(a0, a1);

                                innerN = enable1;
                                if (angleV < innerAngle5) {
                                    innerN = innerAngle5 + (enable1 - innerAngle5) * t;
                                }
                                if (angleH < innerN) {
                                    f32 b0 = Atan2fPoly(Vec2LengthXyVu0(&candToOrigin),
                                                           candToOrigin.z);
                                    f32 b1 = Atan2fPoly(Vec2LengthXyVu0((Vec4 *)aimDir),
                                                           ((f32 *)aimDir)[2]);
                                    angleV = AngleAbsDiffPi(b0, b1);
                                    pitchWide = conePitchN < conePitch4;
                                    coneYawBase = coneYaw2;   /* $f1 = coneYaw2 */
                                    goto join097c;
                                }
                            } else {
                                /* METHOD B: gravity-plane projection + acos angle */
                                f32 lenTanCand;   /* $f21 */
                                f32 lenTanAim;    /* $f20 */
                                f32 between;      /* $f22 */
                                f32 innerN;

                                func_002B0E40((Vec4 *)searchOrigin, &planeNormal, 1);
                                Vec4ScaleVu0(&scratchB, Vec3DotVu0(&planeNormal, &candToOrigin),
                                             &planeNormal);
                                Vec4SubVu0(&tanCand, &candToOrigin, &scratchB);
                                Vec4ScaleVu0(&scratchC, Vec3DotVu0(&planeNormal, (Vec4 *)aimDir),
                                             &planeNormal);
                                Vec4SubVu0(&tanAim, (Vec4 *)aimDir, &scratchC);
                                between = Vec3DotVu0(&tanAim, &tanCand);
                                if (angleV <= between) {
                                    lenTanCand = Vec3LengthVu0(&tanCand);
                                    if (lenTanCand != angleV) {
                                        lenTanAim = Vec3LengthVu0(&tanAim);
                                        if (lenTanAim != angleV) {
                                            angleH = 1.5707964f -
                                                     func_00283B60(between /
                                                                   (lenTanCand * lenTanAim));
                                            innerN = enable1;
                                            if (angleV < innerAngle5) {
                                                innerN = innerAngle5 +
                                                         (enable1 - innerAngle5) * t;
                                            }
                                            if (angleH < innerN) {
                                                f32 mag, projLen;
                                                Vec4ScaleVu0(&scratchD, lenTanAim / lenTanCand,
                                                             &tanCand);
                                                Vec4AddVu0(&scratchD, &scratchD, &scratchC);
                                                projLen = Vec3LengthVu0(&scratchD);
                                                mag = Vec3DotVu0(&scratchD, &candToOrigin);
                                                pitchWide = conePitchN < conePitch4;
                                                angleV = 1.5707964f -
                                                         func_00283B60(mag /
                                                                       (projLen * dist));
                                                coneYawBase = coneYaw2;
                                                goto join097c;
                                            }
                                        }
                                    }
                                }
                            }
                            goto advance;
                        } else {
                            goto score;
                        }

                    join097c:
                        {
                            f32 pitchN = coneYawBase;   /* fVar20 default = coneYaw2 */
                            if (pitchWide) {
                                pitchN = conePitch4 + (coneYaw2 - conePitch4) * t;
                            }
                            if (angleV < pitchN) {
                                goto score;
                            }
                            goto advance;
                        }

                    score:
                        {
                            f32 cost = angleH * 3.0f + angleV * dist * 0.2f + dist * 0.1f;
                            f32 candScore;
                            Moby *candBest;

                            if (gatePass != 0) {
                                cost = cost * 0.8f * range3;
                            }
                            candBest = best;
                            candScore = bestScore;
                            if (cost < bestScore) {
                                if (CollLine(searchOrigin, &candPos, 0x12, moby, 0) != 0) {
                                    if (g_playerProgress == 0xB) {
                                        if (g_pCollHitMoby == 0) {
                                            goto advance;
                                        }
                                        if (*(short *)((u8 *)g_pCollHitMoby + 0xAA) == 0xC20) {
                                            /* .s .L002B0A48 beql delay-slot (swc1 $f20,0xB4)
                                             * + .L002B0A74 (sw $18,0xB0): this accept path
                                             * commits bestScore=cost, best=moby. */
                                            candScore = cost;
                                            candBest = moby;
                                            goto append;
                                        }
                                    }
                                    if (g_pCollHitMoby == 0) {
                                        goto advance;
                                    }
                                    if (func_00301370(*(u16 *)((u8 *)g_pCollHitMoby + 0xAA)) == 0) {
                                        goto advance;
                                    }
                                }
                                candScore = cost;
                                candBest = moby;
                            }
                        append:
                            bestScore = candScore;
                            best = candBest;
                            if ((pOutList != 0) && (*pOutCount < maxOut)) {
                                s32 n = *pOutCount;
                                pOutList[n] = (int)moby;
                                *pOutCount = n + 1;
                            }
                        }
                    }
                }
            }
        advance:
            index = nextIdx;
            moby = (Moby *)g_mobyFlagged1000List[index];
            if (moby == 0) {
                break;
            }
            alive = *(char *)((u8 *)moby + 0x31);
        } while (1);
    }

    /* Move the best (lowest-cost) survivor to the front of the out-list. */
    if (pOutList == 0) {
        return (int)best;
    }
    if (best != 0) {
        s32 i = 0;
        if (maxOut > 0) {
            int first = pOutList[0];
            int *scan = pOutList;
            if (first != (int)best) {
                do {
                    i++;
                    scan++;
                    if (maxOut <= i) {
                        goto done_reorder;
                    }
                } while (*scan != (int)best);
                if (i != 0) {
                    pOutList[0] = (int)best;
                    *scan = first;
                }
            }
        }
    done_reorder:
        if (i == maxOut) {
            pOutList[0] = (int)best;
        }
    }
    if (maxOut <= *pOutCount) {
        return (int)best;
    }
    pOutList[*pOutCount] = 0;
    return (int)best;
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0BD8);

/** func_002B0BF0 — transform the local vector (x,y,z) by obj's matrix (at +0xC0)
 *  and accumulate it into `out` (out += M * (x,y,z)). */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 88.42%
   -> SCHED-TIEBREAK (prologue interleave), ORDER-ONLY */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0BF0);
#else
void func_002B0BF0(void *obj, Vec4 *out, f32 x, f32 y, f32 z) {
    Vec4 tmp;
    tmp.x = x;
    tmp.y = y;
    tmp.z = z;
    func_00283A48(&tmp, &tmp, (Vec4 *)((char *)obj + 0xC0));
    Vec4AddVu0(out, out, &tmp);
}
#endif

/**
 * Transform vector `a` into `out` by a rotation. When a matrix `b` is supplied
 * (b != 0) use it directly; otherwise build one from the quaternion at ctx+0xC0
 * into a scratch matrix and transform through that.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 61.56%
   -> UNKNOWN-@1: ROM `(none)` vs `daddu v0,a0,zero` */
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 51.96%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-48` vs `addiu sp,sp,-32` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0CC0);
#else
void func_002B0CC0(s32 ctx, Vec4 *out, void *a, void *b, f32 len) {
    func_002B0C40(ctx, out, a, b);
    func_00283920(out, out, len);
    func_00283A48(out, out, (Vec4 *)(ctx + 0xC0));
    __asm__ __volatile__("");
}
#endif

/**
 * Resolve `a` into `out` via func_002B0C40, override out.z with the supplied
 * height t, then transform out in place by the object's matrix at ctx+0xC0.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 39.80%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-48` vs `addiu sp,sp,-32` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0D20);
#else
void func_002B0D20(s32 ctx, Vec4 *out, void *a, void *b, f32 t) {
    func_002B0C40(ctx, out, a, b);
    out->z = t;
    func_00283A48(out, out, (Vec4 *)(ctx + 0xC0));
    __asm__ __volatile__("");
}
#endif

/**
 * Resolve a tracked position into a local vector, then forward it to the
 * shared vector consumer.
 */
void func_002B0D70(s32 ctx, void *a, void *b) {
    Vec4 out;

    func_002B0C40(ctx, &out, a, b);
    Vec2LengthXyVu0(&out);
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
/* TODO(match): functional equivalent - save-layout wall ($16/$17/$31 packed
   8-byte by the later cc1 vs 16-byte by the pinned cc1). The Sub/flatten/
   rescale/conditional-negate sequence is otherwise straightforward. Revisit
   once the gameplay-TU compiler is available. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 75.77%
   -> UNKNOWN-@2: ROM `(none)` vs `sd s1,8(sp)` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0DC8);
#else
void func_002B0DC8(Vec4 *src, Vec4 *out, s32 keepSign) {
    Vec4SubVu0(out, (Vec4 *)&D_1A8CB0, src);
    out->z = 0.0f;
    Vec3RescaleToLenVu0(out, 1.0f, out);
    if (keepSign == 0) {
        Vec4ScaleVu0(out, -1.0f, out);   /* sig is (dst, f32 scale, src); negate in place */
    }
    __asm__ __volatile__("");
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 81.74%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-48` vs `lw v0,0(gp)  [GPREL16 D_1A8CA0]` */
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
    __asm__ __volatile__("");
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
 *
 * MATCH: the original passes only p+0x10 in $4 (a 1-arg call), forwarding its own
 * $5/$6 through to the 3-arg func_002B0E40. A DIRECT cast-call on the function
 * NAME lets that 1-arg call coexist with func_002B0E40's real 3-arg prototype and
 * emits `jal func_002B0E40` (byte-exact) — not a fn-ptr local (which would spill +
 * `jalr`).
 */
s32 func_002B0F18(u8 *p) {
    return ((s32 (*)(u8 *))func_002B0E40)(p + 0x10);
}

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0F38);

/**
 * Offset a base position `src` into `out` by a direction of magnitude t:
 *  - D_1A8CA0 == 0 (simple mode): out.z = src.z - t (straight vertical drop).
 *  - else: build a direction from `a` (func_002B0E40 with flag=0), rescale it to
 *    length t, and add it to src: out = src + t*dir.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 67.41%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-64` vs `lw v0,0(gp)  [GPREL16 D_1A8CA0]` */
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

/**
 * Probe the ground/surface height near `pos` and return the signed vertical
 * clearance, optionally writing the contact point to `out`.
 *  - D_1A8CA0 == 0 (normal): cast a short line from just above pos (z+0.5) down to
 *    near pos (z=0.01) via CollLine; on a hit return pos.z - hit.z (out = hit),
 *    else return pos.z (out = pos).
 *  - else (alt surface, e.g. water): sample against a reference point
 *    (D_001B1750, or pos pushed -10 when D_1A8CA4 set), cast from a -0.5 offset,
 *    and on a hit return the pos->hit distance, negated when pos is nearer the ray
 *    origin than the hit is; else return the pos→reference distance.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 30.56%
   -> UNKNOWN-@0: ROM `(none)` vs `lw v0,0(gp)  [GPREL16 D_1A8CA0]` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0FE0);
#else
f32 func_002B0FE0(Vec4 *pos, void *moby, Vec4 *out) {
    Vec4 a;
    Vec4 b;

    if (D_1A8CA0 == 0) {
        a = *pos;
        a.z = 0.01f;
        b = *pos;
        b.z = pos->z + 0.5f;
        if (CollLine(&b, &a, 2, moby, (void *)0) != 0) {
            if (out != 0) {
                *out = g_collHitPoint;
            }
            return pos->z - g_collHitPoint.z;
        }
        if (out != 0) {
            *out = *pos;
        }
        return pos->z;
    }

    a = *(Vec4 *)&D_001B1750;
    if (D_1A8CA4 != 0) {
        a = *pos;
        func_002B0F40(&a, &a, &a, -10.0f);
    }
    func_002B0F40(pos, &b, pos, -0.5f);
    {
        f32 dist = Vec3DistVu0(pos, (f32 *)&a);
        if (CollLine(&b, &a, 2, moby, (void *)0) != 0) {
            f32 hitDist;
            if (out != 0) {
                *out = g_collHitPoint;
            }
            hitDist = Vec3DistVu0(pos, (f32 *)&g_collHitPoint);
            if (dist < Vec3DistVu0(&g_collHitPoint, (f32 *)&a)) {
                hitDist = -hitDist;
            }
            return hitDist;
        }
        if (out != 0) {
            *out = *pos;
        }
        return dist;
    }
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B11C0);

/**
 * Sample the breath/oxygen meter value; when the HUD inversion flag is set
 * the meter counts down from 100 instead.
 *
 * `p` is read directly as an offset-0 16-byte vector by Vec3DistVu0 (lqc2),
 * so it is a Vec4* (a world position point); callers pass moby+0x10, i.e. the
 * moby's position vec (verified in CheckMobyOverWater @ 0x2B7334).
 */
f32 func_002B11C8(Vec4 *p) {
    if (D_1A8CA4 == 0) {
        return Vec3DistVu0(p, &D_001B1750);
    }
    return 100.0f - Vec3DistVu0(p, &D_1A8CB0);
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 77.26%
   -> UNKNOWN-@3: ROM `(none)` vs `sd s0,16(sp)` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1220);
#else
void func_002B1220(void *mtx3x4, Vec4 *gravDir, void *outMtxOpt) {
    Vec4 invGravDir;
    Vec4ScaleVu0(&invGravDir, -1.0f, gravDir);
    func_002B1270(mtx3x4, &invGravDir, outMtxOpt);
}
#endif

/* Callees for func_002B1270's #else (sigs traced from its call registers). */
extern void Vec3CrossVu0(Vec4 *dst, Vec4 *a, Vec4 *b);
extern void QuatToMatrix3(Vec4 *quat, void *outMtx);   /* quaternion -> 3x4 matrix */
extern void func_002840E8(void *dst, void *a, void *b);/* 3x3 matrix multiply (a*b -> dst) */
extern void func_00283460(void *dst, void *src, s32 n);/* byte copy */

/**
 * func_002B1270 = OrientMatrixToGravity: rotate the 3x4 matrix `mtx3x4` so its
 * +0x20 axis aligns toward the gravity direction, in place. Builds the shortest-
 * arc half-angle quaternion from cross(normalize(gravDir), mtx+0x20): the xyz is
 * that cross scaled by 0.5, w = -sqrt(1 - |xyz|^2). Converts the quaternion to a
 * 3x4 matrix (QuatToMatrix3) and multiplies it into mtx3x4 (func_002840E8). If
 * outMtxOpt != 0, the 0x30-byte delta matrix is also copied out.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 66.87%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-144` vs `addiu sp,sp,-128` */
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
    QuatToMatrix3(&quat, quatMtx);
    func_002840E8(mtx3x4, quatMtx, mtx3x4);
    if (outMtxOpt != 0) {
        func_00283460(outMtxOpt, quatMtx, 0x30);
    }
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1340);

/**
 * Resolve a tracked position from a caller vector (128-bit local copy) and
 * forward its x/y to the 2D consumer.
 */
void func_002B1348(s32 ctx, Vec4 *vec, void *b) {
    QVec t;

    t.q = *(u_long128 *)vec;
    func_002B0C40(ctx, (void *)&t, (void *)&t, b);
    Atan2fPoly(t.v.x, t.v.y);
}

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1380);

/* Rain heightmap: a 256x256 grid of byte cell-heights covering the world XZ
 * span from g_rainHeightmapOrigin, one cell every CellW x CellH world units.
 * A cell value of 0xFF means "no data". */
extern u8  *g_pRainHeightmap;             /* 0x1B19A0 */
extern Vec4 g_rainHeightmapOrigin;        /* 0x1B1960 */
extern f32  g_rainHeightmapBaseHeight;    /* 0x1B1968 */
extern f32  g_rainHeightmapCellW;         /* 0x1B1990 */
extern f32  g_rainHeightmapCellH;         /* 0x1B1994 */
extern f32  g_rainHeightmapHeightScale;   /* 0x1B1998 */
extern s32  D_1A91C0;                      /* 0x1A91C0: light-pass validity token */

/* SampleRainHeightmap(pos): return the terrain height under world position `pos`
 * from the rain heightmap. Converts pos into grid coordinates (u, v) relative to
 * the heightmap origin, and if they fall inside the 256x256 grid (and the light
 * pass is current) looks up the cell byte and maps it to a world height
 * (cell * heightScale + baseHeight). Returns 0.0 when no heightmap is loaded and
 * a 1023.0 sentinel when the sample is out of range / unavailable.
 *
 * Matching build stays INCLUDE_ASM: the qword pos copy + Vu0 subtract and the
 * gp/absolute-mixed heightmap globals are an engine-2.96 layout this C won't
 * reproduce. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 43.28%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-48` vs `lw v0,0(gp)  [GPREL16 0x001B19A0]` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", SampleRainHeightmap);
#else
f32 SampleRainHeightmap(Vec4 *pos) {
    Vec4 local;
    Vec4 rel;
    f32  u, v;

    if (g_pRainHeightmap == NULL) {
        return 0.0f;
    }
    local = *pos;
    Vec4SubVu0(&rel, &local, &g_rainHeightmapOrigin);
    u = rel.x / g_rainHeightmapCellW;
    v = rel.y / g_rainHeightmapCellH;
    if (u < 0.0f || u > 256.0f || v < 0.0f || v > 256.0f) {
        return 1023.0f;
    }
    if (D_1A91C0 != *(s32 *)(g_pointLights + 0x2400)) {
        return 1023.0f;
    }
    {
        s32 iv    = FloatToInt(v);
        s32 iu    = FloatToInt(u);
        s32 index = (iv << 8) + iu;
        u8  cell  = g_pRainHeightmap[index];
        if (cell == 0xFF) {
            return 1023.0f;
        }
        return IntToFloat(cell) * g_rainHeightmapHeightScale + g_rainHeightmapBaseHeight;
    }
}
#endif

/* SpawnRaindropImpactFx(point): resolve where a falling rain drop lands and spawn
 * its impact effect. `point` is the drop's world XY (a Vec4; its z is filled in
 * here). Two modes decide the ray's start height:
 *   - heightmap mode (g_pRainHeightmap set): sample the rain heightmap at `point`,
 *     lift by g_rainHeightmapHeightScale; bail if that surface is at/above the
 *     camera (camZ + 5.0); the downward ray starts 2*scale + 2.0 below it.
 *   - weather-cell mode (else): require the XY inside the (0,0)..(1024,1024) cell
 *     and start the ray at camZ - 20.0.
 * A CollLine (mask 0x12) is then cast straight down from `point` to the start
 * height. On a hit: material 0/3/4 (water) spawns an expanding SpawnWaterRipple-
 * Particle at the hit point with a random 0.4..0.6 scale; any other material
 * spawns a SpawnRainSplashParticle whose brightness rises with camera distance.
 * Also reused overlay-side (0x32A588 water-sinking moby), so not weather-only.
 *
 * Matching build stays INCLUDE_ASM: the qword point copies + Vu0 subtract and the
 * gp/absolute-mixed globals are an engine-2.96 layout this C won't reproduce byte
 * for byte. The #else below is a faithful op-for-op transcription for the native
 * cmp/coverage harness. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 47.92%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-96` vs `addiu sp,sp,-80` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", SpawnRaindropImpactFx);
#else
extern f32  SampleRainHeightmap(Vec4 *pos);
extern Vec4 g_cameraPos;                                          /* 0x1B52C0: camera world position; [2]/.z is height */
extern f32  D_1A9FCC;                                             /* ripple surface z-offset (added to hit z) */
extern s32  D_1A9E74;                                             /* ripple surfaceParam arg */
extern s32  SpawnWaterRippleParticle(f32 scale, f32 growRate, Vec4 *pos, s32 surfaceParam, s32 rgba);
extern s32  SpawnRainSplashParticle(Vec4 *pos, s32 brightness, s32 flag);

void SpawnRaindropImpactFx(Vec4 *point) {
    Vec4 dropPoint;   /* fStack_60 (sp+0x00): ray start / hit-point scratch */
    Vec4 lineEnd;     /* fStack_50 (sp+0x10): ray end (down from dropPoint) */
    Vec4 camDelta;    /* auStack_40 (sp+0x20): hit point - camera, for splash brightness */
    f32  valid = 0.0f;

    dropPoint = *point;

    if (g_pRainHeightmap != 0) {
        dropPoint.z = SampleRainHeightmap(&dropPoint) + g_rainHeightmapHeightScale;
        if (g_cameraPos.z + 5.0f <= dropPoint.z) {
            goto done;
        }
        lineEnd = dropPoint;
        valid = 1.0f;
        lineEnd.z = lineEnd.z - (g_rainHeightmapHeightScale + g_rainHeightmapHeightScale) - 2.0f;
    } else {
        if (dropPoint.x <= 0.0f) {
            goto done;
        }
        if (dropPoint.y <= 0.0f) {
            goto done;
        }
        if (1024.0f <= dropPoint.x) {
            goto done;
        }
        if (1024.0f <= dropPoint.y) {
            goto done;
        }
        lineEnd = dropPoint;
        valid = 1.0f;
        lineEnd.z = g_cameraPos.z - 20.0f;
    }

done:
    if (valid != 0.0f && CollLine(&dropPoint, &lineEnd, 0x12, 0, 0) != 0) {
        s32 material = GetCollHitMaterial();
        if (material == 0 || material == 4 || material == 3) {
            f32 scale;
            dropPoint = g_collHitPoint;
            dropPoint.z = dropPoint.z + D_1A9FCC;
            scale = GetRandomFloatRange(0.4f, 0.6f);
            SpawnWaterRippleParticle(scale, 5250.0f, &dropPoint, D_1A9E74, -1);
        } else {
            f32 dist;
            Vec4SubVu0(&camDelta, &g_collHitPoint, &g_cameraPos);
            dist = Vec2LengthXyVu0(&camDelta);
            SpawnRainSplashParticle(&g_collHitPoint, (s32)(dist * 3.1833334f + 64.0f) & 0xff, 0);
        }
    }
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1708);

extern f32 IntToFloat(s32 x);

/** func_002B1710 — map the integer index (a mod b) onto an angle in [-PI, PI):
 *  returns 2*PI*(a%b)/b - PI. (a%b traps on b==0, like the original's div guard.) */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 84.58%
   -> 84.62% with the +1 ULP literal spelling (0x40490FDB); rest SCHED-TIEBREAK (div hoisted
   one slot) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1710);
#else
f32 func_002B1710(s32 a, s32 b) {
    f32 fm = IntToFloat(a % b);
    f32 fb = IntToFloat(b);
    return (fm + fm) * 3.1415927f / fb - 3.1415927f;
}
#endif

/** func_002B1778 — sample the sine of the (a mod b) index angle, remap it from
 *  [-1,1] to [0,1], and drive the packed-vec4 2-colour blend by that weight:
 *  LerpByteVec4Packed(sin(func_002B1710(a,b))*0.5+0.5, dst, src). */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 90.48%
   -> SCHED-TIEBREAK (move s0,a2 vs sd s1 order in the prologue), ORDER-ONLY */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1778);
#else
void func_002B1778(s32 a, s32 b, void *dst, void *src) {
    f32 s = func_00283B48(func_002B1710(a, b));
    LerpByteVec4PackedVu0(s * 0.5f + 0.5f, dst, src);
    __asm__ __volatile__("");
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B17D0);

/**
 * func_002B17F8 — set an eased-rotation driver's blend weight and optionally
 * settle it.
 *
 * Params: value — the weight to track while the "freeze palette cycling" flag
 *         (D_1A7A4F) is set; otherwise the weight snaps to 1.0.
 *         owner, arg3 — opaque controller handles, forwarded to func_002AFAB0.
 *         state — the driver block (+0x70 blend weight, +0x20 base weight).
 *         settle — when nonzero and the weight differs from the base weight,
 *         step the driver once via func_002AFAB0(owner, state, arg3, 0.03, 0.3).
 * Returns nothing.
 *
 * MATCHED on the sdk29 arm (plain C; unit objdiff report via objdiff_build.sh +
 * unit_report.sh, 100.00%; task #758). The ROM keeps `jal` + a 16-byte frame
 * and hoists `ld ra` into the `beqz settle` delay slot. Two sibcall guards
 * compared, one build each:
 *  - `noTailCall = 0;` (a dead local store) after the call blocks the sibling
 *    `j` at expand time and is deleted by flow before scheduling, so reorg is
 *    free to fill the delay slot with `ld ra` — the ROM's shape (task #756's
 *    lever, forum/promotion-grind/27489);
 *  - a trailing `__asm__ __volatile__("")` also keeps `jal`, but stays in the
 *    block as a barrier and the slot is left as a `nop` [97.67];
 *  - no guard at all: cc1 2.9 sibcalls func_002AFAB0 [85.33].
 */
/* Defined later in this unit; declared so the f32 args keep their type. */
void func_002AFAB0(void *owner, u8 *state, void *arg3, f32 rate, f32 cap);
void func_002B17F8(f32 value, void *owner, u8 *state, void *arg3, s32 settle) {
    s32 noTailCall;

    if (D_1A7A4F != 0) {
        *(f32 *)(state + 0x70) = value;
    } else {
        *(f32 *)(state + 0x70) = 1.0f;
    }
    if (settle != 0 && *(f32 *)(state + 0x70) != *(f32 *)(state + 0x20)) {
        func_002AFAB0(owner, state, arg3, 0.03f, 0.3f);
        noTailCall = 0;
    }
}

/* unreachable code fragment (stray sh + $sp tail from splat over-split), not C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1870);

/**
 * Look up a localized string by id and hand it (with the caller's second arg)
 * to the GUI text helper func_0029DAD0.
 */
/* TODO(match): functional equivalent - save-layout wall ($16/$31 packed
   8-byte by the later cc1 vs 16-byte by the pinned cc1). Body is a plain
   two-call forward. Revisit once the gameplay-TU compiler is available. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 98.33%
   -> SCHED-TIEBREAK (only the 2 arg moves before the 2nd jal swap: same-cycle emission order;
   value-return temp phrasing RUN, no change) */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1880);
#else
s32 func_002B1880(s32 stringId, s32 arg) {
    s32 r = func_0029DAD0(GetLocalizedString(stringId), arg);
    __asm__ __volatile__(""); /* cc1 2.96 sibling-call suppression (the ROM never sibcalls) */
    return r;
}
#endif

/**
 * Forward to func_0029DA88 (GUI text helper).
 */
s32 func_002B18B0(s32 a) {
    return func_0029DA88(a);
}

/* func_00273740 (external): emit one debris/effect sub-object for `owner` at
 * scatter `offset` off its position/facing, index `index`, with two extra
 * randomised parameters. */
extern void func_00273740(Moby *owner, s32 arg1, s32 classId, s32 index,
                          Vec4 *pos, Vec4 *facing, Vec4 *offset, s32 kind,
                          f32 f0, f32 f1);

/**
 * func_002B18D0 — spawn a burst of up to 15 debris/effect sub-objects from
 * `owner`, one for each set bit (0..14) of `slotMask`.
 *
 * Each spawn draws a random horizontal scatter: a random angle and a random
 * radius in [radiusMin, radiusMax] (both scaled by 1/60), giving a circular
 * offset (cos, sin) with a random Z in [zMin, zMax]/60. In oriented-gravity mode
 * (D_1A8CA0 != 0) the offset is first rotated into the owner's orientation frame
 * (+0xC0). The offset is added to `basePos`, then handed to func_00273740 along
 * with the owner's class id (+0xAA), world position (+0x10), facing (+0xF0), the
 * bit index, `kind` (low byte), and two more random parameters (a bearing in
 * [90,270] and a value in [10,20]).
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 48.26%
   -> UNKNOWN-@2: ROM `sd s0,16(sp)` vs `(none)` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B18D0);
#else
void func_002B18D0(Moby *owner, s32 slotMask, Vec4 *basePos, s32 kind,
                   f32 radiusMin, f32 radiusMax, f32 zMin, f32 zMax) {
    const f32 invFrameRate = 0.016666668f;   /* 0x3C888889 = 1/60 */
    s32 kindByte = kind & 0xFF;
    s32 i;

    for (i = 0; i < 15; i++) {
        Vec4 offset;
        f32 angle;
        f32 radius;
        f32 bearing;
        f32 value;

        if (((slotMask >> i) & 1) == 0) {
            continue;
        }
        angle  = GetRandomAngle();
        radius = GetRandomFloatRange(radiusMin, radiusMax) * invFrameRate;
        offset.x = func_00283B30(angle) * radius;   /* cos */
        offset.y = func_00283B48(angle) * radius;   /* sin */
        /* original clears the Z int then overwrites it with the float below */
        offset.z = GetRandomFloatRange(zMin, zMax) * invFrameRate;

        if (D_1A8CA0 != 0) {
            func_00283A48(&offset, &offset, (Vec4 *)((u8 *)owner + 0xC0));
        }
        Vec4AddVu0(&offset, &offset, basePos);

        bearing = GetRandomFloatRange(90.0f, 270.0f);
        value   = GetRandomFloatRange(10.0f, 20.0f);
        func_00273740(owner, 2, (s16)owner->oClass, i,   /* lh: signed +0xAA */
                      (Vec4 *)((u8 *)owner + 0x10), (Vec4 *)((u8 *)owner + 0xF0),
                      &offset, kindByte, bearing, value);
    }
}
#endif

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1A80);

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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 81.26%
   -> UNKNOWN-@1: ROM `lui v0,0x0  [HI16 D_1A8C64]` vs `lw v1,0(gp)  [GPREL16 D_1A8C64]` */
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
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 84.84%
   -> UNKNOWN-@2: ROM `(none)` vs `daddu s0,a1,zero` */
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
/* TODO(match): functional equivalent - save-layout wall ($16/$31 packed 8-byte
   by the later cc1 vs 16-byte by the pinned cc1); the subtract and [0,40]
   movz/movn clamp are otherwise exact. Revisit once the gameplay-TU compiler
   is available. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 59.94%
   -> UNKNOWN-@8: ROM `addiu v1,zero,-1` vs `ld ra,8(sp)` */
#ifndef TARGET_NATIVE
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
/* TODO(match): functional equivalent - save-layout wall ($16/$17/$18/$31, the
   later cc1 packs the four callee-save slots 8-byte where the pinned cc1
   reserves 16). The level-skip beql loop and the [0,40] movz/movn clamp are
   otherwise exact. Revisit once the gameplay-TU compiler is available. */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 71.21%
   -> UNKNOWN-@1: ROM `(none)` vs `sd ra,24(sp)` */
#ifndef TARGET_NATIVE
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

/* CountPlatinumBolts: count one level's collected platinum bolts, clamped to
 * [0, 40].
 *   level - level index; its 4 bolt flags are g_platinumBoltFlags[level*4..+3]
 *   returns the number of non-zero flags, plus (for level 2 only) the 4 extra
 *   flags kept at g_platinumBoltFlags+0x68 - the slot of level 0x1A, which
 *   TotalPlatinumBolts skips for exactly that reason.
 *
 * Byte-exact on sdk29 (task #1026; FACT #7937's 97.65% residual closed). The
 * body carries four EE codegen devices, none of which changes the semantics:
 *   - the first loop is written as the ROM's countdown do-while with the
 *     R5900 short-loop pad (R5900_SHORT_LOOP_PAD1, see its comment) before
 *     the bgez; the pad reads `count` so the movn stays after it and reorg
 *     moves it into the delay slot, as in the ROM;
 *   - an EMPTY fence tying the counter to the stepped pointer (emits nothing;
 *     RULING #8483) keeps the ROM's lbu / count+1 / p++ / i-- order;
 *   - CPB_COUNTER_IN_A3 is a REGISTER-PIN DEVICE (EE arm only, empty on
 *     native): it binds the countdown to $7, which leaves `count` in $6 as the
 *     ROM has it - without the pin cc1 2.9 gives count $7 and the loop-2
 *     base $6 (FACT #7937);
 *   - the second loop scans g_platinumBoltExtraFlags, an assembler alias for
 *     g_platinumBoltFlags+0x68 (the FACT #8386 zero-offset alias pattern), by
 *     index with an EMPTY operand-tied fence on the index: the ROM keeps the
 *     +0x68 in the %hi/%lo pair and counts the index up (slti); indexing
 *     g_platinumBoltFlags[0x68 + j] folds 0x68 into the lbu, and without the
 *     fence cc1 reverses the loop into a pointer countdown.
 * History: task #659 reached 97.65% (sdk29, unit objdiff report, VM b) with
 * the countdown do-while and a PAD1 asm reading flags/next/count/set; its
 * residual was count in $7 where the ROM has $6 (and the loop-2 base in $6
 * vs $7). The first loop is 5 instructions padded to 6 by the one `nop`; the
 * second is 6 long and unpadded.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 0.00% ->
   UNKNOWN-@0: ROM `(none)` vs `daddu t0,a0,zero` */
#ifndef TARGET_NATIVE
#define CPB_COUNTER_IN_A3 __asm__("$7")
__asm__("g_platinumBoltExtraFlags = g_platinumBoltFlags + 0x68");
extern u8 g_platinumBoltExtraFlags[];
#else
#define CPB_COUNTER_IN_A3
#define g_platinumBoltExtraFlags (g_platinumBoltFlags + 0x68)
#endif
s32 CountPlatinumBolts(s32 level) {
    u8 *flags = &g_platinumBoltFlags[level * 4];
    s32 count = 0;
    register s32 i CPB_COUNTER_IN_A3;
    s32 j;

    i = 3;
    do {
        s32 set = *flags;
        s32 next = count + 1;
        flags++;
        __asm__("" : "+r"(i) : "r"(flags));
        i--;
        R5900_SHORT_LOOP_PAD1(i, count);
        if (set != 0) {
            count = next;
        }
    } while (i >= 0);
    if (level == 2) {
        for (j = 0; j < 4; j++) {
            if (g_platinumBoltExtraFlags[j] != 0) {
                count = count + 1;
            }
            __asm__("" : "+r"(j));
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

/**
 * func_002B1DF0 — apply a hit's damage to a moby and classify the outcome.
 *
 * Gated on the moby being damageable (flags+0x34 & 0x20) with a live stats block.
 * Resolves the attacker's armor/resist record (func_002AA4A8, dropped if the moby's
 * hit-countdown func_00283328 says inactive), applies the record's +0x2c damage
 * multiplier (matched class + optional param_3 scale), then deals the damage
 * (func_002AA508) to the stats health at stats[0], stamps the hit (+0xa8=0xff),
 * fires the threat flash/burst, and records the attacker's moby slot into hit+0x70.
 *
 * It then classifies the result into a damage tier (0 none .. 4 kill), promotes
 * tiers 1-3 to 6 when param_3 is set, spawns a knockback direction FX for the
 * '\n'-class stats (tiers >= 2), and finally — unless suppressed by hit state —
 * builds a reaction direction (record +0x10, or -Z of the moby's forward at +0xC0
 * with w=5627.9248) and drives the hit reaction (func_002B2268) + stagger anim
 * (func_002A85B8). Returns the damage tier (0 when any gate rejects the hit).
 *
 * Faithful #else transcription — the matching build uses the INCLUDE_ASM arm above.
 */
/* t467 engine96 arm (cc1 2.96-001003-1, objdiff_build.sh+unit_report.sh, 2026-09-19): 66.98%
   -> UNKNOWN-@0: ROM `addiu sp,sp,-96` vs `addiu sp,sp,-128` */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B1DF0);
#else
/* callees with no file-scope decl above this point */
extern long func_002B2268(void *moby, long hit, Vec4 *dir, u32 tier);
extern void UpdateMobyThreatFlashAndBurst(void *moby, s32 threatIdx, f32 *stats);

u32 func_002B1DF0(void *moby, long hit, long param_3) {
    u8  *m = (u8 *)moby;
    u8  *h = (u8 *)hit;
    int *blk;
    f32 *stats;
    s32  threatIdx;
    long armorRec;         /* func_002AA4A8 record (lVar4) */
    u8  *r;                /* = (u8 *)armorRec */
    f32  damage;           /* fVar11 */
    f32  armorBase = 0.0f; /* fVar14: record +0x2c snapshot (0 when no record) */
    s32  timeThresh;
    u32  result = 0;

    if ((*(u16 *)(m + 0x34) & 0x20) == 0) {
        return 0;
    }
    blk = *(int **)(m + 0x68);
    threatIdx = blk[7];
    stats = *(f32 **)blk;
    if (hit == 0 || stats == NULL) {
        return 0;
    }

    func_002AC728(moby, h);

    armorRec = func_002AA4A8((Moby *)moby, 0x10000, 0);
    if (func_00283328((u8 *)stats + 6) == 0) {
        armorRec = 0;
    }
    r = (u8 *)armorRec;

    if (armorRec != 0) {
        f32 dmgMul = *(f32 *)(h + 0xa8);

        armorBase = *(f32 *)(r + 0x2c);
        if (dmgMul != 0.0f && *(int *)(r + 0x20) != 0 &&
            *(s16 *)(m + 0xaa) == *(s16 *)(*(int *)(r + 0x20) + 0xaa)) {
            *(f32 *)(r + 0x2c) = armorBase * dmgMul;
        }
        if (param_3 != 0) {
            *(f32 *)(r + 0x2c) *= *(f32 *)(h + 0xac);
        }
    }

    damage = func_002AA508((Moby *)moby, (void *)armorRec);
    m[0xa8] = 0xff;
    stats[0] -= damage;
    if (threatIdx != 0) {
        UpdateMobyThreatFlashAndBurst(moby, threatIdx, stats);
    }
    if (damage == 0.0f) {
        return 0;
    }

    *(u16 *)(h + 0x70) = 0xffff;
    if (armorRec != 0 && *(int *)(r + 0x20) != 0) {
        *(s16 *)(h + 0x70) =
            (s16)(((u8 *)*(void **)(r + 0x20) - (u8 *)g_mobyTableBase) >> 8);
    }

    /* --- damage-tier classification --- */
    {
        s32 slot = (u32)g_itemEquippedSlot[*(u8 *)((u8 *)stats + 0x17)];
        f32 minHitDmg = (f32)*(s16 *)((u8 *)stats + 4) * *(f32 *)(h + 0x58);
        f32 dmgVsCap  = damage * *(f32 *)((u8 *)g_weaponTable + slot * 0xE0 + 0x54);

        timeThresh = (minHitDmg <= dmgVsCap) ? 0x5a : 300;
    }

    if (h[0x77] == 0) {
        s32 dt = func_002835E0(*(s32 *)g_gameTime - *(s32 *)(h + 0x44));
        if (timeThresh < dt) {
            f32 minHit = (f32)*(s16 *)((u8 *)stats + 4) * *(f32 *)(h + 0x58);

            if (damage < minHit) {
                damage = minHit;
            }
        }
    }

    if (stats[0] <= 0.0f) {
        result = 4;
    } else if ((f32)*(s16 *)((u8 *)stats + 4) * *(f32 *)(h + 0x5c) <= damage) {
        result = 3;
    } else if (damage < (f32)*(s16 *)((u8 *)stats + 4) * *(f32 *)(h + 0x58)) {
        if (damage == 0.0f || h[0x77] != 0 || armorRec == 0 ||
            *(int *)(r + 0x20) == 0 ||
            *(s16 *)(*(int *)(r + 0x20) + 0xaa) != 0x47) {
            if (armorBase != 0.0f) {
                result = 1;
            }
        } else {
            result = 2;
        }
    } else {
        result = 2;
    }

    if (param_3 != 0 && (u32)(result - 1) < 3) {
        result = 6;
    }

    /* knockback FX for the '\n'-class stats on a real (tier >= 2) hit */
    if (1 < result && armorRec != 0 && *(u8 *)((u8 *)stats + 0x17) == '\n') {
        Vec4 *dir = (Vec4 *)(r + 0x10);
        f32   horizLen;
        Vec4  resolved;

        /* inline of func_002B0D70: it resolves `dir` and calls Vec2LengthXyVu0, but is
         * byte-matched as void so it cannot hand back Vec2LengthXyVu0's planar length */
        func_002B0C40((s32)moby, &resolved, dir, (void *)0);
        horizLen = Vec2LengthXyVu0(&resolved);

        Vec4SubVu0(dir, (Vec4 *)(m + 0x10), &g_heroPos);
        func_002B0CC0((s32)moby, dir, dir, (void *)0, horizLen);
    }

    if (result == 0) {
        return 0;
    }
    if ((*(u16 *)(h + 0x38) & 3) != 0) {
        return 0;
    }
    if ((u32)m[0x95] == *(u32 *)(h + 0x48) && result < 4) {
        if (h[0x75] != 0) {
            return 0;
        }
        if (h[0x1a] < 2) {
            return 0;
        }
    }

    /* --- drive the hit reaction --- */
    {
        Vec4 dir;

        if (armorRec == 0 || (*(u32 *)(r + 0x30) & 1) == 0) {
            Vec4ScaleVu0(&dir, -1.0f, (const Vec4 *)(m + 0xc0));
            dir.z = 1.0f;
            *(u32 *)&dir.w = 0x45afdf66;   /* 5627.9248f */
        } else {
            dir = *(Vec4 *)(r + 0x10);
        }

        if (func_002B2268(moby, hit, &dir, result) != 0) {
            *(s32 *)(h + 0x44) = *(s32 *)g_gameTime;
            if (*(s32 *)(h + 0x48) == -1) {
                func_002A85B8((MobyAnim *)moby, h[0x10], -1);
            } else {
                func_002A85B8((MobyAnim *)moby, h[0x10], *(s32 *)(h + 0x48));
            }
            h[0x1a] = 0;
        }
    }
    return result;
}
#endif
