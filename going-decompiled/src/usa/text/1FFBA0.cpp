#include "common.h"

/*
 * text/1FFBA0 — TILE B "bolt economy + turret weapon" band
 * (vaddr 0x2FFC20..0x30808F). Carved by the Phase-A mega-batch as an
 * all-INCLUDE_ASM unit; functions matched here are the small leaf helpers of
 * the water-pool / hero-ground-moby / moby-spawn clusters that head the unit.
 *
 * Built at -O2 -G8 -fno-gcse (per-unit GFLAG/CC1EXTRA in objdiff_build.sh /
 * diff.sh) — the later-SN-cc1 TU model. Extern sizing rules (as in the sibling
 * carve units): size <= 8 -> %gp_rel small-data; size >= 16 -> lui/%lo absolute.
 *
 * The unit's c-range stops at 0x301338. The tail (0x301338..0x30808F) is the
 * spimdisasm c-mode tail-fusion blob (bolt/turret functions reached only by
 * j / data-ref, no jal, so spimdisasm cannot promote them) and is islanded as
 * the asm segment text/2012B8 — it would otherwise fuse into one INCLUDE_ASM
 * that objdiff over-scores as fake matches. The split leaf stubs below are the
 * matching surface.
 */

/* 128-bit quadword type (lq/sq copies), as in the sibling carve units. */
#ifdef TARGET_NATIVE
/* gcc -m32 cannot emulate mode(TI); copy/zero-only here, so a 16-byte aligned
 * struct is an exact portable stand-in. Inert to the matching build. */
typedef struct { unsigned long long _q[2]; } __attribute__((aligned(16))) u_long128;
#else
typedef unsigned long u_long128 __attribute__((mode(TI)));
#endif

/* Canonical Moby entity record (full field layout in include/moby.h, sizeof
 * 0x100). The moby helpers in this unit forward an opaque moby handle to the
 * lifecycle/spawn/grid engine functions; a full-size opaque view is enough here
 * (the bodies do their own (u8*)moby offset arithmetic). Byte-neutral: a struct
 * typedef emits no code, and pointer->pointer retyping is ABI-identical. */
typedef struct Moby { u8 _bytes[0x100]; } Moby;
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(Moby) == 0x100, "Moby must be 0x100 under ILP32");
#endif

#ifdef TARGET_NATIVE
/* Shared decls for the water-pool state machine + spark-burst #else bodies. */
typedef f32 Vec4f[4] __attribute__((aligned(16)));

__asm__(".extern g_cameraMatrix, 16");
extern u8 g_cameraMatrix[];     /* 0x1B54F0 camera rotation matrix; row0 at +0x00 */
__asm__(".extern g_cameraPos, 16");
extern u8 g_cameraPos[];        /* 0x1B52C0 camera world position vec4 */
__asm__(".extern g_cameraState, 16");
extern u8 g_cameraState[];      /* 0x1B5180 camera state block */
extern f32 g_cameraProjScale;   /* 0x1B9070 projection scale cot(fov/2) */
__asm__(".extern g_saveImageArea, 16");
extern u8 g_saveImageArea[];    /* 0x1A53A8 per-area save image RAM buffer */
__asm__(".extern g_nNanotechBonusHealTimer, 16");
extern u8 g_nNanotechBonusHealTimer[]; /* 0x189FFC; +4 (0x18A000) written by S6 */

extern void Vec4ScaleVu0(Vec4f dst, f32 s, const Vec4f src);
extern void AddFxDrawHookLate(void (*hook)(void), s32 arg);
extern void RequestGameStateChange(s32 a, s32 b, s32 c, s32 d, s32 e);
extern void StopDialogVoice(void);
extern void BuildCameraProjection(void);
extern void func_00283D10(Vec4f dst);  /* identity/clear quad (has #else body) */
extern f32  func_00283B48(f32 x);       /* VU0 sine   (has #else body) */
extern f32  func_00283B30(f32 x);       /* VU0 cosine (has #else body) */
extern f32  WrapAnglePiSum(f32 a, f32 b);/* WrapAnglePiSum (has #else body) */
extern void func_00300288(void);
extern void func_003007F8(Moby *moby);
extern void *func_003009F8(Moby *moby);
extern void func_00300B88(void);
extern void func_00300E70(void);
extern void func_00300C08(void);
/* weapon/dialog bookkeeping (trap-stubs in native; no-ops in backend_null.c). */
extern void func_002888D8(s32 itemId);
extern void func_00288F30(s32 itemId);
extern void func_002AE6C8(s32 itemId);

/* gp_rel scratch globals (not yet in symbol_addrs; listed for arena regen). */
extern s32 D_001AD7C4;          /* 0x1AD7C4 S0 charge-enable flag */
extern s32 D_001AD7C8;          /* 0x1AD7C8 S0 charge counter (0..0xB5) */
extern u16 D_001AD7DC;          /* 0x1AD7DC S0 initial pool timer (waterPool+0x32) */
extern u16 D_001AD7E0;          /* 0x1AD7E0 S1 pool timer reload */
extern u16 D_001AD7E4;          /* 0x1AD7E4 S4 pool timer reload */
#endif

/* gp-addressable small globals (<= 8 bytes -> %gp_rel). */
extern void *g_pHeroGroundMoby; /* 0x1AD7CC hero support/ground moby ptr */

/* Large/absolute globals (>= 16 -> lui/%lo absolute macro). */
__asm__(".extern g_pHeroMoby, 16");
extern void *g_pHeroMoby; /* 0x18C0B0 player/hero moby ptr */
/* 0x20-stride sound-pool slot; only the +0x1C "active" field is touched here. */
typedef struct SoundPoolSlot {
    u8 pad[0x1C];
    s32 active; /* +0x1C (struct is exactly 0x20 = the table stride) */
} SoundPoolSlot;
__asm__(".extern D_00220000, 16");
extern u8 D_00220000[]; /* 0x220000 shared data region (sound-pool table at +0x1260) */

__asm__(".extern D_1A8BD0, 16");
extern u8 D_1A8BD0[]; /* 0x1A8BD0 transform/config blob used by hero-moby projection */

__asm__(".extern g_waterPool, 16");
extern u8 g_waterPool[]; /* 0x1B2260 static water-pool block; +0x68 = its moby ptr */

