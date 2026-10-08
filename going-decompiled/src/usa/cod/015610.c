#include "common.h"

/*
 * cod/015610 (0x115690..0x115ABF): the old cod/015180 code between strcmp and
 * strncpy, which are linked from newlib's verbatim .S (going-decompiled/libc/,
 * task #1884). strlen is here as INCLUDE_ASM although its verbatim .S also
 * reproduces the ROM: it starts at 4 mod 8 right after func_00115690, and the
 * .cod output section's SUBALIGN(8) cannot place a library member there.
 * strlen.S assembled by Ps2EeAs is byte-identical to this body (FACT #9816).
 */

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015610", func_00115690);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015610", strlen);
INCLUDE_ASM_ALIAS(func_001157AC, strlen);

/* 0xCDCDCDCD inter-function-fill class. Each of these symbols is one or more
 * leading 0xCDCDCDCD debug-fill words (sometimes with dead stores/nops) emitted
 * between functions; spimdisasm folds the fill into the FOLLOWING symbol and
 * demotes the real prologue to an interior `alabel`.
 *
 * Task #472 (FACT #7359): where the real start is a `jal` target in the ROM
 * (func_001158F8, func_00124418, func_00125D98, func_00126108, func_00126288,
 * func_00126470, func_00126730, func_00126DC0, func_00128F50, func_001290E0)
 * a `type:func` pin on the real start alone makes splat carve the fill off as
 * its own fragment (a lone CD word terminates fine as a 0x4 symbol — see
 * func_001253A4) and the body gets its own .s AS LONG AS an INCLUDE_ASM names
 * it: splat writes nonmatchings/<unit>/<fn>.s only for names the .c references
 * (segtypes/common/c.py global_asm_funcs), which is the "coverage hole" an
 * earlier attempt hit. The pad fragments below are INCLUDE_ASM_FRAGMENT.
 *
 * Still fused: func_0011F364, func_0011FB8C, func_00130A8C — their real bodies
 * are reached only by `j`/data reference, never by `jal`, so they are the
 * j-target class of task #471, not a carve defect of this class. */
INCLUDE_ASM_FRAGMENT("going-decompiled/asm/usa/nonmatchings/cod/015610", func_001158F4);

INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015610", func_001158F8);
