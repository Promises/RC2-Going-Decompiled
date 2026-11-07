/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x003417D0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L11's copies of a boot vector helper and of level-02 code, at their v2.00
 * places in this level. */
extern void func_L11_00301210(void *, void *, void *);
extern int func_L11_003415E8(char *, char *, void *);

/**
 * func_L11_003417D0 - test the actor against a point, from its position.
 *
 * @object: the actor; its position is at +0x10.
 * @point:  the point.
 *
 * Fills a 4-float diff with func_L11_00301210(diff, point, object + 0x10)
 * and returns func_L11_003415E8(object, object + 0x10, diff).
 * Return: func_L11_003415E8's result.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_002DE9E0.c, function
 * func_L11_00340500 (v1.01 address), author commit c7d9e30a (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L11_00340500 -> func_L11_003417D0 (our split name at the
 *     v2.00 place);
 *   - func_00282CF0 -> func_L11_00301210 and func_L02_00333480 ->
 *     func_L11_003415E8: L11's copies, the targets of the v2.00 ROM's own
 *     relocations (relverify, task #2553);
 *   - the file's `#include "include_asm.h"`, `#include "common.h"`, other
 *     functions and their externs not carried (nothing here uses them);
 *   - layout only: the opening brace moved onto the definition line, so
 *     level_gate.sh --selftest's SEEDED C arm can find this, the table's
 *     first row (its seeder matches `fn(...) {` on one line).
 */
int func_L11_003417D0(char *object, void *point) {
    char *target = object + 0x10;
    float diff[4];

    func_L11_00301210(diff, point, target);
    return func_L11_003415E8(object, target, diff);
}
