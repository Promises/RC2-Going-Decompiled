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
typedef unsigned long u_long128 __attribute__((mode(TI)));

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

extern void func_002AE0B8(void *a, void *b, void *c, void *d);
extern void func_002ADF48(void *a, void *b, void *c, void *d, void *e, void *f);
extern void func_00300120(void *moby, s32 classId);
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

/* Moby spawn arena: aux blocks are indexed off the spawn-pool base. */
__asm__(".extern g_mobySpawnStart, 16");
extern u8 g_mobySpawnStart[];   /* 0x1B22E0 base of moby spawn slots */
__asm__(".extern g_mobyAuxBlockBase, 16");
extern u8 g_mobyAuxBlockBase[]; /* 0x1B22EC base of 0x80-byte per-moby aux blocks */

/* Allocate a sound-pool slot (0x20-stride table at D_00220000+0x1260): find the
 * first slot whose +0x1C "active" word is 0, claim it (store handle `id`), stash
 * the source transform (from `src` or, if null, from `moby`+0x10), record the
 * three caller params at +0x10..+0x18, and either snapshot the hero transform
 * (when D_0018A168 set) or kick the pool update (func_002B0E40). Returns the slot
 * index, or -1 if all 0x80 slots are busy.
 * TODO(match): lq/sq quad copies + two callee-saves + branch-likely scan loop
 * cc1 schedules differently; the C is functionally faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_002FFC20);
#else
extern void func_002B0E40(void *transform, void *slot, s32 flag);
__asm__(".extern g_soundBankHandles, 16");
extern u8 g_soundBankHandles[]; /* 0x18A148 sound-bank handle table (+0x20 base) */
extern s16 D_0018A168;          /* hero-attached-sound enable flag */
s32 func_002FFC20(void *p1, void *p2, void *p3, u8 *moby, void *src) {
    SoundPoolSlot *table = (SoundPoolSlot *)(D_00220000 + 0x1260);
    u_long128 xform;
    s32 i;
    for (i = 0; i < 0x80; i++) {
        u8 *slot = (u8 *)&table[i];
        if (*(s32 *)(slot + 0x1C) == 0) {
            if (src != 0) {
                xform = *(u_long128 *)src;
            } else {
                xform = *(u_long128 *)(moby + 0x10);
            }
            *(void **)(slot + 0x1C) = moby;     /* claim slot with the moby handle */
            *(void **)(slot + 0x10) = p1;       /* caller params at +0x10..+0x18 */
            *(void **)(slot + 0x14) = p2;
            *(void **)(slot + 0x18) = p3;
            if (D_0018A168 != 0) {
                *(u_long128 *)slot = *(u_long128 *)(g_pHeroMoby + 0xE0);
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_002FFD00);
#else
void func_002FFD00(s32 idx, void *dir, void *src) {
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
    *(s32 *)(st + 0x00) = 0;

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
        *(s32 *)(st + 0x00) = 0;
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
 * TODO(match): two callee-saves (0x10 frame) + branch-likely epilogue cc1 won't
 * reproduce here; the C is functionally faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00300120);
#else
void func_00300120(void *moby, s32 classId) {
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
    func_00300120(m, classId);
    *(u16 *)(m + 0x34) &= 0xFFBC;
    *(u8 *)(m + 0x30) = 0xFF;
    *(u16 *)(m + 0x32) = 0x20;
    *(u8 *)(m + 0x31) = 1;
    *(f32 *)(m + 0x14) = 5.0f;       /* 0x40A00000 */
    *(f32 *)(m + 0x18) = 900.0f;     /* 0x44610000 */
    *(f32 *)(m + 0x1C) = 1.0f;       /* 0x3F800000 */
    *(f32 *)(m + 0x10) = 5.0f;
    *(u_long128 *)(m + 0xF0) = 0;
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
    void *m = *(void **)(g_waterPool + 0x68);
    func_00300120(m, 0x3EF);
    m = *(void **)(g_waterPool + 0x68);
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
void func_003002E0(void) {
    /* TODO(hle): water-pool VU0 state machine (jtbl_0026DD50_text). */
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
void func_003007F8(void *moby) {
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
void *func_003009F8(void *moby) {
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
    alpha = FloatToInt((1.0f - IntToFloat(*(s16 *)(g_waterPool + 0x32))
                               * *(f32 *)(g_waterPool + 0x34)) * 255.0f);
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
void func_00300C08(void) {
    /* TODO(hle): VU0 spark-burst particle emit (SpawnParticleType55). */
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
    AppendGsRegPacket(0x42, ((s64)*(s16 *)(g_waterPool + 0x66) << 32) | 0x44);
    func_0027E4D0(0, *(s16 *)(g_gsScreenContext + 0x152), 0,
                  *(s16 *)(g_gsScreenContext + 0x150),
                  (*(s16 *)(g_waterPool + 0x66) << 24) | 0xFFFFFF);
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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00301010);

/* If a hero ground-moby is bound, transform `in` through it relative to the hero
 * moby (func_002ADF48 with D_1A8BD0 config + a stack scratch quad) into `out`;
 * otherwise copy `in` straight to `out`.
 * Near-miss (objdiff ~90%): register-allocation / branch-scheduling differences
 * this cc1 won't reproduce; the C is faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", AdjustPointForHeroGroundMoby);
#else
void AdjustPointForHeroGroundMoby(u_long128 *out, u_long128 *in) {
    u_long128 scratch;
    if (g_pHeroGroundMoby != 0) {
        func_002ADF48(g_pHeroMoby, g_pHeroGroundMoby, in, D_1A8BD0, out, &scratch);
    } else {
        *out = *in;
    }
}
#endif

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

/* If a hero ground-moby exists, project a point through it relative to the hero
 * moby (forwards to func_002AE0B8). a0/a1 are the in/out point pair.
 * Near-miss (objdiff ~90%): the original schedules the g_pHeroMoby load into the
 * jal delay slot, which this cc1 won't reproduce; the C is faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_003010D8);
#else
void func_003010D8(void *a0, void *a1) {
    if (g_pHeroGroundMoby != 0) {
        func_002AE0B8(g_pHeroMoby, g_pHeroGroundMoby, a1, a0);
        __asm__ __volatile__("");
    }
}
#endif

/* Allocate a tracer slot in turret state block `state`: scan entries
 * state[0..0x3F] for the first free (zero) one, store `value` there, set the
 * "any active" flag (+0x224), bump the active count (+0x220), clear the slot's
 * parallel +0x100 flag word, and return the slot index (-1 if all 0x40 full).
 * TODO(match): the scan loop + branch-likely + delayed return-value move cc1
 * schedules differently; the C is functionally faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00301110);
#else
s32 func_00301110(s32 *state, s32 value) {
    s32 i;
    s32 result = -1;
    s32 *entry = state;
    if (*entry == 0) {
        *entry = value;
        state[0x89] = 1;
        state[0x88]++;
        result = 0;
        entry[0x40] = 0;
        return result;
    }
    for (i = 1; ++entry, i < 0x40; i++) {
        if (*entry == 0) {
            *entry = value;
            state[0x89] = 1;
            state[0x88]++;
            result = i;
            entry[0x40] = 0;
            return result;
        }
    }
    return result;
}
#endif

/* Clear tracer slot `slot` of turret state block `state` (entry at state[slot],
 * plus its parallel +0x100 flag word); decrement the active-tracer count at
 * +0x220 and, when it hits 0, clear the +0x224 "any active" flag.
 * Near-miss (objdiff ~91%): the original emits the two zero-stores in the
 * opposite order (+0 before +0x100) via a copied pointer; cc1's scheduler picks
 * the other order here. The C is faithful. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00301190);
#else
void func_00301190(s32 *state, s32 slot) {
    s32 *entry = (s32 *)((u8 *)state + (slot << 2));
    s32 count;
    entry[0] = 0;
    *(s32 *)((u8 *)entry + 0x100) = 0;
    count = *(s32 *)((u8 *)state + 0x220) - 1;
    *(s32 *)((u8 *)state + 0x220) = count;
    if (count == 0) {
        *(s32 *)((u8 *)state + 0x224) = 0;
    }
}
#endif

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
