/* Level 07 (Siberius), USA v2.00: a level-only function built from C (row
 * L07-A, task #2656, RULING #13291). Compiled ALONE as its own TU and linked
 * at its one place, 0x003E6DB0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L07/config/L07_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern void func_L07_002E4BB8(char *);

/**
 * func_L07_003E6DB0 - clear the two resource slots and step the state
 * counter while it has room.
 *
 * The two u16 slots at +0x240 and +0x242 of the object's state block (its
 * +0x68 pointer) are each passed to func_L07_002E4BB8. The object's u16
 * counter at +0x96 is then incremented unless it is already 0xFFFF.
 *
 * @arg: the object.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/07_siberius/L07_00389EB8.c, function func_L07_003E5A70
 * (v1.01 address), author commit 91e04732 (llesieur99), by blame -M -C of the
 * definition (all 11 lines; no rename-copy found).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (BYTE-IDENTICAL), each name to the target
 * of the v2.00 disc's own relocation at this place (task #2656 relverify by
 * name):
 *   - func_L07_003E5A70 -> func_L07_003E6DB0 (our split name);
 *   - func_00282978 -> func_L07_002E4BB8 (L07's own copy at the v2.00
 *     place);
 *   - `#include "common.h"`/`#include "include_asm.h"` and the file's other
 *     declarations dropped: this function uses no common.h typedef; and
 *     upstream's one-line comment folded into this one.
 */
void func_L07_003E6DB0(char *arg)
{
    char *o = *(char **)(arg + 104);
    unsigned short v;

    func_L07_002E4BB8(o + 0x240);
    func_L07_002E4BB8(o + 0x242);
    v = *(unsigned short *)(arg + 0x96);
    if (v <= 0xFFFE)
        *(unsigned short *)(arg + 0x96) = v + 1;
}
