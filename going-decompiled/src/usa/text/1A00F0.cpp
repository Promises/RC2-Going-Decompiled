#include "common.h"

/*
 * text/1A00F0 — the ".text tail head" sub-TU (carve-pipeline TIER-1-C,
 * 2026-06-14; vaddr 0x2A0170..0x2A81FC, 59 real functions): the moby
 * lifecycle/render/animation/spatial-grid band plus a few gameplay helpers
 * (PickLowAmmoWeaponForDrop, IncrementBestiaryKillCount). It is the asm tile
 * that sits between the matched text/198FA0 (ends 0x2A016F) and text/1A8180
 * (starts 0x2A8200) units, retyped asm->c.
 *
 * The matcher builds THIS unit at -O2 -G8 -fno-gcse (per-unit GFLAG/CC1EXTRA
 * override in tools/ee/objdiff_build.sh / diff.sh / build.sh) — the same
 * later-SN-cc1 gameplay/UI TU model as the neighbouring text units.
 *
 * -G8 extern-sizing rules (carried over from text/1A8180 / text/198FA0):
 *   - size <= 8: true small data, %gp_rel everywhere;
 *   - size 16: cc1-small / assembler-absolute (lui/$at macro everywhere);
 *   - size 9..15 (we use 12): gp-addressable / assembler-absolute (absolute
 *     macro in straight-line code, 1-insn %gp_rel in a branch-delay slot).
 *
 * SAVE-LAYOUT WALL: this TU was built by the later SN cc1 that packs
 * callee-save slots 8-byte; the pinned cc1 reserves 16 bytes per save. Every
 * function that saves two or more GPRs (incl. $ra) is blocked on that wall and
 * stays INCLUDE_ASM (check the prologue: two+ sd of s-regs/$ra at 8-byte
 * spacing). The handwritten VU-chain builders (lqc2/vmul/vadd/sqc2 runs) and
 * the switch / jtbl-reloc-gap functions also stay INCLUDE_ASM.
 *
 * PADDING/FRAGMENT PSEUDO-FUNCTIONS: spimdisasm fused each function's trailing
 * epilogue-pad ("addiu $sp,+N; nop" runs) into the NEXT symbol start; those
 * pad fragments (func_002A0360/0460/0798/0C20) were split off via
 * symbol_addrs.txt size pins and keep their INCLUDE_ASM permanently.
 */

extern void FillMemory32(void *dst, u32 pattern, s32 len);

/* Absolute-addressing overrides (task #671). Each of these globals is 4 bytes,
 * so under -G8 cc1 and the assembler would reach it gp-relative; the ROM loads
 * them with lui/%lo instead. Declaring a larger size to the assembler makes it
 * expand the `lw`/`sw` macros absolutely. Size 16 = absolute everywhere; size 12
 * = absolute in straight-line code but gp-relative in a branch delay slot (the
 * SN-parity rule in tools/ee/asm_unit.sh), which is exactly how the ROM reaches
 * g_mobyVuChainCursor in RenderMobys. These are directives to the EE assembler
 * only, so the native build does not see them. */
#ifndef TARGET_NATIVE
__asm__(".extern g_mobySpawnStart, 16");
__asm__(".extern g_gameTime, 16");
__asm__(".extern g_renderTaskList, 16");
__asm__(".extern g_mobyTableBase, 16");
__asm__(".extern g_frameDmaCursor, 16");
__asm__(".extern g_mobyVuChainCursor, 12");
#endif

/* Canonical Moby entity record (full field layout in include/moby.h, sizeof
 * 0x100). The moby lifecycle helpers in this unit forward an opaque moby handle;
 * the bodies do their own (u8*)moby offset arithmetic, so a full-size opaque view
 * suffices. Byte-neutral (a struct typedef emits no code; the matching arms are
 * INCLUDE_ASM regardless). */
typedef struct Moby { u8 _bytes[0x100]; } Moby;
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
_Static_assert(sizeof(Moby) == 0x100, "Moby must be 0x100 under ILP32");
#endif

/*
 * FreeMoby — release a moby slot.
 *
 *   moby  the moby to release
 *
 * Marks the state byte (+0x20) 0xFD for a static slot (below the dynamic-spawn
 * region, g_mobySpawnStart) or 0xFE for a dynamic one, schedules the release time
 * (+0xA0) two frames out (g_gameTime + 2), then removes the moby from the spatial
 * grid by re-bucketing it with the 0x80807F7F "off-grid" sentinel range.
 *
 * Non-obvious (task #671; the old "if-conversion wall, not reachable by source
 * form" note had tried the guard construct only):
 *  - g_mobySpawnStart and g_gameTime are loaded absolute (lui/lw) although both
 *    are 4-byte, gp-sized under -G8: the `.extern ..., 16` overrides at the top
 *    of this file make the assembler expand them absolutely.
 *  - The empty asm at the head of the static arm keeps cc1 from if-converting
 *    the 0xFD/0xFE choice into `movn`, and the one after the if/else keeps the
 *    state store out of both arms, so the ROM's `beq`/`b` diamond comes out with
 *    0xFD in the `b` delay slot.
 *  - The empty asm after the state store holds the g_gameTime load ahead of the
 *    call's argument setup, where the ROM issues it.
 *  - The call goes through a value-returning cast, which keeps cc1 from turning
 *    the void tail call into a sibling `j` (the ROM keeps jal + epilogue).
 */
extern void *g_mobySpawnStart;
extern s32 g_gameTime;
extern void UpdateMobyGridCells(void *moby, u32 sentinelRange);

void FreeMoby(Moby *moby) {
    u8 state;
    if ((u32)moby < (u32)g_mobySpawnStart) {
        __asm__ __volatile__("");
        state = 0xFD;
    } else {
        state = 0xFE;
    }
    __asm__ __volatile__("");
    *(u8 *)((u8 *)moby + 0x20) = state;
    __asm__ __volatile__("");
    *(s32 *)((u8 *)moby + 0xA0) = g_gameTime + 2;
    ((s32 (*)(void *, u32))UpdateMobyGridCells)(moby, 0x80807F7F);
}

/*
 * ResolveMobyAnimFramePtrs — cache a moby's current animation frame pointers.
 *
 *   moby  the moby whose primary (+0x42 sequence, +0x40 frame) and secondary
 *         (+0x43 sequence, +0x41 frame) animation state is resolved
 *
 * The moby's class record (+0x24) holds a sequence table at +0x48, indexed by
 * a sequence byte. Each sequence's frame-data pointer array starts at +0x1C.
 *  - Primary sequence != 0xFF: +0x58 = that sequence's frame-data pointer for
 *    frame +0x40; +0x6E = seq[0x12]; +0x6C = seq[0x11] (the loop-sound index).
 *  - Primary sequence 0xFF ("procedural"): +0x58 points into the procedural
 *    frame pool (g_proceduralAnimFrames + frame * 0x800), +0x6C = 0xFF and
 *    +0x6E = 0.
 *  - Secondary: +0x5C = the frame-data pointer for (+0x43, +0x41), always.
 * No return value.
 *
 * Non-obvious (task #759; replaces a "WALLED at 63.60%" note):
 *  - The ROM reloads the primary sequence byte and its table entry in each of
 *    the three statements. The sequence reads are still hoisted above the +0x58
 *    store. A volatile read (MOBY_PRIMARY_SEQ) keeps cse from merging them
 *    but lets sched hoist them past the non-volatile store. The class-pointer
 *    load is volatile too, so the two volatile loads keep their source order,
 *    which is the ROM's `lw` before `lbu`.
 *  - The table entry is read as `void *`, the same type as the +0x58 store.
 *    The store then aliases the later entry loads and they stay below it.
 *  - `table - -(i * 4)` keeps the base first in `addu` (#756's operand-order
 *    lever). The `+` form puts the shifted index first.
 *  - The test reads the sequence into a `u32`, not a `u8`. With a `u8`, cc1
 *    copies it into a second register for the 0xFF arm's store and loses the
 *    `beql`.
 */
extern u8 g_proceduralAnimFrames[];

#define MOBY_PRIMARY_SEQ(m)       (*(volatile u8 *)((m) + 0x42))
#define ANIM_SEQ_AT(table, seq)   ((u8 *)*(void **)((table) - -((seq) * 4)))

void ResolveMobyAnimFramePtrs(Moby *moby) {
    u8 *m = (u8 *)moby;
    u32 seq = m[0x42];

    if (seq != 0xFF) {
        u8 *seqTable = *(u8 *volatile *)(m + 0x24) + 0x48;
        *(void **)(m + 0x58) = *(void **)(ANIM_SEQ_AT(seqTable, MOBY_PRIMARY_SEQ(m)) - -(m[0x40] * 4) + 0x1C);
        m[0x6E] = ANIM_SEQ_AT(seqTable, MOBY_PRIMARY_SEQ(m))[0x12];
        m[0x6C] = ANIM_SEQ_AT(seqTable, MOBY_PRIMARY_SEQ(m))[0x11];
    } else {
        m[0x6C] = seq;
        m[0x6E] = 0;
        *(void **)(m + 0x58) = g_proceduralAnimFrames + (m[0x40] << 11);
    }
    *(void **)(m + 0x5C) = *(void **)(ANIM_SEQ_AT(*(u8 **)(m + 0x24) + 0x48, m[0x43]) - -(m[0x41] * 4) + 0x1C);
}

/* UpdateMobyAnimLoopSound: maintain the moby's looping sequence sound. The active
 * emitter slot is held in +0x6D (0xFF = none) and the current sequence id in +0x6C.
 * If a slot is active, it is stopped (and the slot released) when our slot was
 * stolen (the emitter's owner at +0x88 no longer points at this moby), the playing
 * sequence changed (emitter +0x7E != +0x6C), the moby's stop flag +0x34 bit 0x40 is
 * set, or +0x7C bit 0x8000 is set. If no slot is active and the sequence is present
 * (+0x6C != 0xFF) and neither stop flag is set, it starts the loop via
 * PlayMobySound(seq, 4, moby) and records the returned slot in +0x6D. Emitter slots
 * live at g_listenerPosHistory + slot*0x70. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", UpdateMobyAnimLoopSound);
#else
extern u8 g_listenerPosHistory[];
extern s32 PlayMobySound(s32 seq, s32 mode, Moby *moby);
extern void StopSoundEmitter(s32 slot);
void UpdateMobyAnimLoopSound(Moby *moby) {
    u8 *m = (u8 *)moby;
    s32 slot = m[0x6D];

    if (slot != 0xFF) {
        u8 *emitter = g_listenerPosHistory + slot * 0x70;
        if (*(void **)(emitter + 0x88) != (void *)moby) {
            /* our slot was stolen by another moby */
            m[0x6D] = 0xFF;
            return;
        }
        if (*(s16 *)(emitter + 0x7E) != m[0x6C] ||
            (*(u16 *)(m + 0x34) & 0x40) ||
            (*(u16 *)(m + 0x7C) & 0x8000)) {
            StopSoundEmitter(slot);
            m[0x6D] = 0xFF;
        }
        return;
    }

    /* no active emitter: start the loop unless the sequence is absent or blocked */
    if (m[0x6C] == 0xFF ||
        (*(u16 *)(m + 0x34) & 0x40) ||
        (*(u16 *)(m + 0x7C) & 0x8000)) {
        return;
    }
    m[0x6D] = PlayMobySound(m[0x6C], 4, moby);
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0360);

