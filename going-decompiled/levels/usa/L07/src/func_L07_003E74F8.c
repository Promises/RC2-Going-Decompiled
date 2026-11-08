/* Level 07 (Siberius), USA v2.00: a level-only function built from C (row
 * L07-A, task #2656, RULING #13291). Compiled ALONE as its own TU and linked
 * at its one place, 0x003E74F8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L07/config/L07_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, plain as.
 */

extern int D_L07_001B0A90 __attribute__((sda));

extern void func_L07_003078E8(int);

/**
 * func_L07_003E74F8 - run the shared per-object pass once a level latch is
 * set.
 *
 * While the level word D_L07_001B0A90 ($gp-relative) is zero this does
 * nothing; otherwise @arg goes to func_L07_003078E8.
 *
 * @arg: the object (passed through unchanged).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/07_siberius/L07_00389EB8-g8.c, function func_L07_003E61B8
 * (v1.01 address), author commit 91e04732 (llesieur99), by blame -M -C of the
 * definition (all 5 lines; no rename-copy found).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (BYTE-IDENTICAL), each name to the target
 * of the v2.00 disc's own relocation at this place (task #2656 relverify by
 * name):
 *   - func_L07_003E61B8 -> func_L07_003E74F8 (our split name);
 *   - D_L07_001B0AE0 -> D_L07_001B0A90 (our committed split label at the
 *     v2.00 target; upstream's `sda` marker kept, inert at -G8 where a 4-byte
 *     object is $gp-relative by its size alone);
 *   - func_0029F338 -> func_L07_003078E8 (L07's own copy at the v2.00
 *     place);
 *   - `#include "common.h"`/`#include "include_asm.h"` and the file's other
 *     declarations dropped: this function uses no common.h typedef; and
 *     upstream's one-line comment folded into this one.
 */
void func_L07_003E74F8(int arg)
{
    if (D_L07_001B0A90 != 0)
        func_L07_003078E8(arg);
}
