/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x00456538, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L11's copies of boot code, at their v2.00 places in this level. */
extern void func_L11_003327B8(void *a0, void *a1);
extern void func_L11_00325678(void *a0, int a1, int a2, int a3);

/**
 * func_L11_00456538 - per-frame update: refresh the contact block and re-arm
 * the effect once the contact count drops to zero.
 *
 * @p: the actor; its state block q is at +0x68.
 *
 * func_L11_003327B8(p, q + 0x50) runs first. If q's halfword +0x50 is then 0
 * and +0x1FE is still 0, it calls func_L11_00325678(p, q+0x1F0, q+0x1F4,
 * q+0x1F8 words) and sets +0x1FE to 1; a nonzero +0x50 clears +0x1FE. Last,
 * the frame counter at p+0x96 counts up, saturating at 0xFFFF.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_00453358.c, function
 * func_L11_00454AE0 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291).
 * ADAPTED, the body's statements unchanged (verbatim but for names; no
 * device); edits:
 *   - name func_L11_00454AE0 -> func_L11_00456538 (our split name at the
 *     v2.00 place);
 *   - func_002AB8C0 -> func_L11_003327B8 and func_002A0488 ->
 *     func_L11_00325678: L11's copies, the targets of the v2.00 ROM's own
 *     relocations (relverify, task #2553);
 *   - the audit's `long long` -> `long` respelling (RULING #13197 class,
 *     needed because cc1 s136 refuses the file's other `long long` uses) is
 *     NOT applied: this one-function TU carries no `long long`;
 *   - `#include "common.h"` dropped (no typedef of it is used here); the
 *     file's other functions and their externs not carried.
 */
void func_L11_00456538(char *p)
{
    char *q = *(char **)(p + 0x68);
    unsigned short c;

    func_L11_003327B8(p, q + 0x50);
    if (*(short *)(q + 0x50) == 0) {
        if (*(short *)(q + 0x1FE) == 0) {
            func_L11_00325678(p, *(int *)(q + 0x1F0), *(int *)(q + 0x1F4), *(int *)(q + 0x1F8));
            *(short *)(q + 0x1FE) = 1;
        }
    } else {
        *(short *)(q + 0x1FE) = 0;
    }
    c = *(unsigned short *)(p + 0x96);
    if (c <= 0xFFFE)
        *(unsigned short *)(p + 0x96) = c + 1;
}
