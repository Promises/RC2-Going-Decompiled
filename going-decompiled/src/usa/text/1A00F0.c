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
 * Releases a moby slot: marks its state byte (+0x20) 0xFD for a static slot
 * (below the dynamic-spawn region) or 0xFE for a dynamic slot, schedules its
 * release time (+0xA0) two frames out, then removes it from the spatial grid by
 * re-bucketing with the 0x80807F7F "off-grid" sentinel range.
 * WALL (~57%): with the empty-asm guard restoring the jal+frame, the body still
 * misses — the original lowers `(moby < start) ? 0xFD : 0xFE` as a two-way
 * branch (`b`-skip into $v0 with two `addiu`s), while this cc1 if-converts it to
 * a default-then-conditional-override in $a0. A fixed if-conversion / branch-
 * shape difference, not reachable by source form. Left INCLUDE_ASM.
 */
extern void *g_mobySpawnStart;
extern s32 g_gameTime;
extern void UpdateMobyGridCells(void *moby, u32 sentinelRange);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", FreeMoby);
#else
void FreeMoby(Moby *moby) {
    u8 state;
    if ((u32)moby < (u32)g_mobySpawnStart) {
        state = 0xFD;
    } else {
        state = 0xFE;
    }
    *(u8 *)((u8 *)moby + 0x20) = state;
    *(s32 *)((u8 *)moby + 0xA0) = g_gameTime + 2;
    UpdateMobyGridCells(moby, 0x80807F7F);
}
#endif

/*
 * ResolveMobyAnimFramePtrs(moby): resolve the moby's primary + secondary anim
 * frame-data pointers from its animation set table. The table lives at
 * `animBase(moby+0x24) + 0x48` and is indexed by a 1-byte frame id; each entry
 * points at an "anim set" record. From the PRIMARY frame (moby+0x42, sub-index
 * moby+0x40) it caches the frame-data pointer (set[+idx*4+0x1C]) at moby+0x58 and
 * two set bytes at moby+0x6E (set[0x12]) and moby+0x6C (set[0x11]). When the
 * primary frame id is the 0xFF sentinel ("procedural"), it instead points moby+0x58
 * straight into the procedural frame pool (g_proceduralAnimFrames + idx*0x800),
 * stamps moby+0x6C=0xFF and moby+0x6E=0. The SECONDARY frame (moby+0x43, sub-index
 * moby+0x41) always caches its frame-data pointer at moby+0x5C.
 *
 * Leaf, no callee-saves -> not blocked by this unit's save-slot wall. MATCH-FIRST
 * PROBED on the R5900 toolchain (2026-06-29): best 63.60% (cached-local 57.72% ->
 * inline no-cache 63.60%). WALLED - this cc1's instruction scheduling + the repeated
 * reload of table[frame]/moby[0x42] don't reproduce from source form. Kept as the
 * TARGET_NATIVE #else arm, cmp-oracle-ready.
 */
extern u8 g_proceduralAnimFrames[];

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", ResolveMobyAnimFramePtrs);
#else
void ResolveMobyAnimFramePtrs(Moby *moby) {
    u8 *m = (u8 *)moby;
    void **table = (void **)(*(u8 **)(m + 0x24) + 0x48);
    u8 frame = m[0x42];

    if (frame != 0xFF) {
        u8 *set = (u8 *)table[frame];
        *(void **)(m + 0x58) = *(void **)(set + m[0x40] * 4 + 0x1C);
        m[0x6E] = set[0x12];
        m[0x6C] = set[0x11];
    } else {
        m[0x6C] = frame;          /* 0xFF sentinel */
        m[0x6E] = 0;
        *(void **)(m + 0x58) = g_proceduralAnimFrames + (m[0x40] << 11);
    }

    {
        u8 *set2 = (u8 *)table[m[0x43]];
        *(void **)(m + 0x5C) = *(void **)(set2 + m[0x41] * 4 + 0x1C);
    }
}
#endif

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0360);

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
 * Save-wall blocked for matching (saves $16+$31 at 8-byte spacing under the later
 * cc1 model); provided as the TARGET_NATIVE #else arm, cmp-oracle-ready.
 */
extern s32 Log2Floor(s32 value);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0480);
#else
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
#endif

