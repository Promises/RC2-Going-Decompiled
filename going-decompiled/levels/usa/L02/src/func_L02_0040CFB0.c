/* Level 2 (Maktar Nebula), USA v2.00, adopted from OpenRAC (row L02-D, task
 * #2560). Compiled ALONE as its own TU and linked at its one place, 0x0040CFB0,
 * in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm tools/ee/ee_cc1.sh
 * s136 at -O2 -G0 -fopt-stack.
 *
 * func_L02_0040CFB0 - brings the actor into its armed state: converts its
 * short at state+0x24 to the float at state+0x20, runs the shared four-float
 * tune, raises the mode, copies the type record's word at +0x10 to +0x98,
 * sets the armed/shown bytes, applies the flag mask at +0x34, stores a1 to
 * state+0x2E8, and releases a pending slot record.
 *
 * @a0: the actor object; @a1: stored to the state halfword at +0x2E8.
 * Return: nothing.
 *
 * Non-obvious: the byte at +0x7F is cleared between reading the flags
 * halfword and masking it, as retail does.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file src/levels/02_maktar_nebula/
 * L02_0040A4A8.c, function func_L02_0040B868 (v1.01 address), author commit
 * 91e04732 (llesieur99). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082). BYTE-IDENTICAL: verbatim but for names, plus:
 *   - `#include "include_asm.h"` dropped (nothing in the body used it); the
 *     u8/u16/s16/u32 typedef lines are written out at the top instead;
 *   - name respellings, upstream v1.01-address names -> our v2.00 split names
 *     (relverify by address, NOTE #13279's relverify.py, BAD 0):
 *     func_L02_0040B868 -> func_L02_0040CFB0,
 *     func_002B60B0 -> func_L02_00333AC0,
 *     func_002A7750 -> func_L02_0031F998 (upstream declares it with an empty
 *       parameter list in this unit; kept verbatim),
 *     func_002A7470 -> func_L02_0031F6B8,
 *     func_002A0488 -> func_L02_00317418.
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;

extern void func_L02_00333AC0(u8 *, float, float, float, float);
extern void func_L02_0031F998();
extern void func_L02_0031F6B8(void *, int, int, int);
extern void func_L02_00317418(void *, void *, void *, void *);

/* brings the actor into its armed state */
void func_L02_0040CFB0(u8 *a0, int a1) {
    u8 *s0 = *(u8 **)(a0 + 0x68);
    int one = 1;
    u32 v;

    *(float *)(s0 + 0x20) = (float)*(s16 *)(s0 + 0x24);
    *(u16 *)(s0 + 0x2E2) = one;
    func_L02_00333AC0(a0, 0.004444444552063942f, 0.13333334028720856f,
                  0.0011635529808700085f, 0.10471977293491364f);
    func_L02_0031F998(a0, 3, 1);
    if (*(u8 *)(a0 + 0x43) != 2)
        func_L02_0031F6B8(a0, 2, 0, 0);
    *(u32 *)(a0 + 0x98) = *(u32 *)(*(u32 *)(a0 + 0x24) + 0x10);
    *(u8 *)(s0 + 0x6A) = 0;
    *(u8 *)(s0 + 0x2E) = one;
    *(u8 *)(a0 + 0x31) = one;
    v = *(u16 *)(a0 + 0x34);
    *(u8 *)(a0 + 0x7F) = 0;
    *(u16 *)(a0 + 0x34) = (v | 0x1000) & 0xFFB8;
    *(u16 *)(s0 + 0x2E8) = a1;
    *(u16 *)(s0 + 0x88) = 0;
    if (*(s16 *)(s0 + 0x50) != 0) {
        *(u16 *)(s0 + 0x50) = 0;
        func_L02_00317418(a0, *(u8 *)(s0 + 0x54), *(u8 *)(s0 + 0x55), *(u8 *)(s0 + 0x56));
        *(u16 *)(s0 + 0x52) = one;
    }
}
