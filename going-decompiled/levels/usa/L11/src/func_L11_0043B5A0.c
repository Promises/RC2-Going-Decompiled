/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x0043B5A0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/**
 * func_L11_0043B5A0 - choose state code 3 or 4 from the state's 0x364 word.
 *
 * @p: the object; its state is at +0x68.
 * Return: 3 when the state word at +0x364 is non-zero, else 4.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_00432628.c, function func_L11_00439D60 (v1.01
 * address), author commit fa8d6672 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device):
 *   - func_L11_00439D60 -> func_L11_0043B5A0 (our split name at the v2.00 place);
 *   - `#include "common.h"` dropped (nothing in this function used it).
 */
int func_L11_0043B5A0(char *p)
{
        if (*(int *)(*(char **)(p + 0x68) + 0x364))
        return 3;
    return 4;
}
