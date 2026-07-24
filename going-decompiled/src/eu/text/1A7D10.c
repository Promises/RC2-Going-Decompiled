#include "common.h"

/*
 * text/1A7D10 (EU SCES_516.07) — the PAL twin of USA text/1A8180 (the
 * "game-state cluster"): moby helpers (group iteration, ground probes, hit
 * events, list maintenance), small math/easing helpers, and the progress
 * counters. Phase-B port (2026-06-14): the 24 functions USA matches whose EU
 * symbol survives Phase A's re-tile as a clean 1:1 glabel are ported here.
 * The C bodies are region-agnostic; objdiff masks the gp/reloc address deltas
 * between the two builds, so the same source compiles byte-exact against both.
 *
 * Built at -O2 -G8 -fno-gcse (per-unit GFLAG/CC1EXTRA in objdiff_build.sh /
 * diff.sh / build.sh) — same later-SN-cc1 TU model as the USA sibling.
 *
 * RECOVERED (2026-06-14): the four leading-padding mis-splits were split off via
 * symbol_addrs pins (padding func + real-start func) and their region-agnostic
 * bodies ported byte-exact: func_002A9278 (USA func_002A96C8), func_002AABA8
 * (USA func_002AAFA8 lerp), func_002B0EC8 (USA func_002B11C8), func_002B1048
 * (USA func_002B1348). The pinned padding funcs (func_002A9268/002AAB98/002B0EC0/
 * 002B1040) stay INCLUDE_ASM by design (they are bare epilogue padding).
 *
 * REGION HOLD (genuine PAL/NTSC divergence, left INCLUDE_ASM):
 *   - USA func_002B1D18 (bounds-checked per-level lookup, bound 0x15) has NO
 *     EU counterpart: no EU function in this unit performs an `sltiu ,0x15`
 *     table read — the PAL build's per-level lookup diverges (different level
 *     count / table), the project's first confirmed PAL/NTSC behavioural delta.
 */

/* gp-addressable / assembler-absolute symbols (size class 12, see USA header). */
__asm__(".extern g_mobyTableBase, 12");
__asm__(".extern g_mobyTableEnd, 12");
__asm__(".extern g_mobyGroupCount, 12");
__asm__(".extern g_pMobyGroupIterCursor, 12");
__asm__(".extern g_mobyGroupIterSlot, 12");
__asm__(".extern g_pMobyGroupIterMoby, 12");
__asm__(".extern D_1A8D54, 12");
__asm__(".extern D_1A8D60, 12");
__asm__(".extern g_pActiveNanotechOrb, 12");
__asm__(".extern D_1A7A4F, 12");
__asm__(".extern D_1A8BD0, 12");
__asm__(".extern g_skillPointFlags, 12");
__asm__(".extern g_abLevelAvailableFlags, 12");

#ifdef TARGET_NATIVE
/* gcc -m32 cannot emulate mode(TI); this unit only copies QVecs, so a 16-byte
   aligned struct suffices for the native structure-model gate (mirrors USA 1A8180). */
typedef struct { unsigned long long _q[2]; } __attribute__((aligned(16))) u_long128;
#else
typedef unsigned long u_long128 __attribute__((mode(TI)));
#endif
typedef struct Vec4 { f32 x, y, z, w; } Vec4;
typedef union QVec { u_long128 q; Vec4 v; } QVec;

/* Minimal moby view (0x100-stride table entries). Only fields this unit
 * touches are declared. */
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

/* Moby animation header view for func_002A8168 (anim id bookkeeping). */
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
extern s32 D_1A8D54;                   /* breath/oxygen HUD inversion flag (USA D_1A8CA4) */
extern f32 D_1A8D60;                   /* inverted breath meter source value (USA D_1A8CB0) */
extern u8 g_pActiveNanotechOrb[];      /* breath meter source at +0x24 (USA D_001B1750) */
extern u8 D_1A7A4F;                    /* "freeze palette cycling" flag */
extern f32 D_1A8BD0;                   /* default hit-direction Vec4 for func_002A9AE0 */

/* Large/absolute globals. */
extern u16 *g_mobyGroupLists[];        /* per-group i16 slot lists (negative-terminated) */
extern Moby *g_pHeroMoby[];            /* incomplete-array decl: keeps the 4-byte ptr out of cc1 small data */
extern Vec4 g_collHitPoint;
extern Vec4 g_collHitPointNudged;
extern u8 g_skillPointFlags[8];   /* really u8[0x20]; declared 8 so cc1 schedules the address materialisation as one insn */
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

/* Callees (value-returning declarations keep cc1 from sibling-call optimising
 * forwarding tails — see USA text/198FA0). */
extern f32 ProbeGroundHeight(Vec4 *pos, f32 zOffset, s32 mask);
extern s32 func_001163B0(void);        /* core random-state step */
/* Angle helpers (EU twins of USA func_00284590/func_00284548 == WrapAnglePi*).
 * func_002844A0 genuinely returns f32 in $f0; func_00284458 also returns f32,
 * but the matching build's func_002AABB8 byte-matches ONLY with the s32 form —
 * the s32-callee + s32-return type errors cancel into the exact $f0 passthrough
 * the original emits (same trick as USA func_00284548 → func_002AAFB8). */
extern f32 func_002844A0(f32 a, f32 b);
extern s32 func_00284458(f32 a, f32 b);
extern s32 ProbeMobyGroundBelow(Moby *moby);
extern s32 func_002846E8(void *a, void *b, void *c);
extern s32 func_0029DA88(s32 a);
extern s32 func_002B0940(s32 ctx, void *out, void *a, void *b); /* EU 4-arg tracked-position resolver (USA func_002B0C40) */
extern s32 func_002B0B40(void *p);     /* EU forward target (USA func_002B0E40) */
extern s32 func_002B0C40(void *p);     /* EU forward target (USA func_002B0F40) */
extern s32 func_002837D0(void *vec);
extern s32 func_00283B08(f32 x, f32 y);  /* EU 2D consumer (USA func_00283BF8) */
extern s32 func_002A12C0(void *p, s32 r, s32 g, s32 b);
extern s32 func_00283638(Moby *moby);
extern s32 PostMobyHitEvent(Moby *moby, s32 a, s32 b, s32 c, Vec4 *dir);
extern f32 func_00283708(void *p, f32 *src);  /* EU breath-meter sampler (USA func_002837F8) */
extern f32 GetFloatAbs(f32 x);


/* Set a moby's active animation sequence + frame, clamping the frame to the
 * sequence frame count, then resolving frame pointers and caching loop sound.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002A8200: ResolveMobyAnimFramePtrs (func_002A01C8) -> func_0029FD50. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A7D90);
#else
extern void func_0029FD50(Moby *moby);   /* resolve anim frame ptrs (+0x58/+0x5C) */

void func_002A7D90(Moby *moby, s32 seq, s32 frameIdx) {
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

    func_0029FD50(moby);

    *(f32 *)(m + 0x4C) = *(f32 *)(*(u8 **)(m + 0x58));   /* animRate2 = *animFramePtr */
    m[0x60] &= 0xFD;                                     /* animEventByte: clear bit 1 */
    m[0x6C] = *(u8 *)(seqEntry + 0x11);                  /* loopSoundIdx */
}
#endif

/* Start anim sequence idx at frame arg3 blended over arg4 frames; arg4 <= 0
 * delegates to func_002A7D90 (instant set), else seeds a timed blend + optional
 * procedural-anim slot snapshot. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * Symbol map vs USA func_002A82D8: func_002A08C0 -> func_002A0448, func_002A3288 -> func_002A2E30,
 * func_002A8200 -> func_002A7D90, ResolveMobyAnimFramePtrs (func_002A01C8) -> func_0029FD50,
 * IntToFloat (func_00284690) -> func_002845A0, g_proceduralAnimBounds -> EU g_proceduralAnimBounds.
 *
 * NOT a pure symbol-swap of the USA twin. EU is 100 instrs vs USA's 92: the PAL
 * build carries an EU-ONLY conditional reset of the blend progress (+0x44) that
 * has no counterpart in func_002A82D8. USA .L002A833C opens straight with
 * `lwc1 $f1,0x44($16)`; EU inserts 8 words at 002A7ECC-002A7EE8 first. See the
 * body comment at the reset for the branch-polarity derivation. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A7E68);
#else
extern void func_0029FD50(Moby *moby);            /* resolve anim frame ptrs */
extern s32  func_002A0448(void *obj);             /* acquire procedural-anim slot; <0 = none */
extern void func_002A2E30(void *obj, s32 flags);  /* bind the acquired procedural-anim slot */
extern u8   g_proceduralAnimBounds[];             /* per-slot cached pose-bounds Vec4, stride 0x10 */
extern f32  func_002845A0(s32 x);                 /* int -> float */