/* func_002A0368: compute the moby's current animation frame time (in 1/16 units).
 * Selects the active sequence descriptor (+0x5C when the +0x42 sequence id is 0xFF,
 * else +0x58) and reads its frame count (descriptor +0x4). With no blend
 * (+0x44 == 0) the result is frames/16. When a blend is active and the two sequence
 * ids (+0x42/+0x43) match with +0x41 >= +0x40, it linearly interpolates the frame
 * counts of the +0x58 and +0x5C descriptors by the blend and scales by 1/16;
 * otherwise it is frames/16 + blend. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0368);
#else
extern f32 IntToFloat(s32 n);
f32 func_002A0368(Moby *moby) {
    u8 *m = (u8 *)moby;
    void *animA = (m[0x42] == 0xFF) ? *(void **)(m + 0x5C) : *(void **)(m + 0x58);
    f32 blend = *(f32 *)(m + 0x44);
    f32 framesA = IntToFloat(*(s16 *)((char *)animA + 4));

    if (blend == 0.0f) {
        return framesA * 0.0625f;
    }
    if (m[0x42] == m[0x43] && m[0x41] >= m[0x40]) {
        void *animB = *(void **)(m + 0x5C);
        f32 framesB = IntToFloat(*(s16 *)((char *)animB + 4));
        return (framesA * (1.0f - blend) + framesB * blend) * 0.0625f;
    }
    return framesA * 0.0625f + blend;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0460);

/*
 * func_002A0480(moby, out1, out2): read the moby's animation-set dimension and
 * its log2. animBase(moby+0x24) holds a count byte at +0x2C; when nonzero the
 * last set record (animBase + count*0x10) carries a signed 16-bit dimension at
 * +0x2 -> stored to *out1, with Log2Floor(dimension*2) stored to *out2 (the
 * power-of-two bucket for that dimension). When the count is zero both outs are
 * cleared. Used when sizing/allocating the moby's anim working buffer.
 *
 * Byte-exact on the engine96 arm (cc1 2.96-ee-001003 via MATCH_func_002A0480,
 * task #565): unit objdiff 100.00% (objdiff_build.sh + unit_report.sh, clean
 * tree), raw-verified byte-identical. The 2.9 arm differs from the ROM in
 * 6 of its 22 words, in THREE classes (word-by-word audit, task #577):
 *   - callee-save stride/frame, words 0/2/20: the ROM uses a 0x10 frame with
 *     $31 at +0x8; 2.9 uses 0x20 with $31 at +0x10.
 *   - `addu` operand order, word 8: ROM `addu $2,$3,$2`, 2.9 `addu $2,$2,$3`.
 *   - restore order, words 17/18: the ROM restores $16 then $31, 2.9 the
 *     reverse.
 * Fixing the stride alone leaves words 8, 17 and 18 differing, so the stride
 * is NOT the only residual -- an earlier version of this comment claimed it
 * was, and that was measured false. The engine96 arm reproduces all three,
 * which is why it is the arm here: an ARM CHOICE, not a wall.
 * The INCLUDE_ASM below still feeds the 2.9 link in build.sh, which defines
 * no MATCH_.
 */
extern s32 Log2Floor(s32 value);

#if defined(MATCH_func_002A0480) || defined(TARGET_NATIVE)
void func_002A0480(Moby *moby, s32 *out1, s32 *out2) {
    u8 *anim = *(u8 **)((u8 *)moby + 0x24);
    u8 count = anim[0x2C];
    if (count != 0) {
        s16 dim = *(s16 *)(anim + count * 0x10 + 0x2);
        *out1 = dim;
        *out2 = Log2Floor(dim << 1);
    } else {
        *out1 = 0;
        *out2 = 0;
    }
}
#else
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0480);
#endif

/* func_002A04D8 — pose and average two collision-mesh keyframe vectors for a
 * moby into dst. Locates the class-header entry (header at obj+0x24, base index
 * at header+0x2C, plus arg1) and takes hi = max of the two frame counts at
 * entry+0x6 / entry+0xE. When hi >= 0 it skins the moby's collision mesh
 * (SkinMobyCollisionMesh(obj, hi + 1, 0x80000000)) into the SPR cache. It then loads the two packed
 * keyframe vectors (func_00283AE0 -> vecA/vecB, w set to 1), optionally samples
 * the per-frame scratchpad vectors at 0x70000000 + count*64 (func_00283A70),
 * scales each by (obj+0x2C)/1024, applies the moby's 3x3 rotation (func_00283A48
 * over obj+0xC0) and translation (Vec4AddVu0 obj+0x10), averages the two into dst
 * (add then *0.5), and stores the Vec3DistVu0 scalar into dst.w. Callee roles
 * func_00283A48/func_00283AE0/Vec3DistVu0 UNCONFIRMED (named by shape); helper
 * signatures cross-referenced to text/183558.c (scale families take f32 2nd).
 * Params: obj — the moby; arg1 — entry index past the header's base; dst —
 * receives the midpoint (xyz) and the half-span distance (w). No return. */
#ifdef TARGET_NATIVE
extern void Vec4AddVu0(void *dst, void *a, void *b);
extern void Vec4ScaleVu0(void *dst, f32 s, void *src);
extern void func_00283A48(void *out, void *v, void *m);
extern void func_00283A70(void *out, void *v, void *m);
extern void func_00283AE0(void *dst, u64 packed);
extern f32  Vec3DistVu0(void *a, void *b);
extern void SkinMobyCollisionMesh(void *moby, s32 count, u32 flags);
#endif

/* MATCHED on the s136os arm (task #1375): byte-exact under SN 2.95.3 v1.36
 * -fopt-stack (verify_match_unit + image cmp). Levers, each measured by undoing
 * it alone (FACT filed with task #1375):
 *  - SkinMobyCollisionMesh gets the MOBY (`obj`), not the entry: the ROM leaves
 *    $a0 = obj untouched for that call (0x2A0534), as 1A8180.c's other caller
 *    passes its moby. The earlier arm passed `entry` — a semantic bug, not only
 *    a codegen one;
 *  - hi is an s32 maximum written `hi = a; if (hi < b) hi = b;`, which the ROM
 *    lowers to slt + movn (0x2A0520) with no s16 re-truncation;
 *  - the entry address is formed as `hdr + base*16`, then `+= arg1*16 + 0x40`
 *    (the ROM's two adds into $s1, 0x2A0510);
 *  - vecA[3] is written before vecB[3]; cc1 emits them in the ROM's reverse
 *    order (last-store-first, FACT #8947). */
/* GUARD (task #1375): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002A04D8)
S136OS_SLOT(func_002A04D8);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void SkinMobyCollisionMesh(void *moby, s32 count, u32 flags);
extern f32 Vec3DistVu0(void *a, void *b);
extern void Vec4AddVu0(void *dst, void *a, void *b);
extern void Vec4ScaleVu0(void *dst, f32 s, void *src);
extern void func_00283A48(void *out, void *v, void *m);
extern void func_00283A70(void *out, void *v, void *m);
extern void func_00283AE0(void *dst, u64 packed);
/* (end of this body's declarations) */
void func_002A04D8(void *obj, s32 arg1, void *dst) {
    u8 *o = (u8 *)obj;
    u8 *hdr = *(u8 **)(o + 0x24);
    u8 *entry = hdr + *(u8 *)(hdr + 0x2C) * 16;
    s32 hi;
    s32 b;
    f32 vecA[4];
    f32 vecB[4];

    entry += arg1 * 16 + 0x40;
    hi = *(s16 *)(entry + 0x6);
    b = *(s16 *)(entry + 0xE);
    if (hi < b) {
        hi = b;
    }
    if (hi >= 0) {
        SkinMobyCollisionMesh(obj, hi + 1, 0x80000000);
    }
    func_00283AE0(vecA, *(u64 *)(entry + 0));
    func_00283AE0(vecB, *(u64 *)(entry + 8));
    vecA[3] = 1.0f;
    vecB[3] = 1.0f;
    if (hi >= 0) {
        func_00283A70(vecA, vecA, (void *)(0x70000000 + (*(s16 *)(entry + 0x6) << 6)));
        func_00283A70(vecB, vecB, (void *)(0x70000000 + (*(s16 *)(entry + 0xE) << 6)));
    }
    Vec4ScaleVu0(vecA, *(f32 *)(o + 0x2C) * (1.0f / 1024.0f), vecA);
    Vec4ScaleVu0(vecB, *(f32 *)(o + 0x2C) * (1.0f / 1024.0f), vecB);
    func_00283A48(vecA, vecA, o + 0xC0);
    func_00283A48(vecB, vecB, o + 0xC0);
    Vec4AddVu0(vecA, vecA, o + 0x10);
    Vec4AddVu0(vecB, vecB, o + 0x10);
    Vec4AddVu0(dst, vecA, vecB);
    Vec4ScaleVu0(dst, 0.5f, dst);
    *(f32 *)((u8 *)dst + 0xC) = Vec3DistVu0(dst, vecA);
}
#endif

/* func_002A0678 — transform a moby's indexed sub-vector set into world space.
 * The moby's class header (obj+0x24) holds a count at +0x2E; the entry table
 * sits at header + count*16. Byte arg3 offsets into that table to a sentinel:
 * if the following byte is 0xFF (empty) it returns 0, else that byte indexes the
 * source vec4 (entry + idx*16). The source is scaled by (obj+0x2C)*(1/1024) via
 * ScaleVec4IncludingW, then transformed by the moby's matrix rows (obj+0xC0 /
 * obj+0x10) into dst. When outArray is non-null and the entry's count field
 * (+0x16) is positive, each of those extra vec4s is likewise loaded
 * (func_00283AE0), scaled (Vec4ScaleVu0) and w-terminated (1.0) into outArray.
 * Returns the entry count. Callee roles func_00283A48/func_00283AE0 UNCONFIRMED
 * (named by shape); the 8-byte func_00283AE0 arg is the raw qword the asm loads.
 * MATCHED on the s136os arm (task #1344): byte-identical to the ROM in the
 * image. On EE this C is compiled alone by SN 2.95.3 v1.36 -fopt-stack (row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh; a build that skips the splice drops the function.
 * Closing shape: the count, header count and loop counter are s32 (s16 locals
 * cost a sll/sra pair per use), the sentinel byte is read as entry[arg3 + 1]
 * (gives the ROM's `addu arg3, entry` operand order), and the loop advances
 * its counter and source pointer after the body (the ROM decrements before
 * the first call and steps the source in its delay slot). */
#ifdef TARGET_NATIVE
/* ScaleVec4IncludingW def site is text/183558.c: the f32 scale is the 2nd param
 * (dst, s, src), NOT trailing. Vec4AddVu0/Vec4ScaleVu0/func_00283A48/func_00283AE0
 * are already declared with func_002A04D8 above. */
extern void ScaleVec4IncludingW(void *dst, f32 s, void *src);
#endif

#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002A0678)
S136OS_SLOT(func_002A0678);
#else
/* Prototypes this body needs whose declarations sit in other guarded arms:
 * the s136os arm compiles this arm alone, so it must see them here. */
