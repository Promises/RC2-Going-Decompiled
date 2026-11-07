/* Level 01 (Oozla), USA v2.00: a level-only function built from C (row L01-G1,
 * task #2626, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003BEF30, in place of its split asm; the row is
 * going-decompiled/levels/usa/L01/config/L01_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef struct {
    char pad0[0x280];
    int f280;
    int f284;
} L01_003BDB28_STATE;

extern void func_L01_003127B8(int, int);

/**
 * func_L01_003BEF30 - release the object's own group index and the two
 * pending indices of its state, then mark it released.
 *
 * For a grouped object (+0x21 != 0xFF) its own index is released first; the
 * state's +0x280 and +0x284 slots follow when set. Upstream spells the third
 * release with f280 again (not f284) -- that is what the ROM does at this
 * place and the body keeps it verbatim. The +0xBC byte is set to 1 in every
 * case (the released mark func_L01_003B59E0 checks).
 *
 * @obj: the object record (its state is at *(L01_003BDB28_STATE **)(obj + 0x68)).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/01_oozla/L01_003BB9D0.c, function func_L01_003BDB28 (v1.01
 * address), author commit 4541c4ce (ReaperOfVeriod). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2626 relverify by name):
 *   - func_L01_003BDB28 -> func_L01_003BEF30 (our split name);
 *   - func_002A8598 -> func_L01_003127B8 (L01's own copy at the v2.00 place);
 *   - `#include "common.h"` and the file's other declarations dropped: this
 *     function references no other symbol.
 */
void func_L01_003BEF30(char *obj)
{
    L01_003BDB28_STATE *s0 = *(L01_003BDB28_STATE **)(obj + 0x68);

    if (*(unsigned char *)(obj + 0x21) != 0xFF) {
        func_L01_003127B8(*(unsigned char *)(obj + 0x21), 1);
    }
    if (s0->f280 != -1) {
        func_L01_003127B8(s0->f280, 1);
    }
    if (s0->f284 != -1) {
        func_L01_003127B8(s0->f280, 1);
    }
    *(unsigned char *)(obj + 0xBC) = 1;
}