/* func_002A04D8 — pose and average two collision-mesh keyframe vectors for a
 * moby into dst. Locates the class-header entry (header at obj+0x24, base index
 * at header+0x2C, plus arg1) and takes hi = max of the two frame counts at
 * entry+0x6 / entry+0xE. When hi >= 0 it skins the collision mesh
 * (SkinMobyCollisionMesh) into the SPR cache. It then loads the two packed
 * keyframe vectors (func_00283AE0 -> vecA/vecB, w set to 1), optionally samples
 * the per-frame scratchpad vectors at 0x70000000 + count*64 (func_00283A70),
 * scales each by (obj+0x2C)/1024, applies the moby's 3x3 rotation (func_00283A48
 * over obj+0xC0) and translation (Vec4AddVu0 obj+0x10), averages the two into dst
 * (add then *0.5), and stores the func_002837F8 scalar into dst.w. Callee roles
 * func_00283A48/func_00283AE0/func_002837F8 UNCONFIRMED (named by shape); helper
 * signatures cross-referenced to text/183558.c (scale families take f32 2nd).
 * The matching build keeps the asm; faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern void Vec4AddVu0(void *dst, void *a, void *b);
extern void Vec4ScaleVu0(void *dst, f32 s, void *src);
extern void func_00283A48(void *out, void *v, void *m);
extern void func_00283A70(void *out, void *v, void *m);
extern void func_00283AE0(void *dst, u64 packed);
extern f32  func_002837F8(void *a, void *b);
extern void SkinMobyCollisionMesh(void *entry, s32 count, u32 flags);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A04D8);
#else
void func_002A04D8(void *obj, s32 arg1, void *dst) {
    u8 *o = (u8 *)obj;
    u8 *hdr = *(u8 **)(o + 0x24);
    u8 *entry = hdr + *(u8 *)(hdr + 0x2C) * 16 + arg1 * 16 + 0x40;
    s16 a = *(s16 *)(entry + 0x6);
    s16 b = *(s16 *)(entry + 0xE);
    s16 hi = (a < b) ? b : a;
    f32 vecA[4];
    f32 vecB[4];

    if (hi >= 0) {
        SkinMobyCollisionMesh(entry, hi + 1, 0x80000000);
    }
    func_00283AE0(vecA, *(u64 *)(entry + 0));
    func_00283AE0(vecB, *(u64 *)(entry + 8));
    vecB[3] = 1.0f;
    vecA[3] = 1.0f;
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
    *(f32 *)((u8 *)dst + 0xC) = func_002837F8(dst, vecA);
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
 * The matching build keeps the asm; faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
/* ScaleVec4IncludingW def site is text/183558.c: the f32 scale is the 2nd param
 * (dst, s, src), NOT trailing. Vec4AddVu0/Vec4ScaleVu0/func_00283A48/func_00283AE0
 * are already declared with func_002A04D8 above. */
