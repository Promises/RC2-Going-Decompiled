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

typedef unsigned long u_long128 __attribute__((mode(TI)));
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


INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A7D90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A7E68);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A7FF8);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A8238);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A8290);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A82F0);

/**
 * Random small angle: uniform in [-0x800, 0x800) scaled by pi/2048 — i.e. a
 * random angle in [-pi, pi). (EU twin of USA func_002A87A8.)
 */
f32 func_002A8358(void) {
    return (f32)(((func_001163B0() >> 16) & 0xFFF) - 0x800) * 0.0015339808f;
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A83A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A8418);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A84F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A8618);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A86B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A86B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A8820);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A88A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A88B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A8C50);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9100);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A92B8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A93C0);

/**
 * Ground probe with the default parameters: half a unit above the query point,
 * line mask 0x20. (EU twin of USA func_002A9888.)
 */
f32 func_002A9438(Vec4 *pos) {
    return ProbeGroundHeight(pos, 0.5f, 0x20);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9460);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9508);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9618);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9640);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9788);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9830);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9838);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9988);

/**
 * Post a moby hit event with the default hit direction vector. (EU twin of USA
 * func_002A9F30.)
 */
s32 func_002A9AE0(Moby *moby, s32 a, s32 b, s32 c) {
    return PostMobyHitEvent(moby, a, b, c, (Vec4 *)&D_1A8BD0);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9B08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9B10);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9C08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9CA0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002A9DC8);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AA108);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AA2B8);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AABB8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AAC00);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AADA8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AAE08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AAEC0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AAFB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AB1A0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AB268);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AB300);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AB468);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AB6E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AB900);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ABA08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ABA90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ABB50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ABBD0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ABC58);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ABC88);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ABCB8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ABE08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC090);

/**
 * Copy the hero's velocity pair (+0x38, 64-bit) onto another moby. (EU twin of
 * USA func_002AC4B8.)
 */
void func_002AC0E0(Moby *moby) {
    *(u64 *)((u8 *)moby + 0x38) = *(u64 *)((u8 *)g_pHeroMoby[0] + 0x38);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC0F8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC160);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC168);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC290);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC3C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC670);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC6D0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC6D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AC718);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD288);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD550);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD558);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD5A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD630);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD6A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD728);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD800);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD890);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD928);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD948);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AD9D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ADA20);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ADAC8);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ADC40);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ADDB0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ADE90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002ADFD0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AE158);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AE250);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", MarkLevelAvailable);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AE3C0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AE4E0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AE6D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AF288);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AF398);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AF420);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AF640);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AF6C0);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AF7B0);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AF9D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AFA90);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AFB58);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AFB68);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AFC08);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AFD38);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AFE50);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002AFFC8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B00E8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B08D8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B0940);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B09A8);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B09C0);

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

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B0B40);

/**
 * Forward to func_002B0B40 on the sub-object at +0x10. (EU twin of USA
 * func_002B0F18.)
 */
s32 func_002B0C18(u8 *p) {
    return func_002B0B40(p + 0x10);
}

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B0C38);

INCLUDE_ASM("going-decompiled/asm/eu/nonmatchings/text/1A7D10", func_002B0C40);

/**
 * Forward to func_002B0C40 on the sub-object at +0x10. (EU twin of USA
 * func_002B0FC0.)
 */
s32 func_002B0CC0(u8 *p) {
    return func_002B0C40(p + 0x10);
}

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
