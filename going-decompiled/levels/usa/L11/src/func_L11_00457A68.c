/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x00457A68, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L11's copy of boot code, at its v2.00 place in this level. */
extern void func_L11_0032F698(void *a0, void *a1, int a2, float f12, float f13, float f14,
                          int a3, int t0, int t1);

/**
 * func_L11_00457A68 - count a frame and push the actor's effect at its
 * position.
 *
 * @p: the actor.
 *
 * The frame counter at p+0x96 counts up, saturating at 0xFFFF. Then
 * func_L11_0032F698(p, p + 0x10, 0x4000000, 1.6f, 1.0f, 1.0f, 0, 1, 0).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_00453358.c, function
 * func_L11_00456010 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged (verbatim but for
 * names; no device); edits:
 *   - name func_L11_00456010 -> func_L11_00457A68 (our split name at the
 *     v2.00 place);
 *   - func_002A8C28 -> func_L11_0032F698: the target of the v2.00 ROM's own
 *     relocation (relverify, task #2553);
 *   - the audit's `long long` -> `long` respelling (RULING #13197 class,
 *     needed because cc1 s136 refuses the file's other `long long` uses) is
 *     NOT applied: this one-function TU carries no `long long`;
 *   - `#include "common.h"` dropped (no typedef of it is used here); the
 *     file's other functions and their externs not carried.
 */
void func_L11_00457A68(char *p)
{
    unsigned short c = *(unsigned short *)(p + 0x96);

    if (c < 0xFFFF)
        *(unsigned short *)(p + 0x96) = c + 1;
    func_L11_0032F698(p, p + 0x10, 0x4000000, 1.6f, 1.0f, 1.0f, 0, 1, 0);
}