void func_002A7E68(Moby *obj, s32 idx, s32 arg3, s32 arg4) {
    u8 *m = (u8 *)obj;
    u8 *pClass = *(u8 **)(m + 0x24);
    u8 *seqEntry = *(u8 **)(pClass + 0x48 + idx * 4);
    s32 frameCount = *(u8 *)(seqEntry + 0x10);
    s32 clampedFrame = (arg3 < frameCount) ? arg3 : (frameCount - 1);

    if (arg4 <= 0) {
        func_002A7D90(obj, idx, clampedFrame);
        return;
    }

    /* EU-ONLY (no counterpart in USA func_002A82D8): drop any in-flight blend
     * progress before it is sampled below, when the currently-playing sequence
     * (+0x42) already equals the last requested one (+0x43) and the +0x60 bit 1
     * flag is set. From the encoded words at 002A7ECC-002A7EE8:
     * (.s prints words little-endian; the decoded instruction word follows)
     *   42000392 -> 92030042  lbu  $3,0x42($16)
     *   43000292 -> 92020043  lbu  $2,0x43($16)
     *   06006254 -> 54620006  bnel $3,$2,.L002A7EF0   BNEL op 0x15; differ->skip
     *   440001C6 -> C6010044   lwc1 $f1,0x44($16)      delay slot, TAKEN only
     *   60000292 -> 92020060  lbu  $2,0x60($16)
     *   02004230 -> 30420002  andi $2,$2,0x2
     *   01004054 -> 54400001  bnel $2,$0,.L002A7EEC   BNEL; bit SET -> TAKEN
     *   440000AE -> AE000044   sw   $0,0x44($16)       delay slot, TAKEN only
     * A likely branch executes its delay slot when TAKEN and nullifies it when
     * not taken, so the zeroing store is gated on the bit being SET, i.e.
     * `(m[0x60] & 2) != 0` -- the same idiom already modelled below for the
     * 0xA9 store. Both paths converge on the reload of +0x44, so the 0.025f
     * test below observes the value this block may have zeroed. */
    if (m[0x42] == m[0x43] && (m[0x60] & 0x02) != 0) {
        *(f32 *)(m + 0x44) = 0.0f;                       /* reset blend progress */
    }

    if (0.025f < *(f32 *)(m + 0x44) ||
        *(s32 *)(m + 0x50) != 0 ||
        *(s32 *)(m + 0x54) != 0) {
        s32 slot = func_002A0448(obj);
        if (slot < 0) {
            m[0x41] = (u8)clampedFrame;
        } else {
            func_002A2E30(obj, slot | 0x300);
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
    func_0029FD50(obj);
    *(f32 *)(m + 0x48) = 1.0f;
    *(f32 *)(m + 0x4C) = 1.0f / func_002845A0(arg4);
    *(f32 *)(m + 0x44) = 0.0f;
    m[0x60] &= 0xFD;
    m[0x6C] = *(u8 *)(seqEntry + 0x11);
}
#endif

/* Parameterized blend-anim start: like func_002A7E68 but with an explicit flags
 * word (arg5 in $8, normal EABI); arg4 <= 0 delegates to func_002A7D90 (instant),
 * else acquires + binds a procedural-anim slot with flags-derived bind bits.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002A8448: func_002A08C0 -> func_002A0448, func_002A3288 -> func_002A2E30,
 * func_002A8200 -> func_002A7D90, ResolveMobyAnimFramePtrs (func_002A01C8) -> func_0029FD50,
 * IntToFloat (func_00284690) -> func_002845A0, g_proceduralAnimBounds -> EU g_proceduralAnimBounds. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A7FF8);
#else
extern void func_0029FD50(Moby *moby);            /* resolve anim frame ptrs */
extern s32  func_002A0448(void *obj);             /* acquire procedural-anim slot; <0 = none */
extern void func_002A2E30(void *obj, s32 flags);  /* bind the acquired procedural-anim slot */
extern u8   g_proceduralAnimBounds[];             /* per-slot cached pose-bounds Vec4, stride 0x10 */
extern f32  func_002845A0(s32 x);                 /* int -> float */

void func_002A7FF8(Moby *obj, s32 idx, s32 frame, s32 arg4, s32 flags) {
    u8 *m = (u8 *)obj;
    u8 *seqEntry;

    if (arg4 <= 0) {
        func_002A7D90(obj, idx, frame);
        return;
    }

    if (0.025f < *(f32 *)(m + 0x44) ||
        *(s32 *)(m + 0x50) != 0 ||
        *(s32 *)(m + 0x54) != 0 ||
        (flags & 4) != 0) {
        s32 slot = func_002A0448(obj);
        if (slot >= 0) {
            s32 bind = (flags & 1) ? (slot | 0x100) : slot;
            if (flags & 2) {
                bind |= 0x200;
            }
            func_002A2E30(obj, bind);
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
    func_0029FD50(obj);
    *(f32 *)(m + 0x48) = 1.0f;
    *(f32 *)(m + 0x4C) = 1.0f / func_002845A0(arg4);
    *(f32 *)(m + 0x44) = 0.0f;
    m[0x60] &= 0xFD;
    seqEntry = *(u8 **)(*(u8 **)(m + 0x24) + 0x48 + idx * 4);
    m[0x6C] = *(u8 *)(seqEntry + 0x11);
}
#endif

/* func_002A8168: begin a moby animation (EU twin of USA func_002A85B8). Stash
 * the current anim id into the "prev anim id" slot, install the new anim id,
 * reset the anim timer, and clear the "done" flag (bit 0 of animFlags). If a
 * sub-anim id is supplied (!= -1), record it and additionally clear bit 1. */
void func_002A8168(MobyAnim *m, s32 animId, s32 subAnim) {
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
 * Quadratic ease-in: x squared. (EU twin of USA func_002A85F8.)
 */
f32 func_002A81A8(f32 x) {
    return x * x;
}

/**
 * Quadratic ease-out: 1 - (1-x)^2. (EU twin of USA func_002A8600.)
 */
f32 func_002A81B0(f32 x) {
    f32 t = 1.0f - x;

    return 1.0f - t * t;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A81D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A81F8);

/* RandRangeInclusive: uniform random int in [lo, hi] inclusive - a 15-bit LCG
 * value (func_001163B0 >> 16 & 0x7FFF) reduced modulo the span then biased by lo.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002A8688: func_001163B0 unchanged (delta 0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A8238);
#else
s32 func_002A8238(s32 lo, s32 hi) {
    s32 r = (func_001163B0() >> 16) & 0x7FFF;
    s32 span = hi - lo + 1;

    return r % span + lo;
}
#endif

/* EU twin of USA func_002A86E0 (1A8180): random f32 in [lo, hi). Pure compute
 * (func_001163B0 random-state step + 2^-15 scale), no data globals -> no +0x80
 * swap, region-co-located callee (delta 0). */
f32 func_002A8290(f32 lo, f32 hi) {
    return lo + (f32)((func_001163B0() >> 16) & 0x7FFF) * (hi - lo) * 0.000030517578125f;
}

/* EU twin of USA func_002A8740 (1A8180): signed random f32 in [-hi, hi)-ish
 * (12-bit random, 2^-12 scale, coin-flip negate). Pure compute, no data globals. */
f32 func_002A82F0(f32 lo, f32 hi) {
    s32 r = func_001163B0() >> 16;
    f32 v = lo + (f32)(r & 0xFFF) * (hi - lo) * 0.000244140625f;

    if (r & 1) {
        v = -v;
    }
    return v;
}

/**
 * Random small angle: uniform in [-0x800, 0x800) scaled by pi/2048 — i.e. a
 * random angle in [-pi, pi). (EU twin of USA func_002A87A8.)
 */
f32 func_002A8358(void) {
    return (f32)(((func_001163B0() >> 16) & 0xFFF) - 0x800) * 0.0015339808f;
}

/* Spawn/place helper: draw two random angles and a random value in [lo, hi],
 * then hand them to func_002AFB68 for handle.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002A87F0: GetRandomAngle (func_002A87A8) -> func_002A8358,
 * GetRandomFloatRange (func_002A86E0) -> func_002A8290, func_002AFE68 -> func_002AFB68. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A83A0);
#else
extern void func_002AFB68(void *handle, f32 value, f32 angle1, f32 angle2);

void func_002A83A0(void *handle, f32 lo, f32 hi) {
    f32 angle1 = func_002A8358();
    f32 angle2 = func_002A8358();
    f32 value = func_002A8290(lo, hi);
    func_002AFB68(handle, value, angle1, angle2);
}
#endif

/* Random direction vector into dst: a random magnitude in [lo, hi] and two random
 * angles, mapped to cartesian (x = r*cos(a2)*sin(a1), y = r*sin(a2)*sin(a1),
 * z = r*cos(a1)). Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002A8868: func_002A86E0 -> func_002A8290,
 * GetRandomAngle (func_002A87A8) -> func_002A8358, cos (func_00283B30) -> func_00283A40,
 * sin (func_00283B48) -> func_00283A58. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A8418);
#else
extern f32 func_00283A40(f32 angle);   /* cosine */
extern f32 func_00283A58(f32 angle);   /* sine */

void func_002A8418(Vec4 *dst, f32 lo, f32 hi) {
    f32 radius = func_002A8290(lo, hi);
    f32 angle2 = func_002A8358();
    f32 angle1 = func_002A8358();
    dst->x = radius * func_00283A40(angle2) * func_00283A58(angle1);
    dst->y = radius * func_00283A58(angle2) * func_00283A58(angle1);
    dst->z = radius * func_00283A40(angle1);
}
#endif

/**
 * Cubic blend through two value pairs: hermite-style interpolation of the
 * (a0,a1) -> (b0,b1) segment at parameter t. (EU twin of USA func_002A8910.)
 */
f32 func_002A84C0(f32 a1, f32 a0, f32 b0, f32 b1, f32 t) {
    f32 c = (b1 - b0) - (a1 - a0);
    f32 t2 = t * t;
    f32 t3 = t2 * t;

    return c * t3 + ((a1 - a0) - c) * t2 + (b0 - a1) * t + a0;
}

/* Per-component cubic (Catmull-Rom-style) blend of four control vectors p1..p4
 * into out at parameter t: for x/y/z the vector form of func_002A84C0(p3,p1,p2,p4,t),
 * with w forced to 0; t==0 copies p1, t==1 copies p2 verbatim.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002A8948: func_002A8910 -> func_002A84C0 (reuse file-scope). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A84F8);
#else
void func_002A84F8(Vec4 *out, Vec4 *p1, Vec4 *p2, Vec4 *p3, Vec4 *p4, f32 t) {
    if (t == 0.0f) { *out = *p1; return; }
    if (t == 1.0f) { *out = *p2; return; }
    out->x = func_002A84C0(p3->x, p1->x, p2->x, p4->x, t);
    out->y = func_002A84C0(p3->y, p1->y, p2->y, p4->y, t);
    out->w = 0.0f;
    out->z = func_002A84C0(p3->z, p1->z, p2->z, p4->z, t);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A8618);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A86B0);

/* UpdateMobyFacingAngle: damped turn of a moby's heading (Moby+0xF8, yaw radians)
 * toward a target angle, then re-wrap into [-pi,pi]. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. EU-lockstep of USA func_002A8B08: WrapAnglePiDiff ->
 * func_002844A0, WrapAnglePiSum -> func_00284458 (EU .s: facingAngle at moby+0xF8,
 * offset raw since EU Moby typedef doesn't name it - verified vs func_002A86B8.s). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A86B8);
#else
void func_002A86B8(Moby *moby, f32 *driveOut, f32 targetAngle,
                   f32 gain, f32 damp, f32 clampLimit) {
    f32 *facing = (f32 *)((char *)moby + 0xF8);   /* Moby facingAngle (yaw radians) */
    f32 d = func_002844A0(targetAngle, *facing);  /* signed shortest turn to target */
    f32 s = d * 6.366197f;                         /* 0x40CBB7E3 = 20/pi */
    f32 v;
    f32 ad;

    if (s > 1.0f) {
        s = 1.0f;
    } else if (s < -1.0f) {
        s = -1.0f;
    }
    v = *driveOut;
    v = v + (gain * s - damp * v);                 /* damped feed-forward integrate */
    *driveOut = v;
    if (clampLimit != 0.0f) {
        if (clampLimit < v) {
            *driveOut = clampLimit;
        } else if (v < -clampLimit) {
            *driveOut = -clampLimit;
        }
    }
    ad = GetFloatAbs(d);                            /* never step past the target */
    if (ad < *driveOut) {
        *driveOut = GetFloatAbs(d);
    } else if (-GetFloatAbs(d) > *driveOut) {
        *driveOut = -GetFloatAbs(d);
    }
    *facing = func_00284458(*facing, *driveOut);   /* advance + re-wrap heading */
}
#endif

/* UpdateMobyFacingTowardPoint: point a moby at a target - heading from the planar
 * (x,y) delta (Moby pos at +0x10/+0x14), fed to func_002A86B8. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. EU-lockstep of USA func_002A8C70:
 * atan2 func_00283BF8 -> func_00283B08, UpdateMobyFacingAngle -> func_002A86B8
 * (pos.x/y raw offsets +0x10/+0x14 verified vs func_002A8820.s). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A8820);
#else
void func_002A8820(Moby *moby, Moby *target, f32 *driveOut,
                   f32 gain, f32 damp, f32 clampLimit) {
    f32 *mPos = (f32 *)((char *)moby + 0x10);     /* Moby pos.xy */
    f32 *tPos = (f32 *)((char *)target + 0x10);
    f32 dx = tPos[0] - mPos[0];
    f32 dy = tPos[1] - mPos[1];
    f32 angle = func_00283B08(dx, dy);
    func_002A86B8(moby, driveOut, angle, gain, damp, clampLimit);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A88A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A88B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A8C50);

/* func_002A8EF8: PARKED (bare). Mnemonic-hash MIS-MAP + STUMP-FUSION. The assigned
 * twin USA func_002A90A8 is a large VU0 directional-collision sweep (CollLine/
 * CollSphere/Vec*Vu0 jals); EU func_002A8EF8 has ZERO calls and only scans a
 * moby-group count/filter (g_nBoltCounterDisplayed+0x214 / D_001E0019+0xE587) - a
 * different function. Its .s head is also 5 orphan epilogue stumps (positive-$sp)
 * fused before the real start at func_002A8F20. The genuine func_002A90A8 twin
 * lives at a different EU address; leave INCLUDE_ASM pending a resplit/re-map. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A8EF8);

/**
 * Set the light-mode byte of every moby in a group. (EU twin of USA
 * func_002A9400.)
 */
void func_002A8FB0(s32 group, s32 lightMode) {
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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9008);

/* Moby-group iterator ADVANCE/filter step: walk the per-group slot list via the
 * iterator globals, return the next moby passing the active/inactive filter.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002A9550: all iterator globals are region-agnostic named
 * symbols (EU splat anchors them off g_nBoltCounterDisplayed, same absolute addrs). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9100);
#else
s32 func_002A9100(Moby **out, Moby *moby, s32 wantInactive, s32 wantActive) {
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

/* func_002A9268: leading 0x10 padding pair (orphaned epilogue of func_002A9100),
 * split off via the symbol_addrs pin so the real body below starts clean. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9268);

/**
 * func_002A9278 (USA func_002A96C8) - decrement the 8-bit countdown packed in
 * the top byte of *p (clamped at zero); returns nonzero once it reaches zero.
 * Region-agnostic body (recovered from the EU padding mis-split).
 */
s32 func_002A9278(u32 *p, s32 dec) {
    s32 t = ((s32)*p >> 24) - dec;

    if (t < 0) {
        t = 0;
    }
    *p = (*p & 0xFFFFFF) | (t << 24);
    return t == 0;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A92B0);

/* Solve the quadratic a*x^2 + b*x + c = 0 for real roots. Returns the root count
 * and writes them to out1/out2 (out1 >= out2). Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002A9708: sqrtf func_002835C0 -> func_002834D0, GetFloatAbs kept. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A92B8);
#else
extern f32 func_002834D0(f32 x);   /* sqrtf twin (USA func_002835C0) */

s32 func_002A92B8(f32 a, f32 b, f32 c, f32 *out1, f32 *out2)
{
    f32 disc = b * b - a * (c * 4.0f);

    if (disc == 0.0f) {
        *out1 = -b / (a + a);
        return 1;
    } else {
        f32 root = func_002834D0(GetFloatAbs(disc)); /* sqrt(|disc|) */
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

/* ProbeGroundHeight: ground height under a point - CollLine from z=0.01 up to
 * pos.z + zOffset, returns the hit z or 0. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA ProbeGroundHeight: CollLine -> func_00276098, g_collHitPoint kept
 * (EU splat anchors g_collHitPoint.z off g_nVendorBuyQuantity+0x8D60, same absolute addr). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A93C0);
#else
extern s32 CollLine(void *to, void *from, s32 mask, void *moby, void *out);  /* func_00276098 */

f32 func_002A93C0(Vec4 *pos, f32 zOffset, s32 mask) {
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
 * Ground probe with the default parameters: half a unit above the query point,
 * line mask 0x20. (EU twin of USA func_002A9888.)
 */
f32 func_002A9438(Vec4 *pos) {
    return ProbeGroundHeight(pos, 0.5f, 0x20);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9460);

/* func_002A9508(point, poly, count): even-odd point-in-polygon test in the XY
 * plane. For each polygon edge (adjacent vertices poly[i], poly[(i+1)%count])
 * that straddles the horizontal line y == point->y, it computes the edge's X
 * intersection with that line and toggles an inside flag when the crossing lies
 * left of point->x. Returns 1 if the point is inside (odd crossings), else 0.
 * Vertices are 16-byte Vec4 records; only .x/.y participate. Pure leaf.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002A9958: no calls, no data refs (delta 0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9508);
#else
s32 func_002A9508(Vec4 *point, Vec4 *poly, s32 count) {
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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A95D8);

/**
 * Re-target an active hit record: store the new owner/params and clear its
 * pending flag, then forward the moby to the hit-refresh helper. (EU twin of
 * USA func_002A9A38.)
 */
s32 func_002A95E8(f32 power, Moby *moby, s32 a, s32 b) {
    Moby *m = moby;

    *(s32 *)((u8 *)m + 0x10) = a;
    *(s32 *)((u8 *)m + 0x14) = b;
    *(f32 *)((u8 *)m + 0x1C) = power;
    *(s32 *)((u8 *)m + 0x20) = 0;
    return func_00283638(moby);
}

/* func_002A9618: fill a hit-event record (params + 128-bit source vector +
 * live flag). Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002A9A68: no calls, no data refs (delta 0). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9618);
#else
void func_002A9618(void *rec, s32 a, s32 b, f32 power, void *src) {
    *(s32 *)((u8 *)rec + 0x10) = a;
    *(s32 *)((u8 *)rec + 0x14) = b;
    *(f32 *)((u8 *)rec + 0x1C) = power;
    *(s32 *)((u8 *)rec + 0x20) = 1;          /* live flag */
    *(u_long128 *)rec = *(u_long128 *)src;    /* 128-bit source vector at +0x00 */
}
#endif

/* func_002A9640: issue a directional moby sphere-collision / damage query.
 * Builds a spread direction from the moby's orientation - for an oriented moby
 * (modeBits & 0x100) its motion vector (+0xE0) rescaled to 0.25 then offset by
 * the matrix column at +0xC0; otherwise a unit heading from the facing yaw
 * (cos, sin, z=1). The direction is scaled by scale and stamped with a fixed
 * .w magnitude (0x45AFDF66), then folded into a hit-event record together with
 * the source moby, class id, the two byte tags (arg4/arg5) and the impact
 * power. CollMobysSphere then gathers/broadcasts against that record within
 * radius and returns the hit count. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002A9A90: Vec3RescaleToLenVu0 -> func_002837E0,
 * Vec4AddVu0 -> func_00283580, cos -> func_00283A40, sin -> func_00283A58,
 * Vec4ScaleVu0 -> func_002835F0, func_002A9A68 -> func_002A9618,
 * CollMobysSphere -> func_00277DE8. self->facingAngle -> raw +0xF8 (EU Moby
 * lacks the named field; verified lwc1 0xF8 in func_002A9640.s). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9640);
#else
extern void func_002837E0(Vec4 *dst, f32 len, Vec4 *src);   /* Vec3RescaleToLenVu0 */
extern void func_00283580(Vec4 *dst, Vec4 *a, Vec4 *b);     /* Vec4AddVu0 */
extern void func_002835F0(Vec4 *dst, f32 s, const Vec4 *src); /* Vec4ScaleVu0 */
extern s32  func_00277DE8(void *targetList, void *filter, Moby *self,
                          void *hitEvent, f32 radius);       /* CollMobysSphere */

s32 func_002A9640(Moby *self, void *targetList, s32 arg3, s32 arg4, s32 arg5,
                  void *filter, f32 radius, f32 power, f32 scale) {
    Vec4 dir;
    u8   hitEvent[0x30];

    if (self->modeBits & 0x100) {
        func_002837E0(&dir, 0.25f, (Vec4 *)((u8 *)self + 0xE0));
        func_00283580(&dir, &dir, (Vec4 *)((u8 *)self + 0xC0));
    } else {
        dir.x = func_00283A40(*(f32 *)((u8 *)self + 0xF8));   /* cos(facingAngle) */
        dir.y = func_00283A58(*(f32 *)((u8 *)self + 0xF8));   /* sin(facingAngle) */
        dir.z = 1.0f;
    }
    func_002835F0(&dir, scale, &dir);
    dir.w = 5627.9248f;   /* 0x45AFDF66 */

    func_002A9618(hitEvent, (s32)self, arg3, power, &dir);
    *(u8  *)(hitEvent + 0x18) = (u8)arg4;
    *(u8  *)(hitEvent + 0x19) = (u8)arg5;
    *(u16 *)(hitEvent + 0x1A) = self->oClass;

    return func_00277DE8(targetList, filter, self, hitEvent, radius);
}
#endif

/* func_002A9788: attach-point variant of the directional sphere query.
 * Resolves a world-space query origin at moby attach point attachId
 * (func_002A0680), then runs the directional moby sphere-collision query
 * (func_002A9640) from that origin - forwarding the class/tag params and the
 * radius/power/scale floats unchanged. Returns the collision hit count.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002A9BD8: func_002A0AF8 -> func_002A0680,
 * func_002A9A90 -> func_002A9640. self is only passed through (no field access). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9788);
#else
extern void func_002A0680(Moby *self, s32 attachId, void *outPoint); /* attach-point world pos */

s32 func_002A9788(Moby *self, s32 attachId, s32 arg3, s32 arg4, s32 arg5,
                  void *filter, f32 radius, f32 power, f32 scale) {
    Vec4 origin;
    func_002A0680(self, attachId, &origin);
    return func_002A9640(self, &origin, arg3, arg4, arg5, filter,
                         radius, power, scale);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9830);

/* func_002A9838: record a moby collision hit into the 64-entry hit-event ring,
 * deduped per moby. If the moby already owns a ring slot (hitEventSlot != 0xFF)
 * that still belongs to it: when the new hit is closer (hitInfo depth +0x1C <
 * the slot's +0x2C) just OR the new flags into it and return; otherwise carry
 * the slot's existing flags forward into a fresh entry. A new entry is written
 * at the ring write cursor (g_pCollWorldData+0x14, wrapping mod 64): the hit
 * vec, flags (|carried), material/normal fields, depth, and the owning moby;
 * the moby's hitEventSlot is pointed at it and the cursor advances.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002A9C88: func_00283638 -> func_00283548;
 * g_collHitEventRing (file-scope) + g_pCollWorldData kept (EU anchors
 * g_nVendorBuyQuantity+0xCFB8 / +0x8D38 = same absolute addrs). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9838);
#else
extern u8  g_pCollWorldData[];          /* collision-world block; +0x14 = ring write cursor */
extern s32 func_00283548(Moby *moby);   /* zero entry[0..0x10] (EU twin of func_00283638) */

void func_002A9838(Moby *moby, void *hitInfo) {
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
        /* else: slot was reused by another moby - fall through to a new entry */
    }

    entry = g_collHitEventRing + ringCursor * 0x40;
    func_00283548((Moby *)entry);                       /* zero entry[0..0x10] */
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

/* func_002A9988 (== PostMobyHitEvent @0x2A9988): record a 0x40-byte hit/damage
 * event for moby into the 64-slot ring g_collHitEventRing (write cursor at
 * g_pCollWorldData+0x14, wraps mod 64). Dedupe: moby+0xA8 holds this moby's last
 * event slot (0xFF = none); if that slot still belongs to moby and the new hit is
 * CLOSER (dist < stored dist), just OR the new flags into the existing record and
 * return - otherwise a fresh slot is allocated, carrying the old flags forward
 * when the prior record was this moby's. Return is the record flags (discarded).
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * NAMING: kept as func_002A9988 (matches the .s glabel) to avoid touching the
 * pre-existing 5-arg PostMobyHitEvent decl/caller (arity-mismatch cleanup flagged
 * separately). EU-lockstep of USA PostMobyHitEvent: Vec3LengthVu0 -> func_002836B0;
 * g_collHitEventRing + g_pCollWorldData kept (same absolute addrs). dist in $f12. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9988);
#else
extern f32 func_002836B0(Vec4 *v);   /* Vec3LengthVu0 */

s32 func_002A9988(Moby *moby, s32 a1, s32 flags, Vec4 *vecA, Vec4 *vecB, f32 dist) {
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
    *(s32 *)(rec + 0x30)   = (0.0001f < func_002836B0(vecB)) ? 1 : 0; /* hasDir */
    *(Vec4 *)(rec + 0x00)  = *vecA;
    *(Vec4 *)(rec + 0x10)  = *vecB;
    ((u8 *)moby)[0xA8]     = (u8)ringCursor;
    *(s32 *)(g_pCollWorldData + 0x14) = (ringCursor + 1) & 0x3F;
    return *(s32 *)(rec + 0x24);
}
#endif

/**
 * Post a moby hit event with the default hit direction vector. (EU twin of USA
 * func_002A9F30.)
 */
s32 func_002A9AE0(Moby *moby, s32 a, s32 b, s32 c) {
    return PostMobyHitEvent(moby, a, b, c, (Vec4 *)&D_1A8BD0);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9B08);

/* func_002A9B10 (== GetWaterSurfaceHeight): water surface z (up axis) under a
 * point. First tries the dynamic wave heightfield (when g_bWaterWavesActive);
 * otherwise tests the static water-pool disc g_waterPool (xy centre, z surface,
 * w radius) - the point is "in the pool" when |pos.z - pool.z| < 0.5 and its xy
 * distance to the pool centre is within the radius. Returns the pool surface z
 * when inside, else the point's own z. When outNormal is non-null it gets a
 * unit +Z normal. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA GetWaterSurfaceHeight: SampleWaterHeightfield ->
 * func_002FE880, DistXYVu0 -> func_00283740, SetVec4UnitZ -> func_00283558,
 * GetFloatAbs kept. g_bWaterWavesActive/g_bWaterPoolActive/g_waterPool at
 * g_nVendorBuyQuantity +0x90/+0x94/+0x98. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9B10);
#else
extern s32  g_bWaterWavesActive;     /* gate for the wave-heightfield path */
extern s32  g_bWaterPoolActive;      /* gate for the static water-pool test */
extern Vec4 g_waterPool;             /* xy centre, z surface height, w radius */
extern s32  func_002FE880(f32 x, f32 y, f32 z, f32 *outHeight);  /* SampleWaterHeightfield */
extern f32  func_00283740(Vec4 *a, Vec4 *b);                     /* DistXYVu0 */
extern void func_00283558(Vec4 *dst);                            /* SetVec4UnitZ */

f32 func_002A9B10(Vec4 *pos, Vec4 *outNormal) {
    f32 height;

    if (g_bWaterWavesActive != 0) {
        if (func_002FE880(pos->x, pos->y, pos->z, &height) != 0) {
            return height;
        }
    }
    if (g_bWaterPoolActive != 0 &&
        GetFloatAbs(pos->z - g_waterPool.z) < 0.5f &&
        func_00283740(pos, &g_waterPool) < g_waterPool.w) {
        if (outNormal != 0) {
            func_00283558(outNormal);
        }
        return g_waterPool.z;
    }
    if (outNormal != 0) {
        func_00283558(outNormal);
    }
    return pos->z;
}
#endif

/* Euler-angles -> quaternion: build three axis-angle quaternions from
 * eulerAngles.x/.y/.z about axes 0/1/2 and concatenate (out = qx * qy, then
 * out = out * qz) via the VU0 quaternion helpers. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. EU-lockstep of USA func_002AA058:
 * func_00284248 (axis-angle quat build) -> func_00284158,
 * func_00284180 (quat multiply) -> func_00284090. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9C08);
#else
extern void func_00284158(Vec4 *out, f32 angle, s32 axis);   /* axis-angle quat build */
extern void func_00284090(Vec4 *out, Vec4 *a, Vec4 *b);      /* quaternion multiply */

void func_002A9C08(Vec4 *outQuat, Vec4 *eulerAngles) {
    Vec4 qx;
    Vec4 qy;
    Vec4 qz;

    func_00284158(&qx, eulerAngles->x, 0);
    func_00284158(&qy, eulerAngles->y, 1);
    func_00284158(&qz, eulerAngles->z, 2);
    func_00284090(outQuat, &qx, &qy);
    func_00284090(outQuat, outQuat, &qz);
}
#endif

/* QueueMobyBlobShadow: enqueue a ground blob shadow under a moby (drop-shadow
 * render pass). No-op unless the moby's shadow-enable byte (+0x31) is set and
 * the queue has room (< 32). Copies the moby position (+0x10) into the next slot,
 * probes the ground straight down (ProbeGroundHeight, zOffset 0.5, mask 0); on a
 * real surface (material != none) lifts the shadow 0.025 above the ground, stores
 * the hit normal, and computes an alpha from the moby's height above the ground:
 * alpha = baseAlpha * max(0.125*(8 - |moby.z - groundZ|), 0.25).
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA QueueMobyBlobShadow: ProbeGroundHeight -> func_002A93C0
 * (file-scope), GetCollHitMaterial -> func_00278D18, GetFloatAbs kept.
 * g_blobShadowCount = D_001B1380+0x2BC, g_blobShadowQueue = g_nVendorBuyQuantity+0x83F8
 * (stride 0x20), g_collHitNormal = g_nVendorBuyQuantity+0x8D78 (same absolute addrs). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9CA0);
#else
extern s32 GetCollHitMaterial(void);   /* func_00278D18: low 5 bits of hit poly info, 0 = none/water */
extern s32 g_blobShadowCount;          /* live entries (max 32); EU D_001B1380+0x2BC */
typedef struct BlobShadow {
    /* 0x00 */ Vec4 pos;               /* shadow centre (moby pos +0x10), z bumped to ground, w = alpha */
    /* 0x10 */ Vec4 normal;            /* ground normal (g_collHitNormal) */
} BlobShadow;                          /* 0x20 stride */
extern BlobShadow g_blobShadowQueue[]; /* EU g_nVendorBuyQuantity+0x83F8 */
extern Vec4 g_collHitNormal;           /* EU g_nVendorBuyQuantity+0x8D78 */

void func_002A9CA0(Moby *moby, f32 baseAlpha) {
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
    groundZ = func_002A93C0((Vec4 *)((u8 *)moby + 0x10), 0.5f, 0);
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
 * Two modes selected by D_1A8D50: in the default (0) mode a downward CollLine
 * (mask 0x22) is cast from the moby position (+0x10) over a 16-unit drop, gated by
 * the moby's on-ground factor (+0xE8) being >= 0.9; the ground z-delta
 * (g_collHitPoint.z minus moby.z) goes to moby+0x70 and a scale (moby+0xC * 1/4096)
 * to moby+0x74. In axis mode (nonzero) the ray is cast along the moby's scaled axis
 * vector (+0xE0): on a hit it stores the dot of (hitPoint - moby.pos) with the axis
 * to moby+0x70, 0.2 to +0x74, and flags moby+0xBD = 0xFF. No hit (or the height
 * gate) zeroes moby+0x70/+0x74. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. EU-lockstep of USA ProbeMobyGroundBelow: guard D_1A8CA0 ->
 * D_1A8D50, CollLine -> func_00276098, g_collHitPoint = g_nVendorBuyQuantity+0x8D58,
 * ScaleVec4IncludingW -> func_00283620, Vec4ScaleVu0 -> func_002835F0,
 * Vec4AddVu0 -> func_00283580, Vec4SubVu0 -> func_002835B0, Vec3DotVu0 -> func_00283670. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9DC8);
#else
extern s32  D_1A8D50;                                          /* probe-mode flag (USA D_1A8CA0) */
extern s32  func_00276098(void *to, void *from, s32 mask, void *moby, void *out); /* CollLine */
extern void func_00283620(Vec4 *dst, f32 scale, const Vec4 *src); /* ScaleVec4IncludingW */
extern void func_002835F0(Vec4 *dst, f32 s, const Vec4 *src);  /* Vec4ScaleVu0 */
extern void func_00283580(Vec4 *dst, Vec4 *a, Vec4 *b);        /* Vec4AddVu0 */
extern void func_002835B0(Vec4 *dst, Vec4 *a, Vec4 *b);        /* Vec4SubVu0 */
extern f32  func_00283670(Vec4 *a, Vec4 *b);                   /* Vec3DotVu0 */

s32 func_002A9DC8(Moby *moby) {
    Vec4 from;
    Vec4 to;
    Vec4 hitDelta;
    f32 zDelta;
    f32 scale;

    if (D_1A8D50 == 0) {
        if (*(f32 *)((u8 *)moby + 0xE8) < 0.9f) {   /* 0x3F666666 */
            *(s32 *)((u8 *)moby + 0x74) = 0;
            *(s32 *)((u8 *)moby + 0x70) = 0;
            return 0;
        }
        from = *(Vec4 *)((u8 *)moby + 0x10);
        from.z = *(f32 *)((u8 *)moby + 0x18) - 16.0f;   /* 0x41800000 */
        if (from.z < 0.5f) {
            from.z = 0.5f;
        }
        to = *(Vec4 *)((u8 *)moby + 0x10);
        to.z = *(f32 *)((u8 *)moby + 0x18) + 0.5f;
        if (func_00276098(&to, &from, 0x22, 0, 0) != 0) {
            zDelta = g_collHitPoint.z - *(f32 *)((u8 *)moby + 0x18);
            scale = *(f32 *)((u8 *)moby + 0xC) * 0.000244140625f;   /* 1/4096, 0x39800000 */
            *(f32 *)((u8 *)moby + 0x70) = zDelta;
            *(f32 *)((u8 *)moby + 0x74) = scale;
            return 0;
        }
    } else {
        func_00283620(&to, 0.0009765625f, (Vec4 *)moby);   /* 1/1024, 0x3A800000 */
        func_002835F0(&from, -8.0f, (Vec4 *)((u8 *)moby + 0xE0));   /* Vec4ScaleVu0 */
        func_00283580(&from, &to, &from);
        if (func_00276098(&to, &from, 0x22, 0, 0) != 0) {
            func_002835B0(&hitDelta, &g_collHitPoint, (Vec4 *)((u8 *)moby + 0x10));
            zDelta = func_00283670(&hitDelta, (Vec4 *)((u8 *)moby + 0xE0));
            scale = 0.200000003f;   /* 0x3E4CCCCD */
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
 * Forward to the moby ground-fit probe (drop-shadow placement). (EU twin of
 * USA func_002AA3B0.)
 */
s32 func_002A9F60(Moby *moby) {
    return ProbeMobyGroundBelow(moby);
}

/**
 * Forward to the shared vector helper func_002846E8. (EU twin of USA
 * func_002AA3D0.)
 */
s32 func_002A9F80(void *a, void *b, void *c) {
    return func_002846E8(a, b, c);
}

/* func_002A9FA0 (positional twin of USA func_002AA3F0, the ping-pong colour-ramp
 * triangle-wave blend): GENUINE PAL/NTSC SOURCE DIVERGENCE — do NOT port the USA
 * body. USA computes the wave over [0, period*2) directly (`sll period,1` for the
 * modulus base, compare `pos < period`), 0xB4 bytes. The EU twin instead rescales
 * the period: half-period = (period*5 + 2)/6 and full-period = (period*10 + 2)/6
 * (hardcoded mult-by-10 / div-by-6 with rounding, asm 0x108 bytes) — a deliberate
 * frame-rate retime of the animation for 50Hz PAL. Same phase counters
 * (D_1A9F14/D_1A9F18 = EU twins of USA D_1A9E94/D_1A9E98), same movz selector and
 * same func_002845F8 lerp tail (= USA func_002846E8), but the period arithmetic is
 * different source, not an address shift or schedule delta. Stays INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9FA0);

/**
 * Query a moby's pending hit-event record: returns the record when it carries
 * any of the requested event bits, otherwise 0 (releasing the record unless
 * keep is set). (EU twin of USA func_002AA4A8.)
 */
s32 func_002AA0A8(Moby *m, u32 mask, s32 keep) {
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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AA100);

/* func_002AA108: advance the per-owner "focus target" announcer state and return
 * the emphasis weight for this frame. owner's pvar word 0 holds a small state
 * block (st): current focus target moby (+0x18), a hold/cooldown tick counter
 * (+0x1C), the resolved announcer id (+0x17), a priority level (+0x04). Each call:
 * (1) drops the focus target if its moby has despawned (state 0xFE/0xFD);
 * (2) advances the hold counter, resetting target+counter after 0x64 (100) ticks.
 * Then, for the candidate event: latches a new focus target (resolving its
 * announcer id) or, on the same target, suppresses the weight (returns 0) while
 * inside the per-kind re-trigger window (kind 9 -> 0x32 ticks, else 0x8 ticks). The
 * weight (event +0x2C) is clamped up to 1.0 for low-priority owners, and a companion
 * state flag (owner pvar word 4, +0x18) is toggled by event flag 0x100000.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AA508: func_002AC058 -> func_002ABC58,
 * func_002AE7E8 -> func_002AE4E0, func_002AC088 -> func_002ABC88.
 * GENUINE PAL/NTSC TIMING DIFF: hold-expire 0x64 (USA 0x78), kind-9 window 0x32
 * (USA 0x3C), else window 0x8 (USA 0xA) -- 60->50 Hz frame-count retimes,
 * verified vs func_002AA108.s slti immediates (0x65 / 0x32 / 0x8). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AA108);
#else
extern void *func_002ABC58(Moby *owner);   /* focus-target state (pvar word 0) */
extern s32   func_002AE4E0(Moby *cand);    /* resolve candidate announcer id */
extern void *func_002ABC88(Moby *owner);   /* companion state (pvar word 4) */

f32 func_002AA108(Moby *owner, void *event) {
    u8 *st = (u8 *)func_002ABC58(owner);   /* focus-target state (pvar word 0) */
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

    /* advance the hold counter; expire target + counter after 0x64 (100) ticks (PAL) */
    counter = *(s16 *)(st + 0x1C);
    if (counter != 0) {
        *(s16 *)(st + 0x1C) = (s16)(*(u16 *)(st + 0x1C) + 1);
        if (counter >= 0x65) {
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
                *(u8 *)(st + 0x17) = (u8)func_002AE4E0(cand);
                *(s16 *)(st + 0x1C) = 1;
            } else {
                /* same target: gate the re-trigger window by event kind (PAL windows) */
                u8  kind = *(u8 *)((u8 *)event + 0x28);
                s16 c    = *(s16 *)(st + 0x1C);
                if (kind == 9) {
                    if (c < 0x32) {
                        weight = 0.0f;
                    } else {
                        *(s16 *)(st + 0x1C) = 1;
                    }
                } else {
                    if (c < 0x8) {
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
        u8 *st2 = (u8 *)func_002ABC88(owner);
        if (st2 != NULL) {
            *(u8 *)(st2 + 0x18) = 1;
        }
    } else {
        u8 *st2 = (u8 *)func_002ABC88(owner);
        if (st2 != NULL) {
            *(u8 *)(st2 + 0x18) = 0;
        }
    }

    return weight;
}
#endif

/* func_002AA2B8: broadcast a per-target directional damage packet to a moby list.
 * For each of the count mobys in list (skipping skip), computes the bearing from
 * refPos to that moby (atan2 on the xy delta), builds a hit-event packet -- a
 * magnitude-scaled direction (cos/sin of the bearing) in xy, zComp in z, the fixed
 * 0x45AFDF66 .w magnitude, plus the source moby / its class / the tag words (arg6)
 * and bytes (arg7/arg8) / the power float -- and posts it to that moby via the
 * hit-event recorder. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AA6B8: atan2 func_00283BF8 -> func_00283B08 (file-scope),
 * cos func_00283B30 -> func_00283A40, sin func_00283B48 -> func_00283A58,
 * func_002A9C88 -> func_002A9838 (file-scope). entry->pos.xy at raw +0x10/+0x14,
 * self->oClass at +0xAA (verified vs func_002AA2B8.s). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AA2B8);
#else
void func_002AA2B8(Moby *self, const Vec4 *refPos, Moby **list, s32 count,
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
        bearing = func_00283B08(*(f32 *)((u8 *)entry + 0x10) - ref.x,
                                *(f32 *)((u8 *)entry + 0x14) - ref.y);
        *(f32 *)(packet + 0x00) = func_00283A40(bearing) * magnitude;  /* cos */
        *(f32 *)(packet + 0x04) = func_00283A58(bearing) * magnitude;  /* sin */
        *(f32 *)(packet + 0x08) = zComp;
        *(f32 *)(packet + 0x0C) = 5627.9248f;   /* 0x45AFDF66 */
        *(s32 *)(packet + 0x10) = (s32)self;
        *(s32 *)(packet + 0x14) = arg6;
        *(u8  *)(packet + 0x18) = (u8)arg7;
        *(u8  *)(packet + 0x19) = (u8)arg8;
        *(u16 *)(packet + 0x1A) = *(u16 *)((u8 *)self + 0xAA);   /* self->oClass */
        *(f32 *)(packet + 0x1C) = power;
        *(s32 *)(packet + 0x20) = arg6;
        func_002A9838(entry, packet);
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AA408);

/* func_002AAB98: leading 0x10 padding pair, split off via the symbol_addrs pin
 * so the real lerp body below (func_002AABA8) starts clean. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AAB98);

/**
 * func_002AABA8 (USA func_002AAFA8) - linear interpolation: a + (b - a) * t.
 * Region-agnostic body (recovered from the EU padding mis-split).
 */
f32 func_002AABA8(f32 a, f32 b, f32 t) {
    return a + (b - a) * t;
}

/*
 * EU twin of USA func_002AAFB8 (0x2AAFB8). Two-stage scalar transform: feed
 * (b, a) through func_002844A0, scale the result by c, and forward (a, scaled)
 * to func_00284458. Byte-identical to the USA twin except the two callee
 * addresses (EU twins, shifted -0xF0). The s32 return + s32 func_00284458 decl
 * cancel into the original's exact $f0 passthrough (see the extern note above).
 */
s32 func_002AABB8(f32 a, f32 b, f32 c) {
    return func_00284458(a, func_002844A0(b, a) * c);
}

/* func_002AAC00: rate-limited integrator with symmetric clamps on *state. First
 * clamps *state to [-|maxDelta|, |maxDelta|]; then integrates
 * *state = v + (b*maxDelta - c*v) (i.e. v*(1-c) + b*maxDelta) and clamps to
 * [-bound, bound] when bound > 0; finally re-clamps to [-|maxDelta|, |maxDelta|].
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AB000: GetFloatAbs kept. No data globals.
 * b/c/bound arrive in $f13/$f14/$f15. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AAC00);
#else
void func_002AAC00(f32 *state, f32 maxDelta, f32 b, f32 c, f32 bound) {
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
 * target the stepped value still is (absolute). (EU twin of USA func_002AB150.)
 */
f32 func_002AAD50(f32 target, f32 rate, f32 *p) {
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

/* func_002AADA8: integer approach-by-rate step (int twin of the float approach).
 * Move *p toward target by at most rate, store it back, and return the remaining
 * signed delta mapped to a float. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. EU-lockstep of USA func_002AB1A8: int->float helper
 * func_002835E0 -> func_002834F0. No data globals. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AADA8);
#else
extern s32 func_002834F0(s32 x);   /* signed int -> float-source helper (USA func_002835E0) */

f32 func_002AADA8(s32 *p, s32 target, s32 rate) {
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
    return (f32)func_002834F0(target - nv);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AAE08);

/* func_002AAEC0: critically-damped Vec4 approach. Drives position `cur` toward
 * `target` along the connecting direction, integrating the scalar approach speed
 * into *vel via the spring step (func_002AAC00 over the current separation dist,
 * stiffness/damping/dt b/c/eps). The direction is rescaled to the new speed and
 * added back onto `cur`. Returns the residual separation (dist - speed); when that
 * drops below eps*0.01 the position snaps to `target` and the velocity is zeroed
 * (returning 0). Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AB2C0: Vec4SubVu0 -> func_002835B0, Vec3LengthVu0 ->
 * func_002836B0, func_002AB000 -> func_002AAC00, Vec3RescaleToLenVu0 -> func_002837E0,
 * Vec4AddVu0 -> func_00283580, GetFloatAbs kept. b/eps in $f12/$f14, c in $f13. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AAEC0);
#else
f32 func_002AAEC0(Vec4 *cur, Vec4 *target, f32 *vel, f32 b, f32 c, f32 eps) {
    Vec4 dir;
    f32 dist;
    f32 residual;

    func_002835B0(&dir, target, cur);                     /* dir = target - cur */
    dist = func_002836B0(&dir);                           /* separation distance */
    func_002AAC00(vel, dist, b, c, eps);                  /* spring-step the speed */
    func_002837E0(&dir, *vel, &dir);                      /* rescale dir to new speed */
    func_00283580(cur, cur, &dir);                        /* cur += dir */
    residual = dist - *vel;
    if (GetFloatAbs(residual) < eps * 0.009999999776f) {  /* 0x3C23D70A */
        *cur = *target;
        *vel = 0.0f;
        return *vel;
    }
    return residual;
}
#endif

/* func_002AAFB0: 1-D acceleration-limited "arrival" controller. Advance *pPos
 * toward `target` by integrating a velocity *pVel that is ramped up and braked so
 * the motion decelerates to a stop at the target. Returns the velocity applied this
 * step, or the residual distance when the step reaches/overshoots (then *pPos snaps
 * to target). Cases: stopped-on-target -> 0; velocity opposing target -> bleed by
 * accel then integrate; inside braking distance 0.5*vel^2/accel -> step velocity
 * toward 0; outside -> ramp toward arrival speed sqrt(2*accel*diff) capped at
 * maxSpeed. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AB3B0: sqrtf func_002835C0 -> func_002834D0,
 * func_002AB150 -> func_002AAD50, GetFloatAbs kept. pPos=$4, pVel=$5;
 * target/velRate/accel/maxSpeed in $f12/$f13/$f14/$f15. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AAFB0);
#else
f32 func_002AAFB0(f32 target, f32 velRate, f32 accel, f32 maxSpeed, f32 *pPos, f32 *pVel) {
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
                func_002AAD50(0.0f, accel, pVel);
            } else {
                func_002AAD50(0.0f, accel * 1.1f, pVel);
            }
        } else {
            /* still approaching: ramp velocity toward the arrival speed */
            f32 speed = func_002834D0(2.0f * accel * diff);
            if (maxSpeed < speed) {
                speed = maxSpeed;
            }
            if (diff < 0.0f) {
                func_002AAD50(-speed, velRate, pVel);
            } else {
                func_002AAD50(speed, velRate, pVel);
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

/* func_002AB1A0: wrap an angle into (-pi, pi] (via WrapAnglePiDiff), then when a
 * direction sign is supplied bias the result onto the requested rotation side: if
 * the wrapped delta already agrees with the sign (delta*sign > 0) keep it; an
 * almost-zero delta collapses to 0; otherwise add/subtract a full 2*pi turn so the
 * result rotates the requested way. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. EU-lockstep of USA func_002AB5A0: WrapAnglePiDiff func_00284590
 * -> func_002844A0, GetFloatAbs kept. sign=$4.
 * bc1f/bc1fl SIGN TRAP: the `0.0f < d*(f32)sign` test is a NON-likely bc1f (always
 * runs its delay slot); the sign multiply is d*cvt.s.w(sign) -> exactly d*(f32)sign.
 * The 2*pi branch `if (0.0f < d)` is a plain bc1t: taken -> d-2pi, else d+2pi. Sign
 * resolution matches USA func_002AB5A0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AB1A0);
#else
f32 func_002AB1A0(f32 a, f32 b, s32 sign) {
    f32 d = func_002844A0(a, b);   /* func_002844A0 == WrapAnglePiDiff(a - b) */

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

/* func_002AB268: step a stored angle (*p) toward target `a` by at most `maxStep`
 * (clamped both ways), wrapping the sum into (-pi, pi], and return the residual
 * signed angle difference after the step. `sign` forces the rotation side (see
 * func_002AB1A0). Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AB668: func_002AB5A0 -> func_002AB1A0,
 * WrapAnglePiSum func_00284548 -> func_00284458. a=$f12, maxStep=$f13, p=$4, sign=$5. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AB268);
#else
f32 func_002AB268(f32 a, f32 maxStep, f32 *p, s32 sign) {
    f32 delta = func_002AB1A0(a, p[0], sign);

    if (maxStep < delta) {
        delta = maxStep;
    } else if (delta < -maxStep) {
        delta = -maxStep;
    }
    p[0] = func_00284458(p[0], delta);   /* func_00284458 == WrapAnglePiSum */
    return func_002AB1A0(a, p[0], sign);
}
#endif

/* func_002AB300: critically-damped scalar angle driver. Eases stored angle *p
 * toward `target` while tracking its angular velocity in *vel. When mode==2 a
 * rotation-side `sign` is derived from the relative signs of target and *p (sign=+1
 * when target>0 & *p<0, -1 when target<0 & *p>0, else 0); any other mode is used
 * directly as the sign. func_002AB1A0 gives the signed wrapped delta; func_002AAC00
 * integrates the spring step into *vel; *p re-wraps against *vel and the residual is
 * recomputed. Below a dt-scaled epsilon the angle snaps to target and *vel is zeroed.
 * Returns the residual signed angle error (0 when snapped). Matching arm stays
 * INCLUDE_ASM; #else is the structure model. EU-lockstep of USA func_002AB700:
 * func_002AB5A0 -> func_002AB1A0, func_002AB000 -> func_002AAC00, WrapAnglePiSum
 * func_00284548 -> func_00284458, GetFloatAbs kept. p=$4, vel=$5, mode=$6;
 * target/b/c/d in $f12/$f13/$f14/$f15. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AB300);
#else
f32 func_002AB300(f32 *p, f32 *vel, s32 mode, f32 target, f32 b, f32 c, f32 d) {
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

    delta = func_002AB1A0(target, *p, sign);
    func_002AAC00(vel, delta, b, c, d);
    *p = func_00284458(*p, *vel);            /* func_00284458 == WrapAnglePiSum */
    r = func_002AB1A0(target, *p, sign);
    if (GetFloatAbs(r) < d * 0.009999999776f) {   /* 0x3C23D70A */
        *p = target;
        *vel = 0.0f;
        return *vel;
    }
    return r;
}
#endif

/* func_002AB468: angular counterpart of func_002AAFB0. Drive an angle *pAngle
 * toward `target` with an acceleration-limited angular velocity *pVel, honouring
 * shortest-arc (-pi, pi] wrapping. Deltas go through func_002AB1A0 (signed wrapped
 * diff on the chosen rotation side), the accelerating velocity step goes through
 * func_002AB268 (clamped, wrapping), and the angle integrates via WrapAnglePiSum.
 * mode==2 derives the shorter-arc sign from the signs of target and *pAngle;
 * otherwise mode is used directly. Returns the residual signed angle error.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AB868: func_002AB5A0 -> func_002AB1A0, sqrtf
 * func_002835C0 -> func_002834D0, func_002AB668 -> func_002AB268, func_002AB150 ->
 * func_002AAD50, WrapAnglePiSum func_00284548 -> func_00284458, GetFloatAbs kept.
 * pAngle=$4, pVel=$5, mode=$6; target/maxStep/accel/maxSpeed in $f12..$f15.
 * bc1f/bc1fl SIGN TRAP: the mode==2 sign lattice uses NON-likely bc1f sign-selects
 * (delay slots always run: they hold the sign result), and a LIKELY bc1fl whose
 * nullified delay reloads *pAngle for the func_002AB1A0 call only on fall-through.
 * Sign resolution matches USA func_002AB868; this sign feeds func_002AB1A0's delta
 * (== USA func_002AB5A0 delta) with the correct 0/+1/-1 value. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AB468);
#else
f32 func_002AB468(f32 target, f32 maxStep, f32 accel, f32 maxSpeed, f32 *pAngle, f32 *pVel, s32 mode) {
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

    delta = func_002AB1A0(target, *pAngle, sign);

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
                func_002AAD50(0.0f, accel, pVel);
            } else {
                func_002AAD50(0.0f, accel * 1.1f, pVel);
            }
        } else {
            /* still approaching: ramp velocity toward the arrival speed */
            f32 speed = func_002834D0(2.0f * accel * delta);
            if (maxSpeed < speed) {
                speed = maxSpeed;
            }
            if (delta < 0.0f) {
                func_002AB268(-speed, maxStep, pVel, 0);
            } else {
                func_002AB268(speed, maxStep, pVel, 0);
            }
        }

        absDelta = GetFloatAbs(delta);
        absVel = GetFloatAbs(*pVel);
        if (absVel < absDelta) {
            *pAngle = func_00284458(*pVel, *pAngle);   /* func_00284458 == WrapAnglePiSum */
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
        *pAngle = func_00284458(*pVel, *pAngle);       /* func_00284458 == WrapAnglePiSum */
        return *pVel;
    }
}
#endif

/* func_002AB6E8: single-state variant of func_002AAFB0. Ramp/brake a velocity *pVel
 * so it arrives at a target `dist` units away, WITHOUT integrating a position (the
 * caller adds *pVel to its own position each step). `dist` is the signed remaining
 * distance, passed directly. Same three cases as func_002AAFB0 (brake inside the
 * stopping distance 0.5*vel^2/accel; else ramp toward arrival speed sqrt(2*accel*dist)
 * capped at maxSpeed; velocity opposing target bled off by accel), all via
 * func_002AAD50. The finalize clamps *pVel so its magnitude never exceeds |dist|.
 * The two callers discard the result, so this is void. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. EU-lockstep of USA func_002ABAE8: sqrtf func_002835C0
 * -> func_002834D0, func_002AB150 -> func_002AAD50, GetFloatAbs kept. pVel=$4;
 * dist/velRate/accel/maxSpeed in $f12/$f13/$f14/$f15. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AB6E8);
#else
void func_002AB6E8(f32 dist, f32 velRate, f32 accel, f32 maxSpeed, f32 *pVel) {
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
                func_002AAD50(0.0f, accel, pVel);
            } else {
                func_002AAD50(0.0f, accel * 1.1f, pVel);
            }
        } else {
            /* still approaching: ramp velocity toward the arrival speed */
            f32 speed = func_002834D0(2.0f * accel * dist);
            if (maxSpeed < speed) {
                speed = maxSpeed;
            }
            if (dist < 0.0f) {
                func_002AAD50(-speed, velRate, pVel);
            } else {
                func_002AAD50(speed, velRate, pVel);
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

/* Pack four [0,1] float colour components into a 0xAABBGGRR u32. Each of
 * r/g/b/a ($f12/$f13/$f14/$f15) is scaled by 255.0, truncated to int
 * (FloatToInt), masked to a byte and shifted into its channel: r=bits 0-7,
 * g=8-15, b=16-23, a=24-31. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. EU-lockstep of USA func_002ABD00: FloatToInt -> func_002845B0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AB900);
#else
extern s32 func_002845B0(f32 x);   /* FloatToInt */
s32 func_002AB900(f32 r, f32 g, f32 b, f32 a) {
    s32 ri = func_002845B0(r * 255.0f) & 0xFF;
    s32 gi = func_002845B0(g * 255.0f) & 0xFF;
    s32 bi = func_002845B0(b * 255.0f) & 0xFF;
    s32 ai = func_002845B0(a * 255.0f);
    return ri | (gi << 8) | (bi << 16) | (ai << 24);
}
#endif

/**
 * Conditionally exchange three values: bit0 swaps a/b, bit1 swaps b/c, bit2
 * swaps a/c. (EU twin of USA func_002ABDA8.)
 */
void func_002AB9A8(s32 *a, s32 *b, s32 *c, s32 bits) {
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

/* Conditionally permute the low three colour channels of a packed 0xAABBGGRR
 * word. bits==0 returns the word unchanged; otherwise R/G/B are unpacked and
 * swapped per func_002AB9A8's bit mask (bit0 R<->G, bit1 G<->B, bit2 R<->B),
 * alpha (top byte) preserved. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. EU-lockstep of USA func_002ABE08: func_002ABDA8 -> func_002AB9A8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ABA08);
#else
u32 func_002ABA08(u32 word, s32 bits) {
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
    func_002AB9A8(&r, &g, &b, bits);
    return (a << 24) | (b << 16) | (g << 8) | r;
}
#endif

/* Orthonormalise the 3 columns of the rotation 3x3 of a Mat4x4. For each column
 * i (0..2): gather the i-th element of rows 0,1,2 (stride 0x10) into a scratch
 * Vec3 (w zeroed), normalise to unit length via func_002837E0 (len 1.0), scatter
 * it back into column i. Matching arm stays INCLUDE_ASM; #else is the structure
 * model. EU-lockstep of USA func_002ABE90: Vec3RescaleToLenVu0 -> func_002837E0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ABA90);
#else
void func_002ABA90(f32 *mat) {
    f32 *base = mat;
    s32 i;

    for (i = 0; i < 3; i++) {
        Vec4 col;
        f32 *src = base + i;        /* mat row0, col i */
        s32 j;

        col.w = 0.0f;
        for (j = 0; j < 3; j++) {
            ((f32 *)&col)[j] = *src;
            src += 4;               /* next row (stride 0x10 bytes) */
        }
        func_002837E0(&col, 1.0f, &col);   /* Vec3RescaleToLenVu0 */
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

/* Transform the delta (c - a) into the local frame of quaternion q and return
 * component `idx` of the result. Builds the rotation matrix from q (func_00283F58),
 * transforms the delta by it (func_00283980), reads out result[idx]. Matching arm
 * stays INCLUDE_ASM; #else is the structure model. EU-lockstep of USA func_002ABF50:
 * Vec4SubVu0 -> func_002835B0, func_00284048 -> func_00283F58, func_00283A70 -> func_00283980. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ABB50);
#else
typedef struct Mat4x4 { Vec4 row[4]; } Mat4x4;
extern void func_00283F58(Mat4x4 *dst, const Vec4 *quat);            /* quat -> rot matrix */
extern void func_00283980(Vec4 *dst, const Vec4 *vec, const Mat4x4 *mat); /* dst = mat * vec */
f32 func_002ABB50(const Vec4 *a, const Vec4 *q, const Vec4 *c, s32 idx) {
    Vec4 delta;
    Vec4 result;
    Mat4x4 mat;

    func_002835B0(&delta, (Vec4 *)c, (Vec4 *)a);   /* Vec4SubVu0: delta = c - a */
    func_00283F58(&mat, q);
    func_00283980(&result, &delta, &mat);
    return ((f32 *)&result)[idx];
}
#endif

/* Remove the component of `vec` along `axis`, scaled by `scale`. Normalises axis
 * (func_002837E0, len 1.0), projects vec onto it (func_00283670), scales the
 * projection by `scale`, scales the unit axis by that amount and subtracts it
 * from vec into *out: out = vec - scale * dot(vec, unit(axis)) * unit(axis).
 * scale in $f12. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002ABFD0: Vec3RescaleToLenVu0 -> func_002837E0,
 * Vec3DotVu0 -> func_00283670, Vec4ScaleVu0 -> func_002835F0, Vec4SubVu0 -> func_002835B0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ABBD0);
#else
void func_002ABBD0(Vec4 *out, Vec4 *vec, Vec4 *axis, f32 scale) {
    Vec4 unit;
    Vec4 proj;
    f32 amount;

    func_002837E0(&unit, 1.0f, axis);                /* Vec3RescaleToLenVu0 */
    amount = func_00283670(&unit, vec) * scale;      /* Vec3DotVu0 */
    func_002835F0(&proj, amount, &unit);             /* Vec4ScaleVu0 */
    func_002835B0(out, vec, &proj);                  /* Vec4SubVu0 */
}
#endif

/* Read word 0 of a moby's extra/pvar block (mode bit 0x20 gates the block); NULL
 * moby or missing block returns 0. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. EU-lockstep of USA func_002AC058 (no callees). PIN:
 * void *(Moby *) - matches the file-scope decl + batch-4 caller func_002AA108. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ABC58);
#else
void *func_002ABC58(Moby *owner) {
    if (owner == 0) {
        return (void *)0;
    }
    if ((owner->modeBits & 0x20) != 0) {
        return (void *)owner->pExtra[0];
    }
    return (void *)0;
}
#endif

/* Read word 4 of a moby's extra/pvar block (mode bit 0x20 gates the block); NULL
 * moby or missing block returns 0. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. EU-lockstep of USA func_002AC088 (no callees). PIN:
 * void *(Moby *) - matches the file-scope decl + batch-4 caller func_002AA108. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ABC88);
#else
void *func_002ABC88(Moby *owner) {
    if (owner == 0) {
        return (void *)0;
    }
    if ((owner->modeBits & 0x20) != 0) {
        return (void *)owner->pExtra[4];
    }
    return (void *)0;
}
#endif

/* func_002ABCB8: emit a small burst of type-04 particles around a point. Reserves
 * up to 20 particle slots for owner within radius 14 of basePos (func_002AA408),
 * then for each granted slot builds a jittered offset direction from two random
 * angles (func_002AFB68, magnitude 0.036), adds it to basePos, nudges the result up
 * in Z by 0.018, and spawns a type-04 particle there with two randomised lifetime
 * parameters (func_002A8238 20..35 and 40..60). arg3 -> the reservation helper
 * (owner-context, UNCONFIRMED). Matching arm stays INCLUDE_ASM; #else is the structure
 * model. EU-lockstep of USA func_002AC0B8: func_002AA808 -> func_002AA408,
 * GetRandomAngle -> func_002A8358, func_002AFE68 -> func_002AFB68, Vec4AddVu0 ->
 * func_00283580, RandRangeInclusive -> func_002A8238, SpawnParticleType04 -> func_002BBA10.
 * GENUINE PAL/NTSC DIFF: the jitter magnitude is 0.036 (0x3D1374BD) and the Z nudge
 * 0.018 (0x3C9374BD) in PAL; the USA twin uses 0.03 / 0.015 -- 60->50 Hz per-frame
 * rate retimes (ratio 0.8333). Verified vs func_002ABCB8.s lui/ori pairs. Take these
 * from the EU .s, never from the USA body: an earlier symbol-swap port inherited the
 * NTSC pair here. The literals are spelled to the exact ROM bits -- plain 0.036f /
 * 0.018f compile 1 ULP low (0x3D1374BC / 0x3C9374BC).
 * SECOND GENUINE PAL/NTSC DIFF -- expressed as ARITHMETIC, not a constant: PAL
 * retimes the FIRST lifetime by 5/6 after the call. The EU .s hoists the divisor
 * (addiu $20, $0, 0x6) out of the loop, then does sll $16,$2,2 / addu $16,$16,$2 /
 * addiu $16,$16,0x2 / div $0,$16,$20 / mflo $16, i.e. r1 = (ret * 5 + 2) / 6 with an
 * explicit rounding term. The USA twin (func_002AC0B8) instead has a bare
 * daddu $16,$2,$0 -- raw. Because the retime is arithmetic rather than a literal, no
 * constant-comparison audit can see it; take it from the EU .s.
 * The RandRangeInclusive bounds (20..35, 40..60), the SECOND lifetime r2 (EU
 * daddu $10,$2,$0 == USA daddu $10,$2,$0, raw in both) and the 30 (0x1E in both) ARE
 * region-NEUTRAL -- verified instruction-for-instruction against both .s files. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ABCB8);
#else
extern s32  func_002AA408(void *owner, s32 max, void *recs, void *ctx, f32 radius);
extern void func_002BBA10(void *rec, Vec4 *pos, u32 color, s32 a, s32 b,
                          s32 c, s32 d, s32 e);

void func_002ABCB8(void *owner, Vec4 *basePos, void *arg3) {
    u8  spawnRecs[20][0x10];   /* func_002AA408 fills up to 20 0x10-byte records */
    Vec4 dir;
    s32 count;
    s32 i;

    count = func_002AA408(owner, 20, spawnRecs, arg3, 14.0f);
    if (count <= 0) {
        return;
    }
    for (i = 0; i < count; i++) {
        f32 angle1 = func_002A8358();
        f32 angle2 = func_002A8358();
        s32 r1;
        s32 r2;

        func_002AFB68(&dir, 0.036000002f, angle1, angle2);  /* 0x3D1374BD (USA 0.03f) */
        func_00283580(&dir, &dir, basePos);
        dir.z += 0.018000001f;                              /* 0x3C9374BD (USA 0.015f) */

        /* PAL: .s applies the 5/6 retime as arithmetic, (ret*5+2)/6, not as a constant */
        r1 = (func_002A8238(20, 35) * 5 + 2) / 6;   /* RandRangeInclusive */
        r2 = func_002A8238(40, 60);
        func_002BBA10(spawnRecs[i], &dir, 0x7000A0FFu, 0xFF, r1, 30, r2, 1);
    }
}
#endif

/* func_002ABE08: convert a 3x3 rotation matrix (row stride 0x10 = 3 floats + pad)
 * into a quaternion out (x,y,z,w at +0x0/+0x4/+0x8/+0xC) - Shepperd's method. If
 * the trace is positive, use the direct w-largest form; otherwise pick the largest
 * diagonal element as the pivot p (with j,k the cyclic successors from the {1,2,0}
 * table D_1A9F20) and build the quaternion around out[p]. func_002834D0 = sqrtf.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AC1E0: func_002835C0 -> func_002834D0,
 * D_1A9EA0 -> D_1A9F20. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ABE08);
#else
extern s32 D_1A9F20[];   /* cyclic next-index table {1, 2, 0} */

void func_002ABE08(void *out, void *matrix) {
    f32 *q = (f32 *)out;
    f32 *m = (f32 *)matrix;
#define M(i, j) m[(i) * 4 + (j)]     /* M[i][j]; rows are 0x10 bytes (4 floats) apart */
    f32 trace = M(0, 0) + M(1, 1) + M(2, 2);

    if (0.0f < trace) {
        f32 s = func_002834D0(trace + 1.0f);
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
        j = D_1A9F20[p];
        k = D_1A9F20[j];

        s = func_002834D0(M(p, p) - M(j, j) - M(k, k) + 1.0f);
        q[p] = 0.5f * s;
        scale = (s == 0.0f) ? s : (0.5f / s);
        q[3] = (M(k, j) - M(j, k)) * scale;     /* w */
        q[j] = (M(j, p) + M(p, j)) * scale;
        q[k] = (M(k, p) + M(p, k)) * scale;
    }
#undef M
}
#endif

/* func_002AC090: build a scratch transform/matrix from in (func_00283F18), feed it
 * through func_002ABE08 with out, then resolve in against it (func_00283F38). The
 * 0x40-byte scratch is a 4x4 matrix shared by all three helpers.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AC468: func_00284008 -> func_00283F18,
 * func_002AC1E0 -> func_002ABE08, func_00284028 -> func_00283F38. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC090);
#else
extern void func_00283F18(void *dst, void *src);
extern void func_00283F38(void *src, void *m);

void func_002AC090(void *out, void *in) {
    u8 scratch[0x40];   /* 4x4 matrix */

    func_00283F18(scratch, in);
    func_002ABE08(out, scratch);
    func_00283F38(in, scratch);
}
#endif

/**
 * Copy the hero's velocity pair (+0x38, 64-bit) onto another moby. (EU twin of
 * USA func_002AC4B8.)
 */
void func_002AC0E0(Moby *moby) {
    *(u64 *)((u8 *)moby + 0x38) = *(u64 *)((u8 *)g_pHeroMoby[0] + 0x38);
}

/* func_002AC0F8: axis-angle -> quaternion. out.xyz = axis(src) * sin(angle/2),
 * out.w = cos(angle/2). Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AC4D0: sin (func_00283B48) -> func_00283A58,
 * Vec4ScaleVu0 -> func_002835F0, cos (func_00283B30) -> func_00283A40. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC0F8);
#else
void func_002AC0F8(Vec4 *out, const Vec4 *src, f32 angle) {
    f32 half = angle * 0.5f;

    func_002835F0(out, func_00283A58(half), src);   /* Vec4ScaleVu0(out, sin(half), src) */
    out->w = func_00283A40(half);                    /* +0xC = w = cos(half) */
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC160);

/* func_002AC168 (MatrixToEulerAngles): extract ZYX-style euler angles from a
 * rotation matrix mtx into out[0..2]. Copies the 3x3 into a scratch matrix
 * (translation row zeroed to {0,0,0,1}), then peels the angles with three atan2
 * (func_00283B08) + Givens rotations that successively zero the off-axis terms:
 * a1=atan2(row0.x,row0.y) about -Z, then a2=atan2(row0.x,-row0.z) about -Y, then
 * a3=atan2(row1.y,row1.z) on the residual. Writes out[0]=a3, out[1]=a2, out[2]=a1.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA MatrixToEulerAngles: func_00283638 -> func_00283548, atan2
 * (func_00283BF8) -> func_00283B08, func_00283DC0 -> func_00283CD0,
 * MatrixMultiplyVu0 -> func_00284048, func_00283DE0 -> func_00283CF0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC168);
#else
extern void func_00283CD0(Mat4x4 *dst, Vec4 *in);   /* build a rotation matrix from a vec (VU0) */
extern void func_00283CF0(Mat4x4 *dst, Vec4 *in);   /* sibling rotation builder */
extern void func_00284048(Mat4x4 *dst, Mat4x4 *a, Mat4x4 *b);  /* MatrixMultiplyVu0 */

void func_002AC168(Mat4x4 *mtx, void *out) {
    Mat4x4 m;
    Mat4x4 rot;
    Vec4 axis;
    f32 *e = (f32 *)out;
    f32 a1, a2, a3;

    m = *mtx;
    func_00283548((Moby *)((u8 *)&m + 0x30));   /* zero the translation row */
    *(f32 *)((u8 *)&m + 0x3C) = 1.0f;

    a1 = func_00283B08(*(f32 *)((u8 *)&m + 0x00), *(f32 *)((u8 *)&m + 0x04));
    axis.x = 0.0f;
    axis.y = 0.0f;
    axis.z = -a1;
    func_00283CD0(&rot, &axis);
    func_00284048(&m, &rot, &m);

    a2 = func_00283B08(*(f32 *)((u8 *)&m + 0x00), -*(f32 *)((u8 *)&m + 0x08));
    axis.x = 0.0f;
    axis.y = -a2;
    axis.z = 0.0f;
    func_00283CF0(&rot, &axis);
    func_00284048(&m, &rot, &m);

    a3 = func_00283B08(*(f32 *)((u8 *)&m + 0x14), *(f32 *)((u8 *)&m + 0x18));
    e[2] = a1;
    e[1] = a2;
    e[0] = a3;
}
#endif

/* func_002AC290: advance a countdown/fade field pair on obj. When the counter
 * (+0x0) is running (!=0) but its active flag (+0x2) is clear, does nothing.
 * Otherwise clears the flag, reloads the counter from its reset value (+0xC), and
 * either: counter was 0 -> refresh the RGB bytes (+0x4/+0x5/+0x6) from src's packed
 * colour (func_002A0E78); counter was running -> rescale it to resetValue *
 * counter / divisor(+0xE), clamped to at least 1. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. Symbol map vs USA func_002AC668: IntToFloat ->
 * func_002845A0, FloatToInt -> func_002845B0, func_002A12F0 -> func_002A0E78.
 *
 * NOT a pure symbol-swap of the USA twin. EU is 76 instrs vs USA's 48 because
 * the PAL build retimes three of the durations it reads by 5/6 (60Hz frame
 * counts -> 50Hz), expressed in the .s as ARITHMETIC rather than as changed
 * constants: `$17 = 6` is pinned as a divisor across the body (hence the extra
 * callee-saved $18 and $31 moving 0x20 -> 0x28) and each site computes
 * `(v * 5 + 2) / 6` via sll/addu/addiu/div/mflo. The retime is SELECTIVE: the
 * live counter read from +0x0 is fed to the FPU UN-retimed. EU also loads +0xC
 * with `lh` (SIGNED) where USA uses `lhu`. Mechanically porting the USA body
 * here silently reinstates NTSC timings -- see the per-site notes below. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC290);
#else
extern void func_002A0E78(void *src, s32 *r, s32 *g, s32 *b);

/* PAL 60Hz->50Hz frame-count retime, `(v * 5 + 2) / 6` with a rounding bias of 2.
 * EU-only; the USA twin uses the raw value at each of these sites. In the .s this
 * is open-coded per site against the pinned divisor in $17 (addiu $17,$0,0x6):
 *   80180200 sll $3,$2,2 / 21186200 addu $3,$3,$2 / 02006324 addiu $3,$3,0x2
 *   1A007100 div $0,$3,$17 / 12180000 mflo $3 */
#define EU_PAL_RETIME(v) (((v) * 5 + 2) / 6)

void func_002AC290(void *src, u8 *obj) {
    s16 counter = *(s16 *)(obj + 0x0);

    if (counter != 0 && *(s16 *)(obj + 0x2) == 0) {
        return;
    }

    *(s16 *)(obj + 0x2) = 0;
    /* Site 1: counter reload from +0xC. EU `0C000286` lh $2,0xC($16) is SIGNED
     * (USA's `0C000296` is lhu), then retimed before `000003A6` sh $3,0x0($16). */
    *(s16 *)(obj + 0x0) = (s16)EU_PAL_RETIME(*(s16 *)(obj + 0xC));

    if (counter == 0) {
        s32 r, g, b;
        func_002A0E78(src, &r, &g, &b);
        obj[0x4] = (u8)r;
        obj[0x6] = (u8)b;
        obj[0x5] = (u8)g;
    } else {
        /* Site 2: the divisor read from +0xE (`0E000386` lh $3,0xE($16)) is
         * retimed into $4 before `68110A0C` jal func_002845A0.
         * NOT retimed: `counter`, the live +0x0 value in $18, which goes
         * straight to `00089244` mtc1 $18,$f1 / cvt.s.w. */
        f32 ratio = (f32)counter / func_002845A0(EU_PAL_RETIME(*(s16 *)(obj + 0xE)));
        /* Site 3: the multiplicand is a fresh signed load of +0xC
         * (`0C000486` lh $4,0xC($16)), retimed into $2 before
         * `00608244` mtc1 $2,$f12 / cvt.s.w / `02630146` mul.s. */
        s32 scaled = func_002845B0((f32)EU_PAL_RETIME(*(s16 *)(obj + 0xC)) * ratio);
        *(s16 *)(obj + 0x0) = (s16)scaled;
        if ((s16)scaled <= 0) {
            *(s16 *)(obj + 0x0) = 1;
        }
    }
}

#undef EU_PAL_RETIME
#endif

/* func_002AC3C0: per-frame RGB colour fade / ping-pong applied to a target object.
 * s is the fade state block: s[0x0](s16) frames remaining (ticked via func_00283238),
 * s[0x2](s16) direction flag (0=current->dest, nonzero=dest->current), s[0x4..0x6](u8)
 * current RGB, s[0x7..0x9](u8) per-channel destination (a 0 byte leaves that channel
 * unchanged), s[0xC](s16) forward-fade duration, s[0xE](s16) reverse-fade duration +
 * restart reload. Each frame: while the counter is nonzero, tick it; at 0 either latch
 * the destination and stop (dir set) or restart reversed (dir clear, counter reloaded).
 * Otherwise interpolate each enabled channel by frac=(duration-counter)/duration and
 * push the colour to target via func_002A0E48 (returns 0 when inactive). Matching arm
 * stays INCLUDE_ASM; #else is the structure model. Symbol map vs USA func_002AC728:
 * func_00283328 -> func_00283238, func_002A12C0 -> func_002A0E48, IntToFloat ->
 * func_002845A0, FloatToInt -> func_002845B0.
 *
 * NOT a pure symbol-swap of the USA twin. EU is 172 instrs vs USA's 147 because the
 * PAL build retimes all three of the fade DURATIONS it reads by 5/6 (60Hz frame
 * counts -> 50Hz), expressed in the .s as ARITHMETIC rather than as changed
 * constants: each site materialises its own `addiu $N,$0,0x6` divisor and computes
 * `(v * 5 + 2) / 6` via sll/addu/addiu/div/mflo (three div/mflo pairs in EU, zero in
 * USA). The retime is SELECTIVE: the live counter read from +0x0 is fed to the FPU
 * UN-retimed at both interpolation sites -- it is already a PAL-rate tick produced by
 * func_00283238, so only the durations it is measured against are converted. EU also
 * loads the +0xE restart reload with `lh` (SIGNED) where USA uses `lhu`. Mechanically
 * porting the USA body here silently reinstates NTSC timings -- see the per-site
 * notes below. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC3C0);
#else
extern s32 func_00283238(void *state); /* tick countdown: 0=counting, 1=already 0, 2=just hit 0 */
extern s32 func_002A0E48(void *target, s32 r, s32 g, s32 b);

/* PAL 60Hz->50Hz frame-count retime, `(v * 5 + 2) / 6` with a rounding bias of 2.
 * EU-only; the USA twin uses the raw value at each of these sites. Re-defined locally
 * here because func_002AC290's copy is #undef'd before this body. In the .s each site
 * open-codes it against its own freshly materialised divisor, e.g. at .L002AC468:
 *   06000224 addiu $2,$0,0x6 / 80200300 sll $4,$3,2 / 21208300 addu $4,$4,$3
 *   02008424 addiu $4,$4,0x2 / 1A008200 div $0,$4,$2 / 12200000 mflo $4
 * The `01004050 beql $2,$0` + `CD010000 break 0,7` pair guarding each div is
 * ee-gcc's divide-by-zero trap, confirming a real C-level division. */
#define EU_PAL_RETIME(v) (((v) * 5 + 2) / 6)

s32 func_002AC3C0(void *target, u8 *s) {
    s32 counter;
    s32 dir;
    f32 frac;
    s32 r, g, b;

    if (*(s16 *)(s + 0x0) == 0) {
        return 0;
    }

    if (func_00283238(s) != 0) {
        /* countdown expired this frame */
        if (*(s16 *)(s + 0x2) != 0) {
            /* latch the destination colour and stop */
            return func_002A0E48(target, s[0x4], s[0x5], s[0x6]);
        }
        /* restart the fade running in reverse.
         * Site 1: the reverse duration read from +0xE is retimed before the reload.
         * EU `0E000586` lh $5,0xE($16) is SIGNED (USA's `0E000396` is lhu), goes
         * through sll/addu/addiu/div/mflo against `06000324` addiu $3,$0,0x6, and
         * only then reaches `000002A6` sh $2,0x0($16). USA stores the raw value. */
        *(s16 *)(s + 0x2) = 1;
        *(s16 *)(s + 0x0) = (s16)EU_PAL_RETIME(*(s16 *)(s + 0xE));
    }

    dir = *(s16 *)(s + 0x2);
    counter = *(s16 *)(s + 0x0);

    if (dir == 0) {
        /* Site 2: the forward duration from +0xC (`0C000386` lh $3,0xC($16), reached
         * when the likely branch `3A004054` bnel $2,$0 is NOT taken and nullifies its
         * `0E000386` lh $3,0xE delay slot) is retimed into $4 before `68110A0C`
         * jal func_002845A0. The retimed float lands in $f0 and serves as BOTH the
         * minuend and the divisor (`81000146` sub.s $f2,$f0,$f1 / `03150046`
         * div.s $f20,$f2,$f0), so both uses of the duration are retimed.
         * NOT retimed: `counter`, the live +0x0 value, whose `00000286` lh $2,0x0($16)
         * feeds `00088244` mtc1 $2,$f1 / cvt.s.w directly with no div on its path. */
        f32 dur = func_002845A0(EU_PAL_RETIME(*(s16 *)(s + 0xC)));
        frac = (dur - (f32)counter) / dur;
        r = s[0x7] ? func_002845B0((f32)s[0x4] + (f32)((s32)s[0x7] - (s32)s[0x4]) * frac) : s[0x4];
        g = s[0x8] ? func_002845B0((f32)s[0x5] + (f32)((s32)s[0x8] - (s32)s[0x5]) * frac) : s[0x5];
        b = s[0x9] ? func_002845B0((f32)s[0x6] + (f32)((s32)s[0x9] - (s32)s[0x6]) * frac) : s[0x6];
    } else {
        /* Site 3: the reverse duration from +0xE, loaded in the taken delay slot of
         * `3A004054` bnel $2,$0 as `0E000386` lh $3,0xE($16), is retimed by the same
         * chain at .L002AC53C (`06000224` addiu $2,$0,0x6 ... `12200000` mflo $4)
         * before `68110A0C` jal func_002845A0, and likewise serves as both minuend
         * and divisor via $f0. NOT retimed: `counter` (+0x0), same as site 2. */
        f32 dur = func_002845A0(EU_PAL_RETIME(*(s16 *)(s + 0xE)));
        frac = (dur - (f32)counter) / dur;
        r = s[0x7] ? func_002845B0((f32)s[0x7] + (f32)((s32)s[0x4] - (s32)s[0x7]) * frac) : s[0x4];
        g = s[0x8] ? func_002845B0((f32)s[0x8] + (f32)((s32)s[0x5] - (s32)s[0x8]) * frac) : s[0x5];
        b = s[0x9] ? func_002845B0((f32)s[0x9] + (f32)((s32)s[0x6] - (s32)s[0x9]) * frac) : s[0x6];
    }
    return func_002A0E48(target, r, g, b);
}

#undef EU_PAL_RETIME
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC670);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC6D0);

/* func_002AC6D8: true when m is a valid moby-table entry with class id in
 * [500, 540]. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AC9E0: g_mobyTableBase/g_mobyTableEnd are
 * region-agnostic named symbols (EU splat anchors them off
 * g_nBoltCounterDisplayed+0x214/+0x21C, same absolute addrs); moby->oClass at
 * +0xAA is the named EU Moby field. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC6D8);
#else
s32 func_002AC6D8(Moby *m) {
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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC718);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD288);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD550);

/* func_002AD558: clamp a vector's length - if vec's 3-component length exceeds
 * maxLen, rescale it in place down to maxLen (else leave it unchanged).
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AD860: Vec3LengthVu0 -> func_002836B0,
 * Vec3RescaleToLenVu0 -> func_002837E0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD558);
#else
void func_002AD558(Vec4 *vec, f32 maxLen) {
    if (maxLen < func_002836B0(vec)) {          /* Vec3LengthVu0 */
        func_002837E0(vec, maxLen, vec);        /* Vec3RescaleToLenVu0 */
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD5A8);

/* func_002AD630: remove a moby from an i16 count-prefixed list by swapping the
 * last live entry into its slot (list[0] = count). Matching arm stays INCLUDE_ASM;
 * #else is the structure model. EU-lockstep of USA func_002AD938: g_mobyTableBase
 * is a region-agnostic named symbol (EU splat anchors it off
 * g_nBoltCounterDisplayed+0x214, same absolute addr). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD630);
#else
void func_002AD630(Moby *moby, s16 *list) {
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

/* func_002AD6A8: jitter a Vec3 in place - add an independent uniform random offset
 * in [-amt, amt) to each of x/y/z. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. EU-lockstep of USA func_002AD9B0: GetRandomFloatRange
 * (func_002A86E0) -> func_002A8290. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD6A8);
#else
void func_002AD6A8(Vec4 *p, f32 amt) {
    p->x = p->x + func_002A8290(-amt, amt);   /* func_002A8290 == GetRandomFloatRange */
    p->y = p->y + func_002A8290(-amt, amt);
    p->z = p->z + func_002A8290(-amt, amt);
}
#endif

/* func_002AD728: test whether pos falls inside segment segIdx's unit-cube bounds.
 * Subtract the segment origin (segment-table base ptr held at an anchored global,
 * entries 0x80-stride, origin at +0x30), rotate the offset into the segment's local
 * frame by the segment's 3x3 rotation matrix (seg+0x40) via func_00283958
 * (out = m * local), and return 1 only if all of x/y/z land in [-1, 1]. segIdx == -1
 * returns 0. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002ADA30: Vec4SubVu0 -> func_002835B0, func_00283A48
 * (out = m * v, 3x3 rotate) -> func_00283958; USA *(g_deferredSegment2Tag+0xCC)
 * segment-table base is the EU-anchored pointer g_deferredSegmentTable2 (EU splat
 * anchors it off g_nBoltCounterDisplayed+0x304, same absolute addr). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD728);
#else
extern void func_00283958(Vec4 *out, Vec4 *v, Vec4 *m);   /* out = m * v (3x3 rotate) */
extern u8 *g_deferredSegmentTable2;   /* segment-table base ptr (EU g_nBoltCounterDisplayed+0x304) */

s32 func_002AD728(Vec4 *pos, s32 segIdx) {
    u8 *seg;
    Vec4 local;
    Vec4 out;

    if (segIdx == -1) {
        return 0;
    }
    seg = g_deferredSegmentTable2 + segIdx * 0x80;
    func_002835B0(&local, pos, (Vec4 *)(seg + 0x30));   /* Vec4SubVu0 */
    local.w = 0.0f;
    func_00283958(&out, &local, (Vec4 *)(seg + 0x40));
    if (-1.0f <= out.x && out.x <= 1.0f &&
        -1.0f <= out.y && out.y <= 1.0f &&
        -1.0f <= out.z && out.z <= 1.0f) {
        return 1;
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD800);

/* func_002AD890(subject): scan the point-light manager block back-to-front - for
 * each active light entry (0x10 stride, count at mgr+0xC) call the predicate
 * func_00284640(subject, &entry, &entry+0x20); on the first hit record the 1-based
 * index and stop. Returns 1 iff that hit index equals the manager's head field
 * (mgr+0x0). Matching arm stays INCLUDE_ASM; #else is the structure model.
 * TWIN NOTE: the mnemonic-hash twin-map assigned func_002ADB10, but EU func_002AD890's
 * .s structure is USA func_002ADBA0 (point-light scan), NOT func_002ADB10 (segment
 * distance test) - modeled against the ground-truth EU .s.
 * EU-lockstep of USA func_002ADBA0: predicate func_00284730 -> func_00284640;
 * g_pointLights+0x2400/+0x2420 -> the EU-anchored point-light block (EU splat anchors
 * mgr off g_nVendorBuyQuantity+0x12CF8, entries +0x12D18, same absolute addrs). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD890);
#else
extern u8 g_pointLightMgr[];   /* point-light manager block (EU g_nVendorBuyQuantity+0x12CF8) */
extern s32 func_00284640(void *subject, void *entry, void *entryHi);

s32 func_002AD890(void *subject) {
    u8 *mgr = g_pointLightMgr;
    u8 *entries = g_pointLightMgr + 0x20;   /* +0x12D18 - +0x12CF8 == 0x20 */
    s32 found = 0;
    s32 i;

    for (i = *(s32 *)(mgr + 0xC) - 1; i >= 0; i--) {
        u8 *entry = entries + i * 0x10;
        if (func_00284640(subject, entry, entry + 0x20) != 0) {
            found = i + 1;
            break;
        }
    }
    return found == *(s32 *)mgr;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD928);

/* func_002AD948: rotate vector v by quaternion q, writing the result to out:
 * out = q * (v as a pure quaternion, w=0) * conjugate(q). The conjugate negates
 * the xyz of q and keeps its w. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. EU-lockstep of USA func_002ADC50: Vec4ScaleVu0 -> func_002835F0,
 * func_00284180 (quaternion multiply) -> func_00284090. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD948);
#else
void func_002AD948(Vec4 *out, Vec4 *v, Vec4 *q) {
    Vec4 conj, vpure, rotated;

    func_002835F0(&conj, -1.0f, q);   /* Vec4ScaleVu0: conj = -q */
    conj.w = q->w;
    vpure = *v;
    vpure.w = 0.0f;
    func_00284090(&rotated, q, &vpure);   /* func_00284180: rotated = q * vpure */
    func_00284090(out, &rotated, &conj);  /* out = rotated * conj */
}
#endif

/* func_002AD9D8: rescale `src`'s vec3 to length `t` into `dst`, then set dst->w
 * to sqrt(1 - t*t) (the w that keeps a unit quaternion / normal). Matching arm
 * stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002ADCE0: Vec3RescaleToLenVu0 -> func_002837E0,
 * sqrtf (USA func_002835C0) -> func_002834D0. dst->w verified at swc1 +0xC. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD9D8);
#else
extern void func_002837E0(Vec4 *dst, f32 len, Vec4 *src);   /* Vec3RescaleToLenVu0 */

void func_002AD9D8(Vec4 *dst, f32 t, Vec4 *src) {
    func_002837E0(dst, t, src);              /* Vec3RescaleToLenVu0(dst, t, src) */
    dst->w = func_002834D0(1.0f - t * t);    /* sqrtf */
}
#endif

/* func_002ADA20: rotate `src` about `axis` by `angle` into `dst`. For a
 * negligible angle (|angle| < 1e-5) it copies src to dst; otherwise it normalises
 * the axis, builds the axis-angle quaternion (func_002AC0F8), and rotates src by it
 * (func_002AD948). Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002ADD28: GetFloatAbs kept, Vec3RescaleToLenVu0 ->
 * func_002837E0, func_002AC4D0 (axis-angle quat) -> func_002AC0F8, func_002ADC50
 * (rotate) -> func_002AD948. 1e-5 = 0x3727C5AC. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ADA20);
#else
extern void func_002837E0(Vec4 *dst, f32 len, Vec4 *src);   /* Vec3RescaleToLenVu0 */

void func_002ADA20(Vec4 *dst, Vec4 *src, Vec4 *axis, f32 angle) {
    if (GetFloatAbs(angle) < 1e-5f) {
        *dst = *src;
    } else {
        Vec4 quat;
        func_002837E0(&quat, 1.0f, axis);       /* Vec3RescaleToLenVu0 */
        func_002AC0F8(&quat, &quat, angle);     /* axis-angle -> quat */
        func_002AD948(dst, src, &quat);         /* rotate src by quat */
    }
}
#endif

/* func_002ADAC8: rotate vector `a` a fraction `t` of the way toward `b`, into
 * `out` (slerp-like). Axis = normalize(cross(b, a)); cosA = dot(a, b) - divided by
 * |a|*|b| unless `useRaw` != 0 (and if that product is 0, out = a and returns).
 * Builds a rotation quaternion by ((pi/2 - acos(cosA)) * t * 0.5) about the axis
 * and rotates a by it (func_002AD948). Matching arm stays INCLUDE_ASM; #else is the
 * structure model. EU-lockstep of USA func_002ADDD0: Vec3CrossVu0 -> func_00283698,
 * Vec3RescaleToLenVu0 -> func_002837E0, Vec3DotVu0 -> func_00283670, Vec3LengthVu0
 * -> func_002836B0, arccos (USA func_00283B60) -> func_00283A70, sin -> func_00283A58,
 * cos -> func_00283A40, Vec4ScaleVu0 -> func_002835F0, func_002ADC50 -> func_002AD948.
 * pi/2 = 0x3FC90FDB, 0.5 = 0x3F000000. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ADAC8);
#else
extern void func_00283698(Vec4 *dst, Vec4 *a, Vec4 *b);     /* Vec3CrossVu0 */
extern void func_002837E0(Vec4 *dst, f32 len, Vec4 *src);   /* Vec3RescaleToLenVu0 */
extern f32  func_00283670(Vec4 *a, Vec4 *b);                /* Vec3DotVu0 */
extern f32  func_002836B0(Vec4 *v);                         /* Vec3LengthVu0 */
extern f32  func_00283A70(f32 x);                           /* arccos */
extern void func_002835F0(Vec4 *dst, f32 s, const Vec4 *src); /* Vec4ScaleVu0 */

void func_002ADAC8(Vec4 *out, Vec4 *a, Vec4 *b, s32 useRaw, f32 t) {
    Vec4 quat;
    f32 cosA;
    f32 angle;
    f32 rotAngle;

    func_00283698(&quat, b, a);                 /* Vec3CrossVu0(quat, b, a) */
    func_002837E0(&quat, 1.0f, &quat);          /* Vec3RescaleToLenVu0 */
    cosA = func_00283670(a, b);                 /* Vec3DotVu0(a, b) */

    if (useRaw == 0) {
        f32 lenAB = func_002836B0(a) * func_002836B0(b);
        if (lenAB == 0.0f) {
            *out = *a;
            return;
        }
        cosA = cosA / lenAB;
    }

    angle = func_00283A70(cosA);                /* arccos */
    rotAngle = (1.5707964f - angle) * (t * 0.5f);
    func_002835F0(&quat, func_00283A58(rotAngle), &quat);  /* Vec4ScaleVu0(sin) */
    quat.w = func_00283A40(rotAngle);           /* cos */
    func_002AD948(out, a, &quat);               /* rotate a by quat */
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ADC08);

/**
 * Read word 2 of a moby's extra/pvar block. (EU twin of USA func_002ADF18.)
 */
s32 func_002ADC10(Moby *moby) {
    if (moby == 0) {
        return 0;
    }
    if ((moby->modeBits & 0x20) != 0) {
        return moby->pExtra[2];
    }
    return 0;
}

/* func_002ADC40: AE-family combined - map a point AND compose an orientation
 * through obj's sub-source. No sub-source (func_002ADC10==0) -> arg5=arg3,
 * arg6=arg4, return 0. Otherwise: bring (arg3 + src pos - obj pos) into local
 * space and rotate into arg5 (src+0x20 folded with obj matrix +0xC0 when src flag
 * +0x3C bit 0x2, else base matrix), re-add obj pos; and compose arg4's rotation
 * with the base matrix -> euler into arg6. Returns 1. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. EU-lockstep of USA func_002ADF48: func_002ADF18 ->
 * func_002ADC10, func_00283DC0 -> func_00283CD0, Vec4AddVu0 -> func_00283580,
 * Vec4SubVu0 -> func_002835B0, func_00283A48 (out=m*v) -> func_00283958,
 * func_00284048 (quat->rot mtx) -> func_00283F58, MatrixMultiplyVu0 -> func_00284048,
 * MatrixToEulerAngles -> func_002AC168. src flag lw +0x3C, src pos +0x10, obj pos
 * +0x10, obj matrix +0xC0, src rot vec +0x20 (verified). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ADC40);
#else
extern void func_00283CD0(Mat4x4 *dst, Vec4 *in);              /* build rot matrix from vec */
extern void func_00283580(Vec4 *dst, Vec4 *a, Vec4 *b);       /* Vec4AddVu0 */
extern void func_002835B0(Vec4 *dst, Vec4 *a, Vec4 *b);       /* Vec4SubVu0 */
extern void func_00283958(Vec4 *out, Vec4 *v, Vec4 *m);       /* out = m * v (3x3 rotate) */
extern void func_00283F58(Mat4x4 *dst, const Vec4 *quat);     /* quat -> rot matrix */
extern void func_00284048(Mat4x4 *dst, Mat4x4 *a, Mat4x4 *b); /* MatrixMultiplyVu0 */

s32 func_002ADC40(void *self, Moby *obj, Vec4 *arg3, Vec4 *arg4, Vec4 *arg5, void *arg6) {
    s32 src = func_002ADC10(obj);
    Mat4x4 base;
    Mat4x4 composed;
    (void)self;

    if (src == 0) {
        *arg5 = *arg3;
        *(Vec4 *)arg6 = *arg4;
        return 0;
    }
    func_00283CD0(&base, (Vec4 *)src);
    func_00283580(arg5, arg3, (Vec4 *)(src + 0x10));            /* arg5 = arg3 + src pos */
    func_002835B0(arg5, arg5, (Vec4 *)((u8 *)obj + 0x10));      /* arg5 -= obj pos */
    if (*(s32 *)(src + 0x3C) & 0x2) {
        Mat4x4 rot;
        func_00283CD0(&rot, (Vec4 *)(src + 0x20));
        func_00283F58(&composed, (const Vec4 *)&rot);
        func_00283958(arg5, arg5, (Vec4 *)&composed);
        func_00283958(arg5, arg5, (Vec4 *)((u8 *)obj + 0xC0));
    } else {
        func_00283958(arg5, arg5, (Vec4 *)&base);
    }
    func_00283580(arg5, arg5, (Vec4 *)((u8 *)obj + 0x10));      /* arg5 += obj pos */
    func_00283CD0(&composed, arg4);
    func_00284048(&composed, &base, &composed);
    func_002AC168(&composed, arg6);
    return 1;
}
#endif

/* func_002ADDB0: transform `in` into `out` by the orientation of obj's resolved
 * sub-source (func_002ADC10 = pvar word 2). No sub-source -> out = in (copy) and
 * returns 0. Otherwise builds its rotation matrix; when the source's flag (+0x3C)
 * has bit 0x2, it composes an extra rotation from the source's +0x20 vec and
 * re-applies obj's own matrix at +0xC0. Returns 1 when transformed. Matching arm
 * stays INCLUDE_ASM; #else is the structure model. EU-lockstep of USA func_002AE0B8:
 * func_002ADF18 -> func_002ADC10, func_00283DC0 -> func_00283CD0, func_00283A48
 * (out=m*v) -> func_00283958, func_00284048 (quat->rot mtx) -> func_00283F58. src
 * flag lw +0x3C; obj matrix +0xC0, src rot vec +0x20 (verified). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ADDB0);
#else
extern void func_00283CD0(Mat4x4 *dst, Vec4 *in);              /* build rot matrix from vec */
extern void func_00283958(Vec4 *out, Vec4 *v, Vec4 *m);       /* out = m * v (3x3 rotate) */
extern void func_00283F58(Mat4x4 *dst, const Vec4 *quat);     /* quat -> rot matrix */

s32 func_002ADDB0(void *self, void *obj, Vec4 *in, Vec4 *out) {
    s32 src = func_002ADC10((Moby *)obj);
    Mat4x4 rot;

    (void)self;
    if (src == 0) {
        *out = *in;
        return 0;
    }
    func_00283CD0(&rot, (Vec4 *)src);
    if (*(s32 *)(src + 0x3C) & 0x2) {
        Mat4x4 rot2;
        Mat4x4 mtx;
        func_00283CD0(&rot2, (Vec4 *)(src + 0x20));
        func_00283F58(&mtx, (const Vec4 *)&rot2);
        func_00283958(out, in, (Vec4 *)&mtx);
        func_00283958(out, out, (Vec4 *)((u8 *)obj + 0xC0));
    } else {
        func_00283958(out, in, (Vec4 *)&rot);
    }
    return 1;
}
#endif

/* func_002ADE90: map a point `arg3` (given in the sub-source's space) back through
 * obj into `arg4`, relative to arg3. No sub-source (func_002ADC10==0) -> arg4 = 0,
 * return 0. Otherwise: bring (arg3 + src pos - obj pos) into local space, rotate it
 * by the source orientation (from src+0x20 folded with obj's matrix +0xC0 when src
 * flag +0x3C bit 0x2 is set, else the base matrix), add obj pos back, and write
 * (result - arg3) to arg4. Returns 1. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. EU-lockstep of USA func_002AE198: func_002ADF18 -> func_002ADC10,
 * func_00283DC0 -> func_00283CD0, Vec4AddVu0 -> func_00283580, Vec4SubVu0 ->
 * func_002835B0, func_00283A48 (out=m*v) -> func_00283958, func_00284048 (quat->rot
 * mtx) -> func_00283F58. src flag lw +0x3C; src pos +0x10, obj pos +0x10, obj matrix
 * +0xC0, src rot vec +0x20 (verified). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ADE90);
#else
extern void func_00283CD0(Mat4x4 *dst, Vec4 *in);              /* build rot matrix from vec */
extern void func_00283580(Vec4 *dst, Vec4 *a, Vec4 *b);       /* Vec4AddVu0 */
extern void func_002835B0(Vec4 *dst, Vec4 *a, Vec4 *b);       /* Vec4SubVu0 */
extern void func_00283958(Vec4 *out, Vec4 *v, Vec4 *m);       /* out = m * v (3x3 rotate) */
extern void func_00283F58(Mat4x4 *dst, const Vec4 *quat);     /* quat -> rot matrix */

s32 func_002ADE90(void *self, Moby *obj, Vec4 *arg3, Vec4 *arg4) {
    s32 src = func_002ADC10(obj);
    Mat4x4 base;
    Vec4 p;
    (void)self;

    if (src == 0) {
        Vec4 zero = {0.0f, 0.0f, 0.0f, 0.0f};
        *arg4 = zero;
        return 0;
    }
    func_00283CD0(&base, (Vec4 *)src);
    func_00283580(&p, arg3, (Vec4 *)(src + 0x10));             /* p = arg3 + src pos */
    func_002835B0(&p, &p, (Vec4 *)((u8 *)obj + 0x10));         /* p -= obj pos */
    if (*(s32 *)(src + 0x3C) & 0x2) {
        Mat4x4 rot;
        Mat4x4 mtx;
        func_00283CD0(&rot, (Vec4 *)(src + 0x20));
        func_00283F58(&mtx, (const Vec4 *)&rot);
        func_00283958(&p, &p, (Vec4 *)&mtx);
        func_00283958(&p, &p, (Vec4 *)((u8 *)obj + 0xC0));
    } else {
        func_00283958(&p, &p, (Vec4 *)&base);
    }
    func_00283580(&p, &p, (Vec4 *)((u8 *)obj + 0x10));         /* p += obj pos */
    func_002835B0(arg4, &p, arg3);                            /* arg4 = p - arg3 */
    return 1;
}
#endif

/* func_002ADFD0: AE-family placement with a hero/priority fallback. Resolves obj's
 * sub-source (func_002ADC10); no source -> returns 0. Builds the source's base
 * rotation matrix - from obj facing (+0xF0) via func_00283CD0 when the source flag
 * (+0x3C) bit 0x40 is set, else loaded from obj's matrix rows (+0xC0) via
 * func_00283F18 - transforms `point` into that frame (out `outPos`) and re-adds
 * obj's world position (+0x10); then composes `rotIn`'s rotation with the base
 * matrix and writes euler angles to `outAngles`. Then, on source flag bit 0x4,
 * re-adds the source's own offset (+0x10) into `outPos` and defers to func_002AE158.
 * Otherwise, unless `self` is the hero, and only when `self` sorts below `obj` by
 * classSlot (+0x22), adds the source offset clamped to unit length (func_002AD558).
 * Always returns 1 once a source exists. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. EU-lockstep of USA func_002AE2D8: func_002ADF18 -> func_002ADC10,
 * func_00283DC0 -> func_00283CD0, func_00284008 -> func_00283F18, func_00283A48
 * (out=m*v) -> func_00283958, Vec4AddVu0 -> func_00283580, MatrixMultiplyVu0 ->
 * func_00284048, MatrixToEulerAngles -> func_002AC168, func_002AE460 -> func_002AE158,
 * func_002AD860 (clamp len) -> func_002AD558. g_pHeroMoby kept (EU anchored). src flag
 * lw +0x3C (bit 0x40/0x4); classSlot lbu +0x22; src pos +0x10; obj facing +0xF0, obj
 * matrix +0xC0, obj pos +0x10 (verified). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ADFD0);
#else
extern void func_00283CD0(Mat4x4 *dst, Vec4 *in);              /* build rot matrix from vec */
extern void func_00283F18(void *dst, void *src);              /* build/convert matrix */
extern void func_00283958(Vec4 *out, Vec4 *v, Vec4 *m);       /* out = m * v (3x3 rotate) */
extern void func_00283580(Vec4 *dst, Vec4 *a, Vec4 *b);       /* Vec4AddVu0 */
extern void func_00284048(Mat4x4 *dst, Mat4x4 *a, Mat4x4 *b); /* MatrixMultiplyVu0 */
extern s32  func_002AE158(void *self, Moby *obj, Vec4 *arg3, Vec4 *arg4,
                          void *arg5, void *arg6);              /* USA func_002AE460 */

s32 func_002ADFD0(Moby *self, Moby *obj, Vec4 *point, Vec4 *rotIn,
                  Vec4 *outPos, void *outAngles) {
    s32 src = func_002ADC10(obj);
    Mat4x4 base;
    Mat4x4 composed;

    if (src == 0) {
        return 0;
    }
    if (*(s32 *)(src + 0x3C) & 0x40) {
        func_00283CD0(&base, (Vec4 *)((u8 *)obj + 0xF0));
    } else {
        func_00283F18(&base, (u8 *)obj + 0xC0);
    }
    func_00283958(outPos, point, (Vec4 *)&base);
    func_00283580(outPos, outPos, (Vec4 *)((u8 *)obj + 0x10));

    func_00283CD0(&composed, rotIn);
    func_00284048(&composed, &base, &composed);
    func_002AC168(&composed, outAngles);

    if (*(s32 *)(src + 0x3C) & 0x4) {
        func_00283580(outPos, outPos, (Vec4 *)(src + 0x10));
        func_002AE158(self, obj, outPos, outAngles, point, rotIn);
        return 1;
    }
    if (self == g_pHeroMoby[0]) {
        return 1;
    }
    if (self->classSlot < obj->classSlot) {
        Vec4 srcOffset = *(Vec4 *)(src + 0x10);

        func_002AD558(&srcOffset, 1.0f);
        func_00283580(outPos, outPos, &srcOffset);
    }
    return 1;
}
#endif

/* Compute a moby's local-frame offset + composed orientation from its resolved
 * sub-source. No source (func_002ADC10==0) -> returns 0. Otherwise builds the
 * source's base rotation matrix - from obj+0xF0 when the source flag +0x3C bit 0x40
 * is set, else obj+0xC0 - transforms (arg3 - obj pos+0x10) into that frame (out
 * arg5), then composes with arg4's rotation and writes euler angles to arg6.
 * Returns 1. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AE460: func_002ADF18 -> func_002ADC10, func_00283DC0 ->
 * func_00283CD0, func_00284048 (quat->rot) -> func_00283F58, Vec4SubVu0 ->
 * func_002835B0, func_00283A70 (dst=mat*vec) -> func_00283980, MatrixMultiplyVu0 ->
 * func_00284048, MatrixToEulerAngles -> func_002AC168. Offsets verified vs
 * func_002AE158.s: src+0x3C bit0x40, obj+0xF0/+0xC0/+0x10. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AE158);
#else
extern void func_00283980(Vec4 *dst, const Vec4 *vec, const Mat4x4 *mat); /* dst = mat * vec */

s32 func_002AE158(void *self, Moby *obj, Vec4 *arg3, Vec4 *arg4, void *arg5, void *arg6) {
    s32 src = func_002ADC10(obj);
    Mat4x4 rot;
    Vec4 delta;
    Mat4x4 composed;
    (void)self;

    if (src == 0) {
        return 0;
    }
    if (*(s32 *)(src + 0x3C) & 0x40) {
        func_00283CD0(&rot, (Vec4 *)((u8 *)obj + 0xF0));
        func_00283F58(&rot, (const Vec4 *)&rot);
    } else {
        func_00283F58(&rot, (const Vec4 *)((u8 *)obj + 0xC0));
    }
    func_002835B0(&delta, arg3, (Vec4 *)((u8 *)obj + 0x10));
    func_00283980((Vec4 *)arg5, &delta, &rot);
    func_00283CD0(&composed, arg4);
    func_00284048(&composed, &rot, &composed);
    func_002AC168(&composed, arg6);
    return 1;
}
#endif

/* Compose two orientation matrices (from m1 and m2), convert the product to euler
 * angles written into `out`, then stash the source vectors: out+0x20 gets m1 when
 * out's flag word (+0x3C) has bit 0x2 set, and out+0x10 always gets arg2.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AE558: func_00283DC0 -> func_00283CD0, func_00284048
 * (quat->mtx) -> func_00283F58, MatrixMultiplyVu0 -> func_00284048,
 * MatrixToEulerAngles -> func_002AC168. Offsets verified vs func_002AE250.s:
 * out+0x3C bit0x2, sq m1->out+0x20, sq arg2->out+0x10. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AE250);
#else
void func_002AE250(void *out, Vec4 *arg2, Vec4 *m1, Vec4 *m2) {
    Mat4x4 rot1;
    Mat4x4 mtx1;
    Mat4x4 rot2;
    Mat4x4 product;
    u8 *o = (u8 *)out;

    func_00283CD0(&rot1, m1);
    func_00283F58(&mtx1, (const Vec4 *)&rot1);
    func_00283CD0(&rot2, m2);
    func_00284048(&product, &mtx1, &rot2);
    func_002AC168(&product, out);
    if (*(s32 *)(o + 0x3C) & 0x2) {
        *(Vec4 *)(o + 0x20) = *m1;
    }
    *(Vec4 *)(o + 0x10) = *arg2;
}
#endif

/* MarkLevelAvailable: set a level's available flag and append it to the ordered
 * level list (regular levels < 0x15, plus level 0x18). NAMED fn (same name both
 * regions). Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA MarkLevelAvailable: g_abLevelAvailableFlags /
 * g_anAvailableLevelOrder are region-agnostic file-scope symbols (verified vs
 * MarkLevelAvailable.s: same 0x15/0x18 gate + 0x1C-iteration order scan). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", MarkLevelAvailable);
#else
void MarkLevelAvailable(s32 level) {
    s32 i;
    s32 idx;

    if (g_abLevelAvailableFlags[level] != 0) {
        return;   /* already available */
    }
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

