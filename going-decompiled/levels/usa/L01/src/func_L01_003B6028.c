/* Level 01 (Oozla), USA v2.00: a level-only function built from C (row L01-G1,
 * task #2626, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003B6028, in place of its split asm; the row is
 * going-decompiled/levels/usa/L01/config/L01_c.tsv, the arm 2.9-ee-991111b/r4
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, plain as.
 */

typedef unsigned char u8;

/**
 * func_L01_003B6028 - take the group's measurement: bump the record at +0x33C
 * once and mark the state active.
 *
 * For a grouped object (+0x21 != 0xFF) whose state's +0x340 word is clear,
 * the word the pointer at +0x33C names is incremented (the opening count of
 * the measurement func_L01_003B5FE0 later releases). The +0x340 word is set
 * to 1 in every case and 1 is returned.
 *
 * @arg0: the object record (its state is at *(char **)(arg0 + 0x68)).
 * Return: 1 always.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/01_oozla/L01_003835D8.c, function func_L01_003B4C20 (v1.01
 * address), author commit 91e04732 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (BYTE-IDENTICAL):
 *   - func_L01_003B4C20 -> func_L01_003B6028 (our split name);
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped, common.h's typedefs this body uses written out
 *     in the TU instead (validator #2638): the body references no other symbol (0
 *     relocations).
 */
int func_L01_003B6028(u8 *arg0) {
    char *a1 = *(char **)(arg0 + 0x68);
    int r = 1;

    if (arg0[0x21] != 0xFF) {
        if (*(int *)(a1 + 0x340) == 0) {
            int *p = *(int **)(a1 + 0x33C);
            *p = *p + 1;
        }
    }
    *(int *)(a1 + 0x340) = r;
    return r;
}
