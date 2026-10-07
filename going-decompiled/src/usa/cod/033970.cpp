#include "common.h"

/*
 * cod/033970 — core.text tail (0x1339F0..0x133B7F), the functions after the
 * carved 989snd sub-TU cod/0321A0. Built at the default -O2 -G0 like
 * cod/015180.
 */

extern u8 g_discToc[];
extern s32 CdReadSync(s32 arg0, s32 arg1, void *arg2);

/* Callees in cod/0321A0, declared with their definitions' types. */
extern s32 CdStartRead(s32 lbn, s32 sectors, s32 dest, void *rmode);
extern s32 func_00133230(void);
extern s32 snd_Pump(void);
extern s32 snd_CheckLoadInProgress(s32 noWait);

/**
 * func_001339F0 - copy `count` bytes from src to dst, one byte at a time by
 * index (dst[i] = src[i]). A non-positive count copies nothing. LoadLevelToc
 * runs it over each TOC block it loads.
 *
 * Compiled by the s136os arm (SN 2.95.3 v1.36 -fopt-stack, selected in
 * tools/ee/s136os_functions.txt). That compiler fills the loop's `bnez` delay
 * slot with the `sb` and aligns the loop head with one nop, as the ROM does;
 * cc1 2.9 put the store before the branch (~88%).
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_func_001339F0)
S136OS_SLOT(func_001339F0);
#else
void func_001339F0(u8 *dst, u8 *src, s32 count) {
    s32 i;

    for (i = 0; i < count; i++) {
        dst[i] = src[i];
    }
}
#endif

/**
 * CdReadSync - read `sectors` sectors from disc LBA `lbn` into buf and wait
 * for the transfer to finish.
 *
 * Builds a 4-byte sceCdRMode on the stack (trycount 0x20, spindle control 1,
 * data pattern 0) and starts the read through CdStartRead, then runs the
 * 989snd tick (func_00133230), snd_Pump, and snd_CheckLoadInProgress(0),
 * which blocks until the load is no longer in progress. Returns
 * snd_CheckLoadInProgress's result.
 *
 * Compiled by the s136os arm (SN 2.95.3 v1.36 -fopt-stack). It saves $ra
 * before the mode-byte stores and puts the fourth `sb` in the jal delay slot,
 * as the ROM does; cc1 2.9 grouped the stores (~54%).
 */
#if !defined(TARGET_NATIVE) && !defined(S136OS_CdReadSync)
S136OS_SLOT(CdReadSync);
#else
s32 CdReadSync(s32 lbn, s32 sectors, void *buf) {
    u8 mode[4];

    mode[0] = 0x20; /* trycount */
    mode[1] = 1;    /* spindlctrl */
    mode[2] = 0;    /* datapattern */
    mode[3] = 0;
    CdStartRead(lbn, sectors, (s32)buf, mode);
    func_00133230();
    snd_Pump();
    return snd_CheckLoadInProgress(0);
}
#endif

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
 * func_001339F0 over each. The 3-operand `mult $18,$4,$2` is NOT a wall:
 * ee-gcc 2.9 emits the R5900 mult-rd form for exactly this `level*0x18` shape
 * (FACT #6218). The binding constraint is the save layout. The ROM saves
 * s0,s1,s2,ra at 0/8/16/24, stride 8 (FACT #8163), and held cc1 2.9 emits
 * 16-byte slots for a multi-register save. INCLUDE_ASM. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/033970", LoadLevelToc);