/* Equip gadget `itemId`. Looks up its weapon-variant entry (g_weaponTable indexed
 * by g_itemEquippedSlot[itemId], 0xE0 stride) and dispatches on its mode (+0xC):
 * mode 0 needs a resource (func_00294F40 resident? / file-load busy? -> may abort;
 * else set active + kick load func_00294D30); modes 1/2/3 record itemId into the
 * matching active-gadget slot. Returns 1 when equipped/queued, 0 on abort.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AE6C8: func_00294EE0 -> func_00294F40, func_00294CD0 ->
 * func_00294D30; USA g_activeGadgetItem[0..3] -> EU (g_bPlayerMode + 0x14/0x18/0x1C/
 * 0x20), USA g_fileLoadState -> EU (g_saveImageArea + 0x1004). Verified vs
 * func_002AE3C0.s: entry +0xC mode, +0x14 resource field, lh g_saveImageArea+0x1004. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AE3C0);
#else
extern s32 func_00294F40(s32 fileId);   /* nonzero if the item's resource is resident */
extern s32 func_00294D30(s32 fileId);   /* kick the item's resource load */
extern u8  g_bPlayerMode[];             /* +0x14/+0x18/+0x1C/+0x20 = active-gadget slots */
extern u8  g_saveImageArea[];           /* +0x1004 = s16 file-load-in-progress state */

