/* Level 01 (Oozla), USA v2.00: a level-only function built from C (row L01-G1,
 * task #2626, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003C1C38, in place of its split asm; the row is
 * going-decompiled/levels/usa/L01/config/L01_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, plain as.
 */

typedef struct L01_003C0830_OWNED {
    char pad0[0x2DE];
    unsigned short f2DE;
} L01_003C0830_OWNED;

typedef struct L01_003C0830_OBJ {
    char pad0[0x68];
    L01_003C0830_OWNED *f68;
    char pad1[0x9C - 0x6C];
    int f9C;
} L01_003C0830_OBJ;

extern int func_L01_00311748(int, int);

extern void func_L01_00311638(void *, int, int);

extern void func_L01_003C11D0(void *, int, int, int);

extern unsigned short D_L01_001AFE9C __attribute__((sda));

/**
 * func_L01_003C1C38 - retire the third odd-framed Oozla native variant: state
 * 14, group 4, the +0x9C flag word raised and the .lit word D_L01_001AFE9C.
 *
 * func_L01_003C11D0 (the shared setter, our place of upstream
 * func_L01_003BFDC8) receives state 14 and r = func_L01_00311748(7, 11);
 * func_L01_00311638 switches the object to group 4, speed 3; this variant
 * also ORs 0x30 into the object's +0x9C flag word (its only step beyond the
 * sibling bodies); the owned record's +0x2DE u16 becomes the .lit constant
 * D_L01_001AFE9C.
 *
 * @a0: the object record (its owned state is at a0->f68).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/01_oozla/L01_003BB9D0-g8.c, function func_L01_003C0830
 * (v1.01 address), author commit 91e04732 (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2626 relverify by name):
 *   - func_L01_003C0830 -> func_L01_003C1C38 (our split name);
 *   - D_L01_001AFEDC -> D_L01_001AFE9C (our committed .lit label at the
 *     v2.00 target);
 *   - func_002A7820 -> func_L01_00311748, func_002A7750 ->
 *     func_L01_00311638 and func_L01_003BFDC8 -> func_L01_003C11D0 (L01's
 *     own functions at the v2.00 places);
 *   - `#include "include_asm.h"` and the file's other declarations dropped:
 *     this function references no other symbol.
 */
void func_L01_003C1C38(L01_003C0830_OBJ *a0)
{
    L01_003C0830_OBJ *p = a0;
    L01_003C0830_OWNED *q = p->f68;
    int r = func_L01_00311748(7, 11);

    func_L01_003C11D0(p, 14, 0, r);
    func_L01_00311638(p, 4, 3);
    p->f9C = p->f9C | 0x30;
    q->f2DE = D_L01_001AFE9C;
}
