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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", FreeMoby);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", ResolveMobyAnimFramePtrs);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", UpdateMobyAnimLoopSound);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0360);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0368);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0460);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0480);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A04D8);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0678);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0798);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A07B0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0828);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A08C0);

/* Clears the two 16-word moby spawn-credit sub-tables at g_mobySpawnCredit
 * +0x40 and +0x80 (e.g. on level reset).
 * WALL (~77%): the original materialises both loop base addresses independently
 * (two lui/addiu pairs) and fills the branch delay with the second pointer
 * increment; cc1 strength-reduces the second base to `addu b,a,64` and schedules
 * the loop body differently. Reduced-strength + loop-scheduling. Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A0918);

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
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A1028);

/* Loads the procedural-anim bounds (g_proceduralAnimBounds[0x280]) into the
 * scratch block at 0x70003A00 via CopyQwords.
 * WALL (81.82%): same prologue-scheduling artifact as func_002A1028 (cc1 sinks
 * the `sd $ra` one slot past the original interleave). Left INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A1058);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", BeginMobyDrawSegment);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A1138);

/*
 * Closes out the per-frame moby render chain: flush the moby DMA segment, run
 * the deferred sprite/SPR render pipeline, then (if the second deferred segment
 * still has an open tag) close the moby glow segment.
 * WALL (92.67%): the original emits a per-branch epilogue (duplicate
 * `ld $ra; jr $ra` after the conditional CloseMobyGlowSegment jal, with a nop
 * in the jal delay slot); the pinned cc1 shares one epilogue between both
 * paths and sinks `ld $ra` into the jal delay slot (5 fewer instructions).
 * A structural branch-tail-duplication codegen difference. Left INCLUDE_ASM.
 */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", FinishMobyRenderChain);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", RenderMobys);

/*
 * Marks a platinum-bolt slot as collected: sets bit 0x80 in
 * g_platinumBoltFlags at offset 0x70 + slot + 16*progress (the current save
 * profile's level/progress index). A slot of 0xFF means "no bolt", a no-op.
 * WALL (99.29%): the final index add is `addu $2,$3,$2` (slot + product) in
 * the original but cc1 canonicalises the commutative add to `addu $2,$2,$3`
 * (product + slot) regardless of source operand order or reassociation — a
 * fixed operand-ordering artifact. Left INCLUDE_ASM.
 */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A1268);

/*
 * Packs a 4-byte tuple (hi,b1,b2,b3) into the 64-bit field at +0x38 of a moby:
 * the high 32 bits hold hi, the low 32 bits pack b1 | b2<<8 | b3<<16. Used to
 * stash a render/anim parameter word into the moby record.
 * WALL (90.62%): the shifts + sd all reproduce, but cc1 associates the OR tree
 * differently from the original — original folds `(hi|b1) | b2<<8 | b3<<16` left
 * to right into $v0, cc1 builds `(b2<<8 | b1)` as a sub-tree first. A fixed
 * commutative-OR canonicalisation, not reachable by reassociation. INCLUDE_ASM.
 */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A12A0);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A12C0);

/*
 * Unpacks 3 bytes out of the high half of the +0x38 field of a moby into three
 * separate s32 out-params (inverse of the high-half writer func_002A12C0).
 * WALL: the original loads the full u64 (`ld`) and extracts via 3 independent
 * `dsrl32` + `andi` with NO sign-extension; this cc1 byte-loads the first lane
 * (`lbu +0x3C`) and sign-extends each `(v>>n)&0xFF` result (dsll32/dsra32)
 * before the `sw`. Its 64-bit narrowing/sub-word lowering differs from the
 * later SN cc1 that built the original. Left INCLUDE_ASM.
 */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/text/1A00F0", func_002A12F0);

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