s32 func_002AE3C0(s32 itemId) {
    u8 *w = (u8 *)&g_weaponTable[g_itemEquippedSlot[itemId]];
    s32 mode = *(s32 *)(w + 0xC);

    if (mode == 0) {
        if (func_00294F40(*(s32 *)(w + 0x14)) == 0 &&
            *(s16 *)(g_saveImageArea + 0x1004) != 0) {
            return 0;
        }
        *(s32 *)(g_bPlayerMode + 0x14) = itemId;
        if (func_00294F40(*(s32 *)(w + 0x14)) != 0) {
            return 1;
        }
        func_00294D30(*(s32 *)(w + 0x14));
        return 1;
    }
    if (mode == 3) {
        *(s32 *)(g_bPlayerMode + 0x20) = itemId;
        return 1;
    }
    if (mode == 2) {
        *(s32 *)(g_bPlayerMode + 0x1C) = itemId;
        return 1;
    }
    if (mode == 1) {
        *(s32 *)(g_bPlayerMode + 0x18) = itemId;
        return 1;
    }
    return 0;
}
#endif

/* Map a moby's class id (+0xAA) to an announcer/category code. A direct class-id ->
 * code table for the known classes; any other class falls through to a lookup
 * against the equipped block (base +0x1220/+0x1248) and then a linear scan of the
 * equipped-slot list (g_itemEquippedSlot, 0x38 entries) matching the class id (or the
 * moby's linked +0xB8 moby's class) against g_weaponTable[slot].+0x14, returning the
 * slot index or 0xFF. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AE7E8: USA g_soundBankHandlesBlk -> EU
 * (g_sndChannelVolumes + 0x1778) [+0x1220/+0x1248]. Verified vs func_002AE4E0.s:
 * lh class +0xAA, linked moby +0xB8, 0x38-entry scan. PIN: s32(Moby*) matches the
 * batch-4 caller func_002AA108. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AE4E0);
#else
extern u8 g_sndChannelVolumes[];   /* +0x1778 = equipped-announcer block base */

