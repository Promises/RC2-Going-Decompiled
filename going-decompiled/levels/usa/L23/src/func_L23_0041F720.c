/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x0041F720, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm ee-gcc
 * 2.9-ee-991111b/r4 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, assembled by
 * plain GNU as (the row's `as` column).
 */

typedef signed short s16;

/**
 * func_L23_0041F720 - read a moby's float parameter by its class.
 *
 * Classes 0xE3A read the float at state +0x2C; class 0xE3B, 0xE3C and every
 * other class read state +0x44. The class is the halfword at +0xAA.
 *
 * @arg0: the moby; its +0x68 points at the state.
 * Return: the float.
 *
 * Non-obvious: the third label 0xE3C shares the default's body. With only two
 * cases cc1 emits a comparison chain; the third label makes it build the
 * decision tree rooted on 0xE3B that the ROM has (upstream's note, which
 * cites its own func_L22_003A65E0 as the recipe).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/23_damosel_orbit/L23_00416388.c, function func_L23_0041E220
 * (v1.01 address), author commit 86ff805f (bigmak94). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_L23_0041E220 -> func_L23_0041F720
 *   - `#include "include_asm.h"` dropped; the typedefs it uses written in the
 *     TU (the file's own where it has them, else common.h's); only the
 *     file-level declarations it uses kept, without their comments
 *   - upstream's comment folded into this one
 */
float func_L23_0041F720(char *arg0) {
    switch (*(s16 *)(arg0 + 0xAA)) {
    case 0xE3A:
        return *(float *)(*(char **)(arg0 + 0x68) + 0x2C);
    case 0xE3B:
        return *(float *)(*(char **)(arg0 + 0x68) + 0x44);
    case 0xE3C:
    default:
        return *(float *)(*(char **)(arg0 + 0x68) + 0x44);
    }
}
