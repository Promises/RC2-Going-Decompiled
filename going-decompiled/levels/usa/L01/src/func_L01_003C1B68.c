/* Level 01 (Oozla), USA v2.00: a level-only function built from C (row L01-G1,
 * task #2626, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003C1B68, in place of its split asm; the row is
 * going-decompiled/levels/usa/L01/config/L01_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, plain as.
 */

typedef struct L01_003C0760_OWNED {
    char pad0[0x2DC];
    unsigned short f2DC;
} L01_003C0760_OWNED;

typedef struct L01_003C0760_OBJ {
    char pad0[0x68];
    L01_003C0760_OWNED *f68;
} L01_003C0760_OBJ;

extern int func_L01_00311748(int, int);

extern void func_L01_00311638(void *, int, int);

extern void func_L01_003C11D0(void *, int, int, int);

extern unsigned short D_L01_001AFE94 __attribute__((sda));

/**
 * func_L01_003C1B68 - retire the first odd-framed Oozla native variant: state
 * 8, group 3, a random 7..12 frame life and the .lit word D_L01_001AFE94.
 *
 * func_L01_003C11D0 (the shared setter, our place of upstream
 * func_L01_003BFDC8) receives state 8 and r = func_L01_00311748(7, 12);
 * func_L01_00311638 switches the object to group 3, speed 3; the owned
 * record's +0x2DC u16 becomes the .lit constant D_L01_001AFE94.
 *
 * @a0: the object record (its owned state is at a0->f68).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/01_oozla/L01_003BB9D0-g8.c, function func_L01_003C0760
 * (v1.01 address), author commit 91e04732 (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2626 relverify by name):
 *   - func_L01_003C0760 -> func_L01_003C1B68 (our split name);
 *   - D_L01_001AFED4 -> D_L01_001AFE94 (our committed .lit label at the
 *     v2.00 target);
 *   - func_002A7820 -> func_L01_00311748, func_002A7750 ->
 *     func_L01_00311638 and func_L01_003BFDC8 -> func_L01_003C11D0 (L01's
 *     own functions at the v2.00 places);
 *   - `#include "include_asm.h"` and the file's other declarations dropped:
 *     this function references no other symbol.
 */
void func_L01_003C1B68(L01_003C0760_OBJ *a0)
{
    L01_003C0760_OBJ *p = a0;
    L01_003C0760_OWNED *q = p->f68;
    int r = func_L01_00311748(7, 12);

    func_L01_003C11D0(p, 8, 0, r);
    func_L01_00311638(p, 3, 3);
    q->f2DC = D_L01_001AFE94;
}
