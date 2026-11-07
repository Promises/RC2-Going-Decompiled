/* Level 2 (Maktar Nebula), USA v2.00, adopted from OpenRAC (row L02-D, task
 * #2560). Compiled ALONE as its own TU and linked at its one place, 0x0040CBF8,
 * in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm tools/ee/ee_cc1.sh
 * s136 at -O2 -G0 -fopt-stack.
 *
 * func_L02_0040CBF8 - starts the three effect tracks of the actor.
 *
 * @a0: the actor object; its state is *(a0 + 0x68).
 * Return: nothing.
 *
 * Non-obvious: all three tracks start only when the byte at +0x95 is set; the
 * two big constants per track are the track's kind and its blend words.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file src/levels/02_maktar_nebula/
 * L02_0040A4A8.c, function func_L02_0040B4B0 (v1.01 address), author commit
 * 91e04732 (llesieur99). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082). BYTE-IDENTICAL: verbatim but for names, plus:
 *   - `#include "include_asm.h"` dropped (nothing in the body used it); the
 *     u8 typedef line is written out at the top instead;
 *   - name respellings, upstream v1.01-address names -> our v2.00 split names
 *     (relverify by address, NOTE #13279's relverify.py, BAD 0):
 *     func_L02_0040B4B0 -> func_L02_0040CBF8,
 *     func_L00_003D7250 -> func_L02_0040CD30 (x3 call sites).
 */
typedef unsigned char u8;

extern void func_L02_0040CD30(void *, void *, float, int, int, int);

/* starts the three effect tracks of the actor */
void func_L02_0040CBF8(u8 *a0) {
    u8 *s1 = *(u8 **)(a0 + 0x68);

    if (a0[0x95] != 0) {
        func_L02_0040CD30(a0, s1 + 0x290, 0.125f, 0x36F6, 0x20000000, 0x20000000);
        func_L02_0040CD30(a0, s1 + 0x2A0, 0.15f, 0x1050B0, 0x30000000, 0x20000000);
        func_L02_0040CD30(a0, s1 + 0x2B0, 0.15f, 0x1050B0, 0x10000000, 0x30000000);
    }
}