extern void ScaleVec4IncludingW(void *dst, f32 s, void *src);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0678);
#else
s32 func_002A0678(void *obj, void *dst, void *outArray, s32 arg3) {
    u8 *hdr = *(u8 **)((u8 *)obj + 0x24);
    s16 n = *(s16 *)(hdr + 0x2E);
    u8 *entry;
    u8 *sub;
    u8 *p;
    f32 scale;
    s16 count;
    u32 idx;

    if (n == 0) {
        return 0;
    }
    entry = hdr + n * 16;
    p = entry + arg3;
    if (*(u8 *)(p + 1) == 0xFF) {
        return 0;
    }
    idx = *(u8 *)(p + 1);
    scale = *(f32 *)((u8 *)obj + 0x2C) * (1.0f / 1024.0f);
    sub = entry + idx * 16;
    ScaleVec4IncludingW(dst, scale, sub);
    sub += 0x10;
    func_00283A48(dst, dst, (u8 *)obj + 0xC0);
    Vec4AddVu0(dst, dst, (u8 *)obj + 0x10);

    count = *(s16 *)(sub + 6);
    if (outArray != 0 && count > 0) {
        u8 *loopDst = (u8 *)outArray;
        s16 i = count;
        do {
            func_00283AE0(loopDst, *(u64 *)sub);
            sub += 8;
            Vec4ScaleVu0(loopDst,
                         *(f32 *)((u8 *)obj + 0x2C) * (1.0f / 1024.0f), loopDst);
            *(f32 *)(loopDst + 0xC) = 1.0f;
            i--;
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
 * WALL (~15%): all instructions reproduce but this cc1 software-pipelines the
 * descriptor-table loads up early and reschedules the four 1.0f swc1 stores
 * (emits +0x24 first), while the original keeps the float block grouped and the
 * pointer chain at the tail. A fixed instruction-scheduling artifact, not
 * reachable by source statement order. Left INCLUDE_ASM.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A07B0);
#else
void func_002A07B0(u8 *moby, u8 sub, u8 *rec) {
    u8 *p;
    s32 idx;
    if (rec[1] == 0) {
        rec[0] = sub;
        rec[1] = 1;
        *(float *)(rec + 0x28) = 1.0f;
        *(float *)(rec + 0x1C) = 1.0f;
        *(float *)(rec + 0x20) = 1.0f;
        *(float *)(rec + 0x24) = 1.0f;
        {
            u8 *tbl = *(u8 **)(moby + 0x24);
            u8 *base = *(u8 **)(tbl + 0x1C);
            p = *(u8 **)(base + (rec[0] << 2) + 4);
        }
        idx = p[0];
        *(u32 *)(rec + 4) = ((p + idx)[4] << 6) + 0x70000000;
        *(u32 *)(rec + 8) = *(u32 *)(moby + 0x54);
        *(u8 **)(moby + 0x54) = rec;
    }
}
#endif

/*
 * func_002A0828(list, node): unlink `node` from `list`'s singly-linked free/active
 * chain (head at list+0x54, nodes linked through +0x8) and then wipe the removed
 * node with FillMemory32(node, 0, 0x40). A null `node` is a no-op. Walks from the
 * head to find `node`'s predecessor, splices it out (prev->next = node->next or
 * head = node->next when it was first), then zeroes the node's 0x40-byte record.
 * WALL: the original lowers the search loop and the head-vs-body tests as a chain
 * of branch-LIKELY forms (bne/beql/bnel) that reload the head from memory and put
 * the `prev = prev->next` advance in nullified delay slots, and tail-calls
 * FillMemory32 sharing the epilogue; this cc1 produces a plain-branch loop shape
 * with extra reloads and a different beql/bnel placement (~41%). A fixed
 * branch-likely / loop-scheduling artifact, not reachable by source form.
 * Left INCLUDE_ASM.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0828);
#else
struct A0828Node { u8 _pad[8]; struct A0828Node *next; };
struct A0828List { u8 _pad[0x54]; struct A0828Node *head; };
void func_002A0828(struct A0828List *list, struct A0828Node *node) {
    if (node != 0) {
        if (list->head == node) {
            list->head = node->next;
        } else {
            struct A0828Node *prev = list->head;
            if (prev != 0) {
                struct A0828Node *cur = prev->next;
                if (cur != 0) {
                    while (cur != node) {
                        prev = cur;
                        cur = prev->next;
                        if (cur == 0) {
                            break;
                        }
                    }
                    if (cur == node) {
                        prev->next = node->next;
                    }
                }
            }
        }
        FillMemory32(node, 0, 0x40);
    }
}
#endif

/*
 * AcquireProceduralAnimSlot (func_002A08C0): first-fit/reuse a procedural-anim-frame
 * slot for a moby. Scans g_proceduralAnimSlotOwners[0..0xF]; reuses the slot already
 * owned by `moby`, or claims the first free (==0) slot, storing `moby` as the owner
 * and resetting that slot's frame counter (g_proceduralAnimSlotTimer); returns the
 * slot index, or -1 if all 16 slots are taken by other mobys.
 * WALL (~67%): the original lowers the two slot tests (==0 and ==moby) as a pair of
 * branch-LIKELY forms (beqzl/bnel) that put the `sw moby` store and the `slot++`
 * increment in the (nullified) delay slots; this cc1 always emits a plain bnez
 * skip-forward with a separate beql tail and never the branch-likely store pattern.
 * A fixed branch-shape / branch-likely lowering, not reachable by source form.
 * Left INCLUDE_ASM.
 */
extern u32 g_proceduralAnimSlotOwners[];
extern u32 g_proceduralAnimSlotTimer[];

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A08C0);
#else
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
#endif

/* Clears the two 16-word moby spawn-credit sub-tables at g_mobySpawnCredit
 * +0x40 and +0x80 (e.g. on level reset).
 * WALL (~77%): the original materialises both loop base addresses independently
 * (two lui/addiu pairs) and fills the branch delay with the second pointer
 * increment; cc1 strength-reduces the second base to `addu b,a,64` and schedules
 * the loop body differently. Reduced-strength + loop-scheduling. Left INCLUDE_ASM. */
extern u32 g_mobySpawnCredit[];

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0918);
#else
void func_002A0918(void) {
    u32 *a = &g_mobySpawnCredit[0x10];
    u32 *b = &g_mobySpawnCredit[0x20];
    s32 i = 0xF;
    do {
        *a++ = 0;
        *b++ = 0;
        i--;
    } while (i >= 0);
}
#endif

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
 * text/183558.c. Callee func_002A4D60 UNCONFIRMED (named by shape). The matching
 * build keeps the asm; faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern void func_002A4D60(void *obj, s32 flag, void *inParams, void *outBuf);
