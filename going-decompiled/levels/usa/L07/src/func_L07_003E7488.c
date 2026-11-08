/* Level 07 (Siberius), USA v2.00: a level-only function built from C (row
 * L07-A, task #2656, RULING #13291). Compiled ALONE as its own TU and linked
 * at its one place, 0x003E7488, in place of its split asm; the row is
 * going-decompiled/levels/usa/L07/config/L07_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern void func_L07_00318120(char *, int, char *, char *, char *, char *);

/**
 * func_L07_003E7488 - run the shared pass on both halves of the object, then
 * again on the secondary half with a stack buffer when the state byte reads
 * nine.
 *
 * func_L07_00318120 is called with the state block's (+0x68) +0x260 word and
 * the object's +0x10 / +0xF0 blocks, each passed twice. When the object's
 * state byte at +0x20 is 9 it is called again with the state block's +0x230
 * block and a 16-byte stack buffer, again each passed twice.
 *
 * @arg: the object.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/07_siberius/L07_00389EB8.c, function func_L07_003E6148
 * (v1.01 address), author commit 91e04732 (llesieur99), by blame -M -C of the
 * definition (all 9 lines; no rename-copy found).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (BYTE-IDENTICAL), each name to the target
 * of the v2.00 disc's own relocation at this place (task #2656 relverify by
 * name):
 *   - func_L07_003E6148 -> func_L07_003E7488 (our split name);
 *   - func_002AD0E0 -> func_L07_00318120 (L07's own copy at the v2.00
 *     place);
 *   - `#include "common.h"`/`#include "include_asm.h"` and the file's other
 *     declarations dropped: this function uses no common.h typedef; and
 *     upstream's comment folded into this one.
 */
void func_L07_003E7488(char *arg)
{
    char *o = *(char **)(arg + 104);
    char tmp[16];

    func_L07_00318120(arg, *(int *)(o + 0x260), arg + 0x10, arg + 0xF0, arg + 0x10, arg + 0xF0);
    if (*(unsigned char *)(arg + 0x20) == 9)
        func_L07_00318120(arg, *(int *)(o + 0x260), o + 0x230, tmp, o + 0x230, tmp);
}