s32 func_002AE4E0(Moby *cand) {
    s16 cls = *(s16 *)((u8 *)cand + 0xAA);
    u8 *blk;
    void *equipped;
    void *linked;
    s32 i;

    switch (cls) {
    case 0xB2E: return 0x1D;
    case 0x87A: return 0x16;
    case 0xAC:  return 0xE;
    case 0x79:  return 0xC;
    case 0x5DA: return 0x1B;
    case 0xA66:
    case 0xA82: return 0x29;
    case 0x9A9:
    case 0xA9B: return 0x1C;
    case 0xB58:
    case 0xE79: return 0x20;
    case 0xC07:
    case 0xCE4:
    case 0xCE5: return 0x2A;
    case 0xE64:
    case 0xECE: return 0x25;
    case 0xD01: return 0x18;
    default:    break;
    }

    blk = g_sndChannelVolumes + 0x1778;
    equipped = *(void **)(blk + 0x1220);
    if ((void *)cand == equipped) {
        return *(s32 *)(blk + 0x1248);
    }
    linked = *(void **)((u8 *)cand + 0xB8);
    if (linked != 0 && linked == equipped) {
        return *(s32 *)(blk + 0x1248);
    }

    for (i = 0; i < 0x38; i++) {
        s32 f14 = *(s32 *)((u8 *)&g_weaponTable[g_itemEquippedSlot[i]] + 0x14);
        if (cls == f14) {
            return i;
        }
        if (linked != 0 && *(s16 *)((u8 *)linked + 0xAA) == f14) {
            return i;
        }
    }
    return 0xFF;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AE6D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AF288);

