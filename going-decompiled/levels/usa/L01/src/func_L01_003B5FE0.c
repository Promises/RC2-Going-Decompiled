/* Level 01 (Oozla), USA v2.00: a level-only function built from C (row L01-G1,
 * task #2626, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003B5FE0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L01/config/L01_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;

extern void func_L01_002E3DE8();

/**
 * func_L01_003B5FE0 - stop the group's active measurement: release the
 * record at +0x33C once and clear the active word at +0x340.
 *
 * For a grouped object (+0x21 != 0xFF) whose state's +0x340 word is set,
 * func_L01_002E3DE8 (a timer/loop release) receives the pointer held at
 * +0x33C; the +0x340 word is then cleared in every case.
 *
 * @arg0: the object record (its state is at *(u8 **)(arg0 + 0x68)).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/01_oozla/L01_003835D8.c, function func_L01_003B4BD8 (v1.01
 * address), author commit 91e04732 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2626 relverify by name):
 *   - func_L01_003B4BD8 -> func_L01_003B5FE0 (our split name);
 *   - func_00282948 -> func_L01_002E3DE8 (L01's own copy at the v2.00 place;
 *     upstream's prototype-less declaration kept);
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped, common.h's typedefs this body uses written out
 *     in the TU instead (validator #2638): this function references no other symbol.
 */
void func_L01_003B5FE0(u8 *arg0) {
    char *s0 = *(char **)(arg0 + 0x68);

    if (arg0[0x21] != 0xFF) {
        if (*(int *)(s0 + 0x340) != 0) {
            func_L01_002E3DE8(*(int **)(s0 + 0x33C));
        }
    }
    *(int *)(s0 + 0x340) = 0;
}
