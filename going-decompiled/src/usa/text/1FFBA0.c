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
extern void AppendGsRegPacket(s32 regId, s64 value);
extern f32 IntToFloat(s32 v);
extern s32 FloatToInt(f32 v);
extern void func_0027E4D0(s32 a, s32 b, s32 c, s32 d, s32 color);

/* Clear the +0x1C activity field of sound-pool slot `idx` (0x20-stride table at
 * D_00220000+0x1260). Negative index is a no-op.
 * Near-miss (objdiff 99.4%, instruction-identical): cc1 splits the +0x1260 into
 * the addiu immediate instead of folding it into the %hi/%lo reloc addend the
 * original carries (the reloc-addend-fold wall) — the C is correct, not exact. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_002FFCE0);
#else
void func_002FFCE0(s32 idx) {
    if (idx >= 0) {
        ((SoundPoolSlot *)(D_00220000 + 0x1260))[idx].active = 0;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_002FFD00);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_002FFF40);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_002FFF68);

/* func_00300118: empty/no-op leaf (original compiles to jr ra; nop). */
void func_00300118(void) {
}

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00300120);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00300190);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_003002E0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_003007F8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_003009F8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00300C08);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00300ED8);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00301070);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_00301110);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1FFBA0", func_003011C0);

/* The unit tail (0x301338..0x30808F) is the spimdisasm c-mode tail-fusion blob:
 * functions reached only by j / data-ref (no jal) that spimdisasm cannot promote
 * to their own symbols, so they fuse into one INCLUDE_ASM. It is islanded as the
 * asm segment text/2012B8 to keep the objdiff count honest. */
