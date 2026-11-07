/* Level 2 (Maktar Nebula), USA v2.00, adopted from OpenRAC (row L02-D, task
 * #2560). Compiled ALONE as its own TU and linked at its one place, 0x0040E7C8,
 * in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm tools/ee/ee_cc1.sh
 * s136 at -O2 -G0 -fopt-stack.
 *
 * func_L02_0040E7C8 - resets the actor's aim: three records and the two
 * shared limits; stores a1 to the state halfword at +0x1BA.
 *
 * @a0: the actor object; @a1: the aim mode stored at state+0x1BA.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file src/levels/02_maktar_nebula/
 * L02_0040A4A8.c, function func_L02_0040D080 (v1.01 address), author commit
 * 91e04732 (llesieur99). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082). BYTE-IDENTICAL: verbatim but for names, plus:
 *   - `#include "include_asm.h"` dropped (nothing in the body used it); the
 *     u8/u16/s16/u32 typedef lines are written out at the top instead;
 *   - name respellings, upstream v1.01-address names -> our v2.00 split names
 *     (relverify by address, NOTE #13279's relverify.py, BAD 0):
 *     func_L02_0040D080 -> func_L02_0040E7C8,
 *     func_002A7750 -> func_L02_0031F998 (upstream declares it with an empty
 *       parameter list in this unit; kept verbatim),
 *     func_002A7470 -> func_L02_0031F6B8,
 *     func_002A7878 -> func_L02_0031FB00,
 *     func_002A0488 -> func_L02_00317418.
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;

extern void func_L02_0031F998();
extern void func_L02_0031F6B8(void *, int, int, int);
extern float func_L02_0031FB00(float, float);
extern void func_L02_00317418(void *, void *, void *, void *);

/* resets the actor's aim: three records and the two shared limits */
void func_L02_0040E7C8(u8 *a0, int a1) {
    u8 *s0 = *(u8 **)(a0 + 0x68);

    *(u16 *)(s0 + 0x1B8) = 0;
    *(float *)(s0 + 0x20) = (float)*(s16 *)(s0 + 0x24);
    func_L02_0031F998(a0, 3, 1);
    if (*(u8 *)(a0 + 0x43) != 0)
        func_L02_0031F6B8(a0, 0, 0, 10);
    *(u32 *)(a0 + 0x98) = *(u32 *)(*(u32 *)(a0 + 0x24) + 0x10);
    *(u8 *)(s0 + 0x2E) = 1;
    *(u8 *)(a0 + 0x7F) = 0;
    *(u8 *)(a0 + 0x31) = 1;
    *(u16 *)(a0 + 0x34) = (*(u16 *)(a0 + 0x34) | 0x1000) & 0xFFB8;
    *(float *)(s0 + 0x18C) = func_L02_0031FB00(0.0f, 90.0f) * 0.01745329424738884f;
    *(u16 *)(s0 + 0x1BA) = a1;
    *(u16 *)(s0 + 0x3C) = 0;
    *(u32 *)(s0 + 0x38) = 0;
    if (*(s16 *)(s0 + 0x50) != 0) {
        *(u16 *)(s0 + 0x50) = 0;
        func_L02_00317418(a0, *(u32 *)(s0 + 0x1A0), *(u32 *)(s0 + 0x1A4),
                      *(u32 *)(s0 + 0x1A8));
        *(u16 *)(s0 + 0x52) = 1;
    }
}