__asm__(".extern g_gsScreenContext, 16");
extern u8 g_gsScreenContext[]; /* 0x1A6480 GS screen context (disp dims at +0x150/+0x152) */

/* gadget/turret display scratch block @0x1B2290. The water-pool fade quad reads
 * its level at +0x2 and depth-recip at +0x4; the original addresses them off
 * g_gadgetDisplay (nearest preceding symbol), so we must too for the %hi/%lo
 * reloc to pair by name. */
__asm__(".extern g_gadgetDisplay, 16");
extern u8 g_gadgetDisplay[]; /* 0x1B2290 */

/* 0x1B229A active-gadget swap index; the water-pool tint level lives at +0x2C
 * (== g_waterPool+0x66). The original tint/fade quad emitters address this field
 * off g_swapGadgetItemIndex (nearest preceding symbol), so we must too for the
 * %hi/%lo reloc to pair by name. */
__asm__(".extern g_swapGadgetItemIndex, 16");
extern u8 g_swapGadgetItemIndex[]; /* 0x1B229A */

extern void func_002AE0B8(void *a, void *b, void *c, void *d);
extern void func_002ADF48(void *a, void *b, void *c, void *d, void *e, void *f);
extern void func_00300120(Moby *moby, s32 classId);
extern void *func_00300190(s32 classId, s32 animArg);
extern void AppendGsRegPacket(s32 regId, s64 value);
extern f32 IntToFloat(s32 v);
extern s32 FloatToInt(f32 v);
extern void func_0027E4D0(s32 a, s32 b, s32 c, s32 d, s32 color);

/* Engine helpers used by the #else functional-equivalent bodies below. */
extern void InitMobyFromClass(void *moby, s32 classId);
extern void FillMemory32(void *dst, s32 value, s32 count);
extern void UpdateMobyBSphereAndGrid(void *moby);
extern void *SetMobyAnimSequence(void *moby, s32 seq, s32 flags);
extern void func_002AE198(void *a, void *b, void *c, void *d);
extern void Vec4AddVu0(void *out, void *a, void *b);
extern f32 Vec3DistSqVu0(void *a, void *b);

/* Reference world point (camera/hero position) the turret targeting sorts by. */
__asm__(".extern g_heroPos, 16");
extern u8 g_heroPos[]; /* 0x189EA0 reference position for turret target ranking */

/* Moby spawn arena: aux blocks are indexed off the spawn-pool base. Both globals
 * hold a POINTER to the respective arena base (the code loads through them). */
__asm__(".extern g_mobySpawnStart, 16");
extern u8 *g_mobySpawnStart;   /* 0x1B22E0 -> base of moby spawn slots */
__asm__(".extern g_mobyAuxBlockBase, 16");
extern u8 *g_mobyAuxBlockBase; /* 0x1B22EC -> base of 0x80-byte per-moby aux blocks */

/* Allocate a sound-pool slot (0x20-stride table at D_00220000+0x1260): find the
 * first slot whose +0x1C "active" word is 0, claim it (store handle `id`), stash
 * the source transform (from `src` or, if null, from `moby`+0x10), record the
 * three caller params at +0x10..+0x18, and either snapshot the hero transform
 * (when D_0018A168 set) or kick the pool update (func_002B0E40). Returns the slot
 * index, or -1 if all 0x80 slots are busy.
 * TODO(match): lq/sq quad copies + two callee-saves + branch-likely scan loop
 * cc1 schedules differently; the C is functionally faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", AcquireProjectileCurveAnchor);
#else
extern void func_002B0E40(void *transform, void *slot, s32 flag);
__asm__(".extern g_soundBankHandles, 16");
extern u8 g_soundBankHandles[]; /* 0x18A148 sound-bank handle table (+0x20 base) */
extern s16 D_0018A168;          /* hero-attached-sound enable flag */
s32 AcquireProjectileCurveAnchor(void *p1, void *p2, void *p3, Moby *moby, void *src) {
    SoundPoolSlot *table = (SoundPoolSlot *)(D_00220000 + 0x1260);
    u8 *m = (u8 *)moby;
    u_long128 xform;
    s32 i;
    for (i = 0; i < 0x80; i++) {
        u8 *slot = (u8 *)&table[i];
        if (*(s32 *)(slot + 0x1C) == 0) {
            if (src != 0) {
                xform = *(u_long128 *)src;
            } else {
                xform = *(u_long128 *)(m + 0x10);
            }
            *(void **)(slot + 0x1C) = moby;     /* claim slot with the moby handle */
            *(void **)(slot + 0x10) = p1;       /* caller params at +0x10..+0x18 */
            *(void **)(slot + 0x14) = p2;
            *(void **)(slot + 0x18) = p3;
            if (D_0018A168 != 0) {
                *(u_long128 *)slot = *(u_long128 *)((u8 *)g_pHeroMoby + 0xE0);
            } else {
                func_002B0E40(&xform, slot, 1);
            }
            return i;
        }
    }
    return -1;
}
#endif

/* Clear the +0x1C activity field of sound-pool slot `idx` (0x20-stride table at
 * D_00220000+0x1260). Negative index is a no-op. */
void func_002FFCE0(s32 idx) {
    if (idx >= 0) {
        ((SoundPoolSlot *)(D_00220000 + 0x1260))[idx].active = 0;
    }
}

/* Steer the sound-pool slot `idx` toward its target direction `dir`: snapshots
 * the slot transform, blends a per-slot turn factor (selected by the three
 * D_001A8CA0/A4 + D_0018A168 attach flags), then rotates `dir` by a half-angle
 * quaternion built from the cross/dot of the current and target axes (CosfVu0/
 * SinfVu0 + QuatMultiplyVu0) and rescales back to the original length. No-op for
 * negative idx or when all three attach flags are clear.
 * TODO(hle): dominated by VU0 vector/quaternion intrinsics (Vec3CrossVu0,
 * Vec3DotVu0, QuatMultiplyVu0, ...) — tier-3 hardware math; no byte-exact path. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", AdvanceProjectileCurve);
#else
void AdvanceProjectileCurve(s32 idx, void *dir, void *src) {
    (void)idx; (void)dir; (void)src; /* TODO(hle): VU0 quaternion steering */
}
#endif

