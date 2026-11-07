/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x0041F760, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm ee-gcc
 * 2.9-ee-991111b/r4 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G8, assembled by
 * plain GNU as (the row's `as` column).
 */

extern char *D_L23_001B1434 __attribute__((sda));

/**
 * func_L23_0041F760 - test the level object's state byte.
 *
 * Returns 1 when byte +0x20 of the level object D_L23_001B1434 is 0x10 or
 * 0xC, else 0.
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
 * src/levels/23_damosel_orbit/L23_00416388-g8.c, function func_L23_0041E260
 * (v1.01 address), author commit 773eafc2 (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_L23_0041E260 -> func_L23_0041F760; D_L23_001B1484 ->
 *     D_L23_001B1434
 *   - `#include "include_asm.h"` dropped; the typedefs it uses written in the
 *     TU (the file's own where it has them, else common.h's); only the
 *     file-level declarations it uses kept, without their comments
 *   - upstream's one-line comment folded into this one
 */
int func_L23_0041F760(void) {
    unsigned char state = *(unsigned char *)(D_L23_001B1434 + 0x20);

    if (state == 0x10 || state == 0xC)
        return 1;
    return 0;
}