/* Decode a packed RGBA colour pointed to by `colorPtr` into a signed direction/
 * offset vector scaled by its alpha: unpack to floats, recentre RGB around 127 (so
 * 0x80 -> 0), and scale the whole vector by alpha * 1e-4. Writes to `out`. Matching
 * arm stays INCLUDE_ASM; #else is the structure model. EU-lockstep of USA func_002AF6A0:
 * func_00283AA0 -> func_002839B0, Vec4SubVu0 -> func_002835B0, Vec4ScaleVu0 ->
 * func_002835F0 (no data refs). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AF398);
#else
extern void func_002839B0(Vec4 *dst, u32 packed);            /* unpack RGBA word -> Vec4 */

void func_002AF398(Vec4 *out, u32 *colorPtr) {
    Vec4 color;
    Vec4 offset;
    f32 alpha;

    offset.x = 127.0f;
    offset.y = 127.0f;
    offset.z = 127.0f;
    offset.w = 0.0f;
    func_002839B0(&color, *colorPtr);
    alpha = color.w * 0.0001f;
    func_002835B0(&color, &color, &offset);
    func_002835F0(out, alpha, &color);
}
#endif

/* Per-frame wander/approach AI step for a moby with a control block `ctrl`. When the
 * wander phase (ctrl+0x28) is idle, pick a fresh heading jitter (ctrl+0x24) and a
 * random dwell timer (ctrl+0x2A). Otherwise step the heading toward the target
 * (func_002AB268) and tick the dwell timer (func_00283238), clearing the phase when
 * it expires. Each frame: advance a probe along the yaw (moby+0xF8) by ctrl+0x14,
 * sweep it (func_002A88B8), snap ground height (func_002A93C0). If unobstructed OR out
 * of leash range (ctrl+0x1C), re-aim at target (ctrl+0x0/+0x4) and re-roll; else,
 * within hero range (ctrl+0x20), steer toward the hero. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. EU-lockstep of USA func_002AF728: GetRandomFloatSigned
 * -> func_002A82F0, RandRangeInclusive -> func_002A8238, func_002AB668 -> func_002AB268,
 * func_00283328 -> func_00283238, cos -> func_00283A40, sin -> func_00283A58,
 * func_002A8D08 -> func_002A88B8, ProbeGroundHeight -> func_002A93C0, DistXYVu0 ->
 * func_00283740, atan2 -> func_00283B08, func_002AAFB8 -> func_002AABB8. USA g_heroPos
 * -> EU anchored (objdiff masks the anchor). Offsets verified vs func_002AF420.s.
 * GENUINE PAL/NTSC TIMING DIFF: the dwell-timer re-roll is func_002A8238(0x19, 0x4B)
 * = 25..75 ticks in PAL; the USA twin uses (0x1E, 0x5A) = 30..90 -- a 60->50 Hz
 * frame-count retime (ratio 0.8333). Verified vs func_002AF420.s (addiu $4,$0,0x19 /
 * delay-slot addiu $5,$0,0x4B). Take these immediates from the EU .s, never from the
 * USA body: an earlier symbol-swap port inherited the NTSC pair here. The other
 * func_002A8238 call in this body is the register form (ctrl+0x2C / +0x2E) and is
 * region-neutral. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AF420);
#else
extern s32  func_002A88B8(void *moby, Vec4 *ref, Vec4 *pos, s32 flags,
                          f32 stepZ, f32 radius, f32 snapEps, f32 hitEps); /* vertical sweep (deferred) */
