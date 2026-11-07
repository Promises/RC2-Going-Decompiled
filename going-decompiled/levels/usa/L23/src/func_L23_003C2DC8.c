/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x003C2DC8, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern void func_L23_0031B350(int);

/**
 * func_L23_003C2DC8 - releases the two handles at state +0x328 and +0x32C
 * through func_L23_0031B350, zeroing each one that was set.
 *
 * @arg0: the object (moby); +0x68 points at its state block.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_0035CE90.c, function
 * func_L23_003C1988 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED in names only (verbatim but for names; the audit's file-
 * level `long long` code does not apply, since this body's own text has no
 * `long long`, FACT #13321); the body's statements unchanged. Edits:
 *   - func_L23_003C1988 -> func_L23_003C2DC8 (our split name at the v2.00
 *     place)
 *   - func_31A108 -> func_L23_0031B350
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the TU
 *     declares what it uses, and upstream's comments are folded into this one
 */
void func_L23_003C2DC8(char *arg0) {
    int *p = (int *)(*(int *)(arg0 + 0x68) + 0x328);
    int i;

    for (i = 1; i >= 0; i--) {
        int v = *p;

        if (v != 0) {
            func_L23_0031B350(v);
            *p = 0;
        }
        p++;
    }
}
