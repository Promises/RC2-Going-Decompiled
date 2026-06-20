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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", ResolveMobyAnimFramePtrs);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", UpdateMobyAnimLoopSound);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0360);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0368);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0460);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0480);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A04D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0678);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0958);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0A58);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0AF8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0B80);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0C20);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", CloseMobyDmaSegment);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", PatchMobyPacketTex0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0DF0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", CloseMobyGlowSegment);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", RunSprRenderPipeline);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", BeginMobyDrawSegment);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A1138);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", RenderMobys);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A1320);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A1390);

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

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", PickLowAmmoWeaponForDrop);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", IncrementBestiaryKillCount);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A7AA8);
