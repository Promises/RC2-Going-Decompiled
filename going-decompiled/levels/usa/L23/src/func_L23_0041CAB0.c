/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x0041CAB0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm ee-gcc
 * 2.9-ee-991111b/r4 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, assembled by
 * plain GNU as (the row's `as` column).
 */

typedef signed int s32;

/**
 * func_L23_0041CAB0 - clear three words of a moby's state.
 *
 * Zeroes the words at state +0x5CC, +0x5C8 and +0x5DC, in that order.
 *
 * @arg0: the moby; its +0x68 points at the state.
 * Return: nothing.
 *
 * Non-obvious: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/23_damosel_orbit/L23_00416388.c, function func_L23_0041B5B0
 * (v1.01 address), author commit 773eafc2 (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_L23_0041B5B0 -> func_L23_0041CAB0
 *   - `#include "include_asm.h"` dropped; the typedefs it uses written in the
 *     TU (the file's own where it has them, else common.h's); only the
 *     file-level declarations it uses kept, without their comments
 *   - upstream's one-line comment folded into this one
 */
void func_L23_0041CAB0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    *(s32 *)(state + 0x5CC) = 0;
    *(s32 *)(state + 0x5C8) = 0;
    *(s32 *)(state + 0x5DC) = 0;
}
