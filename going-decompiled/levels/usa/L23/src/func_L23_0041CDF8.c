/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x0041CDF8, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern char *D_L23_001B1434;
extern int D_L23_001B139C;
extern int func_L23_00368698(int index, int a1, char *a2);

/**
 * func_L23_0041CDF8 - unless D_L23_001B139C is set, calls
 * func_L23_00368698(0x10, 0, D_L23_001B1434) when that level object pointer is
 * set, then stores 1 at state +0x604. (Upstream's comment: "Stores one in the
 * state at +0x68, after the measured shared call.")
 *
 * @arg0: the object (moby); +0x68 points at its state block.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_00416388-g8.c,
 * function func_L23_0041B8F8 (v1.01 address), author commit 773eafc2
 * (llesieur99). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED (the audit's verdict was BYTE-IDENTICAL): names
 * respelled and upstream's inert `sda` attribute dropped, as listed below;
 * the body's statements unchanged (RULING #13291 section 7). Edits:
 *   - func_L23_0041B8F8 -> func_L23_0041CDF8 (our split name at the v2.00
 *     place)
 *   - D_L23_001B13EC -> D_L23_001B139C, D_L23_001B1484 -> D_L23_001B1434 (the
 *     v2.00 gp-relative targets)
 *   - func_002E5CF8 -> func_L23_00368698
 *   - upstream's `__attribute__((sda))` on both globals dropped: inert at -G8
 *     for 4-byte objects
 *   - `#include "include_asm.h"` dropped: the TU declares what it uses, and
 *     upstream's comments are folded into this one
 */
void func_L23_0041CDF8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (D_L23_001B139C == 0 && D_L23_001B1434 != 0)
        func_L23_00368698(0x10, 0, D_L23_001B1434);
    *(int *)(state + 0x604) = 1;
}