extern void ScaleVec4IncludingW(void *dst, f32 s, void *src);
extern void Vec4AddVu0(void *dst, void *a, void *b);
extern void Vec4ScaleVu0(void *dst, f32 s, void *src);
extern void func_00283A48(void *out, void *v, void *m);
extern void func_00283AE0(void *dst, u64 packed);
s32 func_002A0678(void *obj, void *dst, void *outArray, s32 arg3) {
    u8 *hdr = *(u8 **)((u8 *)obj + 0x24);
    s32 n = *(s16 *)(hdr + 0x2E);
    u8 *entry;
    u8 *sub;
    f32 scale;
    s32 count;
    u32 idx;

    if (n == 0) {
        return 0;
    }
    entry = hdr + n * 16;
    if (entry[arg3 + 1] == 0xFF) {
        return 0;
    }
    idx = entry[arg3 + 1];
    scale = *(f32 *)((u8 *)obj + 0x2C) * (1.0f / 1024.0f);
    sub = entry + idx * 16;
    ScaleVec4IncludingW(dst, scale, sub);
    sub += 0x10;
    func_00283A48(dst, dst, (u8 *)obj + 0xC0);
    Vec4AddVu0(dst, dst, (u8 *)obj + 0x10);

    count = *(s16 *)(sub + 6);
    if (outArray != 0 && count > 0) {
        u8 *loopDst = (u8 *)outArray;
        s32 i = count;
        do {
            func_00283AE0(loopDst, *(u64 *)sub);
            Vec4ScaleVu0(loopDst,
                         *(f32 *)((u8 *)obj + 0x2C) * (1.0f / 1024.0f), loopDst);
            *(f32 *)(loopDst + 0xC) = 1.0f;
            i--;
            sub += 8;
            loopDst += 0x10;
        } while (i != 0);
    }
    return count;
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0798);

/*
 * func_002A07B0(moby, sub, rec): one-time init of a moby render/anim sub-record
 * `rec` and link it onto moby's chain (head at moby+0x54). No-op if already
 * initialised (rec[1] != 0). Stamps rec[0]=sub, rec[1]=1, four 1.0f fields at
 * +0x1C/+0x20/+0x24/+0x28, derives a 0x70000000-based scratchpad packet pointer
 * at +0x4 by walking moby's descriptor table (moby+0x24 -> +0x1C -> [rec[0]<<2 +4]
 * -> +idx), then push-links rec at the head of moby's +0x54 list.
 * Left INCLUDE_ASM, but not a wall. Task #759 brought it to one swapped pair
 * of adjacent instructions: the ROM has `sb` (rec[1] = 1) before
 * `lui a1,0x7000`. The float grouping and the late pointer chain do come out
 * of C, using volatile float stores and volatile rec[0]/class-pointer reads.
 * The C that gets there and the variants tried are in FACT #8055.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A07B0);
#else
/* SCREEN-EXACT on the s136os arm (SN 2.95.3 v1.36 -fopt-stack; task #1389,
 * masked word screen + relocation compare, NOT vmu): a CANDIDATE, not a match.
 * Levers, each needed (screen with that one undone): `sub` is an s32 - the ROM
 * stores $5 with no zero-extension (as u8: the original first diff, @1 andi);
 * the four 1.0f stores written +0x1C/+0x20/+0x24/+0x28 so cc1 emits the ROM's
 * +0x28-first order (FACT #8947 last-store-first; written +0x28 first: 4/30);
 * the class-table entry read as base[rec[0] + 1] (the byte-offset spelling
 * colours the add differently: 10/30); and the packet index read as
 * p[idx + 4] (the (p + idx)[4] spelling: 14/30). No volatile is needed on
 * this arm (task #759's 2.9-arm variant used volatile reads, FACT #8055). */
void func_002A07B0(u8 *moby, s32 sub, u8 *rec) {
    u8 *p;
    s32 idx;
    if (rec[1] == 0) {
        rec[0] = sub;
        rec[1] = 1;
        *(float *)(rec + 0x1C) = 1.0f;
        *(float *)(rec + 0x20) = 1.0f;
        *(float *)(rec + 0x24) = 1.0f;
        *(float *)(rec + 0x28) = 1.0f;
        {
            u8 *base = *(u8 **)(*(u8 **)(moby + 0x24) + 0x1C);
            p = ((u8 **)base)[rec[0] + 1];
        }
        idx = *p;
        *(u32 *)(rec + 4) = (p[idx + 4] << 6) + 0x70000000;
        *(u32 *)(rec + 8) = *(u32 *)(moby + 0x54);
        *(u8 **)(moby + 0x54) = rec;
    }
}
#endif

/*
 * func_002A0828(list, node) — unlink `node` from `list`'s singly-linked chain
 * and wipe it.
 *
 * Callers (measured on the USA ROM image, task #783): 2 distinct caller
 * functions, 3 call sites, all direct `jal` (word 0x0C0A820A):
 *   - FreeWeaponEffectSlot (0x2FFBB8), releasing a weapon-effect slot:
 *     1 site, 0x2FFBE4
 *   - func_002AFAB0 (0x2AFAB0): 2 sites, 0x2AFB14 and 0x2AFBE8
 * Method: every word of extracted/usa/SCUS_972.68.rom was checked for that
 * jal, for `j`, for the raw pointer 0x002A0828, and for a lui 0x2A +
 * addiu/ori 0x0828 pair. Only the 3 jal sites were found. The scan does not
 * cover overlays loaded at run time or pointers built by arithmetic.
 * Grepping the source undercounts. FreeWeaponEffectSlot's site is in the
 * bulk asm/usa/text/1FD030.s, not under nonmatchings/. A caller that has
 * been promoted to C has no .s file at all. And no grep can find a `jalr`.
 * func_002AFAB0 has both a nonmatchings .s and a C arm in text/1A8180.c,
 * so taking the union of those two greps counts it twice.
 *
 *   list  owner record; its chain head is at +0x54
 *   node  0x40-byte node, linked through +0x8; NULL is a no-op
 *
 * If `node` is the head, the head moves to node->next. Otherwise the walk
 * finds node's predecessor and splices node out; a node that is not on the
 * chain is left alone. Either way, a non-NULL node is then zeroed with
 * FillMemory32(node, 0, 0x40). No return value.
 *
 * Non-obvious (task #759; this replaces an old "~41% branch-likely wall" note,
 * which had not tried the constructs below):
 *  - The head test comes before `prev = list->head`. That leaves the ROM's
 *    `bne`/`nop` and puts the copy in the next `beqz`'s delay slot. Assigning
 *    prev inside the test moves the copy into a `bnel` slot instead.
 *  - The loop entry test is written out as an `if` around a do/while. The ROM
 *    tests the first link without the pad, then loops with it. A plain
 *    `while` makes cc1 copy the whole condition, pad included, to the loop
 *    entry.
 *  - A0828_LOOP_PAD is the R5900 short-loop pad: two `nop`s that bring the
 *    4-instruction loop up to 6 (see the note in text/1A8180.c). No compiler
 *    or assembler that the project runs emits it.
 *  - `noTailCall = 0` is a dead store that stops cc1 turning the trailing void
 *    call into a sibling `j`. The ROM keeps jal + epilogue. Here the value-
 *    returning cast and the empty volatile asm give the same bytes.
 */
#ifndef TARGET_NATIVE
#define A0828_LOOP_PAD() ({ __asm__ __volatile__(".set noreorder\n\tnop\n\tnop\n\t.set reorder"); })
#else
#define A0828_LOOP_PAD() ((void)0)
#endif
struct LinkedNode { u8 _pad[8]; struct LinkedNode *next; };
struct NodeChainOwner { u8 _pad[0x54]; struct LinkedNode *head; };
void func_002A0828(struct NodeChainOwner *list, struct LinkedNode *node) {
    struct LinkedNode *prev;
    s32 noTailCall;
    if (node != 0) {
        if (list->head == node) {
            list->head = node->next;
        } else if (list->head != 0) {
            prev = list->head;
            if (prev->next != 0 && prev->next != node) {
                do {
                    prev = prev->next;
                } while (prev->next != 0 && (A0828_LOOP_PAD(), prev->next != node));
            }
            if (prev->next == node) {
                prev->next = node->next;
            }
        }
        FillMemory32(node, 0, 0x40);
        noTailCall = 0;
    }
}

/*
 * AcquireProceduralAnimSlot (func_002A08C0): first-fit/reuse a procedural-anim-frame
 * slot for a moby. Scans g_proceduralAnimSlotOwners[0..0xF]; reuses the slot already
 * owned by `moby`, or claims the first free (==0) slot, storing `moby` as the owner
 * and resetting that slot's frame counter (g_proceduralAnimSlotTimer); returns the
 * slot index, or -1 if all 16 slots are taken by other mobys.
 * Byte-exact on the sdk29 arm (cc1 2.9-ee-991111 at -O2 -G8 -fno-gcse, this
 * unit's flags; task #565): unit objdiff 100.00% (objdiff_build.sh +
 * unit_report.sh, clean tree), raw-verified byte-identical. No guard: the 2.9
 * arm owns it, and the engine96 arm reaches only 48.81%.
 *
 * Supersedes this comment's former "WALL (~67%) ... branch-LIKELY (beqzl/bnel)
 * ... not reachable by source form" claim, which re-measures at 100.00% from
 * the same source. The cc1 does emit the branch-likely pair; the ~67% predates
 * this unit's per-unit -G8 -fno-gcse model in objdiff_build.sh.
 */
extern u32 g_proceduralAnimSlotOwners[];
extern u32 g_proceduralAnimSlotTimer[];

s32 func_002A08C0(u32 moby) {
    s32 slot;
    for (slot = 0; slot < 0x10; slot++) {
        u32 cur = g_proceduralAnimSlotOwners[slot];
        if (cur == 0 || cur == moby) {
            g_proceduralAnimSlotOwners[slot] = moby;
            g_proceduralAnimSlotTimer[slot] = 0;
            return slot;
        }
    }
    return -1;
}

/*
 * func_002A0918 (ResetProceduralAnimSlots in symbol_addrs) — clear all 16
 * procedural-animation slots: zero g_proceduralAnimSlotOwners[0..15] and
 * g_proceduralAnimSlotTimer[0..15] (e.g. on level reset). No params, no return.
 *
 * Non-obvious (task #689; FACT #7957 had this walled on the `bgez` slot):
 *  - The array loop gives the ROM's prologue (lui owners, lui timers, then the
 *    two addiu in the opposite order), the down-counting $a0 and the loop
 *    alignment `nop`.
 *  - The ROM's loop is 5 real instructions plus one R5900 short-loop pad `nop`
 *    before `bgez`, with the timer-pointer increment in the delay slot. This
 *    cc1 refuses to fill a backward branch's slot when the filled loop would be
 *    that short, and it counts an inline asm by its template lines. The pad
 *    asm's three lines lift the count over the limit, so reorg fills the slot.
 *  - The pad is a NON-volatile asm: it reads the timer pointer (so the timer
 *    increment stays after it and lands in the slot) and clobbers memory (so it
 *    stays after both stores), but the counter and owner-pointer increments may
 *    still schedule above it, as in the ROM. A non-volatile asm needs an output;
 *    `padOut` is it, pinned to $a1 so it does not take the counter's $a0, and
 *    the empty asm at the loop head consumes it so the pad is not deleted.
 * The native build has no delay slots and gets the plain loop.
 */
