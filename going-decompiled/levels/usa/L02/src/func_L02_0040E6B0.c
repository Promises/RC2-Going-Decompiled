/* Level 2 (Maktar Nebula), USA v2.00, adopted from OpenRAC (row L02-D, task
 * #2560). Compiled ALONE as its own TU and linked at its one place, 0x0040E6B0,
 * in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm tools/ee/ee_cc1.sh
 * s136 at -O2 -G0 -fopt-stack.
 *
 * func_L02_0040E6B0 - updates the actor's halfword at state+0x1B8: below
 * kind 4 it only clears it and the word at +0x184; while it is 1, or when the
 * flag at +0x31 is set and func_L02_003FD670 agrees, it keeps it at 1 and
 * calls func_L02_003309B0 on the slot at +0x1B0; otherwise it clears it and,
 * for kinds 8 and 0xB, picks one of two states by the word at +0x180.
 *
 * @a0: the actor object.
 * Return: nothing.
 *
 * Non-obvious: the lone `&&`-free label compare class — no jump table here;
 * the kind byte at +0x20 is re-read as a byte in the 0xB/8 test after the
 * halfword tests, as retail does.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file src/levels/02_maktar_nebula/
 * L02_0040A4A8.c, function func_L02_0040CF68 (v1.01 address), author commit
 * 7a7816b9 (bigmak94). Adopted with the authors' permission (RULINGs #10405,
 * #12108, #13082). BYTE-IDENTICAL: verbatim but for names, plus:
 *   - `#include "include_asm.h"` dropped (nothing in the body used it); the
 *     u8/u16/s16/u32/s32 typedef lines are written out at the top instead;
 *   - name respellings, upstream v1.01-address names -> our v2.00 split names
 *     (relverify by address, NOTE #13279's relverify.py, BAD 0):
 *     func_L02_0040CF68 -> func_L02_0040E6B0,
 *     func_002B3BA8 -> func_L02_003309B0,
 *     func_L02_003FC0D0 -> func_L02_003FD670,
 *     func_002A7750 -> func_L02_0031F998 (upstream declares it with an empty
 *       parameter list in this unit; kept verbatim).
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;

extern int func_L02_003309B0(void *, u32, float, float, int, int);
extern int func_L02_003FD670(void *, int);
extern void func_L02_0031F998();

void func_L02_0040E6B0(u8 *a0) {
    u8 *s1 = *(u8 **)(a0 + 0x68);

    if (a0[0x20] < 4) {
        *(u16 *)(s1 + 0x1B8) = 0;
        *(s32 *)(s1 + 0x184) = 0;
    } else {
        if (*(s16 *)(s1 + 0x1B8) == 1) {
            *(s16 *)(s1 + 0x1B6) = func_L02_003309B0(a0, *(u32 *)(s1 + 0x1B0), 100.0f, 100.0f, 0, 0);
            *(s32 *)(s1 + 0x184) = *(s32 *)(s1 + 0x160);
        } else {
            if (a0[0x31] != 0 && func_L02_003FD670(*(void **)(a0 + 0xB8), *(s16 *)(a0 + 0xAA)) != 0) {
                *(u16 *)(s1 + 0x1B8) = 1;
                *(s16 *)(s1 + 0x1B6) = func_L02_003309B0(a0, *(u32 *)(s1 + 0x1B0), 100.0f, 100.0f, 0, 0);
                *(s32 *)(s1 + 0x184) = *(s32 *)(s1 + 0x160);
            } else {
                *(u16 *)(s1 + 0x1B8) = 0;
                if (a0[0x20] == 0xB || a0[0x20] == 8) {
                    if (*(s32 *)(s1 + 0x180) != -1)
                        func_L02_0031F998(a0, 7, 3);
                    else
                        func_L02_0031F998(a0, 5, 2);
                }
            }
        }
    }
}
