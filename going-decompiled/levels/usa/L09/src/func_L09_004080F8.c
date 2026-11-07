/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (row L09-B,
 * task #2562, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x004080F8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef int s32;

extern void func_L09_00318D10();

/**
 * func_L09_004080F8 - guard the level's copy of the shared entity-visibility
 * body behind a -1 record index.
 *
 * With @a0 == -1 nothing happens; any other record index is handed to
 * func_L09_00318D10, L09's own copy of the shared body upstream names
 * func_L08_00328BB0.
 *
 * @a0: a record index, or -1 for "none".
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_003FEA48.c, function func_L09_00406C68 (v1.01
 * address), author commit 264cb2be (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2562 relverify by name):
 *   - func_L09_00406C68 -> func_L09_004080F8 (our split name);
 *   - func_L08_00328BB0 -> func_L09_00318D10 (L09's own copy; upstream named
 *     it after level 08's v1.01 place);
 *   - `#include "common.h"` dropped: the typedefs this function uses are
 *     written here and only its declarations kept.
 */
void func_L09_004080F8(s32 a0) {
    if (a0 == -1) {
        return;
    }
    func_L09_00318D10(a0);
}