void func_002A0918(void) {
    s32 i;
#ifndef TARGET_NATIVE
    register s32 padOut asm("$5");
#endif

    for (i = 0; i < 16; i++) {
#ifndef TARGET_NATIVE
        __asm__ __volatile__("" : : "r"(padOut));
#endif
        g_proceduralAnimSlotOwners[i] = 0;
        g_proceduralAnimSlotTimer[i] = 0;
#ifndef TARGET_NATIVE
        __asm__(".set noreorder\n\tnop\n\t.set reorder"
                : "=r"(padOut)
                : "r"(&g_proceduralAnimSlotTimer[i])
                : "memory");
#endif
    }
}

/* func_002A0958: service the 16 procedural-animation slots. For each slot with an
 * owner moby: release the slot (clear the owner) if the owner is being torn down
 * (+0x20 bit 0x80) or is no longer idle (+0x42 sequence != 0xFF). While idle, hold
 * for +0x31 frames or until the per-slot timer reaches 0x14; once elapsed, if the
 * owner's rest animation descriptor (via +0x24 anim set, indexed by +0x43) matches
 * the idle sequence, kick the blend back toward rest (+0x44 = 1 - +0x4C) and step
 * the animation (UpdateMobyAnimation). Slots with no owner reset their timer. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0958);
#else
extern void UpdateMobyAnimation(Moby *moby);
void func_002A0958(void) {
    s32 i;
    for (i = 0; i < 0x10; i++) {
        u8 *owner = (u8 *)g_proceduralAnimSlotOwners[i];
        u8 *animDesc;

        if (owner == NULL) {
            g_proceduralAnimSlotTimer[i] = 0;
            continue;
        }
        if ((owner[0x20] & 0x80) || owner[0x42] != 0xFF) {
            g_proceduralAnimSlotOwners[i] = 0;
            continue;
        }
        if (owner[0x31] != 0 || g_proceduralAnimSlotTimer[i] < 0x14) {
            g_proceduralAnimSlotTimer[i]++;
            continue;
        }
        animDesc = *(u8 **)(*(u8 **)(owner + 0x24) + owner[0x43] * 4 + 0x48);
        if (animDesc[0x13] == owner[0x42]) {
            *(f32 *)(owner + 0x44) = 1.0f - *(f32 *)(owner + 0x4C);
            UpdateMobyAnimation((Moby *)owner);
        }
    }
}
#endif

/* func_002A0A58 — pose one moby keyframe vector into the caller's buffer arg2.
 * func_002A4D60 fills arg2 from the moby's keyframe data (input word = arg1 at a
 * scratch slot). The posed vector at arg2+0x30 is scaled by (obj+0x2C)/1024,
 * then the moby's rotation is built (func_00284008 from obj+0xC0 into a scratch
 * matrix) and applied (MatrixMultiplyVu0 arg2 = matrix * arg2), and the
 * translation (obj+0x10) is added. Helper signatures cross-referenced to
 * text/183558.c. Callee func_002A4D60 UNCONFIRMED (named by shape). */
#ifdef TARGET_NATIVE
extern void func_002A4D60(void *obj, s32 flag, void *inParams, void *outBuf);
extern void func_00284008(void *dst, void *src);
extern void MatrixMultiplyVu0(void *dst, void *a, void *b);
#endif

/* MATCHED on the s136os arm (task #1324): byte-exact solo under SN 2.95.3
 * v1.36 -fopt-stack (verify_match_unit, FACT #8810's method). Closing lever:
 * the callee prototypes (declared only in a TARGET_NATIVE block, so the
 * s136os TU saw them implicitly as int functions) are repeated in this arm. */
/* GUARD (task #1324): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002A0A58)
S136OS_SLOT(func_002A0A58);
#else
/* Prototypes this body needs whose declarations sit in other guarded arms:
 * the s136os arm compiles this arm alone, so it must see them here. */
extern void func_002A4D60(void *obj, s32 flag, void *inParams, void *outBuf);
extern void func_00284008(void *dst, void *src);
extern void MatrixMultiplyVu0(void *dst, void *a, void *b);
extern void Vec4AddVu0(void *dst, void *a, void *b);
extern void Vec4ScaleVu0(void *dst, f32 s, void *src);
void func_002A0A58(void *obj, s32 arg1, void *arg2) {
    u8 *o = (u8 *)obj;
    u8 *a2 = (u8 *)arg2;
    u8 buf[0x50];
    f32 scale = *(f32 *)(o + 0x2C) * (1.0f / 1024.0f);

    *(s32 *)(buf + 0x40) = arg1;
    func_002A4D60(obj, 1, buf + 0x40, arg2);
    Vec4ScaleVu0(a2 + 0x30, scale, a2 + 0x30);
    func_00284008(buf, o + 0xC0);
    MatrixMultiplyVu0(arg2, buf, arg2);
    Vec4AddVu0(a2 + 0x30, a2 + 0x30, o + 0x10);
}
#endif

/* func_002A0AF8 — pose one moby keyframe vector into a local scratch buffer and
 * transform it into dst. func_002A4D60 fills the scratch (input word = arg1); the
 * posed vector at scratch+0x30 is scaled by (obj+0x2C)/1024 into dst, then the
 * moby's rotation (func_00283A48 over obj+0xC0) and translation (Vec4AddVu0
 * obj+0x10) are applied. Callee func_002A4D60 UNCONFIRMED (named by shape). */
/* MATCHED on the s136os arm (task #1324): byte-exact solo under SN 2.95.3
 * v1.36 -fopt-stack (verify_match_unit, FACT #8810's method). Closing lever:
 * the scratch buffer SIZE (0x50, as in func_002A0A58: the 0x80 guess
 * gave a 0xB0 frame against the ROM's 0x80) plus the callee prototypes in this arm. */
/* GUARD (task #1324): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002A0AF8)
S136OS_SLOT(func_002A0AF8);
#else
/* Prototypes this body needs whose declarations sit in other guarded arms:
 * the s136os arm compiles this arm alone, so it must see them here. */
extern void func_002A4D60(void *obj, s32 flag, void *inParams, void *outBuf);
extern void func_00283A48(void *out, void *v, void *m);
extern void Vec4AddVu0(void *dst, void *a, void *b);
extern void Vec4ScaleVu0(void *dst, f32 s, void *src);
void func_002A0AF8(void *obj, s32 arg1, void *dst) {
    u8 *o = (u8 *)obj;
    u8 buf[0x50];
    f32 scale = *(f32 *)(o + 0x2C) * (1.0f / 1024.0f);

    *(s32 *)(buf + 0x40) = arg1;
    func_002A4D60(obj, 1, buf + 0x40, buf);
    Vec4ScaleVu0(dst, scale, buf + 0x30);
    func_00283A48(dst, dst, o + 0xC0);
    Vec4AddVu0(dst, dst, o + 0x10);
}
#endif

/* func_002A0B80 — pose and transform a run of `count` moby keyframe vectors in
 * place. Forwards its args to func_002A4C08 (handwritten VU0 helper that fills
 * the dst run from the moby's keyframe data), then for each of the count vectors
 * at dst[i] (stride 0x10) scales by (obj+0x2C)/1024 (Vec4ScaleVu0), applies the
 * moby rotation (func_00283A48 over obj+0xC0) and adds the translation
 * (Vec4AddVu0 obj+0x10). Callee func_002A4C08 UNCONFIRMED (named by shape). */
#ifdef TARGET_NATIVE
extern void func_002A4C08(void *obj, s32 count, void *arg2, void *dst);
#endif

/* MATCHED on the s136os arm (task #1324): byte-exact solo under SN 2.95.3
 * v1.36 -fopt-stack (verify_match_unit, FACT #8810's method). Closing lever:
 * the LOOP SHAPE (an up-counting `i < count` keeps count in s1 and the
 * cursor in s0, as the ROM does; the down-counting loop swapped them) plus the
 * callee prototypes in this arm. */
/* GUARD (task #1324): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002A0B80)
S136OS_SLOT(func_002A0B80);
#else
/* Prototypes this body needs whose declarations sit in other guarded arms:
 * the s136os arm compiles this arm alone, so it must see them here. */
extern void func_002A4C08(void *obj, s32 count, void *arg2, void *dst);
extern void func_00283A48(void *out, void *v, void *m);
extern void Vec4AddVu0(void *dst, void *a, void *b);
extern void Vec4ScaleVu0(void *dst, f32 s, void *src);
void func_002A0B80(void *obj, s32 count, void *arg2, void *dst) {
    u8 *o = (u8 *)obj;
    u8 *p = (u8 *)dst;
    f32 scale = *(f32 *)(o + 0x2C) * (1.0f / 1024.0f);
    s32 i;

    func_002A4C08(obj, count, arg2, dst);
    for (i = 0; i < count; i++) {
        Vec4ScaleVu0(p, scale, p);
        func_00283A48(p, p, o + 0xC0);
        Vec4AddVu0(p, p, o + 0x10);
        p += 0x10;
    }
}
#endif

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0C20);

/* CloseMobyDmaSegment — close the moby texture-upload DMA segment. Reserves a
 * DMATAG qword at g_frameDmaCursor and back-patches the segment's open tag
 * (g_mobySegmentOpenTag) into a CNT tag chaining to it, uploads the moby
 * textures (UploadMobyTextures over g_vramAllocCursor) + appends the default
 * TEX0 flush, then lays two more CNT DMATAG qwords closing the chain. DMATAGs
 * are 4 words (0x10 bytes); 0x20000000 = CNT. g_frameDmaCursor is re-read after
 * the upload/flush calls (they append through it). No params, no return. */
#ifdef TARGET_NATIVE
extern u32  *g_frameDmaCursor;         /* 0x1B2228 per-frame DMA write pointer */
extern u32  *g_mobySegmentOpenTag;     /* 0x1B1AD0 moby draw-segment open tag  */
extern void *g_vramAllocCursor;        /* 0x1A72D0 VRAM bump cursor            */
extern void  UploadMobyTextures(void *vramCursor);
extern void  AppendTexFlushDefaultTex0(void);
#endif

/* ADDRESSING-MODEL DEVICE (RULING #8620; FACT #8036's size-16 equate form, as
 * 1EFFC0.cpp's *Abs equates): the ROM reaches g_mobySegmentOpenTag absolutely
 * in CloseMobyDmaSegment and BeginMobyDrawSegment (0x2A0C40 `lui v1,
 * %hi(g_mobySegmentOpenTag)`), while the symbol is -G8 small. gas sizes a
 * symbol once per file, so those references name a second assembler symbol
 * EQUATED to it and sized 16; the relocation still names g_mobySegmentOpenTag.
 * Top level, so the s136os TU and the unit's 2.9 TU both define it (the splice
 * refuses a block naming a symbol only its own TU defines). Nothing is moved and
 * no instruction is emitted; native reads the plain symbol. */
