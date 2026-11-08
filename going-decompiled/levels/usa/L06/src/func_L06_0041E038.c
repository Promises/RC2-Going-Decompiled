/* Level 06 (Notak), USA v2.00: a level-only function built from C (row L06-A,
 * task #2653, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x0041E038, in place of its split asm; the row is
 * going-decompiled/levels/usa/L06/config/L06_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern void func_L06_003516D0(char *a0);

/**
 * func_L06_0041E038 - release a child's claim on its parent's slot table, then
 * retire the child.
 * @a0: the child moby.
 * @a1: its state block: the parent moby at +0x10, the slot index byte at +0x1C.
 *
 * When the parent is non-null, of class 0xA58 (+0xAA) and its state byte
 * (+0x20) is neither 0xFE nor 0xFD, the word at +0xA0 + 4 * slot of the
 * parent's pvar block (+0x68) is cleared. Then func_L06_003516D0(@a0).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-
 * origin), file src/levels/06_notak/L06_0033BDC0.c, function
 * func_L06_0041CAB0 (v1.01 address), author commit 52e801be (llesieur99;
 * `git blame -M -C` and `git log -S` agree). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for
 * names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2653 relverify by name); edits:
 *   - func_L06_0041CAB0 -> func_L06_0041E038 (our split name at the v2.00
 *     place)
 *   - func_0029F338 -> func_L06_003516D0 (callees respelled to L06's own
 *     copies at their v2.00 places; level-copy callee names stay
 *     func_L06_<addr>)
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped
 */
void func_L06_0041E038(char *a0, char *a1)
{
    char *m = *(char **)(a1 + 0x10);

    if (m != 0 && *(short *)(m + 0xAA) == 0xA58
        && *(unsigned char *)(m + 0x20) != 0xFE && *(unsigned char *)(m + 0x20) != 0xFD) {
        int *p = *(int **)(m + 0x68);

        p += *(unsigned char *)(a1 + 0x1C);
        p[40] = 0;
    }
    func_L06_003516D0(a0);
}
