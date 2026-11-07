/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x003C1400, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern void func_L23_00325268(char *, int, int, int);

/**
 * func_L23_003C1400 - hands arg1 to func_L23_00325268(target, arg1, 0, 4) for
 * the object itself when bit 0 of arg4 is set, and for the object at state
 * +0x2A0 when bit 1 is set.
 *
 * @arg0: the object (moby); +0x68 points at its state block.
 * @arg1: the value passed on.  @arg4: the target mask.
 * @arg2, @arg3, @arg5: unused (the signature is a callback's).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_0035CE90.c, function
 * func_L23_003BFFC0 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED in names only (verbatim but for names; the audit's file-
 * level `long long` code does not apply, since this body's own text has no
 * `long long`, FACT #13321); the body's statements unchanged. Edits:
 *   - func_L23_003BFFC0 -> func_L23_003C1400 (our split name at the v2.00
 *     place)
 *   - func_323FF0 -> func_L23_00325268
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the TU
 *     declares what it uses, and upstream's comments are folded into this one
 */
void func_L23_003C1400(char *arg0, int arg1, int arg2, int arg3, int arg4, float arg5) {
    char *state = *(char **)(arg0 + 0x68);

    if (arg4 & 1)
        func_L23_00325268(arg0, arg1, 0, 4);
    if (arg4 & 2)
        func_L23_00325268(*(char **)(state + 0x2A0), arg1, 0, 4);
}