extern void func_00284008(void *dst, void *src);
extern void MatrixMultiplyVu0(void *dst, void *a, void *b);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0A58);
#else
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
 * obj+0x10) are applied. Callee func_002A4D60 UNCONFIRMED (named by shape). The
 * matching build keeps the asm; faithful TARGET_NATIVE coverage arm. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0AF8);
#else
void func_002A0AF8(void *obj, s32 arg1, void *dst) {
    u8 *o = (u8 *)obj;
    u8 buf[0x80];
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
 * (Vec4AddVu0 obj+0x10). Callee func_002A4C08 UNCONFIRMED (named by shape). The
 * matching build keeps the asm; faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern void func_002A4C08(void *obj, s32 count, void *arg2, void *dst);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0B80);
#else
void func_002A0B80(void *obj, s32 count, void *arg2, void *dst) {
    u8 *o = (u8 *)obj;
    u8 *p = (u8 *)dst;
    f32 scale = *(f32 *)(o + 0x2C) * (1.0f / 1024.0f);
    s32 i;

    func_002A4C08(obj, count, arg2, dst);
    for (i = count; i > 0; i--) {
        Vec4ScaleVu0(p, scale, p);
        func_00283A48(p, p, o + 0xC0);
        Vec4AddVu0(p, p, o + 0x10);
        p += 0x10;
    }
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0C20);

/* CloseMobyDmaSegment — close the moby texture-upload DMA segment. Reserves a
 * DMATAG qword at g_frameDmaCursor and back-patches the segment's open tag
 * (g_mobySegmentOpenTag) into a CNT tag chaining to it, uploads the moby
 * textures (UploadMobyTextures over g_vramAllocCursor) + appends the default
 * TEX0 flush, then lays two more CNT DMATAG qwords closing the chain. DMATAGs
 * are 4 words (0x10 bytes); 0x20000000 = CNT. g_frameDmaCursor is re-read after
 * the upload/flush calls (they append through it). The matching build keeps the
 * asm; this is the faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern u32  *g_frameDmaCursor;         /* 0x1B2228 per-frame DMA write pointer */
extern u32  *g_mobySegmentOpenTag;     /* 0x1B1AD0 moby draw-segment open tag  */
extern void *g_vramAllocCursor;        /* 0x1A72D0 VRAM bump cursor            */
extern void  UploadMobyTextures(void *vramCursor);
extern void  AppendTexFlushDefaultTex0(void);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", CloseMobyDmaSegment);
#else
void CloseMobyDmaSegment(void) {
    u32 *start = g_frameDmaCursor;

    g_frameDmaCursor += 4;
    g_mobySegmentOpenTag[0] = 0x20000000;
    g_mobySegmentOpenTag[1] = (u32)g_frameDmaCursor;
    g_mobySegmentOpenTag[2] = 0;
    g_mobySegmentOpenTag[3] = 0;

    UploadMobyTextures(g_vramAllocCursor);
    AppendTexFlushDefaultTex0();

    g_frameDmaCursor[0] = 0x20000000;
    g_frameDmaCursor[1] = (u32)(g_mobySegmentOpenTag + 4);
    g_frameDmaCursor[2] = 0;
    g_frameDmaCursor[3] = 0;
    g_frameDmaCursor += 4;

    start[0] = 0x20000000;
    start[3] = 0;
    start[1] = (u32)g_frameDmaCursor;
    start[2] = 0;
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
 * stride 0x40). The matching build keeps the asm; this is the faithful
 * TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern u32   g_mobyClassDataSizes[];   /* 0x1D0D80  &[0xF0] = present-slot list */
