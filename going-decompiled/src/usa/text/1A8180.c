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
} Moby;

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
extern f32 func_00284590(f32 a, f32 b);
extern s32 func_00284548(f32 a, f32 b);
extern void Vec4SubVu0(Vec4 *dst, Vec4 *a, Vec4 *b);
extern void Vec4ScaleVu0(Vec4 *dst, Vec4 *src, f32 s);
extern void Vec3RescaleToLenVu0(Vec4 *dst, f32 len, Vec4 *src);


INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8200);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A82D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8448);

/* func_002A85B8: begin a moby animation (stash prev anim id, reset the anim
 * timer, clear the done flags, optional sub-anim). Best attempt 58%: the
 * pinned cc1 schedules the animFlags store before the timer clear and
 * recomputes the second mask from the original flags where the later cc1
 * keeps an incremental masked value and a -1 compare register in a0 - the
 * register-coloring/store-scheduling wall. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A85B8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8688);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A8948);

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

/* func_002A9370: count a group's active mobys (state == -1 counts all,
 * otherwise only those in the given anim state). Best attempt 82%: byte-
 * identical except the list pointer colours v1 (reusing the address temp)
 * where the original loads it into v0 with a later move into a0 - the
 * register-coloring wall. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9370);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", ProbeGroundHeight);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A98B8);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002A9A68);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", GetWaterSurfaceHeight);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AA058);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", QueueMobyBlobShadow);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", ProbeMobyGroundBelow);

/**
 * Forward to the moby ground-fit probe (drop-shadow placement).
 */
s32 func_002AA3B0(Moby *moby) {
    return ProbeMobyGroundBelow(moby);
}

/**
 * Forward to the shared vector helper func_002846E8.
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
 * blend, and write the interpolated packed vec4 colour.
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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB210);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB2C0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB3B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB5A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB668);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB700);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AB868);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ABAE8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ABD00);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ABE08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ABE90);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ABF50);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ABFD0);

/* func_002AC058: read word 0 of a moby's extra/pvar block (mode bit 0x20
 * gates the block). Best attempt 66%: structure identical (bnezl + shared
 * return-0) but the later cc1 emits two scheduler nops between the andi
 * and its beqz that the pinned cc1 never produces (same wall as
 * func_002AC088/func_002AC9E0). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC058);

/* func_002AC088: read word 4 of a moby's extra/pvar block. Same two-
 * scheduler-nops wall as func_002AC058 (best attempt 66%). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC088);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC0B8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC1E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC468);

/**
 * Copy the hero's velocity pair (+0x38, 64-bit) onto another moby.
 */
void func_002AC4B8(Moby *moby) {
    *(u64 *)((u8 *)moby + 0x38) = *(u64 *)((u8 *)g_pHeroMoby[0] + 0x38);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC4D0);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AC9E0);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AD8B8);

/* func_002AD938: remove a moby from an i16 count-prefixed list by swapping
 * the last entry into its slot. Best attempt 57%: the original keeps the
 * raw count in a register across the scan with branch-likely reloads the
 * pinned cc1 will not produce - scan-loop scheduling wall. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AD938);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AD9B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002ADA30);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", MarkLevelAvailable);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE6C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE7E8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AE9E0);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF590);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF598);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF6A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF728);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF948);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AF9C8);

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFA58);

/**
 * Unpack a 0xBBGGRR colour word and forward to the colour setter.
 */
s32 func_002AFA80(void *p, s32 rgb) {
    s32 b = rgb >> 16;
    s32 g = rgb >> 8;

    __asm__ __volatile__("");
    return func_002A12C0(p, rgb & 0xFF, g & 0xFF, b & 0xFF);
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFAB0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFCD8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002AFD90);

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
        Vec4ScaleVu0(out, out, -1.0f);
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0E40);

/**
 * Forward to func_002B0E40 on the sub-object at +0x10.
 */
s32 func_002B0F18(u8 *p) {
    return func_002B0E40(p + 0x10);
}

/* fill-fragment: orphaned $sp adjustment from splat over-split, not reachable C - keeps INCLUDE_ASM (see unit header). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0F38);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", func_002B0F40);

/**
 * Forward to func_002B0F40 on the sub-object at +0x10.
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
 */
f32 func_002B11C8(void *p) {
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A8180", CountPlatinumBolts);

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