/* func_002FFF40: mis-split tail fragment — lone stack-pointer adjustments and a
 * stray %gp_rel store (D_1AB034) with no prologue/jr, severed from the end of the
 * preceding function. Not an independent function; left as INCLUDE_ASM so the
 * original bytes stay intact (documented mis-split exception). */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_002FFF40);

/* (Re)initialise the active-gadget/turret display state block (g_waterPool-area
 * scratch at 0x1B2290): caches the hero zoom factor + reciprocal, snapshots the
 * hero position/orientation, primes a pending-dialog request when idle, then —
 * keyed on the current gadget id (0x1B2298) — picks the gadget moby either from
 * the equipped-weapon table (g_weaponTable, 0xE0-stride) or by spawning a fresh
 * one (func_00300190), recording the result + a "spawned?" flag.
 * TODO(match): heavy scattered-global state block + id dispatch the cc1 won't
 * lay out byte-identically here; the C is functionally faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_002FFF68);
#else
extern s16 g_heroZoom;          /* 0x1AD7D8 hero zoom level */
extern u8 g_gadgetDisplay[];    /* 0x1B2290 gadget-display scratch block */
extern u8 g_heroPosVec[];       /* 0x189EA0 hero pos quad (g_flHeroPos) */
extern u8 g_heroOrientVec[];    /* 0x189EB0 hero orientation quad */
extern s32 g_dialogBusy;        /* 0x1A6410 dialog-voice busy flag */
extern s32 g_pendingVoiceId;    /* 0x1A63CC pending voice line id (-1 = none) */
extern s32 g_pendingVoiceAux;   /* 0x1A63D0 */
extern s32 g_voiceTrigger;      /* 0x1AD7C4 */
extern s32 g_voiceTriggerAux;   /* 0x1AD7C8 */
extern u8 g_weaponClassTable[]; /* 0x239B34 weapon table classId column (0xE0 stride) */
extern u8 g_weaponSlotTable[];  /* 0x239B2C weapon table slot column (0xE0 stride) */
extern u8 g_itemEquippedSlot[]; /* per-item equipped-slot map */
extern u8 g_mobyClassTable[];   /* 0x18B040 class lookup (0x14 stride) */
extern u8 g_quickSelectMap[];   /* 0x139587 quick-select id map */
void func_002FFF68(void) {
    u8 *st = g_gadgetDisplay;
    s32 gadgetId = *(s16 *)(st + 0x08);
    s32 classId;
    s64 animArg;

    *(s16 *)(st + 0x02) = g_heroZoom;
    *(f32 *)(st + 0x04) = 1.0f / IntToFloat(g_heroZoom);
    *(u_long128 *)(st + 0x10) = *(u_long128 *)g_heroPosVec;     /* +0x10..+0x1C */
    *(u_long128 *)(st + 0x20) = *(u_long128 *)g_heroOrientVec;  /* +0x20..+0x2C */
    *(s16 *)(st + 0x00) = 0;  /* asm: sh (halfword) - must NOT overrun into +0x02 (g_heroZoom) */

    if (g_dialogBusy == 0 && g_pendingVoiceId == -1) {
        g_pendingVoiceAux = 0;
        g_pendingVoiceId = 0xDB7;
        g_voiceTrigger = 1;
        g_voiceTriggerAux = 0;
    }
    *(u8 *)(st + 0x0C) = 0;  /* +0x1B229C */
    *(u8 *)(st + 0x0D) = 0;  /* +0x1B229D */

    if (gadgetId == 0x1F) {
        animArg = 1;
        classId = *(s32 *)(g_weaponClassTable + (u32)g_quickSelectMap[0] * 0xE0);
    } else if (gadgetId == 0x2D) {
        animArg = -1;
        classId = *(s32 *)(g_weaponClassTable + (u32)g_quickSelectMap[1] * 0xE0);
    } else if (gadgetId == 9) {
        classId = 0x259;
        animArg = 0;
    } else {
        *(s16 *)(st + 0x00) = 0;  /* asm: sh (halfword) - must NOT overrun into +0x02 (g_heroZoom) */
        *(u8 *)(st + 0x0C) = 0;
        *(u8 *)(st + 0x0D) = 0;
        *(u_long128 *)(st + 0x18) = *(u_long128 *)(g_heroPosVec + 8);
        *(u_long128 *)(st + 0x28) = *(u_long128 *)(g_heroOrientVec + 8);
        *(void **)(st + 0x30) = ((void **)g_mobyClassTable)[
            *(s32 *)(g_weaponSlotTable + (u32)g_itemEquippedSlot[gadgetId] * 0xE0) * 0x14];
        *(u8 *)(st + 0x35) = 1;
        return;
    }
    *(void **)(st + 0x30) = func_00300190(classId, (s32)animArg);
    *(u8 *)(st + 0x35) = 0;
}
#endif

/* func_00300118: empty/no-op leaf (original compiles to jr ra; nop). */
void func_00300118(void) {
}

/* Spawn-init a moby of `classId`: run InitMobyFromClass, attach the moby's
 * per-instance aux block (g_mobyAuxBlockBase + slotIndex*0x80, slot derived from
 * the moby's offset in the spawn pool), zero it (0x80 bytes), and register the
 * moby in the world bsphere/grid unless its flags (+0x34) already carry bit 0x4.
 * cc1 2.9 does not reproduce its two callee-saves (0x10 frame) + branch-likely
 * epilogue; the s136os arm does (GUARD below). */
