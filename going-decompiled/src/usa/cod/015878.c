#include "common.h"

/*
 * cod/015878 (0x1158F8..0x115ABF): the old cod/015180 code between strlen and
 * strncpy. strcpy (0x115690) and strlen (0x1157AC) before it, and strncpy
 * after it, are linked from newlib's verbatim .S (going-decompiled/libc/,
 * tasks #1884 and #1925). The ROM word 0x1158F4 between strlen and this unit
 * is the .cod section's 0xCDCDCDCD FILL, written by the link because this
 * unit starts 8-aligned; it is no longer an INCLUDE_ASM_FRAGMENT.
 */

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
 * earlier attempt hit. Other units hold such pads as INCLUDE_ASM_FRAGMENT;
 * func_001158F8's one-word pad is now the link's FILL (header comment).
 *
 * Still fused: func_0011F364, func_0011FB8C, func_00130A8C — their real bodies
 * are reached only by `j`/data reference, never by `jal`, so they are the
 * j-target class of task #471, not a carve defect of this class. */
INCLUDE_ASM("going-decompiled/asm/usa/nonmatchings/cod/015878", func_001158F8);
