/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2563, RULING #13291 row L14-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003DF960, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L14's copy of a boot helper, at its v2.00 place. */
extern void func_L14_00333698();

/**
 * func_L14_003DF960 - wake the object's four linked objects.
 *
 * @arg0: the object; four object pointers follow its state block's +0xE0.
 *
 * Each non-null linked object whose state word +0x2E0 is set gets
 * func_L14_00333698(obj, 1).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c,
 * function func_L14_003DE488 (v1.01 address), author commit db93b56d (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L14_003DE488 -> func_L14_003DF960 (our split name at
 *     the v2.00 place);
 *   - func_3322F0 -> func_L14_00333698 (L14's copy);
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L14_003DF960(char *arg0) {
    char **p = (char **)(*(int *)(arg0 + 0x68) + 0xE0);
    int i;

    for (i = 3; i >= 0; i--) {
        char *v = *p;

        if (v != 0 && *(int *)(*(int *)(v + 0x68) + 0x2E0) != 0) {
            func_L14_00333698(v, 1);
        }
        p++;
    }
}
