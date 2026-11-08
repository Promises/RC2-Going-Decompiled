/* Level 06 (Notak), USA v2.00: a level-only function built from C (row L06-A,
 * task #2653, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x004453C0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L06/config/L06_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern void func_L06_0032E0D0(char *);
extern void func_L06_003674D8(char *, char *, float, int, int);

/**
 * func_L06_004453C0 - per-frame upkeep: tick two timers, count the
 * frame and run the moby's motion step.
 * @a0: the moby; its +0x68 points at its pvar block s1.
 *
 * func_L06_0032E0D0 on the halfword timers s1+0x310 and s1+0x312; the
 * halfword frame counter at @a0+0x96 is incremented, saturating at 0xFFFF;
 * then func_L06_003674D8(@a0, s1 + 0x260, 2.5f, 0, 1).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-
 * origin), file src/levels/06_notak/L06_0041DED8.c, function
 * func_L06_00443E18 (v1.01 address), author commit fa8d6672 (llesieur99;
 * `git blame -M -C` and `git log -S` agree). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for
 * names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2653 relverify by name); edits:
 *   - func_L06_00443E18 -> func_L06_004453C0 (our split name at the v2.00
 *     place)
 *   - func_32D268 -> func_L06_0032E0D0, func_366160 -> func_L06_003674D8
 *     (callees respelled to L06's own copies at their v2.00 places; level-
 *     copy callee names stay func_L06_<addr>)
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped
 */
void func_L06_004453C0(char *a0)
{
    char *s1 = *(char **)(a0 + 0x68);
    unsigned int v;

    func_L06_0032E0D0(s1 + 0x310);
    func_L06_0032E0D0(s1 + 0x312);
    v = *(unsigned short *)(a0 + 0x96);
    if (v <= 0xFFFE)
        *(unsigned short *)(a0 + 0x96) = v + 1;
    func_L06_003674D8(a0, s1 + 0x260, 2.5f, 0, 1);
}