/* GUARD (task #1271): on EE this C is the image's body, compiled alone by the s136os
 * arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; tools/ee/s136os_functions.txt) and
 * spliced over the S136OS_SLOT line by tools/ee/s136os_splice.sh. There is no asm
 * fallback: a build that skips the splice loses the function. Native: plain C. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_00300120)
S136OS_SLOT(func_00300120);
#else
void func_00300120(Moby *moby, s32 classId) {
    u8 *aux;
    InitMobyFromClass(moby, classId);
    aux = g_mobyAuxBlockBase
        + (((s32)((u8 *)moby - g_mobySpawnStart) >> 8) * 0x80);
    *(u8 **)((u8 *)moby + 0x68) = aux;
    FillMemory32(aux, 0, 0x80);
    if ((*(u16 *)((u8 *)moby + 0x34) & 4) == 0) {
        UpdateMobyBSphereAndGrid(moby);
    }
}
#endif

/* Spawn + arm the water-pool splash/disturbance moby (g_waterPool+0x68 holds the
 * spawn-pool base DAT_001B22C8). Sets up scale/opacity/anim-clear, drives its
 * animation sequence from `animArg` (negative -> auto-pick by anim-controller
 * state), then re-registers it in the world grid. Returns the moby pointer.
 * TODO(match): sq zero-store + branch-likely chain + 0x20 multi-reg frame the
 * cc1 schedules differently; the C is functionally faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00300190);
#else
void *func_00300190(s32 classId, s32 animArg) {
    u8 *m = *(u8 **)(g_waterPool + 0x68);
    u16 flags;
    func_00300120((Moby *)m, classId);
    *(u16 *)(m + 0x34) &= 0xFFBC;
    *(u8 *)(m + 0x30) = 0xFF;
    *(u16 *)(m + 0x32) = 0x20;
    *(u8 *)(m + 0x31) = 1;
    *(f32 *)(m + 0x14) = 5.0f;       /* 0x40A00000 */
    *(f32 *)(m + 0x18) = 900.0f;     /* 0x44610000 */
    *(f32 *)(m + 0x1C) = 1.0f;       /* 0x3F800000 */
    *(f32 *)(m + 0x10) = 5.0f;
    *(u_long128 *)(m + 0xF0) = (u_long128){0};  /* sq zero (portable: native u_long128 is a struct) */
    if (animArg < 0) {
        void *ctrl = *(void **)(m + 0x24);
        if (ctrl != 0 && *(u8 *)((u8 *)ctrl + 0xC) >= 2) {
            SetMobyAnimSequence(m, 1, 0);
        }
    } else {
        SetMobyAnimSequence(m, animArg, 0);
    }
    flags = *(u16 *)(m + 0x34);
    if ((flags & 4) == 0) {
        UpdateMobyBSphereAndGrid(m);
    }
    return m;
}
#endif

/* Init the water-pool splash moby (g_waterPool+0x68) via class 0x3EF, then arm
 * it: scale 5.0 on +0x10/+0x14/+0x18, opacity 0xFF at +0x30, OR in flags 0x43
 * at +0x34, and clear +0x98.
 * Near-miss (objdiff ~58%): the original reloads the moby ptr + reorders the
 * field stores in a schedule this cc1 won't reproduce; the C is faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00300288);
#else
void func_00300288(void) {
    Moby *m = *(Moby **)(g_swapGadgetItemIndex + 0x2E);
    func_00300120(m, 0x3EF);
    m = *(Moby **)(g_swapGadgetItemIndex + 0x2E);
    *(f32 *)((u8 *)m + 0x10) = 5.0f;
    *(u8 *)((u8 *)m + 0x30) = 0xFF;
    *(u16 *)((u8 *)m + 0x34) = (u16)(*(u16 *)((u8 *)m + 0x34) | 0x43);
    *(s32 *)((u8 *)m + 0x98) = 0;
    *(f32 *)((u8 *)m + 0x18) = 5.0f;
    *(f32 *)((u8 *)m + 0x14) = 5.0f;
}
#endif

/* Water-pool surface state machine: dispatches on the pool state word
 * (g_waterPool+0x30, range 0..7) through jtbl_0026DD50_text to advance the
 * splash/ripple/settle phases — each case runs its own VU0 vector math + helper
 * calls (func_00283328, ...) and writes back the pool's level/scale fields.
 * TODO(hle): jump-table state machine over VU0 hardware math; tier-3, no
 * byte-exact path and no compact faithful C. Behaviour documented; stub no-op. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_003002E0);
#else
/* Step one tick of the pool's countdown timer at *t (g_waterPool+0x32). Inlined
 * from func_00283328 (a trap-stub in native): returns 1 when the timer is idle
 * (c==0), 0 while it is still counting down, 2 on the tick it reaches zero. */
static s32 poolTimerStep(s16 *t) {
    s16 c = *t;
    s16 r;
    if (c == 0) {
        return 1;
    }
    r = (s16)((c < 1 ? 1 : c) - 1);   /* max(c,1) - 1 */
    *t = r;
    return r >= 1 ? 0 : 2;            /* r>0 -> still counting, else finished */
}

