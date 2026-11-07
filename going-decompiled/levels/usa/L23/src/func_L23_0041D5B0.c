/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x0041D5B0, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm ee-gcc
 * 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G8, assembled by plain
 * GNU as (the row's `as` column).
 */

extern int D_L23_001B1644;

/**
 * func_L23_0041D5B0 - reports whether the level counter D_L23_001B1644 is
 * negative.
 *
 * Takes nothing.
 * Return: 1 when negative, else 0. (Upstream's comment, "Zero while the counter
 * is negative.", has it backwards; the code returns 1.)
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_00416388-g8.c,
 * function func_L23_0041C0B0 (v1.01 address), author commit ee3fee45
 * (llesieur99). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED (the audit's verdict was BYTE-IDENTICAL): names
 * respelled and upstream's inert `sda` attribute dropped, as listed below;
 * the body's statements unchanged (RULING #13291 section 7). Edits:
 *   - func_L23_0041C0B0 -> func_L23_0041D5B0 (our split name at the v2.00
 *     place)
 *   - D_L23_001B1694 -> D_L23_001B1644
 *   - upstream's `__attribute__((sda))` dropped: inert at -G8 for a 4-byte
 *     object
 *   - `#include "include_asm.h"` dropped: the TU declares what it uses, and
 *     upstream's comments are folded into this one
 */
int func_L23_0041D5B0(void) {
    if (D_L23_001B1644 < 0)
        return 1;
    return 0;
}