extern void *g_mobyClassHeaders[];     /* 0x1CDB00  class header ptr per slot   */
extern s16   g_mobyTexVramTable[];     /* 0x1D0980  2 s16 VRAM fields per tex    */
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", PatchMobyPacketTex0);
#else
void PatchMobyPacketTex0(void) {
    s32 *classSlot = (s32 *)&g_mobyClassDataSizes[0xF0];

    if (*classSlot < 0) {
        return;
    }
    for (;;) {
        u8 *header = (u8 *)g_mobyClassHeaders[*classSlot];
        u8 *node = *(u8 **)(header + 0x20);

        for (;;) {
            s32 nodeLink = *(s32 *)(node + 0xC);

            if (*node != 0xFF) {
                u8 *packet = (u8 *)(nodeLink & 0x7FFFFFFF);
                u8 *cursor = node;

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
            node += 0x10;
            if (nodeLink < 0) {
                break;
            }
        }
        classSlot++;
        if (*classSlot < 0) {
            break;
        }
    }
}
#endif

/* func_002A0DF0 — recompute the moby glow segment's 2D light direction from the
 * hero. func_002A1320 fills a 2-float vector from g_pHeroMoby; func_00283BF8
 * turns it into an angle, and the sin/cos-style pair func_00283B30 /
 * func_00283B48 is scaled by 0.14 into the glow parameter block at
 * g_deferredSegment2Tag+0x10 (cos) / +0x14 (sin), with a fixed -0.99 at +0x18.
 * Callee roles UNCONFIRMED (named by shape). The matching build keeps the asm;
 * this is the faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern s32   g_deferredSegment2Tag;
extern void *g_pHeroMoby;              /* 0x18C0B0 hero (Ratchet) moby         */
extern void  func_002A1320(void *moby, f32 *outVec);
extern f32   func_00283BF8(f32 a, f32 b);
extern f32   func_00283B30(f32 x);
extern f32   func_00283B48(f32 x);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0DF0);
#else
void func_002A0DF0(void) {
    f32 vec[2];
    f32 angle;
    f32 *glow = (f32 *)((u8 *)&g_deferredSegment2Tag + 0x10);

    func_002A1320(g_pHeroMoby, vec);
    angle = func_00283BF8(vec[0], vec[1]);
    glow[0] = func_00283B30(angle) * 0.14f;
    glow[2] = -0.99f;
    glow[1] = func_00283B48(angle) * 0.14f;
}
#endif

/* CloseMobyGlowSegment — close the deferred moby-glow draw segment. If no glows
 * were queued this frame (g_mobyGlowCount == 0) it writes just an END DMATAG
 * (0x10000000) into the segment tag at *g_deferredSegment2Tag. Otherwise it
 * reserves a CNT qword, builds the glow records (BuildMobyGlowRecords) and emits
 * their packets (EmitMobyGlowPackets over g_mobyGlowWorkBuf), then closes the
 * chain with two more CNT DMATAGs. g_deferredSegment2Tag holds the segment-tag
 * build pointer; g_frameDmaCursor is re-read after the calls. The matching build
 * keeps the asm; this is the faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern s32  g_mobyGlowCount;           /* 0x1B1AFC glow records queued         */
extern u8   g_mobyGlowWorkBuf[];       /* 0x1EF260 glow record work buffer     */
extern void BuildMobyGlowRecords(void);
extern void EmitMobyGlowPackets(void *workBuf);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", CloseMobyGlowSegment);
#else
void CloseMobyGlowSegment(void) {
    u32 *tag;

    if (g_mobyGlowCount == 0) {
        tag = (u32 *)g_deferredSegment2Tag;
        tag[0] = 0x10000000;
        tag[1] = 0;
        tag[2] = 0;
        tag[3] = 0;
    } else {
        u32 *start = g_frameDmaCursor;

        g_frameDmaCursor += 4;
        tag = (u32 *)g_deferredSegment2Tag;
        tag[0] = 0x20000000;
        tag[1] = (u32)g_frameDmaCursor;
        tag[2] = 0;
        tag[3] = 0;

        BuildMobyGlowRecords();
        EmitMobyGlowPackets(g_mobyGlowWorkBuf);

        g_frameDmaCursor[0] = 0x20000000;
        g_frameDmaCursor[1] = (u32)((u32 *)g_deferredSegment2Tag + 4);
        g_frameDmaCursor[2] = 0;
        g_frameDmaCursor[3] = 0;
        g_frameDmaCursor += 4;

        start[0] = 0x20000000;
        start[3] = 0;
        start[1] = (u32)g_frameDmaCursor;
        start[2] = 0;
    }
}
#endif