void func_003002E0(void) {
    s32 state = *(s16 *)(g_waterPool + 0x30);
    s16 *timer = (s16 *)(g_waterPool + 0x32);
    void *poolMoby;

    if ((u32)state >= 8) {
        return;   /* jump-table default: states 0..7 only */
    }

    switch (state) {
    case 0: {
        if (poolTimerStep(timer) == 0) {
            /* still counting: keep the surface-fade hook live and wait */
            AddFxDrawHookLate(func_00300B88, 0);
            return;
        }
        /* decide whether to keep charging or to complete the transition */
        if (D_001AD7C4 != 0) {
            u8 *img = g_saveImageArea + 0x1000;
            s32 charging = (*(s16 *)(img + 0x72) != 3)
                        || (*(s16 *)(img + 0x6C) >= 0x1770);
            if (charging) {
                s32 c = D_001AD7C8;
                D_001AD7C8 = c + 1;
                if (c < 0xB5) {
                    AddFxDrawHookLate(func_00300B88, 0);
                    return;
                }
            }
        }
        /* completion: snap the cinematic camera + arm the pool moby */
        StopDialogVoice();
        *(f32 *)(g_cameraState + 0x140) = 25.0f;   /* 0x41C80000 */
        *(f32 *)(g_cameraState + 0x144) = 25.0f;
        *(f32 *)(g_cameraState + 0x148) = 900.0f;  /* 0x44610000 */
        D_001AD7C4 = 0;
        func_00283D10((f32 *)(g_cameraState + 0x370));
        D_001AD7C8 = 0;
        poolMoby = *(void **)(g_waterPool + 0x60);
        *(f32 *)(g_cameraState + 0x150) = 0.0f;
        *(f32 *)(g_cameraState + 0x154) = 0.0f;
        *(f32 *)(g_cameraState + 0x158) = 0.0f;
        *(s16 *)(g_waterPool + 0x30) = 1;
        *(s16 *)(g_waterPool + 0x32) = (s16)D_001AD7DC;
        *(s16 *)(g_waterPool + 0x3E) = 0;
        *(u_long128 *)((u8 *)poolMoby + 0xF0) =
            *(u_long128 *)(g_cameraState + 0x150);
        if (*(u8 *)(g_waterPool + 0x65) != 0) {
            union { u32 u; f32 f; } negHalfPi = { 0xBFC90FDCu }; /* ~= -pi/2 */
            *(f32 *)((u8 *)poolMoby + 0xF4) =
                WrapAnglePiSum(*(f32 *)((u8 *)poolMoby + 0xF4), negHalfPi.f);
        }
        {
            union { u32 u; f32 f; } k = { 0x3F0FEA69u };
            g_cameraProjScale = func_00283B48(k.f) / func_00283B30(k.f);
        }
        BuildCameraProjection();
        *(u16 *)((u8 *)poolMoby + 0x34) &= 0xFFBE;   /* clear bit 0x41 */
        return;
    }

    case 1: {
        func_003007F8(*(Moby **)(g_waterPool + 0x60));
        if (poolTimerStep(timer) != 0) {
            *(s16 *)(g_waterPool + 0x32) = (s16)D_001AD7E0;
            *(f32 *)(g_waterPool + 0x34) = 1.0f / IntToFloat((s16)D_001AD7E0);
            *(s16 *)(g_waterPool + 0x30) = 2;
            *(u8 *)(g_waterPool + 0x64) = 0;
        }
        return;
    }

    case 2: {
        poolMoby = *(void **)(g_waterPool + 0x60);
        func_003007F8((Moby *)poolMoby);
        func_00300C08();
        if (poolTimerStep(timer) != 0) {
            *(s16 *)(g_waterPool + 0x32) = 2;
            *(s16 *)(g_waterPool + 0x30) = 3;
            *(s16 *)(g_waterPool + 0x66) = 0x80;
            AddFxDrawHookLate(func_00300E70, 0);
        }
        return;
    }

    case 3: {
        poolMoby = *(void **)(g_waterPool + 0x60);
        AddFxDrawHookLate(func_00300E70, 0);
        if (poolTimerStep(timer) != 0) {
            *(s16 *)(g_waterPool + 0x32) = 0xC;
            *(s16 *)(g_waterPool + 0x30) = 4;
            *(void **)(g_waterPool + 0x60) = func_003009F8((Moby *)poolMoby);
        }
        return;
    }

    case 4: {
        poolMoby = *(void **)(g_waterPool + 0x60);
        if (poolMoby != 0) {
            func_003007F8((Moby *)poolMoby);
        }
        AddFxDrawHookLate(func_00300E70, 0);
        /* 0x3DAAAAAB ~= 0.0833333, 0x43000000 = 128.0 */
        *(s16 *)(g_waterPool + 0x66) =
            (s16)FloatToInt(IntToFloat(*(s16 *)(g_waterPool + 0x32))
                            * 0.0833333321f * 128.0f);
        if (poolTimerStep(timer) != 0) {
            *(s16 *)(g_waterPool + 0x30) = 5;
            *(s16 *)(g_waterPool + 0x32) = (s16)D_001AD7E4;
            *(s16 *)(g_waterPool + 0x66) = 0;
        }
        return;
    }

    case 5: {
        poolMoby = *(void **)(g_waterPool + 0x60);
        if (poolMoby != 0) {
            func_003007F8((Moby *)poolMoby);
        }
        if (poolTimerStep(timer) != 0) {
            *(s16 *)(g_waterPool + 0x32) = 3;
            *(s16 *)(g_waterPool + 0x30) = 6;
        }
        return;
    }

    case 6: {
        poolMoby = *(void **)(g_waterPool + 0x60);
        if (poolMoby != 0) {
            func_003007F8((Moby *)poolMoby);
        }
        AddFxDrawHookLate(func_00300B88, 0);
        if (poolTimerStep(timer) == 0) {
            return;
        }
        *(s16 *)(g_waterPool + 0x32) = 0;
        *(u8 *)(g_waterPool + 0x3D) = 1;
        *(s16 *)(g_waterPool + 0x3E) = 1;
        *(s16 *)(g_nNanotechBonusHealTimer + 4) = (s16)(g_heroZoom + 5); /* 0x18A000 */
        *(f32 *)(g_waterPool + 0x34) = 1.0f / IntToFloat(g_heroZoom);
        func_00300288();
        *(void **)(g_waterPool + 0x60) = 0;
        func_002888D8(*(s16 *)(g_waterPool + 0x38));
        func_00288F30(*(s16 *)(g_waterPool + 0x38));
        if (*(u8 *)(g_waterPool + 0x65) != 0) {
            func_002AE6C8(*(s16 *)(g_waterPool + 0x38));
        }
        *(s16 *)(g_waterPool + 0x30) = 7;
        return;
    }

    case 7: {
        u16 cnt = *(u16 *)(g_waterPool + 0x32);
        *(s16 *)(g_waterPool + 0x32) = (s16)(cnt + 1);
        if ((s16)cnt < g_heroZoom) {
            AddFxDrawHookLate(func_00300B88, 0);
        } else {
            RequestGameStateChange(0, 2, 0, 0, 0);
        }
        return;
    }
    }
}
#endif

/* Position + orient the turret/gadget moby `moby` for the current frame:
 * scales a base offset (3.0/2.5/2.0 depending on the moby class id at +0xAA),
 * adds it to the world anchor, advances the spin angle (WrapAnglePiSum), applies
 * the micro-VU0 transform, nudges the position by per-mode offsets, then
 * re-registers the moby (UpdateMobyBSphereAndGrid) and emits its trail
 * (func_00300ED8).
 * TODO(hle): dominated by VU0 vector intrinsics (Vec4ScaleVu0/Vec4AddVu0/
 * TransformVectorMicroVu0) — tier-3 hardware math, no byte-exact path. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_003007F8);
#else
void func_003007F8(Moby *moby) {
    (void)moby; /* TODO(hle): VU0 turret transform + trail */
}
#endif

