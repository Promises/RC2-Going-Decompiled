/* Level 2 (Maktar Nebula), USA v2.00, adopted from OpenRAC (row L02-D, task
 * #2560). Compiled ALONE as its own TU and linked at its one place, 0x0040E630,
 * in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm tools/ee/ee_cc1.sh
 * s136 at -O2 -G0 -fopt-stack.
 *
 * func_L02_0040E630 - steps the actor's state machine, then raises its
 * counter: the state walk reads the state halfword at +0x50, starts the
 * triple at +0x1A0 once when the walk is idle, and clears the pending word
 * at +0x1AC again once it resumes.
 *
 * @a0: the actor object.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file src/levels/02_maktar_nebula/
 * L02_0040A4A8.c, function func_L02_0040CEE8 (v1.01 address), author commit
 * 91e04732 (llesieur99). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082). BYTE-IDENTICAL: verbatim but for names, plus:
 *   - `#include "include_asm.h"` dropped (nothing in the body used it); the
 *     u8/u16/s16/u32 typedef lines are written out at the top instead;
 *   - name respellings, upstream v1.01-address names -> our v2.00 split names
 *     (relverify by address, NOTE #13279's relverify.py, BAD 0):
 *     func_L02_0040CEE8 -> func_L02_0040E630,
 *     func_002AB8C0 -> func_L02_003245E0,
 *     func_002A0488 -> func_L02_00317418.
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;

extern void func_L02_003245E0(void *, void *);
extern void func_L02_00317418(void *, void *, void *, void *);

/* steps the actor's state machine, then raises its counter */
void func_L02_0040E630(u8 *a0) {
    u8 *s0 = *(u8 **)(a0 + 0x68);

    func_L02_003245E0(a0, s0 + 0x50);
    if (*(s16 *)(s0 + 0x50) == 0) {
        if (*(u32 *)(s0 + 0x1AC) == 0) {
            func_L02_00317418(a0, *(u32 *)(s0 + 0x1A0), *(u32 *)(s0 + 0x1A4),
                          *(u32 *)(s0 + 0x1A8));
            *(u32 *)(s0 + 0x1AC) = 1;
        }
    } else {
        *(u32 *)(s0 + 0x1AC) = 0;
    }
    if (*(u16 *)(a0 + 0x96) <= 0xFFFE)
        *(u16 *)(a0 + 0x96) = *(u16 *)(a0 + 0x96) + 1;
}
