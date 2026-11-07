/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x0041D5C0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern int D_L23_001B1644 __attribute__((sda));
extern int D_L23_001B1648 __attribute__((sda));

/**
 * func_L23_0041D5C0 - test that a level counter is positive and below its limit.
 *
 * Returns 1 when 0 < D_L23_001B1644 < D_L23_001B1648, else 0.
 *
 * No parameters.
 * Return: 1 or 0.
 *
 * Non-obvious: both words are 4-byte objects, so at -G8 they are $gp-relative
 * by their size. Upstream's `__attribute__((sda))` text on the extern
 * declarations is kept as written; this cc1 ignores the attribute.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/23_damosel_orbit/L23_00416388-g8.c, function func_L23_0041C0C0
 * (v1.01 address), author commit 773eafc2 (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_L23_0041C0C0 -> func_L23_0041D5C0; D_L23_001B1694 ->
 *     D_L23_001B1644; D_L23_001B1698 -> D_L23_001B1648
 *   - `#include "include_asm.h"` dropped; the typedefs it uses written in the
 *     TU (the file's own where it has them, else common.h's); only the
 *     file-level declarations it uses kept, without their comments
 *   - upstream's one-line comment folded into this one
 */
int func_L23_0041D5C0(void) {
    if (D_L23_001B1644 > 0 && D_L23_001B1644 < D_L23_001B1648)
        return 1;
    return 0;
}
