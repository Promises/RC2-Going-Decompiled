/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x003E0BA8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm Sony 2.9-ee-991111
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

/**
 * func_L14_003E0BA8 - store an angle word into the object's state block.
 *
 * @arg0: the object (moby); its state block pointer is at +0x68.
 * @arg1: the value written to state +0x7C.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c, function
 * func_L14_003DF690 (v1.01 address), author commit adcbd51e (llesieur99; the
 * definition line blames adcbd51e too). Adopted with the authors' permission
 * (RULINGs #10405, #12108, #13082, #13291). BYTE-IDENTICAL: VERBATIM but for
 * names; edits:
 *   - name func_L14_003DF690 -> func_L14_003E0BA8 (our split name at the
 *     v2.00 place);
 *   - the file's `#include` lines, other functions and their externs not
 *     carried (nothing here uses them).
 */
void func_L14_003E0BA8(char *arg0, int arg1) {
    *(int *)(*(int *)(arg0 + 0x68) + 0x7C) = arg1;
}
