/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2563, RULING #13291 row L14-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003CAA18, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm the SDK 2.9 cc1
 * (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

/**
 * func_L14_003CAA18 - test whether an object's byte +0x95 is 1.
 *
 * @arg0: the object.
 *
 * Return: 1 when byte +0x95 equals 1, else 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c,
 * function func_L14_003C9548 (v1.01 address), author commit db93b56d (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L14_003C9548 -> func_L14_003CAA18 (our split name at
 *     the v2.00 place);
 *   - no other symbol;
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
int func_L14_003CAA18(char *arg0) {
    return (arg0[0x95] ^ 1) == 0;
}
