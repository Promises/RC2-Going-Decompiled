/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x00457F98, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L11's own function and its copies of boot code, at their v2.00 places. */
extern void func_L11_0032DBF8(void *a0, int a1, int a2);
extern void func_L11_0032D918(void *a0, int a1, int a2, int a3);
extern void func_L11_00325748(void *a0);
extern void func_L11_004431C8(char *p, int id);

/**
 * func_L11_00457F98 - retire the state's counter and drop the actor to its
 * idle flag block.
 *
 * @p: the actor (type 0xDF9); its state block q is at +0x68.
 *
 * A tracking actor (q+0x256 == 1) is untracked through
 * func_L11_004431C8(group, 0xDF9). Then q+0x256 and q+0x240 are cleared,
 * func_L11_0032D918(p, 0, 0, 3), func_L11_00325748(p) and
 * func_L11_0032DBF8(p, 1, 0) run, and +0x98, the +0x34 flags and +0x31 are
 * reset.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_00453358.c, function
 * func_L11_00456540 (v1.01 address), author commit 431ac503 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged (verbatim but for
 * names; no device); edits:
 *   - name func_L11_00456540 -> func_L11_00457F98 (our split name at the
 *     v2.00 place);
 *   - func_L11_00441978 -> func_L11_004431C8, func_002A7470 ->
 *     func_L11_0032D918, func_002A05A8 -> func_L11_00325748, func_002A7750 ->
 *     func_L11_0032DBF8: the targets of the v2.00 ROM's own relocations
 *     (relverify, task #2553);
 *   - the audit's `long long` -> `long` respelling (RULING #13197 class,
 *     needed because cc1 s136 refuses the file's other `long long` uses) is
 *     NOT applied: this one-function TU carries no `long long`;
 *   - `#include "common.h"` dropped (no typedef of it is used here); the
 *     upstream one-line comment folded into this one; the file's other
 *     functions and their externs not carried.
 */
void func_L11_00457F98(char *p)
{
    char *q = *(char **)(p + 0x68);

    if (*(short *)(q + 0x256) == 1) {
        func_L11_004431C8(*(char **)(p + 0xB8), 0xDF9);
    }
    *(short *)(q + 0x256) = 0;
    *(int *)(q + 0x240) = 0;
    func_L11_0032D918(p, 0, 0, 3);
    func_L11_00325748(p);
    func_L11_0032DBF8(p, 1, 0);
    *(int *)(p + 0x98) = 0;
    *(unsigned short *)(p + 0x34) = (*(unsigned short *)(p + 0x34) & 0xEFFF) | 0x47;
    *(char *)(p + 0x31) = 0;
}