/* SwapActiveGadgetMoby(oldMoby): swap the in-world active-gadget moby to the
 * gadget chosen by the swap index (g_gadgetDisplay+0x0A). Snapshots the old
 * moby's transform (+0x10 pos, +0xF0 orient), then for most indices frees the old
 * moby, ensures its weapon class is resident, and spawns the new gadget moby
 * (func_00300190); a few indices route through the sub-variant path
 * (func_00300288). Copies the saved transform onto the new moby and, for the
 * sub-variant, clears the gadget's table bookkeeping. Returns the new moby.
 * TODO(match): scattered unnamed-global weapon-table indexing + lq/sq transform
 * copies + many callee-saves cc1 won't reproduce byte-exactly; functionally
 * faithful, modeled against named helpers. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_003009F8);
#else
extern void FreeMoby(void *moby);
extern s32 IsGadgetClassResident(s32 classId);
extern void EnsureGadgetClassResident(s32 classId);
extern void PumpDialogVoiceSystem(s32 arg);
extern void ActivateGadgetMobyClass(s32 classId);
void *func_003009F8(Moby *moby) {
    /* TODO(match): see func_002FFF68 — same scattered weapon-table globals.
     * Behaviour documented above; reconstruct once those globals are named. */
    (void)moby;
    return 0;
}
#endif

/* Emit one water-surface fade quad: append GS reg 0x42 (value 0x44), then draw a
 * full-screen sprite whose alpha = (1 - depthScale * level)*255 (white).
 * Near-miss (objdiff ~71%): cc1 CSEs the g_waterPool base into a callee-saved
 * register (extra s0 save) where the original re-derives it per access; the C
 * is faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00300B88);
#else
void func_00300B88(void) {
    s32 alpha;
    AppendGsRegPacket(0x42, 0x44);
    alpha = FloatToInt((1.0f - IntToFloat(*(s16 *)(g_gadgetDisplay + 0x2))
                               * *(f32 *)(g_gadgetDisplay + 0x4)) * 255.0f);
    func_0027E4D0(0, *(s16 *)(g_gsScreenContext + 0x152), 0,
                  *(s16 *)(g_gsScreenContext + 0x150), alpha << 24);
}
#endif

/* Spawn the turret-tracer impact spark burst: builds two offset anchor points
 * (Vec4Scale/Add off the water-pool basis), lerps a blended position, then emits
 * three SpawnParticleType55 sparks (two red, one white at half size) with hue
 * cycled by an incrementing counter (g_waterPool+0x..C4), patching each spark's
 * texture + brightness bytes.
 * TODO(hle): VU0 vector math + particle hardware emit — tier-3, no byte-exact
 * path. Behaviour documented; stub no-op. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00300C08);
#else
/* Vec3LerpVu0: dst = a + (b-a)*t on xyz, dst.w = a.w (has its own #else body). */
extern void func_002836B8(Vec4f dst, f32 t, const Vec4f a, const Vec4f b);
/* Particle spawner (0x2C74C0). Headless: no-op in backend_null.c. In-game it
 * returns the new particle and the caller patches two of its bytes; headless no
 * particle exists, so those patches are dropped (only the durable hue-counter
 * tick at g_waterPool+0x64 is reproduced). */
extern void SpawnParticleType55(f32 sizeStart, f32 sizeEnd, f32 third,
                                void *pos, void *effectDef, s32 life, u32 color,
                                s32 hue, s32 a8, s32 a9, s32 a10);
__asm__(".extern D_1AD7FC, 16");
extern f32 D_1AD7FC; /* 0x1AD7FC == 2.0f, brightness multiplier */

void func_00300C08(void) {
    Vec4f sp0;            /* sp+0x00 scratch scaled axis */
    Vec4f ptA;           /* sp+0x10 = camPos + 9 * camRow0 */
    Vec4f ptB;           /* sp+0x20 = camPos + 0.1 * camRow0 */
    Vec4f P;             /* sp+0x30 = lerp(ptB, ptA, t) */
    Vec4f *camRow0 = (Vec4f *)g_cameraMatrix;
    Vec4f *camPos  = (Vec4f *)g_cameraPos;
    s32 counter = *(u8 *)(g_waterPool + 0x64);
    f32 t, bright;

    Vec4ScaleVu0(sp0, 9.0f, *camRow0);    /* 0x41100000 */
    Vec4AddVu0(ptA, *camPos, sp0);
    Vec4ScaleVu0(sp0, 0.1f, *camRow0);    /* 0x3DCCCCCD */
    Vec4AddVu0(ptB, *camPos, sp0);

    t = IntToFloat(*(s16 *)(g_waterPool + 0x32)) * *(f32 *)(g_waterPool + 0x34);
    func_002836B8(P, t, ptB, ptA);        /* P = ptB + (ptA - ptB) * t */

    /* bright = D_1AD7FC (2.0) * 0x484D1400 (210000.0) = 420000.0 */
    {
        union { u32 u; f32 f; } k = { 0x484D1400u };
        bright = D_1AD7FC * k.f;
    }

    /* spark 1: red, hue = counter */
    SpawnParticleType55(bright, bright, 0.0f, P, D_1A8BD0, 4, 0xFF2020,
                        counter, 0x7F, 0x7F, 0);
    /* spark 2: red, hue = (counter + 0x80) % 255 */
    SpawnParticleType55(bright, bright, 0.0f, P, D_1A8BD0, 4, 0xFF2020,
                        (counter + 0x80) % 0xFF, 0x7F, 0x7F, 0);
    /* spark 3: white, half size, hue = (counter + 0x80) % 255 */
    {
        f32 bright2 = bright * 0.5f;      /* 0x3F000000 */
        SpawnParticleType55(bright2, bright2, 0.0f, P, D_1A8BD0, 4, 0xFFFFFF,
                            (counter + 0x80) % 0xFF, 0x7F, 0x7F, 0);
    }

    /* durable state: advance the spark hue counter (wraps as a u8). */
    *(u8 *)(g_waterPool + 0x64) = (u8)(counter + 1);
}
#endif