extern Vec4 g_heroPos;                         /* hero world position (EU anchored) */

void func_002AF420(Moby *moby, void *ctrlPtr, f32 stepZ, f32 snapEps) {
    u8   *m = (u8 *)moby;
    u8   *c = (u8 *)ctrlPtr;
    Vec4 *mpos = (Vec4 *)(m + 0x10);
    Vec4  probe;
    s32   blocked;
    f32   dist;

    if (*(s16 *)(c + 0x28) == 0) {
        /* idle: choose a new heading jitter + dwell timer */
        *(f32 *)(c + 0x24) += func_002A82F0(0.7853982f, 2.6179941f);
        *(s16 *)(c + 0x2A) = (s16)func_002A8238(*(s16 *)(c + 0x2C), *(s16 *)(c + 0x2E));
        *(s16 *)(c + 0x28) = 1;
    } else {
        /* active: converge the heading + tick the dwell timer */
        func_002AB268(*(f32 *)(c + 0x24), *(f32 *)(c + 0x18), (f32 *)(m + 0xF8), 0);
        if (func_00283238(c + 0x2A) != 0) {
            *(s16 *)(c + 0x28) = 0;
        }
    }

    /* advance the probe point along the current yaw and resolve it */
    probe = *mpos;
    probe.x += func_00283A40(*(f32 *)(m + 0xF8)) * *(f32 *)(c + 0x14);
    probe.y += func_00283A58(*(f32 *)(m + 0xF8)) * *(f32 *)(c + 0x14);
    blocked = func_002A88B8(moby, mpos, &probe, 0, stepZ, *(f32 *)(c + 0x10), snapEps, 0.52359885f);
    *(f32 *)(m + 0x18) = func_002A93C0(mpos, 0.5f, 0);
    dist = func_00283740(mpos, (Vec4 *)c);

    if (blocked == 0 || *(f32 *)(c + 0x1C) < dist) {
        /* clear path or beyond leash: re-aim at the target and re-roll the timer */
        *(f32 *)(c + 0x24) = func_00283B08(*(f32 *)(c + 0x0) - *(f32 *)(m + 0x10),
                                           *(f32 *)(c + 0x4) - *(f32 *)(m + 0x14));
        *(s16 *)(c + 0x2A) = (s16)func_002A8238(0x19, 0x4B);
        *(s16 *)(c + 0x28) = 1;
    } else {
        /* blocked and within leash: steer toward the hero when close enough */
        f32 heroDist = func_00283740(mpos, &g_heroPos);
        if (heroDist < *(f32 *)(c + 0x20)) {
            f32 bearing = func_00283B08(*(f32 *)(m + 0x10) - g_heroPos.x,
                                        *(f32 *)(m + 0x14) - g_heroPos.y);
            *(f32 *)(c + 0x24) = func_002AABB8(*(f32 *)(c + 0x24), bearing,
                                              heroDist / *(f32 *)(c + 0x20));
        }
    }
}
#endif

/* func_002AF640: round a float to `digits` decimal places. Builds the scale
 * 10^digits (digits<=0 -> 1), adds the half-ulp rounding bias 1/(2*scale),
 * truncates (func_002845B0/FloatToInt) the scaled value, and divides back.
 * Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AF948: FloatToInt -> func_002845B0. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AF640);
#else
f32 func_002AF640(s32 digits, f32 x) {
    s32 pow10 = 1;
    f32 scale;

    while (digits > 0) {
        pow10 = pow10 * 10;
        digits = digits - 1;
    }
    scale = (f32)pow10;
    return (f32)func_002845B0((x + 1.0f / (scale + scale)) * scale) / scale;
}
#endif

/* func_002AF6C0: test whether `x` sits just below the object's reference value:
 * returns 1 when x <= round(refValue, 4) AND the gap (rounded - x) is smaller than
 * round(obj->field48 * 0.6, 4); otherwise 0. Both quantities rounded to 4 decimals
 * via func_002AF640. refValue = func_0029FEF0(obj). Matching arm stays INCLUDE_ASM;
 * #else is the structure model. EU-lockstep of USA func_002AF9C8: func_002A0368 ->
 * func_0029FEF0, func_002AF948 -> func_002AF640.
 * DATA-LANE DELTA: the *0.6f (0x3F19999A) here is *0.5f (0x3F000000) in USA -
 * genuine PAL/NTSC constant difference, verified both .s. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AF6C0);
#else
extern f32 func_0029FEF0(void *obj);   /* read reference value from obj (EU twin of func_002A0368) */

s32 func_002AF6C0(void *obj, f32 x) {
    f32 rounded = func_002AF640(4, func_0029FEF0(obj));
    f32 delta = rounded - x;
    f32 half = func_002AF640(4, *(f32 *)((u8 *)obj + 0x48) * 0.6f);
    return (x <= rounded) && (delta < half);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AF758);

/**
 * Unpack a 0xBBGGRR colour word and forward to the colour setter. (EU twin of
 * USA func_002AFA80.)
 */
s32 func_002AF780(void *p, s32 rgb) {
    s32 b = rgb >> 16;
    s32 g = rgb >> 8;

    __asm__ __volatile__("");
    return func_002A12C0(p, rgb & 0xFF, g & 0xFF, b & 0xFF);
}

/* func_002AF7B0: advance an object's eased-rotation driver one integration step and
 * (re)bind it to its owning controller. `state` (+0x01 registered flag, +0x10
 * orientation quat, +0x20 blend-weight fan, +0x40 eased eulers, +0x50 angular
 * velocity, +0x60 target eulers, +0x70 blend weight, +0x78 owner) is re-keyed to
 * `owner`, released early if already at the identity target, else each euler axis is
 * stepped by func_002AB300, registered, the quat rebuilt, the weight fanned, the
 * target quad zeroed and the weight latched to 1.0. Matching arm stays INCLUDE_ASM;
 * #else is the structure model. EU-lockstep of USA func_002AFAB0: func_002A0828 ->
 * func_002A03B0, GetFloatAbs -> func_00283508, func_002AB700 -> func_002AB300,
 * func_002A07B0 -> func_002A0338, func_002AA058 -> func_002A9C08, func_00283638 ->
 * func_00283548. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AF7B0);
#else
extern void func_002A03B0(void *owner, void *state);            /* detach state from owner */
extern void func_002A0338(void *owner, void *arg3, void *state); /* attach state */
extern f32  func_00283508(f32 x);                              /* GetFloatAbs (EU twin) */