#ifndef TARGET_NATIVE
__asm__(".extern g_mobySegmentOpenTagAbs, 16\n\tg_mobySegmentOpenTagAbs = g_mobySegmentOpenTag");
extern u32 *g_mobySegmentOpenTagAbs;
#else
#define g_mobySegmentOpenTagAbs g_mobySegmentOpenTag
#endif

/* MATCHED on the s136os arm (task #1375): byte-exact under SN 2.95.3 v1.36
 * -fopt-stack (verify_match_unit + image cmp). Closing levers, each measured by
 * undoing it alone (FACT filed with task #1375):
 *  - g_mobySegmentOpenTag through the size-16 equate above (ADDRESSING; NOTE
 *    #8992's first diff `lw v1,0(gp)` | `lui v0,0x1b` is this);
 *  - the closing tag's words written in field order [0],[1],[2],[3]; the
 *    scheduler then emits the ROM's [0],[3],[1],[2];
 *  - an empty operand-tied fence on `start` (RULING #8483: it emits nothing).
 *    Without it cc1 loads g_frameDmaCursor straight into $s0; the ROM loads it
 *    into $v0 and copies it to $s0 (0x2A0C38 `daddu s0,v0,zero`). */
/* GUARD (task #1375): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_CloseMobyDmaSegment)
S136OS_SLOT(CloseMobyDmaSegment);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void AppendTexFlushDefaultTex0(void);
extern void UploadMobyTextures(void *vramCursor);
extern u32 *g_frameDmaCursor;
extern u32 *g_mobySegmentOpenTag;
extern void *g_vramAllocCursor;
/* (end of this body's declarations) */
void CloseMobyDmaSegment(void) {
    u32 *start = g_frameDmaCursor;

    __asm__("" : "+r"(start));  /* codegen fence, see above */
    g_frameDmaCursor += 4;
    g_mobySegmentOpenTagAbs[0] = 0x20000000;
    g_mobySegmentOpenTagAbs[1] = (u32)g_frameDmaCursor;
    g_mobySegmentOpenTagAbs[2] = 0;
    g_mobySegmentOpenTagAbs[3] = 0;

    UploadMobyTextures(g_vramAllocCursor);
    AppendTexFlushDefaultTex0();

    g_frameDmaCursor[0] = 0x20000000;
    g_frameDmaCursor[1] = (u32)(g_mobySegmentOpenTagAbs + 4);
    g_frameDmaCursor[2] = 0;
    g_frameDmaCursor[3] = 0;
    g_frameDmaCursor += 4;

    start[0] = 0x20000000;
    start[1] = (u32)g_frameDmaCursor;
    start[2] = 0;
    start[3] = 0;
}
#endif

/* PatchMobyPacketTex0 — stamp texture-VRAM coordinates into every loaded moby
 * class's GIF packets. Walks the present-class-slot list at
 * &g_mobyClassDataSizes[0xF0] (s32 slot indices, terminated by a negative
 * entry). For each slot's class header it follows the texture-binding node
 * chain at header+0x20 (nodes stride 0x10; node+0xC packs the GIF-packet-entry
 * pointer in bits 0..30 and a "has next node" flag in bit 31). Each node holds
 * an inline tex-index byte list (from node+0, terminated by 0xFF); per index it
 * looks up g_mobyTexVramTable[idx] (two s16 VRAM fields) and OR's each non-zero
 * field into the low 14 bits of the packet's +0x30 / +0x40 TEX0 words (packet
 * stride 0x40). No params, no return.
 *
 * Non-obvious (task #689):
 *  - The node's link word is read twice, once for the packet pointer at the top
 *    and again for the "has next node" test at the bottom. The inner loop's u32
 *    stores may alias it, so cc1 must reload, as the ROM does.
 *  - The packet pointer and the cursor are set before the empty-list test, so
 *    the mask lands in that branch's delay slot.
 *  - The class-slot walk keeps a separate next-slot pointer. That gives the
 *    ROM's two registers ($a0 for the slot being read, $t4 for the next one)
 *    and the `move` at the loop bottom. A plain `classSlot++` folds them into
 *    one register.
 */
extern u32   g_mobyClassDataSizes[];   /* 0x1D0D80  &[0xF0] = present-slot list */
extern void *g_mobyClassHeaders[];     /* 0x1CDB00  class header ptr per slot   */
extern s16   g_mobyTexVramTable[];     /* 0x1D0980  2 s16 VRAM fields per tex    */

void PatchMobyPacketTex0(void) {
    s32 *classSlot = (s32 *)&g_mobyClassDataSizes[0xF0];
    s32 *nextSlot;

    while (*classSlot >= 0) {
        u8 *node = *(u8 **)((u8 *)g_mobyClassHeaders[*classSlot] + 0x20);
        s32 nodeLink;

        nextSlot = classSlot + 1;
        do {
            u8 *packet = (u8 *)(*(u32 *)(node + 0xC) & 0x7FFFFFFF);
            u8 *cursor = node;

            if (*node != 0xFF) {
                do {
                    s16 *vram = &g_mobyTexVramTable[*cursor * 2];

                    if (vram[0] != 0) {
                        *(u32 *)(packet + 0x30) =
                            (*(u32 *)(packet + 0x30) & 0xFFFFC000) | vram[0];
                    }
                    if (vram[1] != 0) {
                        *(u32 *)(packet + 0x40) =
                            (*(u32 *)(packet + 0x40) & 0xFFFFC000) | vram[1];
                    }
                    cursor++;
                    packet += 0x40;
                } while (*cursor != 0xFF);
            }
            nodeLink = *(s32 *)(node + 0xC);
            node += 0x10;
        } while (nodeLink >= 0);
        classSlot = nextSlot;
    }
}

/* func_002A0DF0 — recompute the moby glow segment's 2D light direction from the
 * hero. func_002A1320 fills a 2-float vector from g_pHeroMoby; Atan2fPoly
 * turns it into an angle, and the sin/cos-style pair func_00283B30 /
 * func_00283B48 is scaled by 0.14 into the glow parameter block at
 * g_deferredSegment2Tag+0x10 (cos) / +0x14 (sin), with a fixed -0.99 at +0x18.
 * Callee roles UNCONFIRMED (named by shape). No params, no return. */
#ifdef TARGET_NATIVE
extern s32   g_deferredSegment2Tag;
extern void *g_pHeroMoby;              /* 0x18C0B0 hero (Ratchet) moby         */
extern void  func_002A1320(void *moby, f32 *outVec);
extern f32   Atan2fPoly(f32 a, f32 b);
extern f32   func_00283B30(f32 x);
extern f32   func_00283B48(f32 x);
#endif

/* MATCHED on the s136os arm (task #1350): byte-exact solo under SN 2.95.3
 * v1.36 -fopt-stack (verify_match_unit, FACT #8810's method). Closing lever: the
 * declarations in this arm (NOTE #8954: it did not compile solo without them);
 * g_pHeroMoby and g_deferredSegment2Tag read through section(".data") ASM-LABEL
 * aliases (ADDRESSING-MODEL DEVICES, RULING #8620: they move no data, emit
 * nothing, and the relocations name the real symbols), because the ROM
 * reaches both absolute; and the three stores written in field order
 * +0x10/+0x14/+0x18 straight through the address (a `glow` pointer local is
 * hoisted into $s0 before the first call, the ROM forms it after the second). */
/* GUARD (task #1350): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002A0DF0)
S136OS_SLOT(func_002A0DF0);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern f32 Atan2fPoly(f32 a, f32 b);
extern f32 func_00283B30(f32 x);
extern f32 func_00283B48(f32 x);
extern void func_002A1320(void *moby, f32 *outVec);
extern s32 g_deferredSegment2Tag;
extern void *g_pHeroMoby;
/* ADDRESSING-MODEL DEVICES (RULING #8620), EE arm only: second C names for
 * g_deferredSegment2Tag / g_pHeroMoby (same assembler symbols via the asm
 * labels). section(".data") tells cc1 they are not -G8 small data, so it splits
 * %hi/%lo as the ROM does (0x2A0DF0 `lui v0,%hi(g_pHeroMoby)`, 0x2A0E3C
 * `lui v0,%hi(g_deferredSegment2Tag+0x10)`); they move no data and emit nothing. */
#ifndef TARGET_NATIVE
extern s32 g_deferredSegment2TagAbs __asm__("g_deferredSegment2Tag") __attribute__((section(".data")));
extern void *g_pHeroMobyAbs __asm__("g_pHeroMoby") __attribute__((section(".data")));
#else
#define g_pHeroMobyAbs g_pHeroMoby
#define g_deferredSegment2TagAbs g_deferredSegment2Tag
#endif
/* (end of this body's declarations) */
void func_002A0DF0(void) {
    f32 vec[2];
    f32 angle;
    func_002A1320(g_pHeroMobyAbs, vec);
    angle = Atan2fPoly(vec[0], vec[1]);
    ((f32 *)((u8 *)&g_deferredSegment2TagAbs + 0x10))[0] = func_00283B30(angle) * 0.14f;
    ((f32 *)((u8 *)&g_deferredSegment2TagAbs + 0x10))[1] = func_00283B48(angle) * 0.14f;
    ((f32 *)((u8 *)&g_deferredSegment2TagAbs + 0x10))[2] = -0.99f;
}
#endif

/* CloseMobyGlowSegment — close the deferred moby-glow draw segment. If no glows
 * were queued this frame (g_mobyGlowCount == 0) it writes just an END DMATAG
 * (0x10000000) into the segment tag at *g_deferredSegment2Tag. Otherwise it
 * reserves a CNT qword, builds the glow records (BuildMobyGlowRecords) and emits
 * their packets (EmitMobyGlowPackets over g_mobyGlowWorkBuf), then closes the
 * chain with two more CNT DMATAGs. g_deferredSegment2Tag holds the segment-tag
 * build pointer, so every access re-reads it; g_frameDmaCursor is re-read after
 * the calls. No params, no return. */
#ifdef TARGET_NATIVE
extern s32  g_mobyGlowCount;           /* 0x1B1AFC glow records queued         */
extern u8   g_mobyGlowWorkBuf[];       /* 0x1EF260 glow record work buffer     */
extern void BuildMobyGlowRecords(void);
extern void EmitMobyGlowPackets(void *workBuf);
#endif

/* ADDRESSING-MODEL DEVICES (RULING #8620; FACT #8036's size-16 equate form):
 * CloseMobyGlowSegment reaches g_mobyGlowCount (0x2A0E84) and every
 * g_deferredSegment2Tag reference (0x2A0E9C...) absolutely, while both are -G8
 * small and this unit's own `.extern g_deferredSegment2Tag, 16` (further down,
 * above the frame-close function) comes after this function, so it does not pin
 * these uses: gas decides them from the file's last size, cc1's `, 4`. So
 * those references name second assembler symbols EQUATED to the real ones and
 * sized 16; the relocations still name g_mobyGlowCount / g_deferredSegment2Tag.
 * (`...Abs16`, because func_002A0DF0's arm already spells a C-level
 * g_deferredSegment2TagAbs asm-label alias.) Top level, so both the s136os TU
 * and the unit's 2.9 TU define them. Nothing is moved, nothing is emitted;
 * native reads the plain symbols. */
