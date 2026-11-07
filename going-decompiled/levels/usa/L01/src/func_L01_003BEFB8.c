/* Level 01 (Oozla), USA v2.00: a level-only function built from C (row L01-G1,
 * task #2626, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003BEFB8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L01/config/L01_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef struct L01_003BDBB0_OWNED {
    char pad0[0x250];
    void *f250;
    int f254;
} L01_003BDBB0_OWNED;

typedef struct L01_003BDBB0_OBJ {
    char pad0[0x68];
    L01_003BDBB0_OWNED *f68;
} L01_003BDBB0_OBJ;

extern int func_L01_002E3DE8(void *);

/**
 * func_L01_003BEFB8 - release the owned record's pending pointer once and
 * clear its pending word.
 *
 * When the owned state's +0x254 word is set, the pointer at +0x250 goes to
 * func_L01_002E3DE8 (a timer/loop release) and the word is cleared; nothing
 * happens otherwise.
 *
 * @a0: the object record (its owned state is at a0->f68).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/01_oozla/L01_003BB9D0.c, function func_L01_003BDBB0 (v1.01
 * address), author commit 91e04732 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2626 relverify by name):
 *   - func_L01_003BDBB0 -> func_L01_003BEFB8 (our split name);
 *   - func_00282948 -> func_L01_002E3DE8 (L01's own copy at the v2.00 place);
 *   - `#include "common.h"` and the file's other declarations dropped: this
 *     function references no other symbol.
 */
void func_L01_003BEFB8(L01_003BDBB0_OBJ *a0)
{
    L01_003BDBB0_OWNED *p = a0->f68;

    if (p->f254 != 0) {
        func_L01_002E3DE8(p->f250);
        p->f254 = 0;
    }
}
