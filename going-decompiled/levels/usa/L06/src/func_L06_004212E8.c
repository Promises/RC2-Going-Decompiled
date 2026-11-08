/* Level 06 (Notak), USA v2.00: a level-only function built from C (row L06-A,
 * task #2653, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x004212E8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L06/config/L06_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern void func_L06_0035CB88(int, int);

/**
 * func_L06_004212E8 - signal a moby's group.
 * @a0: the moby; its byte +0x21 is its group index (0xFF: no group).
 *
 * Calls func_L06_0035CB88(group, 1) unless the group is 0xFF.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-
 * origin), file src/levels/06_notak/L06_0041DED8.c, function
 * func_L06_0041FD60 (v1.01 address), author commit 431ac503 (llesieur99;
 * `git blame -M -C` and `git log -S` agree); its signature line survives
 * from llesieur99's own nonmatching draft
 * nonmatching/levels/func_L06_0041FD60.c (6aa8c748, earlier the same day).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (BYTE-IDENTICAL), each name to the target
 * of the v2.00 disc's own relocation at this place (task #2653 relverify by
 * name); edits:
 *   - func_L06_0041FD60 -> func_L06_004212E8 (our split name at the v2.00
 *     place)
 *   - func_35B810 -> func_L06_0035CB88 (callees respelled to L06's own
 *     copies at their v2.00 places; level-copy callee names stay
 *     func_L06_<addr>)
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped
 */
void func_L06_004212E8(char *a0)
{
    if (*(unsigned char *)(a0 + 0x21) != 0xFF)
        func_L06_0035CB88(*(unsigned char *)(a0 + 0x21), 1);
}
