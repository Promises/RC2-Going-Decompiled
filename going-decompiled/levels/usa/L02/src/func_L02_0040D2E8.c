/* Level 2 (Maktar Nebula), USA v2.00, adopted from OpenRAC (row L02-D, task
 * #2560). Compiled ALONE as its own TU and linked at its one place, 0x0040D2E8,
 * in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm tools/ee/ee_cc1.sh
 * s136 at -O2 -G0 -fopt-stack.
 *
 * func_L02_0040D2E8 - records the actor's two shot counters, then arms it:
 * the kind byte at +0x20 and the counter byte at +0x95 go to the state's
 * halfwords at +0x2E4/+0x2E6, then the shared raise runs.
 *
 * @a0: the actor object.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file src/levels/02_maktar_nebula/
 * L02_0040A4A8.c, function func_L02_0040BBA0 (v1.01 address), author commit
 * ee3fee45 (llesieur99). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082). BYTE-IDENTICAL: verbatim but for names, plus:
 *   - `#include "include_asm.h"` dropped (nothing in the body used it); the
 *     u8/u16 typedef lines are written out at the top instead;
 *   - name respellings, upstream v1.01-address names -> our v2.00 split names
 *     (relverify by address, NOTE #13279's relverify.py, BAD 0):
 *     func_L02_0040BBA0 -> func_L02_0040D2E8,
 *     func_31E6A0 -> func_L02_0031F998.
 */
typedef unsigned char u8;
typedef unsigned short u16;

extern void func_L02_0031F998(void *, int, int);

/* records the actor's two shot counters, then arms it */
void func_L02_0040D2E8(u8 *a0) {
    u8 *t = *(u8 **)(a0 + 0x68);

    *(u16 *)(t + 0x2E4) = *(u8 *)(a0 + 0x20);
    *(u16 *)(t + 0x2E6) = *(u8 *)(a0 + 0x95);
    func_L02_0031F998(a0, 6, 3);
}