void func_002AF7B0(void *owner, u8 *state, void *arg3, f32 rate, f32 cap) {
    void *curOwner = *(void **)(state + 0x78);
    f32 weight;

    if (curOwner != owner) {
        if (curOwner != 0) {
            if (state[1] != 0 && *(u8 *)((u8 *)curOwner + 0x20) < 0x7F) {
                func_002A03B0(curOwner, state);
            }
            state[1] = 0;
        }
        *(void **)(state + 0x78) = owner;
    }

    if (*(f32 *)(state + 0x60) == 0.0f &&
        *(f32 *)(state + 0x64) == 0.0f &&
        *(f32 *)(state + 0x68) == 0.0f &&
        *(f32 *)(state + 0x70) == 1.0f &&
        func_00283508(*(f32 *)(state + 0x40)) < 0.005f &&
        func_00283508(*(f32 *)(state + 0x44)) < 0.005f &&
        func_00283508(*(f32 *)(state + 0x48)) < 0.005f) {
        if (state[1] != 0) {
            func_002A03B0(owner, state);
        }
        return;
    }

    func_002AB300((f32 *)(state + 0x40), (f32 *)(state + 0x50), 0, *(f32 *)(state + 0x60), rate, cap, 0.0f);
    func_002AB300((f32 *)(state + 0x44), (f32 *)(state + 0x54), 0, *(f32 *)(state + 0x64), rate, cap, 0.0f);
    func_002AB300((f32 *)(state + 0x48), (f32 *)(state + 0x58), 0, *(f32 *)(state + 0x68), rate, cap, 0.0f);

    if (state[1] == 0) {
        func_002A0338(owner, arg3, state);
    }

    func_002A9C08((Vec4 *)(state + 0x10), (Vec4 *)(state + 0x40));

    weight = *(f32 *)(state + 0x70);
    *(f32 *)(state + 0x20) = weight;
    *(f32 *)(state + 0x24) = weight;
    *(f32 *)(state + 0x28) = weight;

    func_00283548((Moby *)(state + 0x60));   /* zero the 16-byte target quad */
    *(f32 *)(state + 0x70) = 1.0f;
}
#endif

/* func_002AF9D8: advance one oscillating angle channel and project it to a scalar
 * offset at out+0x18. The phase *p1 is stepped by `b` and wrapped (func_00284458);
 * when amplitude `c` is positive out->0x18 = sin(phase)*a + c, otherwise the channel
 * runs incrementally (subtract the prior *p2, store the new sin(phase)*a into *p2,
 * add it back). func_00283A58 is sine. Matching arm stays INCLUDE_ASM; #else is the
 * structure model. EU-lockstep of USA func_002AFCD8: WrapAnglePiSum/func_00284548 ->
 * func_00284458, func_00283B48 -> func_00283A58. func_00284458 is file-scope s32
 * (WrapAnglePiSum $f0 passthrough) -> assigned to *p1 as f32. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AF9D8);
#else
void func_002AF9D8(void *out, f32 *p1, f32 *p2, f32 a, f32 b, f32 c) {
    f32 *offset = (f32 *)((u8 *)out + 0x18);

    if (c > 0.0f) {
        *p1 = func_00284458(*p1, b);          /* WrapAnglePiSum */
        *offset = func_00283A58(*p1) * a + c;
    } else {
        *p1 = func_00284458(*p1, b);
        *offset = *offset - *p2;
        *p2 = func_00283A58(*p1) * a;
        *offset = *offset + *p2;
    }
}
#endif

/* func_002AFA90: build the XY components of a spherical-swing direction into out+0xF0
 * / out+0xF4 from two phase angles, then advance both phases.
 *   out->0xF0 = scale * sin(p1) * sin(p2);
 *   out->0xF4 = scale * sin(p1) * cos(p2);
 *   *p1 = WrapAnglePiSum(*p1, b);   *p2 = WrapAnglePiSum(*p2, c);
 * The sin/cos sample the phases BEFORE they are advanced. func_00283A58 is sine,
 * func_00283A40 cosine. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002AFD90: func_00283B48 -> func_00283A58, func_00283B30 ->
 * func_00283A40, WrapAnglePiSum -> func_00284458. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AFA90);
#else
void func_002AFA90(void *out, f32 *p1, f32 *p2, f32 scale, f32 b, f32 c) {
    f32 sinP1 = func_00283A58(*p1);

    *(f32 *)((u8 *)out + 0xF0) = scale * sinP1 * func_00283A58(*p2);
    *(f32 *)((u8 *)out + 0xF4) = scale * func_00283A58(*p1) * func_00283A40(*p2);
    *p1 = func_00284458(*p1, b);              /* WrapAnglePiSum */
    *p2 = func_00284458(*p2, c);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AFB58);

/* func_002AFB68: convert spherical coordinates (value, angle1, angle2) to a cartesian
 * vector in `handle`: x = value*cos(angle1)*cos(angle2), y = value*sin(angle1)*
 * cos(angle2), z = value*sin(angle2). func_00283A40 is cosine, func_00283A58 sine.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. EU-lockstep of USA
 * func_002AFE68: func_00283B30 -> func_00283A40, func_00283B48 -> func_00283A58.
 * PIN sig: void(void*,f32,f32,f32) - matches landed func_002A83A0/func_002ABCB8 callers. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AFB68);
#else
void func_002AFB68(void *handle, f32 value, f32 angle1, f32 angle2) {
    Vec4 *dst = (Vec4 *)handle;
    dst->x = value * func_00283A40(angle1) * func_00283A40(angle2);
    dst->y = value * func_00283A58(angle1) * func_00283A40(angle2);
    dst->z = value * func_00283A58(angle2);
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AFC08);

/* func_002AFD38: rebuild a child moby's transform from its parent + a source
 * transform, then re-run its per-frame updates. Composes parent/srcTransform into a
 * scratch matrix (func_002A05E0), copies the translation row (m+0x30) to the child
 * pos (+0x10), optionally mirrors basis rows 0/1/2 per flags bits 1/2/4
 * (func_002835F0, scale -1.0), ticks the animation (func_002A0F68), refreshes the
 * bounding sphere/grid (func_002A1928) unless mode bit 4, installs the matrix at
 * child+0xC0 (func_00283F38/func_002ABA90) and runs func_002A1AC8. Finally mirrors
 * parent's mode bit 0 into the child's flags and forces bits 0x6. Matching arm stays
 * INCLUDE_ASM; #else is the structure model. EU-lockstep of USA func_002B0038:
 * func_002A0A58 -> func_002A05E0, Vec4ScaleVu0 -> func_002835F0, UpdateMobyAnimation
 * -> func_002A0F68, UpdateMobyBSphereAndGrid -> func_002A1928, func_00284028 ->
 * func_00283F38, func_002ABE90 -> func_002ABA90, func_002A1F20 -> func_002A1AC8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AFD38);
#else
extern void func_002A05E0(Moby *parent, void *srcTransform, Mat4x4 *out);
extern void func_002A0F68(Moby *moby);   /* UpdateMobyAnimation (EU twin) */
extern void func_002A1928(Moby *moby);   /* UpdateMobyBSphereAndGrid (EU twin) */
extern void func_002A1AC8(Moby *moby);

void func_002AFD38(Moby *parent, Moby *child, void *srcTransform, s32 flags) {
    u8 *c = (u8 *)child;
    Mat4x4 m;

    func_002A05E0(parent, srcTransform, &m);
    *(Vec4 *)(c + 0x10) = *(Vec4 *)((u8 *)&m + 0x30);
    if (flags & 1) {
        func_002835F0((Vec4 *)&m, -1.0f, (Vec4 *)&m);
    }
    if (flags & 2) {
        func_002835F0((Vec4 *)((u8 *)&m + 0x10), -1.0f, (Vec4 *)((u8 *)&m + 0x10));
    }
    if (flags & 4) {
        func_002835F0((Vec4 *)((u8 *)&m + 0x20), -1.0f, (Vec4 *)((u8 *)&m + 0x20));
    }
    func_002A0F68(child);
    if ((*(u16 *)(c + 0x34) & 4) == 0) {
        func_002A1928(child);
    }
    func_00283F38(c + 0xC0, &m);
    func_002ABA90((f32 *)(c + 0xC0));
    func_002A1AC8(child);
    if (*(u16 *)((u8 *)parent + 0x34) & 1) {
        *(u16 *)(c + 0x34) = (u16)(*(u16 *)(c + 0x34) | 0x41);
    } else {
        *(u16 *)(c + 0x34) = (u16)(*(u16 *)(c + 0x34) & 0xFFBE);
    }
    *(u16 *)(c + 0x34) = (u16)(*(u16 *)(c + 0x34) | 0x6);
}
#endif

/* func_002AFE50: score a candidate `moby` (position at +0x10) against a query point.
 * Samples func_00283708(query, mobyPos); flags the candidate (*outFlag=1) when b
 * exceeds that sample. Then builds two drive-heading angles (func_00284540 over the
 * planar bearing func_00283B08(dx,dy) and over the XY-distance-vs-dz bearing) and
 * flags again when 0<c<heading1 or 0<d<heading2. Returns sample*(1+heading1), plus
 * 8.0 when the moby is a valid class-filtered entry (func_002AC6D8). Matching arm
 * stays INCLUDE_ASM; #else is the structure model. EU-lockstep of USA func_002B0150:
 * func_002837F8 -> func_00283708, atan2 func_00283BF8 -> func_00283B08, drive-heading
 * func_00284630 -> func_00284540, DistXYVu0 -> func_00283740, func_002AC9E0 -> func_002AC6D8. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AFE50);
#else
extern f32 func_00284540(f32 a, f32 b);   /* EU drive-heading angle helper (USA func_00284630) */

f32 func_002AFE50(Vec4 *query, Moby *moby, s32 *outFlag, f32 a, f32 b, f32 c, f32 d) {
    f32 *mpos = (f32 *)((u8 *)moby + 0x10);   /* moby position Vec4 */
    f32 sample = func_00283708(query, mpos);
    f32 heading1, heading2, base;

    *outFlag = 0;
    if (b < sample) {
        *outFlag = 1;
    }

    heading1 = func_00284540(func_00283B08(mpos[0] - query->x, mpos[1] - query->y), a);
    heading2 = func_00284540(func_00283B08(func_00283740(query, (Vec4 *)mpos),
                                           mpos[2] - query->z), 0.0f);

    if (0.0f < c && c < heading1) {
        *outFlag = 1;
    }
    if (0.0f < d && d < heading2) {
        *outFlag = 1;
    }

    base = sample + heading1 * sample;
    if (func_002AC6D8(moby) != 0) {
        base += 8.0f;
    }
    return base;
}
#endif

/* func_002AFFC8: pick the nearest valid moby to `queryVec` from the flagged-moby
 * list. For each entry: skip if it has no pvar block (func_002ABC58 == 0) or its
 * word0 float is 0, then score it with func_002AFE50 (skip on its reject flag).
 * Track the moby with the smallest score. The list cursor only advances on a skip -
 * a scored entry re-reads the same slot (func_002AFE50 consumes/compacts it),
 * mirroring the original's loop exactly. Returns the best moby, or NULL if none.
 * Matching arm stays INCLUDE_ASM; #else is the structure model. EU-lockstep of USA
 * func_002B02C8: g_mobyFlagged1000List anchor D_001E0019+0xE947, func_002AC058 ->
 * func_002ABC58, func_002B0150 -> func_002AFE50. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AFFC8);
#else
extern Moby *g_mobyFlagged1000List[];   /* null-terminated flagged-moby list (EU anchor D_001E0019+0xE947) */

Moby *func_002AFFC8(Vec4 *queryVec, f32 a, f32 b, f32 c, f32 d) {
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
        void *pvar = func_002ABC58(moby);
        if (pvar == 0) {
            cursor++;
        } else if (*(f32 *)pvar == 0.0f) {
            cursor++;
        } else {
            s32 reject;
            f32 score = func_002AFE50(&query, moby, &reject, a, b, c, d);
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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B00E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B08D8);

/* func_002B0940: transform vector `a` into `out` by a rotation. When a matrix `b`
 * is supplied (b != 0) use it directly; otherwise build one from the quaternion at
 * ctx+0xC0 into a scratch matrix and transform through that. Returns 0 (callers
 * discard). Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002B0C40: func_00284048 (quat->rot mtx) -> func_00283F58,
 * func_00283A48 (out=m*v) -> func_00283958. PIN: s32(s32,void*,void*,void*). */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B0940);
#else
s32 func_002B0940(s32 ctx, void *out, void *a, void *b) {
    if (b != 0) {
        func_00283958((Vec4 *)out, (Vec4 *)a, (Vec4 *)b);
    } else {
        Mat4x4 mat;
        func_00283F58(&mat, (const Vec4 *)(ctx + 0xC0));
        func_00283958((Vec4 *)out, (Vec4 *)a, (Vec4 *)&mat);
    }
    return 0;
}
#endif

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B09A8);

/* func_002B09C0: resolve `a` into `out` via func_002B0940, rescale out's XY to
 * horizontal length `len`, then transform out in place by the object's matrix at
 * ctx+0xC0. Matching arm stays INCLUDE_ASM; #else is the structure model.
 * EU-lockstep of USA func_002B0CC0: func_002B0C40 -> func_002B0940, func_00283920
 * (rescale XY to len) -> func_00283830, func_00283A48 (out=m*v) -> func_00283958. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B09C0);
#else
extern void func_00283830(Vec4 *dst, Vec4 *src, f32 len);   /* rescale XY to horizontal len (USA func_00283920) */

void func_002B09C0(s32 ctx, Vec4 *out, void *a, void *b, f32 len) {
    func_002B0940(ctx, out, a, b);
    func_00283830(out, out, len);
    func_00283958(out, out, (Vec4 *)(ctx + 0xC0));
}
#endif

/**
 * Resolve a tracked position into a local vector, then forward it to the shared
 * vector consumer. (EU twin of USA func_002B0D70.)
 */
void func_002B0A70(s32 ctx, void *a, void *b) {
    Vec4 out;

    func_002B0940(ctx, &out, a, b);
    func_002837D0(&out);
}

/**
 * Resolve a tracked position into a local vector and return its z. (EU twin of
 * USA func_002B0DA0.)
 */
f32 func_002B0AA0(s32 ctx, void *a, void *b) {
    Vec4 out;

    func_002B0940(ctx, &out, a, b);
    return out.z;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B0AC8);

/* func_002B0B40 (USA func_002B0E40): PARKED - LOAD-BEARING pin-arity conflict. The
 * real body is 3-arg void(Vec4 *a, Vec4 *out, s32 flag), but this unit already
 * file-scope-declares func_002B0B40 as `s32 func_002B0B40(void *p)` @128 (a forwarder
 * decl a matched caller depends on). A 3-arg #else definition would conflict with the
 * 1-arg prototype, and changing the pin would break that caller's plain 1-arg call
 * (load-bearing). No cast trick exists for a DEFINITION. Kept INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B0B40);

/**
 * Forward to func_002B0B40 on the sub-object at +0x10. (EU twin of USA
 * func_002B0F18.)
 */
s32 func_002B0C18(u8 *p) {
    return func_002B0B40(p + 0x10);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B0C38);

/* func_002B0C40 (USA func_002B0F40): PARKED - LOAD-BEARING pin-arity conflict. The
 * real body is 4-arg void(Vec4 *a, Vec4 *out, Vec4 *src, f32 t), but this unit already
 * file-scope-declares func_002B0C40 as `s32 func_002B0C40(void *p)` @129 (a forwarder
 * decl a matched caller depends on). A 4-arg #else definition would conflict with the
 * 1-arg prototype, and changing the pin would break that caller's 1-arg call
 * (load-bearing). No cast trick exists for a DEFINITION. Kept INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B0C40);

/**
 * Forward to func_002B0C40 on the sub-object at +0x10. (EU twin of USA
 * func_002B0FC0.)
 */
s32 func_002B0CC0(u8 *p) {
    return func_002B0C40(p + 0x10);
}

/* func_002B0CE0 (USA func_002B0FE0): PARKED - ground/surface-clearance probe. Its
 * alt-surface path must call func_002B0C40 (the 4-arg offset-pos-by-dir) with 4 args,
 * but func_002B0C40 is file-scope-pinned `s32(void*)` (see above), so the call needs
 * a `((void(*)(Vec4*,Vec4*,Vec4*,f32))func_002B0C40)(...)` cast - which trips
 * -Wcast-function-type (a NEW gate warning, same class as the parked func_003402B8).
 * The clean fix (retype func_002B0C40 -> the real 4-arg) is load-bearing (breaks the
 * forwarder caller). Deferred with func_002B0B40/C40. Kept INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B0CE0);

/* func_002B0EC0: leading 0x8 padding pair, split off via the symbol_addrs pin
 * so the real body below (func_002B0EC8) starts clean. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B0EC0);

/* func_002B0EC8 (USA func_002B11C8) - sample the breath/oxygen meter value; when
 * the HUD inversion flag is set the meter counts down from 100 instead. The body
 * is C-shape-correct (92.73%) but the EU build reads its normal-path source via
 * `g_pActiveNanotechOrb + 0x24` (a non-zero constant offset) where USA reads the
 * bare symbol D_001B1750 (offset 0). cc1 schedules the offset-0 `%lo` addiu
 * before the jal (nop in the delay slot, the original's form) but SINKS the
 * offset-0x24 addiu INTO the jal delay slot — a scheduler quirk triggered by the
 * non-zero constant offset that no source shape defeats (local-hoist made it
 * worse, 80.91%). This is why the USA twin matches and the EU twin cannot.
 * Left as INCLUDE_ASM. Body for reference:
 *   if (D_1A8D54 == 0) return func_00283708(p, g_pActiveNanotechOrb + 0x24);
 *   return 100.0f - func_00283708(p, &D_1A8D60); */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B0EC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B0F20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B0F70);

/* func_002B1040: leading 0x8 padding pair, split off via the symbol_addrs pin
 * so the real body below (func_002B1048) starts clean. */
INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B1040);

/**
 * func_002B1048 (USA func_002B1348) - resolve a tracked position from a caller
 * vector (128-bit local copy) and forward its x/y to the 2D consumer.
 * Recovered from the EU padding mis-split; EU callees func_002B0940/func_00283B08
 * (USA func_002B0C40/func_00283BF8).
 */
void func_002B1048(s32 ctx, Vec4 *vec, void *b) {
    QVec t;

    t.q = *(u_long128 *)vec;
    func_002B0940(ctx, (void *)&t, (void *)&t, b);
    func_00283B08(t.v.x, t.v.y);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B1080);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B1088);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B11D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B1408);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B1410);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B1478);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B14D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B1570);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B1588);

/**
 * Forward to func_0029D5E8 (GUI text helper; EU twin of USA func_0029DA88).
 * (EU twin of USA func_002B18B0.)
 */
s32 func_002B15B8(s32 a) {
    return func_0029DA88(a);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B15D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B1788);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B17A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B1858);

/**
 * Read the palette-cycle counter base byte. (EU twin of USA func_002B1BC8.)
 */
s32 func_002B18D8(void) {
    return D_139648[0];
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B18E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", CountPlatinumBolts);

/**
 * Count the inventory items whose active weapon variant has reached at least
 * upgrade level 2, clamped to [0, 20]. (EU twin of USA func_002B1D40.)
 */
s32 func_002B19A0(void) {
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
 * Count the earned skill points (set bytes in g_skillPointFlags, 0x20 scanned),
 * clamped to [0, 30].
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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B1A50);
