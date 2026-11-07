/* Level 01 (Oozla), USA v2.00: a level-only function built from C (row L01-G1,
 * task #2626, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003C1548, in place of its split asm; the row is
 * going-decompiled/levels/usa/L01/config/L01_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, plain as.
 */

typedef struct L01_003C0140_OWNED {
    char pad0[0x68];
    void *f68;
} L01_003C0140_OWNED;

typedef struct L01_003C0140_OBJ {
    char pad0[0x68];
    L01_003C0140_OWNED *f68;
    char pad1[0x96 - 0x6C];
    unsigned short f96;
} L01_003C0140_OBJ;

extern float D_L01_001AFEBC __attribute__((sda));

extern void func_L01_00316288(void *, void *);

extern void func_L01_002E3E18(void *);

extern void func_L01_0031CFA0(void *, void *, int, int, float);

/**
 * func_L01_003C1548 - the shared retirement step of the four Oozla-native
 * creature spawners (func_L01_003C1AF0..func_L01_003C1C38): release the
 * spawn slots, age the spawn counter and stop the animation.
 *
 * func_L01_00316288 releases the owned record's table at +0x50; the u16
 * counter at the object's +0x96 saturates at 0xFFFE; the three slots at the
 * owned record's +0x2DC/+0x2DA/+0x2DE are released through
 * func_L01_002E3E18; func_L01_0031CFA0 finally stops the animation at the
 * owned record's +0x220 with 10 frames, speed 1 and the .lit scale float
 * D_L01_001AFEBC.
 *
 * @a0: the object record (its owned state is at a0->f68).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/01_oozla/L01_003BB9D0-g8.c, function func_L01_003C0140
 * (v1.01 address), author commit 91e04732 (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2626 relverify by name):
 *   - func_L01_003C0140 -> func_L01_003C1548 (our split name);
 *   - D_L01_001AFEFC -> D_L01_001AFEBC (our committed .lit label at the
 *     v2.00 target);
 *   - func_002AB8C0 -> func_L01_00316288, func_00282978 ->
 *     func_L01_002E3E18 and func_002B0990 -> func_L01_0031CFA0 (L01's own
 *     copies at the v2.00 places);
 *   - `#include "include_asm.h"` and the file's other declarations dropped:
 *     this function references no other symbol.
 */
void func_L01_003C1548(L01_003C0140_OBJ *a0)
{
    L01_003C0140_OBJ *p = a0;
    L01_003C0140_OWNED *q = p->f68;

    func_L01_00316288(a0, (char *)q + 0x50);
    if (p->f96 <= 0xFFFE) {
        p->f96 = p->f96 + 1;
    }
    func_L01_002E3E18((char *)q + 0x2DE);
    func_L01_002E3E18((char *)q + 0x2DA);
    func_L01_002E3E18((char *)q + 0x2DC);
    func_L01_0031CFA0(p, (char *)q + 0x220, 10, 1, D_L01_001AFEBC);
}
