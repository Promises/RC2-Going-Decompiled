#include "common.h"

/*
 * cod/015180h (0x115200..0x11533F): the head of the old cod/015180 unit, before
 * the libc members memcmp..strcmp, which are linked from newlib's verbatim .S
 * (going-decompiled/libc/, task #1884).
 */

extern s32 D_00133E74;
extern s32 D_0013A308;

/**
 * Accessor: return the global pointer/handle D_00133E74 (the base of the
 * subsystem context block this unit operates on).
 */
s32 func_00115200(void) {
    return D_00133E74;
}

/**
 * Accessor: return the address of the global D_0013A308.
 */
s32 *func_00115210(void) {
    return &D_0013A308;
}

INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015180h", func_00115220);

/* func_00115228: thin wrapper -- calls func_00115210 and returns its result.
 * ROM arm kept: the original builds a 0x10 frame and saves $31 with sq/lq
 * (quadword), which this cc1 does not emit for a leaf-ish wrapper. A
 * byte-MATCHING wall, not a bar to a faithful portable arm. NOT a byte-match
 * claim. */
#ifndef TARGET_NATIVE
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180h", func_00115228);
#else
/**
 * Wrapper around func_00115210: returns &D_0013A308.
 *
 * ⚠️ THE ROM PASSES AN ARGUMENT THAT THE CALLEE IGNORES, and this arm
 * deliberately does not reproduce it. Verified rather than assumed:
 *
 *   func_00115228.s   lw   $4, %lo(D_00133E74)($2)   <- loads a value into arg1
 *                     jal  func_00115210             <- 0x0C045484 -> 0x00115210
 *   func_00115210.s   lui  $2, %hi(D_0013A308)
 *                     jr   $31
 *                     addiu $2, $2, %lo(D_0013A308)  <- never reads $4
 *
 * So the load is dead at the callee, and func_00115210 is already decompiled
 * here as `(void)`. Reproducing the dead argument would mean declaring a
 * parameter the callee does not have, purely to imitate a register write with
 * no observable effect -- D_00133E74 is ordinary bss, not MMIO, so the read
 * itself is unobservable too.
 *
 * Most likely the original called this through a declaration carrying a
 * context parameter that the implementation dropped -- common in this SDK
 * unit's accessor family. That is a guess about the ORIGINAL SOURCE and is
 * marked as one; the .s facts above are not.
 */
s32 *func_00115228(void) {
    return func_00115210();
}
#endif

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015180h", func_00115250);
