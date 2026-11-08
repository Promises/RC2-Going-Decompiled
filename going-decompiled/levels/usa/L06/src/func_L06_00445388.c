/* Level 06 (Notak), USA v2.00: a level-only function built from C (row L06-A,
 * task #2653, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x00445388, in place of its split asm; the row is
 * going-decompiled/levels/usa/L06/config/L06_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern void func_L06_0036D588();
extern void func_L06_0036D5E0(float);

/**
 * func_L06_00445388 - switch a moby's effect off or on.
 * @a0: the moby; its pvar block (+0x68) holds the effect's float at +0x31C.
 * @a1: unused.
 * @a2: 0 to stop (func_L06_0036D588()), nonzero to start
 *      (func_L06_0036D5E0(pvar float +0x31C)).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-
 * origin), file src/levels/06_notak/L06_0041DED8.c, function
 * func_L06_00443DE0 (v1.01 address), author commit fa8d6672 (llesieur99;
 * `git blame -M -C` and `git log -S` agree. The token-identical PAL copy
 * func_PAL_L06_00448968 is later, Flavius 7a2f01ec 2026-10-21, so it does
 * not change the credit). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082, #13291). VERBATIM but for names (BYTE-IDENTICAL),
 * each name to the target of the v2.00 disc's own relocation at this place
 * (task #2653 relverify by name); edits:
 *   - func_L06_00443DE0 -> func_L06_00445388 (our split name at the v2.00
 *     place)
 *   - func_36C158 -> func_L06_0036D588, func_36C1B0 -> func_L06_0036D5E0
 *     (callees respelled to L06's own copies at their v2.00 places; level-
 *     copy callee names stay func_L06_<addr>)
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped
 */
void func_L06_00445388(char *a0, int a1, int a2)
{
    char *p = *(char **)(a0 + 0x68);

    if (a2 == 0) {
        func_L06_0036D588();
    } else {
        func_L06_0036D5E0(*(float *)(p + 0x31C));
    }
}