#ifndef TARGET_NATIVE
__asm__(".extern g_mobyGlowCountAbs, 16\n\tg_mobyGlowCountAbs = g_mobyGlowCount");
__asm__(".extern g_deferredSegment2TagAbs16, 16\n\tg_deferredSegment2TagAbs16 = g_deferredSegment2Tag");
extern s32 g_mobyGlowCountAbs;
extern s32 g_deferredSegment2TagAbs16;
#else
#define g_mobyGlowCountAbs g_mobyGlowCount
#define g_deferredSegment2TagAbs16 g_deferredSegment2Tag
#endif

/* MATCHED on the s136os arm (task #1375): byte-exact under SN 2.95.3 v1.36
 * -fopt-stack (verify_match_unit + image cmp). Closing levers (FACT filed with
 * task #1375):
 *  - g_mobyGlowCount and g_deferredSegment2Tag through the size-16 equates
 *    above (ADDRESSING; NOTE #8992's first diff `lw v0,0(gp)` | `lui v0,0x1b`);
 *  - each tag word written through `(u32 *)g_deferredSegment2Tag` afresh. The
 *    tag pointer is an s32, which a u32 store may alias, so cc1 re-reads it
 *    before every store as the ROM does; a cached `tag` local loads it once;
 *  - the tag words and the closing `start` words in field order [0],[1],[2],[3];
 *  - an empty operand-tied fence on `start` (RULING #8483), the same as
 *    CloseMobyDmaSegment's: the ROM loads g_frameDmaCursor into $v0 and copies it
 *    to $s0 (0x2A0EE8 `daddu s0,v0,zero`). */
/* GUARD (task #1375): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_CloseMobyGlowSegment)
S136OS_SLOT(CloseMobyGlowSegment);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void BuildMobyGlowRecords(void);
extern void EmitMobyGlowPackets(void *workBuf);
extern s32 g_deferredSegment2Tag;
extern u32 *g_frameDmaCursor;
extern s32 g_mobyGlowCount;
extern u8 g_mobyGlowWorkBuf[];
/* (end of this body's declarations) */
void CloseMobyGlowSegment(void) {
    if (g_mobyGlowCountAbs == 0) {
        /* empty frame: just an END tag */
        ((u32 *)g_deferredSegment2TagAbs16)[0] = 0x10000000;
        ((u32 *)g_deferredSegment2TagAbs16)[1] = 0;
        ((u32 *)g_deferredSegment2TagAbs16)[2] = 0;
        ((u32 *)g_deferredSegment2TagAbs16)[3] = 0;
    } else {
        u32 *start = g_frameDmaCursor;

        __asm__("" : "+r"(start));  /* codegen fence, see above */
        g_frameDmaCursor += 4;
        ((u32 *)g_deferredSegment2TagAbs16)[0] = 0x20000000;
        ((u32 *)g_deferredSegment2TagAbs16)[1] = (u32)g_frameDmaCursor;
        ((u32 *)g_deferredSegment2TagAbs16)[2] = 0;
        ((u32 *)g_deferredSegment2TagAbs16)[3] = 0;

        BuildMobyGlowRecords();
        EmitMobyGlowPackets(g_mobyGlowWorkBuf);

        g_frameDmaCursor[0] = 0x20000000;
        g_frameDmaCursor[1] = (u32)((u32 *)g_deferredSegment2TagAbs16 + 4);
        g_frameDmaCursor[2] = 0;
        g_frameDmaCursor[3] = 0;
        g_frameDmaCursor += 4;

        start[0] = 0x20000000;
        start[1] = (u32)g_frameDmaCursor;
        start[2] = 0;
        start[3] = 0;
    }
}
#endif

/* RunSprRenderPipeline — kick the sprite/moby render pass. Flushes any pending
 * RPC (func_0011AEA0(0)), stages the 0x800-byte DMA/GIF template (D_238E80) into
 * the render scratchpad at 0x70003800 via CopyQwords, then runs the frame's
 * render task list (RunRenderTaskList over g_renderTaskList / g_renderTaskWorkBuf).
 *
 * Non-obvious (task #671): g_renderTaskList is loaded absolute (the `.extern`
 * override at the top of this file) while g_renderTaskWorkBuf stays gp-relative
 * in the jal delay slot, and the final call goes through a value-returning cast
 * so cc1 keeps the ROM's jal + epilogue instead of a sibling `j`. The older
 * "engine save-layout wall" note was wrong: the function saves only $ra. */
extern void  func_0011AEA0(s32 arg);
extern void  CopyQwords(void *dst, void *src, s32 len);
extern void  RunRenderTaskList(void *taskList, void *workBuf);
extern u8    D_238E80[];              /* 0x238E80  0x800-byte SPR render template */
extern void *g_renderTaskList;        /* 0x1B1630 */
extern void *g_renderTaskWorkBuf;     /* 0x1B1634 */

void RunSprRenderPipeline(void) {
    func_0011AEA0(0);
    CopyQwords((void *)0x70003800, D_238E80, 0x800);
    ((s32 (*)(void *, void *))RunRenderTaskList)(g_renderTaskList, g_renderTaskWorkBuf);
}

/* Clears 0x3C0 bytes of the moby scratchpad block at 0x70003A00 to 0x40000000.
 * The empty-asm guard suppresses cc1's sibling-call (tail-jump) so the original
 * jal + frame is reproduced. */
void func_002A1000(void) {
    FillMemory32((void *)0x70003A00, 0x40000000, 0x3C0);
    __asm__ __volatile__("");
}

/* func_002A1028 — save the procedural-anim bounds scratch (0x3C0 bytes at
 * scratchpad 0x70003A00) back to g_proceduralAnimBounds[0x280] via CopyQwords.
 *
 * Non-obvious (task #671): the call goes through a value-returning cast. That
 * stops cc1 turning the void tail call into a sibling `j`, and unlike the older
 * empty-asm guard it leaves the prologue schedule alone, so `sd $ra` lands
 * between the two address `lui`s as in the ROM. The previous "prologue-
 * scheduling wall (81.82%)" was the guard's own side effect. */
extern u8 g_proceduralAnimBounds[];
extern void CopyQwords(void *dst, void *src, s32 len);

void func_002A1028(void) {
    ((s32 (*)(void *, void *, s32))CopyQwords)(&g_proceduralAnimBounds[0x280], (void *)0x70003A00, 0x3C0);
}

/* func_002A1058 — load the procedural-anim bounds (g_proceduralAnimBounds[0x280])
 * into the scratchpad block at 0x70003A00 via CopyQwords. The inverse of
 * func_002A1028, written with the same value-returning cast for the same reason. */
void func_002A1058(void) {
    ((s32 (*)(void *, void *, s32))CopyQwords)((void *)0x70003A00, &g_proceduralAnimBounds[0x280], 0x3C0);
}

/* BeginMobyDrawSegment — open the per-frame moby draw segment. Appends the VIF
 * code-ref tag (D_10FFC0 / D_10FFB0), selects VU1 program 6, kicks the VIF0
 * chain (D_100080) and appends the segment's GS reg packet (reg 0x47 = SCISSOR,
 * value 0x5360B). Then it opens the DMA segment: remembers the current
 * g_frameDmaCursor as the open tag, resets the VRAM bump cursor to
 * g_vramDynamicBase, reserves a qword, points the frame-DMA scratch at
 * g_renderTaskWorkBuf-0x10000, seeds the VU-chain cursor from g_renderTaskList,
 * and clears g_deferredSegment2Tag. The matching build keeps the asm; this is
 * the faithful TARGET_NATIVE coverage arm. Task #759 closed all but one
 * residual in an EE arm: the prologue issues `lui a0` before `lhu a1`, where
 * the ROM has them the other way round. The C, its .externs and the variants
 * tried are in FACT #8055. */
#ifdef TARGET_NATIVE
extern u16   D_10FFB0;                  /* VIF code-ref tag qword count         */
extern u8    D_10FFC0[];                /* VIF code-ref tag template            */
extern u8    D_100080[];                /* VIF0 kick chain                      */
extern s32   g_activeVu1Program;        /* 0x1B161C uploaded VU1 microcode id   */
extern void *g_vramDynamicBase;         /* 0x1A72D4 VRAM dynamic region base    */
extern void *g_mobyVuChainCursor;       /* 0x1B1AD8 moby VU/DMA chain cursor    */
extern void  AppendVifCodeRefTag(void *code, u32 count);
extern void  KickVif0Chain(void *chain);
/* GS A+D reg-write: the DATA is a 64-bit register value. Widen it to u64 for the
 * native/#else build (prevents silent truncation of bits >=32); matching-build
 * decl kept verbatim (byte-neutral). */
#ifdef TARGET_NATIVE
extern void  AppendGsRegPacket(s32 reg, u64 data);
#else
extern void  AppendGsRegPacket(s32 reg, u32 data);
#endif
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", BeginMobyDrawSegment);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void AppendGsRegPacket(s32 reg, u64 data);
extern void AppendVifCodeRefTag(void *code, u32 count);
extern void KickVif0Chain(void *chain);
extern u8 D_100080[];
extern u16 D_10FFB0;
extern u8 D_10FFC0[];
extern s32 g_activeVu1Program;
extern s32 g_deferredSegment2Tag;
extern u32 *g_frameDmaCursor;
extern u32 *g_mobySegmentOpenTag;
extern void *g_mobyVuChainCursor;
extern void *g_vramAllocCursor;
extern void *g_vramDynamicBase;
/* (end of this body's declarations) */
void BeginMobyDrawSegment(void) {
    AppendVifCodeRefTag(D_10FFC0, D_10FFB0);
    g_activeVu1Program = 6;
    KickVif0Chain(D_100080);
    AppendGsRegPacket(0x47, 0x5360B);

    g_mobySegmentOpenTag = g_frameDmaCursor;
    g_vramAllocCursor = g_vramDynamicBase;
    g_frameDmaCursor = (u32 *)((u8 *)g_frameDmaCursor + 0x10);
    ((u32 *)&g_frameDmaCursor)[1] = (u32)((u8 *)g_renderTaskWorkBuf - 0x10000);
    g_mobyVuChainCursor = g_renderTaskList;
    g_deferredSegment2Tag = 0;
}
#endif

/* func_002A1138 — build the moby VU1 render chain for one moby table. Appends
 * the segment's GS SCISSOR reg packet (0x47 / 0x5360B), flushes the pending RPC
 * (func_0011AEA0(0)), swaps in the procedural-anim bounds scratch
 * (func_002A1058), then extends the VU chain (BuildMobyVuChain over the current
 * g_mobyVuChainCursor), saves the scratch back (func_002A1028) and rewinds the
 * cursor by one qword. Params: the moby table base and its entry count; no
 * return. */
#ifdef TARGET_NATIVE
extern void *BuildMobyVuChain(void *tableBase, void *cursor, s32 count, s32 flag);
#endif

/* MATCHED on the s136os arm (task #1350): byte-exact solo under SN 2.95.3
 * v1.36 -fopt-stack (verify_match_unit, FACT #8810's method). Closing lever: the
 * declarations in this arm (NOTE #8954: it did not compile solo without them)
 * — nothing else. */
