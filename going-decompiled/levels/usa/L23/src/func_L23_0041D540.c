/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x0041D540, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern char *D_L23_001B1434;

/**
 * func_L23_0041D540 - for each live object pointer in the level object's state
 * (the two at +0x610/+0x614 and the five at +0x358..+0x368) and for arg0
 * itself, copies the word at +0x10 of the block at +0x24 into +0x98.
 * (Upstream's comment: "Copies the measured word of every live slot into the
 * object the argument names.")
 *
 * @arg0: an object (always updated).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_00416388-g8.c,
 * function func_L23_0041C040 (v1.01 address), author commit adcbd51e
 * (llesieur99). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED (the audit's verdict was BYTE-IDENTICAL): names
 * respelled and upstream's inert `sda` attribute dropped, as listed below;
 * the body's statements unchanged (RULING #13291 section 7). Edits:
 *   - func_L23_0041C040 -> func_L23_0041D540 (our split name at the v2.00
 *     place)
 *   - D_L23_001B1484 -> D_L23_001B1434
 *   - upstream's `__attribute__((sda))` dropped: inert at -G8 for a 4-byte
 *     object
 *   - `#include "include_asm.h"` dropped: the TU declares what it uses, and
 *     upstream's comments are folded into this one
 */
void func_L23_0041D540(char *arg0) {
    char *state = *(char **)(D_L23_001B1434 + 0x68);
    char *slot;
    char **p;
    int i;

    slot = *(char **)(state + 0x610);
    if (slot != 0)
        *(int *)(slot + 0x98) = *(int *)(*(char **)(slot + 0x24) + 0x10);
    slot = *(char **)(state + 0x614);
    if (slot != 0)
        *(int *)(slot + 0x98) = *(int *)(*(char **)(slot + 0x24) + 0x10);
    p = (char **)(state + 0x358);
    for (i = 4; i >= 0; i--) {
        if (*p != 0)
            *(int *)(*p + 0x98) = *(int *)(*(char **)(*p + 0x24) + 0x10);
        p++;
    }
    *(int *)(arg0 + 0x98) = *(int *)(*(char **)(arg0 + 0x24) + 0x10);
}