/* Emit the water-surface tint quad: append GS reg 0x42 with the level value
 * (g_waterPool+0x66) packed into the high word, then draw a full-screen sprite
 * with color (level<<24)|0xFFFFFF.
 * Near-miss (objdiff ~61%): cc1 CSEs the g_waterPool base into a callee-saved
 * register (extra s0 save) where the original re-derives it; the C is faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00300E70);
#else
void func_00300E70(void) {
    AppendGsRegPacket(0x42, ((s64)*(s16 *)(g_swapGadgetItemIndex + 0x2C) << 32) | 0x44);
    func_0027E4D0(0, *(s16 *)(g_gsScreenContext + 0x152), 0,
                  *(s16 *)(g_gsScreenContext + 0x150),
                  (*(s16 *)(g_swapGadgetItemIndex + 0x2C) << 24) | 0xFFFFFF);
}
#endif

/* Emit the turret-tracer smoke trail: five iterations of (build an anchor +
 * jittered offset point via Vec4Scale/Add + GetRandomVectorInSphere) then spawn
 * a smoke streak (func_0032F3A8) with randomized length (RandRangeInclusive 8..12)
 * and the trail's grey->red colour ramp.
 * TODO(hle): VU0 vector math + particle hardware emit — tier-3, no byte-exact
 * path. Behaviour documented; stub no-op. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00300ED8);
#else
void func_00300ED8(void) {
    /* TODO(hle): VU0 smoke-trail particle emit (func_0032F3A8). */
}
#endif

/* func_00301010: mis-split tail fragment — the disassembler severed three lone
 * stack-pointer adjustments (addiu $sp) with no prologue/jr from the end of the
 * preceding function. Not an independent function; left as INCLUDE_ASM so the
 * original bytes stay intact (documented mis-split exception). */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00301010);

/* HERO_MOBY_ABS: g_pHeroMoby read through a C-level 16-byte view of the same
 * symbol (EE only). cc1 then emits its own `lui %hi / lw %lo` pair and can
 * schedule the halves apart, as the ROM does. A plain `extern void *` read is
 * one assembler `lw` macro that cc1 cannot split. Plain g_pHeroMoby on native. */
#ifndef TARGET_NATIVE
extern void *g_pHeroMobyAbs[4] __asm__("g_pHeroMoby");
#define HERO_MOBY_ABS (g_pHeroMobyAbs[0])
#else
#define HERO_MOBY_ABS g_pHeroMoby
#endif

/**
 * AdjustPointForHeroGroundMoby - transform a point into the hero ground moby's
 * frame.
 *
 * If a hero ground-moby is bound, transforms `in` through it relative to the
 * hero moby (func_002ADF48 with the D_1A8BD0 config and a stack scratch quad)
 * into `out`; otherwise copies `in` straight to `out`.
 *
 * Byte-exact on the sdk29 arm (task #895). The plain body differed in 6 words,
 * all register choice and order around the two absolute addresses (the old
 * "register-allocation / branch-scheduling" note). Two spellings fix it:
 *  - HERO_MOBY_ABS lets cc1 place `lui v0,%hi(g_pHeroMoby)` itself, which
 *    leaves the ground moby in v1 as in the ROM;
 *  - D_1A8BD0 is passed through a SMALL alias of the same symbol, so cc1 emits
 *    one `la` macro (the assembler expands it absolute, per the file's
 *    `.extern D_1A8BD0, 16`) that stays a single lui/addiu unit after the
 *    `lw`. Through the unsized array cc1 splits it into %hi/%lo and hoists the
 *    `lui` above the `move a1,v1`.
 * Both are plain symbol names on native.
 */
#ifndef TARGET_NATIVE
extern u8 D_1A8BD0_macro __asm__("D_1A8BD0");
#define PROJ_CONFIG_MACRO (&D_1A8BD0_macro)
#else
#define PROJ_CONFIG_MACRO D_1A8BD0
#endif
void AdjustPointForHeroGroundMoby(u_long128 *out, u_long128 *in) {
    u_long128 scratch;

    if (g_pHeroGroundMoby != 0) {
        func_002ADF48(HERO_MOBY_ABS, g_pHeroGroundMoby, in, PROJ_CONFIG_MACRO, out, &scratch);
    } else {
        *out = *in;
    }
}

/* Project point `in` through the hero ground-moby (relative to point `ref`) into
 * `out`: when a ground moby is bound, transform via func_002AE198 into a stack
 * scratch quad then add `in` back onto it (Vec4AddVu0); otherwise copy `in`->`out`
 * verbatim (128-bit quad).
 * TODO(match): two callee-saves + lq/sq quad copy + Vu0 helper schedule cc1 won't
 * reproduce here; the C is functionally faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00301070);
#else
void func_00301070(u_long128 *out, u_long128 *in, void *ref) {
    u_long128 scratch;
    if (g_pHeroGroundMoby == 0) {
        *out = *in;
    } else {
        func_002AE198(g_pHeroMoby, g_pHeroGroundMoby, ref, &scratch);
        Vec4AddVu0(out, in, &scratch);
    }
}
#endif

/**
 * func_003010D8 - project a point through the hero's ground moby.
 *
 * If a hero ground-moby is bound (g_pHeroGroundMoby), forwards to
 * func_002AE0B8(g_pHeroMoby, g_pHeroGroundMoby, a1, a0); otherwise does
 * nothing. a0/a1 are the in/out point pair.
 *
 * Byte-exact on the sdk29 arm (task #895), with two levers:
 *  - the empty-asm sibling-call guard after the call: without it cc1 2.9 turns
 *    the call into a frameless `j`, and the ROM never tail-jumps (FACT #8177);
 *  - g_pHeroMoby read through a C-level 16-byte view bound to the same symbol,
 *    so cc1 emits its own `lui v0,%hi / lw a0,%lo(v0)` pair and schedules the
 *    `lw` into the jal delay slot, with the ground moby left in v1 as in the
 *    ROM. The plain `extern void *` read is an assembler `lw` macro that cc1
 *    cannot split. The old note ("this cc1 won't reproduce the delay-slot
 *    load") was this lever not yet tried.
 */
void func_003010D8(void *a0, void *a1) {
    if (g_pHeroGroundMoby != 0) {
        func_002AE0B8(HERO_MOBY_ABS, g_pHeroGroundMoby, a1, a0);
        __asm__ __volatile__(""); /* sibling-call guard: the ROM keeps the jal */
    }
}

