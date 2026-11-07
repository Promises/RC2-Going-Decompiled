/* Level 2 (Maktar Nebula), USA v2.00, adopted from OpenRAC (row L02-D, task
 * #2560). Compiled ALONE as its own TU and linked at its one place, 0x0040CB68,
 * in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm tools/ee/ee_cc1.sh
 * s136 at -O2 -G0 -fopt-stack.
 *
 * func_L02_0040CB68 - arms one actor: raises its counter, then two setup
 * calls.
 *
 * @a0: the actor object; its state is *(a0 + 0x68).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file src/levels/02_maktar_nebula/
 * L02_0040A4A8.c, function func_L02_0040B420 (v1.01 address), author commit
 * 91e04732 (llesieur99). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082). BYTE-IDENTICAL: verbatim but for names, plus:
 *   - `#include "include_asm.h"` dropped (nothing in the body used it); the
 *     u8/u16 typedef lines are written out at the top instead;
 *   - name respellings, upstream v1.01-address names -> our v2.00 split names
 *     (relverify by address, NOTE #13279's relverify.py, BAD 0):
 *     func_L02_0040B420 -> func_L02_0040CB68,
 *     func_002B0990 -> func_L02_0032C1D0,
 *     func_002A8C28 -> func_L02_003214D8.
 */
typedef unsigned char u8;
typedef unsigned short u16;

extern void func_L02_0032C1D0(void *, void *, int, int, float);
extern void func_L02_003214D8(void *, void *, int, float, float, float, int, int, int);

/* arms one actor: raises its counter, then two setup calls */
void func_L02_0040CB68(u8 *a0) {
    u8 *q = *(u8 **)(a0 + 0x68);

    if (*(u16 *)(a0 + 0x96) <= 0xFFFE)
        *(u16 *)(a0 + 0x96) = *(u16 *)(a0 + 0x96) + 1;
    func_L02_0032C1D0(a0, q + 0x210, 5, 1, 2.5f);
    func_L02_003214D8(a0, a0 + 0x10, 0x4000000, 1.3f, 1.0f, 1.0f, 0, 1, 0);
}
