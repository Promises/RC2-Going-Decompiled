/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x004565B8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;
typedef unsigned int u32;

/* L11's own function and its copies of boot code, at their v2.00 places.
 * func_L11_0032DBF8 is left unprototyped, as upstream declares it. */
extern int func_L11_004430B8(char *p, int id);
extern void func_L11_0032DBF8();
extern int func_L11_0033DAC8(void *, u32, float, float, int, int);

/**
 * func_L11_004565B8 - keep the actor's tracking flag and target id current.
 *
 * @a0: the actor; its state block s1 is at +0x68, its stage byte at +0x20.
 *
 * Below stage 4 it clears s1+0x1EC and s1+0x1C4. Otherwise, when tracking
 * (s1+0x1EC == 1), or when byte +0x31 is set and func_L11_004430B8 accepts
 * the actor's group and id, it sets tracking, stores
 * func_L11_0033DAC8(a0, s1+0x1E4, 100.0f, 100.0f, 0, 0) at s1+0x1EA and
 * copies s1+0x160 to s1+0x1C4. Else it clears tracking, and in stage 0xB or
 * 8 calls func_L11_0032DBF8(a0, 7, 3) when s1+0x1C0 is not -1, or
 * func_L11_0032DBF8(a0, 5, 2) when it is.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_00453358.c, function
 * func_L11_00454B60 (v1.01 address), author commit 5964c620 (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291).
 * ADAPTED, the body's statements unchanged (verbatim but for names; no
 * device); edits:
 *   - name func_L11_00454B60 -> func_L11_004565B8 (our split name at the
 *     v2.00 place);
 *   - func_L11_00441868 -> func_L11_004430B8, func_002B3BA8_4b60 ->
 *     func_L11_0033DAC8, func_002A7750_4b60 -> func_L11_0032DBF8: the targets
 *     of the v2.00 ROM's own relocations (relverify, task #2553). Upstream's
 *     _4b60 suffixes only gave this function's prototypes their own names;
 *   - the audit's `long long` -> `long` respelling (RULING #13197 class,
 *     needed because cc1 s136 refuses the file's other `long long` uses) is
 *     NOT applied: this one-function TU carries no `long long`;
 *   - `#include "common.h"` dropped; the typedefs used written out; the
 *     upstream one-line comment ("same body as func_L02_0040CF68") not
 *     carried; the file's other functions and their externs not carried.
 */
void func_L11_004565B8(u8 *a0) {
    u8 *s1 = *(u8 **)(a0 + 0x68);

    if (a0[0x20] < 4) {
        *(u16 *)(s1 + 0x1EC) = 0;
        *(s32 *)(s1 + 0x1C4) = 0;
    } else {
        if (*(s16 *)(s1 + 0x1EC) == 1) {
            *(s16 *)(s1 + 0x1EA) = func_L11_0033DAC8(a0, *(u32 *)(s1 + 0x1E4), 100.0f, 100.0f, 0, 0);
            *(s32 *)(s1 + 0x1C4) = *(s32 *)(s1 + 0x160);
        } else {
            if (a0[0x31] != 0 && func_L11_004430B8(*(void **)(a0 + 0xB8), *(s16 *)(a0 + 0xAA)) != 0) {
                *(u16 *)(s1 + 0x1EC) = 1;
                *(s16 *)(s1 + 0x1EA) = func_L11_0033DAC8(a0, *(u32 *)(s1 + 0x1E4), 100.0f, 100.0f, 0, 0);
                *(s32 *)(s1 + 0x1C4) = *(s32 *)(s1 + 0x160);
            } else {
                *(u16 *)(s1 + 0x1EC) = 0;
                if (a0[0x20] == 0xB || a0[0x20] == 8) {
                    if (*(s32 *)(s1 + 0x1C0) != -1)
                        func_L11_0032DBF8(a0, 7, 3);
                    else
                        func_L11_0032DBF8(a0, 5, 2);
                }
            }
        }
    }
}
