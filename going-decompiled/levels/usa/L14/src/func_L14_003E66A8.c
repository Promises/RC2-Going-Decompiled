/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x003E66A8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm Sony 2.9-ee-991111
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

/**
 * func_L14_003E66A8 - clear a counter on the object's target.
 *
 * @arg0: the object; its state block pointer is at +0x68.
 * Return: nothing.
 *
 * Non-obvious: the pointer chase is state +0x30 (may be null, checked), then
 * that record's +0x68; the word cleared is the chase's +0x3C.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c, function
 * func_L14_003E5190 (v1.01 address), author commit db93b56d (llesieur99; the
 * definition line blames db93b56d too). Adopted with the authors' permission
 * (RULINGs #10405, #12108, #13082, #13291). BYTE-IDENTICAL: VERBATIM but for
 * names; edits:
 *   - name func_L14_003E5190 -> func_L14_003E66A8 (our split name at the
 *     v2.00 place);
 *   - the file's `#include` lines, other functions and their externs not
 *     carried (nothing here uses them).
 */
void func_L14_003E66A8(char *arg0) {
    char *v0 = *(char **)(arg0 + 0x68);
    v0 = *(char **)(v0 + 0x30);
    if (v0 != 0) {
        v0 = *(char **)(v0 + 0x68);
        *(int *)(v0 + 0x3C) = 0;
    }
}