/* RunSprRenderPipeline — kick the sprite/moby render pass. Flushes any pending
 * RPC (func_0011AEA0(0)), stages the 0x800-byte DMA/GIF template (D_238E80) into
 * the render scratchpad at 0x70003800 via CopyQwords, then runs the frame's
 * render task list (RunRenderTaskList over g_renderTaskList / g_renderTaskWorkBuf).
 * The matching build keeps the asm (engine save-layout wall). */
#ifdef TARGET_NATIVE
extern void  func_0011AEA0(s32 arg);
extern void  CopyQwords(void *dst, void *src, s32 len);
extern void  RunRenderTaskList(void *taskList, void *workBuf);
extern u8    D_238E80[];              /* 0x238E80  0x800-byte SPR render template */
extern void *g_renderTaskList;        /* 0x1B1630 */
extern void *g_renderTaskWorkBuf;     /* 0x1B1634 */
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", RunSprRenderPipeline);
#else
void RunSprRenderPipeline(void) {
    func_0011AEA0(0);
    CopyQwords((void *)0x70003800, D_238E80, 0x800);
    RunRenderTaskList(g_renderTaskList, g_renderTaskWorkBuf);
}
#endif

/* Clears 0x3C0 bytes of the moby scratchpad block at 0x70003A00 to 0x40000000.
 * The empty-asm guard suppresses cc1's sibling-call (tail-jump) so the original
 * jal + frame is reproduced. */
void func_002A1000(void) {
    FillMemory32((void *)0x70003A00, 0x40000000, 0x3C0);
    __asm__ __volatile__("");
}

/* Saves the procedural-anim bounds scratch (0x3C0 bytes at 0x70003A00) back to
 * g_proceduralAnimBounds[0x280] via CopyQwords.
 * WALL (81.82%): the empty-asm guard restores the jal+frame, but cc1 schedules
 * the `sd $ra` one slot later than the original (which interleaves it between
 * the two address `lui`s). A fixed prologue-scheduling artifact; left INCLUDE_ASM. */
extern u8 g_proceduralAnimBounds[];
extern void CopyQwords(void *dst, void *src, s32 len);

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A1028);
#else
void func_002A1028(void) {
    CopyQwords(&g_proceduralAnimBounds[0x280], (void *)0x70003A00, 0x3C0);
}
#endif

/* Loads the procedural-anim bounds (g_proceduralAnimBounds[0x280]) into the
 * scratch block at 0x70003A00 via CopyQwords.
 * WALL (81.82%): same prologue-scheduling artifact as func_002A1028 (cc1 sinks
 * the `sd $ra` one slot past the original interleave). Left INCLUDE_ASM. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A1058);
#else
void func_002A1058(void) {
    CopyQwords((void *)0x70003A00, &g_proceduralAnimBounds[0x280], 0x3C0);
}
#endif

/* BeginMobyDrawSegment — open the per-frame moby draw segment. Appends the VIF
 * code-ref tag (D_10FFC0 / D_10FFB0), selects VU1 program 6, kicks the VIF0
 * chain (D_100080) and appends the segment's GS reg packet (reg 0x47 = SCISSOR,
 * value 0x5360B). Then it opens the DMA segment: remembers the current
 * g_frameDmaCursor as the open tag, resets the VRAM bump cursor to
 * g_vramDynamicBase, reserves a qword, points the frame-DMA scratch at
 * g_renderTaskWorkBuf-0x10000, seeds the VU-chain cursor from g_renderTaskList,
 * and clears g_deferredSegment2Tag. The matching build keeps the asm; this is
 * the faithful TARGET_NATIVE coverage arm. */
#ifdef TARGET_NATIVE
extern u16   D_10FFB0;                  /* VIF code-ref tag qword count         */
extern u8    D_10FFC0[];                /* VIF code-ref tag template            */
extern u8    D_100080[];                /* VIF0 kick chain                      */
extern s32   g_activeVu1Program;        /* 0x1B161C uploaded VU1 microcode id   */
extern void *g_vramDynamicBase;         /* 0x1A72D4 VRAM dynamic region base    */
extern void *g_mobyVuChainCursor;       /* 0x1B1AD8 moby VU/DMA chain cursor    */
extern void  AppendVifCodeRefTag(void *code, u32 count);
extern void  KickVif0Chain(void *chain);
extern void  AppendGsRegPacket(s32 reg, u32 data);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", BeginMobyDrawSegment);
#else
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
 * cursor by one qword. The matching build keeps the asm; faithful TARGET_NATIVE
 * coverage arm. */
