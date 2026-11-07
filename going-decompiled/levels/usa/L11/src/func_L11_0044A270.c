/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x0044A270, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned short u16;
typedef signed int s32;
/* EE `long` is 64-bit; RULING #13197 spells it s64 in adopted bodies. */
typedef long s64;

/* The level's copies of boot helpers, named by their v2.00 places in L11. */
extern void func_L11_003374C0(char *a0, s32 a1, int a2, int a3);
extern void func_L11_00300E80(void *a0);
extern void func_L11_003395B0(char *a0, char *a1, float a2, int a3, int a4);
extern void func_L11_0032F698(char *a0, char *a1, float f12, float f13, float f14,
                              int a2, int a3, int a4, int a5);

/**
 * func_L11_0044A270 - the object's hit handler: reset its state block, let go
 * of its linked object, count the hit and start the effects.
 *
 * Calls func_L11_00300E80 on the block at state+0x364. When the state links
 * an object (+0x354), calls func_L11_003374C0(a0, linked, 2, 0) and clears
 * flag bit 0x2 in the linked object's halfword +0x34. Increments the 16-bit
 * counter at +0x96 below 0xFFFF, then calls func_L11_003395B0 on state+0x230
 * (3.5, 4, 1) and func_L11_0032F698 at the position (+0x10, mask 0x4000000).
 *
 * @a0: the object; its state is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention; changed upstream since
 * b2a35115), file src/levels/11_joba/L11_004465F8.c, function
 * func_L11_00448848 (v1.01 address), author commit fa8d6672 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED (no device): verbatim but for names and one type spelling; the
 * respellings, each name to the target of the v2.00 disc's own relocation at
 * this place (task #2555 relverify):
 *   - func_L11_00448848 -> func_L11_0044A270 (our split name at the v2.00 place);
 *   - func_300180 -> func_L11_00300E80, func_3362A8 -> func_L11_003374C0,
 *     func_338398 -> func_L11_003395B0, func_32E480 -> func_L11_0032F698;
 *   - the pointer cast's `(long)` is written `(s64)` (RULING #13197), with
 *     `typedef long s64` here: the same type, so codegen-neutral by
 *     construction (the level gate's .text cmp 0 shows the bytes);
 *   - `#include "common.h"` and `#include "include_asm.h"` dropped: the u16/s32
 *     typedefs common.h supplied are written here, and only the declarations
 *     this function uses are kept.
 */
void func_L11_0044A270(char *a0)
{
    char *s1 = *(char **)(a0 + 0x68);

    func_L11_00300E80(s1 + 0x364);
    if (*(s32 *)(s1 + 0x354) != 0) {
        func_L11_003374C0(a0, *(s32 *)(s1 + 0x354), 2, 0);
        *(u16 *)((char *)(s64)*(s32 *)(s1 + 0x354) + 0x34) &= 0xFFFD;
    }
    if (*(u16 *)(a0 + 0x96) < 0xFFFF)
        *(u16 *)(a0 + 0x96) = *(u16 *)(a0 + 0x96) + 1;
    func_L11_003395B0(a0, s1 + 0x230, 3.5f, 4, 1);
    func_L11_0032F698(a0, a0 + 0x10, 1.5f, 1.0f, 1.0f, 0x4000000, 0, 1, 0);
}