/* GUARD (task #1350): on EE this C is the image's body, compiled alone by the
 * s136os arm (SN 2.95.3 v1.36 -fopt-stack, FACT #8810; row in
 * tools/ee/s136os_functions.txt) and spliced over S136OS_SLOT by
 * tools/ee/s136os_splice.sh. There is no asm fallback: a build that skips the
 * splice drops the function. On native it is plain C, as before. */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_002A1138)
S136OS_SLOT(func_002A1138);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern void AppendGsRegPacket(s32 reg, u64 data);
extern void * BuildMobyVuChain(void *tableBase, void *cursor, s32 count, s32 flag);
extern void *g_mobyVuChainCursor;
/* (end of this body's declarations) */
void func_002A1138(void *tableBase, s32 count) {
    AppendGsRegPacket(0x47, 0x5360B);
    func_0011AEA0(0);
    func_002A1058();
    g_mobyVuChainCursor = BuildMobyVuChain(tableBase, g_mobyVuChainCursor, count, 0);
    func_002A1028();
    g_mobyVuChainCursor = (void *)((u8 *)g_mobyVuChainCursor - 0x10);
}
#endif

/*
 * Closes out the per-frame moby render chain: flush the moby DMA segment, run
 * the deferred sprite/SPR render pipeline, then (if the second deferred segment
 * still has an open tag) close the moby glow segment.
 * MATCHED: the prior "branch-tail-duplication" reading was wrong — the only
 * deltas were (1) g_deferredSegment2Tag being sized as small-data (gp_rel) under
 * -G8, fixed by the size-16 absolute-extern override, and (2) cc1 sibling-calling
 * the conditional CloseMobyGlowSegment, suppressed by the empty-asm guard so the
 * original jal + shared epilogue is reproduced.
 */
extern s32 g_deferredSegment2Tag;
__asm__(".extern g_deferredSegment2Tag, 16");
extern void CloseMobyDmaSegment(void);
extern void RunSprRenderPipeline(void);
extern void CloseMobyGlowSegment(void);

void FinishMobyRenderChain(void) {
    CloseMobyDmaSegment();
    RunSprRenderPipeline();
    if (g_deferredSegment2Tag != 0) {
        CloseMobyGlowSegment();
        __asm__ __volatile__("");
    }
}

/* RenderMobys — per-frame moby render driver. Opens the draw segment
 * (BeginMobyDrawSegment), clears the anim-bounds scratch (func_002A1000), builds
 * the moby VU1 chain over the whole table (BuildMobyVuChain(g_mobyTableBase,
 * cursor, -1, 1)); if the chain overran the frame-DMA budget (the limit at
 * g_frameDmaCursor[+0x4] fell below the write cursor) it logs the "mobys dropped"
 * overflow string, then finishes the chain (FinishMobyRenderChain).
 *
 * Non-obvious (task #671):
 *  - Every global here is loaded absolute (the `.extern` overrides at the top of
 *    this file), yet the store of the new chain cursor is gp-relative: it sits in
 *    a branch delay slot, and g_mobyVuChainCursor's size-12 override is the class
 *    that expands absolute in straight-line code and gp-relative in a slot.
 *  - The budget test is written `cursor > limit` so that cc1 loads the cursor
 *    word before the limit word, in the ROM's order.
 *  - FinishMobyRenderChain is called through a value-returning cast so cc1
 *    keeps jal + epilogue instead of a sibling `j`. */
extern void  BeginMobyDrawSegment(void);
extern void *BuildMobyVuChain(void *tableBase, void *cursor, s32 count, s32 flag);
extern u32  *g_frameDmaCursor;       /* 0x1B2228 per-frame DMA write pointer; +0x4 = budget limit */
extern void *g_mobyVuChainCursor;    /* 0x1B1AD8 moby VU/DMA chain cursor */
extern void *g_mobyTableBase;        /* 0x1B1ADC moby entity array base (stride 0x100) */
extern char  D_1A9E48[];             /* "N mobys dropped" overflow log string */
extern s32   DebugPrintStub(void *msg);

void RenderMobys(void) {
    BeginMobyDrawSegment();
    func_002A1000();
    g_mobyVuChainCursor = BuildMobyVuChain(g_mobyTableBase, g_mobyVuChainCursor, -1, 1);
    if ((s32)g_frameDmaCursor > *(s32 *)((u8 *)&g_frameDmaCursor + 0x4)) {
        DebugPrintStub(D_1A9E48);   /* VU chain budget exceeded — mobys dropped */
    }
    ((s32 (*)(void))FinishMobyRenderChain)();
}

/*
 * Marks a platinum-bolt slot as collected: sets bit 0x80 in
 * g_platinumBoltFlags at offset 0x70 + slot + 16*progress (the current save
 * profile's level/progress index). A slot of 0xFF means "no bolt", a no-op.
 * MATCHED: the prior "commutative-add operand order" reading was wrong — the only
 * delta was the +0x70 offset. The original folds it into the base-address reloc
 * (g_platinumBoltFlags + 0x70, hi/lo), so materialising `&g_platinumBoltFlags[0x70]`
 * as the base pointer (instead of adding 0x70 to the index) reproduces the bytes.
 */
extern s32 g_playerProgress;
extern u8 g_platinumBoltFlags[];

void func_002A1268(s32 slot) {
    if (slot != 0xFF) {
        u8 *base = &g_platinumBoltFlags[0x70];
        base[slot + (g_playerProgress << 4)] |= 0x80;
    }
}

/*
 * func_002A12A0 — pack a 4-part tuple into the moby's 64-bit field at +0x38:
 * the high word holds hi, the low word packs b1 | b2<<8 | b3<<16. Stashes the
 * render/anim parameter word that func_002A12F0 / func_002A1320 read back.
 *
 *   moby       the moby record, viewed as s64[]
 *   hi         the value for bits 32..63
 *   b1,b2,b3   byte lanes for bits 0..7, 8..15, 16..23
 *
 * Non-obvious (task #671): the ROM shifts all three operands first, in place
 * (hi in $a1), then folds the OR chain strictly left to right into $v0. A plain
 * expression gets cc1's own association and register choice (90.62%, the old
 * "commutative-OR canonicalisation, not reachable" wall). Binding the result to
 * $v0 and hi to $a1 as local register variables, plus two empty asm statements
 * that fence the shifts from each other and from the ORs, gives the ROM order.
 * The native build has no MIPS registers and gets plain locals.
 */
void func_002A12A0(s64 *moby, s64 hi, s64 b1, s64 b2, s64 b3) {
#ifndef TARGET_NATIVE
    register s64 packed asm("$2");
    register s64 high asm("$5") = hi;
#else
    s64 packed;
    s64 high = hi;
#endif
    high <<= 32;
    __asm__ __volatile__("");
    b2 <<= 8;
    b3 <<= 16;
    __asm__ __volatile__("");
    packed = high | b1;
    packed |= b2;
    packed |= b3;
    moby[7] = packed;
}

/*
 * func_002A12C0 — write a packed 3-byte tuple into the high word of the moby's
 * 64-bit field at +0x38, keeping the low word. The inverse of func_002A12F0.
 *
 *   moby        the moby record, viewed as u64[]
 *   b0, b1, b2  bytes for bits 32..39, 40..47 and 48..55
 *
 * Non-obvious (task #689; FACT #7957 had this walled on the pad `nop`):
 *  - $v0 holds the field from the `ld` on, and the fence straight after the load
 *    stops cc1 narrowing it to `lwu`, so the low word is kept by the ROM's
 *    `dsll32`/`dsrl32` pair.
 *  - Empty asm statements fence the three argument shifts into ROM order,
 *    ahead of the mask.
 *  - The ROM has a lone `nop` directly before `jr $ra`, with the store in the
 *    delay slot. It is written here as an inline `nop`. cc1 cannot move the
 *    store above that asm, so reorg puts it in the `jr` slot.
 * The native build gets plain locals and no `nop`.
 */
void func_002A12C0(u64 *moby, u64 b0, u64 b1, u64 b2) {
#ifndef TARGET_NATIVE
    register u64 packed asm("$2") = moby[7];
    __asm__ __volatile__("" : "+r"(packed));
#else
    u64 packed = moby[7];
#endif
    b0 <<= 32;
    __asm__ __volatile__("");
    b1 <<= 40;
    __asm__ __volatile__("");
    b2 <<= 48;
    __asm__ __volatile__("");
    packed = (packed << 32) >> 32;
    __asm__ __volatile__("");
    packed |= b0;
    packed |= b1;
    packed |= b2;
#ifndef TARGET_NATIVE
    __asm__ __volatile__("nop");
#endif
    moby[7] = packed;
}

/*
 * func_002A12F0 — unpack the three bytes in the high word of the moby's 64-bit
 * field at +0x38 into three u32 out-params. The inverse of func_002A12C0.
 *
 *   moby        the moby record, viewed as u64[]
 *   out0..out2  receive bits 32..39, 40..47 and 48..55
 *
 * Non-obvious (task #671). The ROM body is register-for-register fixed: one `ld`,
 * three `dsrl32` into $at/$v0/$v1, three `andi`, three `sw`, and a `nop` in the
 * `jr` slot. The old wall note ("byte-loads the first lane and sign-extends
 * each lane") came from a plain C spelling. Reproduced here by:
 *  - local register variables on $a0/$at/$v0/$v1. cc1 accepts $at as a register
 *    variable, and the GNU-as "used $at" warning it causes is harmless;
 *  - a u64 view for the shifts and a (u8) narrowing for the masks. That gives
 *    `dsrl32 + andi` with no dsll32/dsra32 sign-extension, which a u32 shift
 *    would add, and `dsrl32 0` rather than `dsra32 0` for the first lane;
 *  - empty asm statements that fence each instruction into the ROM order;
 *  - volatile out-params: cc1 then declines to move the last store into the
 *    `jr` slot, and the SN assembler left that slot a `nop` (asm_unit.sh's
 *    `.set volatile` rule).
 * The native build gets plain locals.
 */
void func_002A12F0(u64 *moby, volatile u32 *out0, volatile u32 *out1, volatile u32 *out2) {
#ifndef TARGET_NATIVE
    register u64 packed asm("$4") = moby[7];
    register u64 w0 asm("$1");
    register u64 w1 asm("$2");
    register u64 w2 asm("$3");
    register u32 b0 asm("$1");
    register u32 b1 asm("$2");
    register u32 b2 asm("$3");
#else
    u64 packed = moby[7];
    u64 w0, w1, w2;
    u32 b0, b1, b2;
#endif
    w0 = packed >> 32;
    __asm__ __volatile__("");
    w1 = packed >> 40;
    __asm__ __volatile__("");
    w2 = packed >> 48;
    __asm__ __volatile__("");
    b0 = (u8)w0;
    __asm__ __volatile__("");
    b1 = (u8)w1;
    __asm__ __volatile__("");
    b2 = (u8)w2;
    __asm__ __volatile__("");
    *out0 = b0;
    *out1 = b1;
    *out2 = b2;
}