#ifdef TARGET_NATIVE
extern void *BuildMobyVuChain(void *tableBase, void *cursor, s32 count, s32 flag);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A1138);
#else
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
 * overflow string, then finishes the chain (FinishMobyRenderChain). Faithful
 * TARGET_NATIVE coverage arm; the matching build keeps the asm. */
#ifdef TARGET_NATIVE
extern void *g_mobyTableBase;        /* 0x1B1ADC moby entity array base (stride 0x100) */
extern char  D_1A9E48[];             /* "N mobys dropped" overflow log string */
extern s32   DebugPrintStub(void *msg);
#endif

#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", RenderMobys);
#else
void RenderMobys(void) {
    BeginMobyDrawSegment();
    func_002A1000();
    g_mobyVuChainCursor = BuildMobyVuChain(g_mobyTableBase, g_mobyVuChainCursor, -1, 1);
    if (*(s32 *)((u8 *)&g_frameDmaCursor + 0x4) < (s32)g_frameDmaCursor) {
        DebugPrintStub(D_1A9E48);   /* VU chain budget exceeded — mobys dropped */
    }
    FinishMobyRenderChain();
}
#endif

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
 * Packs a 4-byte tuple (hi,b1,b2,b3) into the 64-bit field at +0x38 of a moby:
 * the high 32 bits hold hi, the low 32 bits pack b1 | b2<<8 | b3<<16. Used to
 * stash a render/anim parameter word into the moby record.
 * WALL (90.62%): the shifts + sd all reproduce, but cc1 associates the OR tree
 * differently from the original — original folds `(hi|b1) | b2<<8 | b3<<16` left
 * to right into $v0, cc1 builds `(b2<<8 | b1)` as a sub-tree first. A fixed
 * commutative-OR canonicalisation, not reachable by reassociation. INCLUDE_ASM.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A12A0);
#else
void func_002A12A0(s64 *moby, s64 hi, s64 b1, s64 b2, s64 b3) {
    moby[7] = (((hi << 32) | b1) | (b2 << 8)) | (b3 << 16);
}
#endif

/*
 * func_002A12C0: writes the high 32 bits of the moby's 64-bit field at +0x38 as a
 * packed 3-byte tuple (b0 in bits 32..39, b1 in 40..47, b2 in 48..55), preserving
 * the existing low 32 bits. Inverse of func_002A12F0 which reads the three bytes
 * back out.
 * WALL (~83%): the original masks the low half with `ld` + `dsll32 0/dsrl32 0` and
 * folds the OR tree strictly left-to-right (low|a1|a2|a3); this cc1 lowers the
 * low-half mask to a `lwu` word-load and reassociates the OR tree (building an
 * a2|a1 sub-tree, a3<<16 first) — the same commutative-OR canonicalisation +
 * 64-bit narrowing artifact seen in func_002A12A0/func_002A12F0. Left INCLUDE_ASM.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A12C0);
#else
void func_002A12C0(u64 *moby, u64 b0, u64 b1, u64 b2) {
    u64 lo = (u32)moby[7];
    moby[7] = lo | (b0 << 32) | (b1 << 40) | (b2 << 48);
}
#endif

/*
 * Unpacks 3 bytes out of the high half of the +0x38 field of a moby into three
 * separate s32 out-params (inverse of the high-half writer func_002A12C0).
 * WALL: the original loads the full u64 (`ld`) and extracts via 3 independent
 * `dsrl32` + `andi` with NO sign-extension; this cc1 byte-loads the first lane
 * (`lbu +0x3C`) and sign-extends each `(v>>n)&0xFF` result (dsll32/dsra32)
 * before the `sw`. Its 64-bit narrowing/sub-word lowering differs from the
 * later SN cc1 that built the original. Left INCLUDE_ASM.
 */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A12F0);
#else
void func_002A12F0(u64 *moby, u32 *out0, u32 *out1, u32 *out2) {
    u64 packed = moby[7];
    *out0 = (packed >> 32) & 0xFF;
    *out1 = (packed >> 40) & 0xFF;
    *out2 = (packed >> 48) & 0xFF;
}
#endif

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", ReleaseMobyGridBlockBits);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", AllocMobyGridBlockBits);

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
