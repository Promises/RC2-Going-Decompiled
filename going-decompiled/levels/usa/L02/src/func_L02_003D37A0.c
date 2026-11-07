/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003D37A0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G0.
 */

extern void func_L02_0031F998(char *, int, int);
extern void func_L02_0031F6B8(char *, int, int, int);
extern void func_L02_00333AC0(float, float, float, float);
extern void func_L02_00317418(char *, int, int, int);
extern void func_L02_00317448(char *, int *, int *, int *);
extern void func_L02_003D3398(char *);

/**
 * func_L02_003D37A0 - bring the actor into its armed state and re-register
 * its sub-object.
 * @arg0: the actor; its state block is the pointer at +0x68.
 * @arg1: stored to the state's half-word at +0x2FC.
 *
 * Clears the state's +0x2F6 and copies its short at +0x24 into the float at
 * +0x20, calls func_L02_00333AC0 with four float constants, then
 * func_L02_0031F998(@arg0, 3, 1) and, unless +0x43 is already 0,
 * func_L02_0031F6B8(@arg0, 0, 0, 10). It copies the word at (+0x24)->+0x10
 * into +0x98, resets four bytes, sets bit 0x1000 and clears bits 0x47 of the
 * half-word at +0x34, and re-creates the sub-object (func_L02_003D3398).
 * When the state's +0x50 is set, it is cleared, its three bytes at +0x54..56
 * go to func_L02_00317418 and +0x52 is set to 1. A live sub-object at +0x2E4
 * then gets the actor's three values read back by func_L02_00317448, through
 * func_L02_00317418.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_002ED7B0.c, function func_L02_003D2248
 * (v1.01 address), author commit 431ac503 (llesieur99). Lombyte mentions in
 * the file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). BYTE-IDENTICAL: verbatim but for names, at upstream's -G0;
 * respellings:
 *   - name func_L02_003D2248 -> func_L02_003D37A0 (our split name at the
 *     v2.00 place);
 *   - func_332710 -> func_L02_00333AC0, func_31E6A0 -> func_L02_0031F998,
 *     func_31E3C0 -> func_L02_0031F6B8, func_L02_003D1E40 ->
 *     func_L02_003D3398, func_316150 -> func_L02_00317418, func_316180 ->
 *     func_L02_00317448: our split names at the v2.00 addresses the ROM's
 *     own relocations imply (relverify, task #2557);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped (nothing
 *     here used them), every other declaration of the file dropped (the
 *     sibling func_L02_003D1E40 is declared from its definition's prototype)
 *     and the upstream comment folded into this one.
 */
void func_L02_003D37A0(char *arg0, int arg1) {
    char *p = *(char **)(arg0 + 0x68);

    *(short *)(p + 0x2F6) = 0;
    *(float *)(p + 0x20) = (float)*(short *)(p + 0x24);
    func_L02_00333AC0(0.0055555556900799274f, 0.10333333909511566f, 0.0034906589426100254f,
                0.41887909173965454f);
    func_L02_0031F998(arg0, 3, 1);
    if (*(unsigned char *)(arg0 + 0x43) != 0)
        func_L02_0031F6B8(arg0, 0, 0, 0xA);
    *(int *)(arg0 + 0x98) = *(int *)(*(char **)(arg0 + 0x24) + 0x10);
    *(unsigned char *)(p + 0x6A) = 0;
    *(unsigned char *)(p + 0x2E) = 1;
    *(unsigned char *)(arg0 + 0x7F) = 0;
    *(unsigned char *)(arg0 + 0x31) = 1;
    *(unsigned short *)(arg0 + 0x34) = (*(unsigned short *)(arg0 + 0x34) | 0x1000) & 0xFFB8;
    func_L02_003D3398(arg0);
    *(short *)(p + 0x2FC) = arg1;
    *(short *)(p + 0x88) = 0;
    if (*(short *)(p + 0x50) != 0) {
        *(short *)(p + 0x50) = 0;
        func_L02_00317418(arg0, *(unsigned char *)(p + 0x54), *(unsigned char *)(p + 0x55),
                    *(unsigned char *)(p + 0x56));
        *(short *)(p + 0x52) = 1;
    }
    if (*(char **)(p + 0x2E4) != 0) {
        int a, b, c;

        func_L02_00317448(arg0, &a, &b, &c);
        func_L02_00317418(*(char **)(p + 0x2E4), a, b, c);
    }
}
