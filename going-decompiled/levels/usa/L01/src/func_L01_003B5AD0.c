/* Level 01 (Oozla), USA v2.00: a level-only function built from C (row L01-G1,
 * task #2626, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003B5AD0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L01/config/L01_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/**
 * func_L01_003B5AD0 - clear the activation word at +0x9C of every entity in
 * the array.
 *
 * The counterpart of func_L01_003B59E0's activate step: the loop runs
 * *arg1 times (nothing happens for a non-positive count), storing 0 at
 * +0x9C of each pointer in @arg0.
 *
 * @arg0: the entity array gathered by func_L01_003B59E0.
 * @arg1: the number of valid entries in it.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/01_oozla/L01_003835D8.c, function func_L01_003B46C8 (v1.01
 * address), author commit 91e04732 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (BYTE-IDENTICAL):
 *   - func_L01_003B46C8 -> func_L01_003B5AD0 (our split name);
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped: the body references no other symbol (0
 *     relocations).
 */
void func_L01_003B5AD0(char **arg0, int *arg1) {
    int i;

    if (*arg1 <= 0)
        return;
    i = 0;
    do {
        *(int *)(arg0[i] + 0x9C) = 0;
        i++;
    } while (i < *arg1);
}