/* func_002A1320(obj, out): resolve the object's packed directional-light field
 * at obj+0x38 (bytes idx0/idx1/blend, written by func_002A12A0/func_002A12C0)
 * into an interpolated light vector in out. Looks up the 0x40-stride
 * g_dirLightMatrices, using the vec4 row at +0x10 of each entry: when blend == 0
 * it copies matrix idx0's row, otherwise it lerps matrix idx0 -> idx1 by
 * t = (blend*16)/4096 on x/y/z only (the w lane stays matrix idx0's, since the
 * VU0 vmadd is .xyz). Handwritten VU0 (lqc2/vitof12/vmaddx); the matching build
 * keeps the asm — this is the faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern u8 g_dirLightMatrices[];   /* 0x1C26C0  0x40-stride dir-light matrices */
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A1320);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern u8 g_dirLightMatrices[];
/* (end of this body's declarations) */
void func_002A1320(void *obj, f32 *out) {
    u64 v = *(u64 *)((u8 *)obj + 0x38);
    u32 idx0  = (u32)v & 0xFF;
    u32 idx1  = ((u32)v >> 8) & 0xFF;
    u32 blend = ((u32)v >> 16) & 0xFF;
    f32 *m0 = (f32 *)(g_dirLightMatrices + idx0 * 0x40 + 0x10);

    if (blend == 0) {
        out[0] = m0[0];
        out[1] = m0[1];
        out[2] = m0[2];
        out[3] = m0[3];
    } else {
        f32 *m1 = (f32 *)(g_dirLightMatrices + idx1 * 0x40 + 0x10);
        f32 t  = (blend << 4) * (1.0f / 4096.0f);
        f32 it = 1.0f - t;
        out[0] = m0[0] * it + m1[0] * t;
        out[1] = m0[1] * it + m1[1] * t;
        out[2] = m0[2] * it + m1[2] * t;
        out[3] = m0[3];   /* w lane unchanged from m0 (vmadd is .xyz only) */
    }
}
#endif

/* func_002A1390(a, b): signed gap between two bounding spheres, scaled by 1/1024
 * — the centre distance |a.xyz - b.xyz| minus the sum of radii (a.w + b.w), times
 * 1/1024. Negative when the spheres overlap. Handwritten VU0 (lqc2/vsqrt/vmulq);
 * the matching build stays asm — this is the faithful TARGET_NATIVE coverage arm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A1390);
#else
f32 func_002A1390(f32 *a, f32 *b) {
    f32 dx = a[0] - b[0];
    f32 dy = a[1] - b[1];
    f32 dz = a[2] - b[2];
    f32 dist = __builtin_sqrtf(dx * dx + dy * dy + dz * dz);
    return (dist - (a[3] + b[3])) * (1.0f / 1024.0f);
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", UpdateMobyAnimation);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", BuildActiveMobyChain);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A1860);

/* ReleaseMobyGridBlockBits(start, count): clear `count` allocation bits starting
 * at bit index `start` in g_mobyGridBlockBitmap (byte start>>3, bit start&7). The
 * original asserts on double-free — clearing a bit that was already 0 executes an
 * unconditional `teq` trap; the #else models that error path as an early stop
 * (unreachable in correct use). Handwritten; the matching build keeps the asm. */
#ifdef TARGET_NATIVE
extern u8 g_mobyGridBlockBitmap[];   /* 0x1EE460 bit-per-block allocation bitmap */
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", ReleaseMobyGridBlockBits);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern u8 g_mobyGridBlockBitmap[];
/* (end of this body's declarations) */
void ReleaseMobyGridBlockBits(s32 start, s32 count) {
    for (;;) {
        u8 *byte = g_mobyGridBlockBitmap + (start >> 3);
        u8 old = *byte;
        u8 cleared = (u8)(old & ~(1 << (start & 7)));
        start++;
        count--;
        if (cleared == old) {
            /* bit already clear — original does an unconditional teq trap
             * (double-free assert); modelled here as a stop. */
            return;
        }
        *byte = cleared;
        if (count <= 0) {
            return;
        }
    }
}
#endif

/* AllocMobyGridBlockBits(width): allocate a free run of `width` consecutive bits
 * in g_mobyGridBlockBitmap and return its global bit index. Scans 32-bit words;
 * within a non-full word it slides an aligned width-bit mask (stepping by width)
 * until the masked bits are all clear — the mask shifting fully out of the low 32
 * bits yields position 0x20, meaning no fit, so it advances to the next word.
 * Sets the run and returns word*0x20 + position. `width` is a power of two
 * dividing 32. Handwritten (dsllv/dsrlv); the matching build keeps the asm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", AllocMobyGridBlockBits);
#else
/* Declarations this body needs whose only other declarations sit in other
 * guarded arms: the s136os arm compiles this arm alone, so it must see them here. */
extern u8 g_mobyGridBlockBitmap[];
/* (end of this body's declarations) */
s32 AllocMobyGridBlockBits(s32 width) {
    u32 *p = (u32 *)g_mobyGridBlockBitmap;
    u32 mask0 = (1u << width) - 1;
    s32 base = -0x20;

    for (;;) {
        u32 word = *p++;
        base += 0x20;
        if (word == 0xFFFFFFFF) {
            continue;   /* full word */
        }
        {
            u64 mask = mask0;
            s32 pos = 0;
            while (word & (u32)mask) {
                mask <<= width;
                pos += width;
            }
            if (pos == 0x20) {
                continue;   /* no run fits in this word */
            }
            p[-1] = word | (u32)mask;
            return base + pos;
        }
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", UpdateMobyGridCells);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", UpdateMobyBSphereAndGrid);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A1F20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A1F68);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A21B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A22C8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A2570);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A3288);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", SkinMobyCollisionMesh);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A4C08);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A4D60);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A4EC8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", UploadMobyTextures);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", RunRenderTaskList);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", BuildMobyVuChain);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A7490);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A75CC);

/* PickLowAmmoWeaponForDrop: choose which owned weapon an ammo pickup should drop
 * for, writing the chosen item slot to *outSlot and returning how many boxes to
 * drop. A weapon is a candidate when it is owned (g_inventoryOwned[i]), has a
 * non-zero ammo capacity (g_weaponTable[g_itemEquippedSlot[i]*0xE0 + 0x8E]) and is
 * not one of the excluded slots. First it counts how many candidates are below
 * capacity (g_weaponAmmo[i] < cap); if any, it picks a random one of those (excl.
 * slots 0x1F/0x3D); otherwise it picks a random valid candidate (excl. slots
 * 0x1F/0x3D/0x2D/0x4D). Returns 2 one time in five, else 1. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", PickLowAmmoWeaponForDrop);
#else
extern u8 g_inventoryOwned[];
extern u8 g_itemEquippedSlot[];
extern u8 g_weaponTable[];
extern s32 g_weaponAmmo[];
extern s32 GetRandomInt(s32 n);
s32 PickLowAmmoWeaponForDrop(s32 arg0, s32 *outSlot) {
    s32 i, lowCount = 0, validCount = 0;
    (void)arg0;

    /* count valid weapons and how many are below ammo capacity */
    for (i = 0; i < 0x38; i++) {
        u16 cap;
        if (g_inventoryOwned[i] == 0) {
            continue;
        }
        cap = *(u16 *)&g_weaponTable[g_itemEquippedSlot[i] * 0xE0 + 0x8E];
        if (cap == 0 || i == 0x1F || i == 0x3D || i == 0x2D || i == 0x4D) {
            continue;
        }
        validCount++;
        if (g_weaponAmmo[i] < cap) {
            lowCount++;
        }
    }

    if (lowCount != 0) {
        /* pick a random below-capacity weapon (excludes slots 0x1F/0x3D) */
        s32 pick = GetRandomInt(lowCount);
        for (i = 0; i < 0x38; i++) {
            u16 cap;
            if (g_inventoryOwned[i] == 0) {
                continue;
            }
            cap = *(u16 *)&g_weaponTable[g_itemEquippedSlot[i] * 0xE0 + 0x8E];
            if (cap == 0 || !(g_weaponAmmo[i] < cap) || i == 0x1F || i == 0x3D) {
                continue;
            }
            if (pick == 0) {
                *outSlot = i;
                break;
            }
            pick--;
        }
    } else {
        /* none below capacity: pick a random valid weapon */
        s32 pick = GetRandomInt(validCount);
        for (i = 0; i < 0x38; i++) {
            u16 cap;
            if (g_inventoryOwned[i] == 0) {
                continue;
            }
            cap = *(u16 *)&g_weaponTable[g_itemEquippedSlot[i] * 0xE0 + 0x8E];
            if (cap == 0 || i == 0x1F || i == 0x3D || i == 0x2D || i == 0x4D) {
                continue;
            }
            if (pick == 0) {
                *outSlot = i;
                break;
            }
            pick--;
        }
    }

    return (GetRandomInt(5) != 0) ? 1 : 2;
}
#endif

/* IncrementBestiaryKillCount: record a defeated enemy in the bestiary. Looks up
 * the moby's enemy class id (+0xAA) in g_bestiaryEntryTable (0x40 entries, stride
 * 0x18, each listing up to four class ids at +0x0/+0x2/+0x4/+0x6); if no entry
 * matches, does nothing. On a match, applies the "misc extras" challenge bonus
 * (when g_miscExtras is set, D_1A9E70 == -1, and the D_1A7A3A progress counter is
 * below 0x14): advances D_1A7A3B and, once it reaches half of D_1A7A3A, awards
 * func_0029C488(0xB4) and steps D_1A7A3A. Finally bumps the matched entry's u16[2]
 * defeat counter in g_bestiaryKillCounts (stride 4): killType 0 -> +0x0, killType 1
 * -> +0x2, capped at 0x270F. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", IncrementBestiaryKillCount);
#else
extern u8 g_bestiaryEntryTable[];
extern u8 g_bestiaryKillCounts[];
extern u8 g_miscExtras;
extern s32 D_1A9E70;
extern u8 D_1A7A3A, D_1A7A3B;
extern void func_0029C488(s32 arg);
void IncrementBestiaryKillCount(Moby *moby, s32 killType) {
    s16 classId = *(s16 *)((char *)moby + 0xAA);
    s32 found = -1;
    s32 i;

    for (i = 0; i < 0x40; i++) {
        s16 *entry = (s16 *)(g_bestiaryEntryTable + i * 0x18);
        if (entry[0] == classId || entry[1] == classId ||
            entry[2] == classId || entry[3] == classId) {
            found = i;
            break;
        }
    }

    if (found == -1) {
        return;
    }

    /* misc-extras challenge bonus */
    if (g_miscExtras != 0 && D_1A9E70 == -1 && D_1A7A3A < 0x14) {
        D_1A7A3B = D_1A7A3B + 1;
        if (((D_1A7A3B & 0xFF) << 1) >= D_1A7A3A) {
            func_0029C488(0xB4);
            D_1A7A3B = 0;
            D_1A7A3A = D_1A7A3A + 1;
        }
    }

    /* bump the per-entry defeat counter (u16[2], capped at 0x270F) */
    if (killType == 0) {
        u16 *c = (u16 *)(g_bestiaryKillCounts + found * 4);
        if (*c < 0x270F) {
            *c = *c + 1;
        }
    } else if (killType == 1) {
        u16 *c = (u16 *)(g_bestiaryKillCounts + found * 4 + 2);
        if (*c < 0x270F) {
            *c = *c + 1;
        }
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A7AA8);
