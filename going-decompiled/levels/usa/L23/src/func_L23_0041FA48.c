/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x0041FA48, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef int s32;
extern u8 *D_L23_001B1434 __attribute__((sda));

/**
 * func_L23_0041FA48 - test that the level object is in state 4.
 *
 * Returns 0 when the level object D_L23_001B1434 is null, else whether its
 * byte +0x20 is 4.
 *
 * No parameters.
 * Return: 1 or 0.
 *
 * Non-obvious: Upstream's `__attribute__((sda))` text on the extern
 * declarations is kept as written; this cc1 ignores the attribute.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/23_damosel_orbit/L23_0041E418-g8.c, function func_L23_0041E548
 * (v1.01 address), author commit 264cb2be (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_L23_0041E548 -> func_L23_0041FA48; D_L23_001B1484 ->
 *     D_L23_001B1434
 *   - `#include "include_asm.h"` dropped; the typedefs it uses written in the
 *     TU (the file's own where it has them, else common.h's); only the
 *     file-level declarations it uses kept, without their comments
 *   - upstream's one-line comment folded into this one
 */
s32 func_L23_0041FA48(void) {
    u8 *state = D_L23_001B1434;

    if (state == 0) {
        return 0;
    }
    return state[0x20] == 4;
}