/* Allocate a tracer slot in turret state block `state`: scan entries
 * state[0..0x3F] for the first free (zero) one, store `value` there, set the
 * "any active" flag (+0x224), bump the active count (+0x220), clear the slot's
 * parallel +0x100 flag word, and return the slot index (-1 if all 0x40 full).
 *   state - turret state block (s32 words)
 *   value - tracer handle to store in the free slot
 *
 * Byte-exact on sdk29 (task #1026). The earlier TODO blamed the scan loop,
 * the branch-likely and the delayed return move on scheduling; they are the
 * source shape plus two devices:
 *   - one exit: `result` is returned from a single `jr`, slot 0 is tested
 *     before the loop, and the loop starts with `i++` (a join point, so cc1
 *     keeps i live from 0 instead of folding it to 1). reorg later steals
 *     that i++ into the bnel slot, which is the ROM's `bnel; addiu i`;
 *   - the loop is the R5900 short loop `lw; nop; bnel`: TRACER_SLOT_PAD is
 *     that pad (a SCHEDULING DEVICE under RULING #8435: EE arm only,
 *     operand-tied) and the EMPTY fence beside it (emits nothing, RULING
 *     #8483) lifts the loop over cc1's short-loop threshold (FACT #8384);
 *   - `one` is REGISTER-PINNED to $7 by TRACER_ONE_IN_A3 (a pin device,
 *     empty natively): cc1 2.9 otherwise gives the -1/index result $7 and
 *     the constant $8, the reverse of the ROM.
 */
#ifndef TARGET_NATIVE
#define TRACER_ONE_IN_A3 __asm__("$7")
#define TRACER_SLOT_PAD(v, i) \
    __asm__(".set noreorder\n\tnop\n\t.set reorder" : "+r"(v) : "r"(i))
#else
#define TRACER_ONE_IN_A3
#define TRACER_SLOT_PAD(v, i) ((void)0)
#endif
s32 func_00301110(s32 *state, s32 value) {
    s32 *entry = state;
    register s32 one TRACER_ONE_IN_A3;
    s32 result = -1;
    s32 i = 0;

    one = 1;
    if (*entry == 0) {
        result = 0;
        *entry = value;
        entry[0x89] = one;
        entry[0x88]++;
    } else {
        s32 used;
        do {
            i++;
            if (i >= 0x40) {
                goto done;
            }
            entry++;
            used = *entry;
            __asm__("" : "+r"(used));
            TRACER_SLOT_PAD(used, i);
        } while (used != 0);
        *entry = value;
        result = i;
        state[0x89] = one;
        state[0x88]++;
    }
    entry[0x40] = 0;
done:
    return result;
}

/**
 * func_00301190 - release tracer slot `slot` of turret state block `state`.
 *
 * Zeroes the slot's entry (state[slot]) and its parallel +0x100 flag word,
 * decrements the active-tracer count at +0x220 and, when it reaches 0, clears
 * the +0x224 "any active" flag. Inverse of func_00301110.
 *
 * Byte-exact on the sdk29 arm (task #895). The ROM forms the entry address in
 * v0 from the pre-shifted slot in a1, stores +0 through v0, then COPIES the
 * pointer back into a1 (`move a1,v0`) for the +0x100 store. cc1 2.9 coalesces
 * any plain copy into one register and schedules the +0x100 store first; the
 * old "scheduler picks the other order" note was that coalescing. Here:
 *  - the byte offset is bound to $5 and the entry pointer to $2, so the sll
 *    stays in a1 and the addu lands in v0;
 *  - a memory barrier after the first store keeps +0 before +0x100;
 *  - the copy into $5 goes through an empty "+r" asm, so it is not coalesced.
 * The pins are empty on native.
 */
#ifndef TARGET_NATIVE
#define B1190_IN_A1 __asm__("$5")
#define B1190_IN_V0 __asm__("$2")
#else
#define B1190_IN_A1
#define B1190_IN_V0
#endif
void func_00301190(s32 *state, s32 slot) {
    register s32 byteOffset B1190_IN_A1 = slot << 2;
    register s32 *entry B1190_IN_V0 = (s32 *)((u8 *)state + byteOffset);
    register s32 *flagEntry B1190_IN_A1;
    s32 count;

    entry[0] = 0;
    __asm__ __volatile__("" : : : "memory");
    flagEntry = entry;
    __asm__("" : "+r"(flagEntry));
    flagEntry[0x40] = 0;
    count = state[0x88] - 1;
    state[0x88] = count;
    if (count == 0) {
        state[0x89] = 0;
    }
}

/* Re-rank the turret's active tracer targets: for each occupied slot (entry !=
 * 0, +0x100 flag cleared) compute its squared distance to g_heroPos, then pick
 * the up-to maxTargets (+0x228) nearest still-unclaimed slots into the priority
 * list at +0x200, marking each chosen slot's +0x100 flag. No-op unless the "any
 * active" flag (+0x224) is set.
 * TODO(match): six callee-saves (0x140 frame) + c.lt.s/branch-likely selection
 * loop cc1 schedules differently; the C is functionally faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_003011C0);
#else
void func_003011C0(s32 *state) {
    f32 distSq[0x40];
    s32 active;
    s32 i;

    if (state[0x89] == 0) {  /* +0x224 "any active" */
        return;
    }
    active = 0;
    for (i = 0; i < 0x40; i++) {
        if (state[i] != 0) {
            state[i + 0x40] = 0;  /* clear +0x100 claim flag */
            active++;
            distSq[i] = Vec3DistSqVu0(g_heroPos, (u8 *)(s32)state[i] + 0x10);
            if (active == state[0x88]) {  /* +0x220 count */
                break;
            }
        }
    }
    if (state[0x8a] > 0 && state[0x88] > 0) {  /* +0x228 maxTargets */
        s32 pick = 0;
        do {
            s32 best = -1;
            f32 bestDist = 9999.0f;
            s32 j;
            for (j = 0; j < 0x40; j++) {
                if (distSq[j] < bestDist && state[j + 0x40] == 0 && state[j] == 0) {
                    bestDist = distSq[j];
                    best = j;
                }
            }
            state[pick + 0x80] = state[best];  /* +0x200 priority list */
            state[best + 0x40] = 1;            /* mark +0x100 claimed */
            pick++;
        } while (pick < state[0x8a] && pick < state[0x88]);
    }
}
#endif

/* The unit tail (0x301338..0x30808F) is the spimdisasm c-mode tail-fusion blob:
 * functions reached only by j / data-ref (no jal) that spimdisasm cannot promote
 * to their own symbols, so they fuse into one INCLUDE_ASM. It is islanded as the
 * asm segment text/2012B8 to keep the objdiff count honest. */
