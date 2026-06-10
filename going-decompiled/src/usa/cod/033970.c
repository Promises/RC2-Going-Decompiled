#include "common.h"

/*
 * cod/033970 — core.text tail (0x1339F0..0x133B7F), the functions after the
 * carved 989snd sub-TU cod/0321A0. Built at the default -O2 -G0 like
 * cod/015180.
 */

extern u8 g_discToc[];
extern s32 CdReadSync(s32 arg0, s32 arg1, void *arg2);

/* func_001339F0: copy `count` bytes from src to dst via indexed access
 * (dst[i]=src[i]); non-positive count copies nothing. ~88% — the original fills
 * the loop's `bnez` delay slot with the `sb` store while ee-gcc emits the store
 * before the branch and nops the slot (a scheduling choice). Left as
 * INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/033970", func_001339F0);

/* CdReadSync(arg0, arg1, arg2): build a 4-byte descriptor {0x20,1,0,0} on the
 * stack and pass it with (arg0,arg1,arg2) to CdStartRead, then run the trio
 * func_00133230 / snd_Pump / snd_CheckLoadInProgress(0). ~54% — the original
 * schedules the `sd $31` save early (before the descriptor stores) and tucks
 * the 4th `sb` into the jal delay slot while keeping `move $7,$sp` outside it;
 * ee-gcc groups the stores and does the opposite delay-slot fill. A
 * scheduling-only shape this cc1 won't reproduce. Left as INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/033970", CdReadSync);

/**
 * LoadDiscToc - synchronously read the global disc TOC: 0xB sectors from LBA
 * 0x3E9 into g_discToc via CdReadSync, forwarding its return value. (The
 * value-return is what keeps ee-gcc from sibling-call-optimising this into
 * `j CdReadSync` - the original keeps the frame and uses jal+jr.)
 */
s32 LoadDiscToc(void) {
    return CdReadSync(0x3E9, 0xB, g_discToc);
}

/* LoadLevelToc(level, mode): refreshes the per-level TOC blocks - indexes the
 * disc TOC entry at g_discToc + level*0x18 (+0x4FF8/+0x5000/+0x5008) and
 * CdReadSync-loads three blocks into g_discToc+0x5298/+0x52F8/+0x6310, running
 * func_001339F0 over each. Blocked by the 3-operand `mult $18,$4,$2` (R5900
 * mult-rd form) this cc1 never emits - a documented codegen wall. INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/033970", LoadLevelToc);
