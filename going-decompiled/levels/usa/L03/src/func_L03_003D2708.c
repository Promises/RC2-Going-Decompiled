/* Level 03 (Endako), USA v2.00: a level-only function built from C
 * (task #2655, RULING #13291 row L03-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003D2708, in place of its split asm; the row is
 * going-decompiled/levels/usa/L03/config/L03_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* Upstream include/common.h's own definitions of the typedefs the body uses. */
typedef unsigned char u8;
typedef signed short s16;
typedef signed int s32;

/* L03's copies of boot and level helpers, at their v2.00 places. */
extern void func_L03_0031CC98(char *a0, s32 a1, s32 a2);
extern void *func_L03_0031F0F0(char *a0, s32 a1, s32 a2);
extern s32  func_L03_0032BF38(char *a0);
extern s32  func_L03_00361DC8(s32 a0, s32 a1, char *a2);
extern void func_L03_00312AA0(void *a0);
extern void func_L03_003D27E8(char *a0);

/**
 * func_L03_003D2708 - two-state update: arm the object, then free it once it
 * is triggered.
 *
 * @a0: the object; its block pointer is at +0x68, its state byte at +0x20.
 * Return: nothing.
 *
 * State 0 sets bit 3 of the block's flags at +0x5C and switches to state 1
 * (func_L03_0031CC98(a0, 1, -1)). State 1 fires when func_L03_0032BF38(a0)
 * is non-zero, or when func_L03_0031F0F0(a0, 0x10000, 0) finds a record whose
 * +0x20 points at a moby of class 0xB1B (halfword +0xAA). Firing runs
 * func_L03_003D27E8(a0), func_L03_00361DC8(0, 0, a0) and frees the object
 * (func_L03_00312AA0); otherwise its byte +0xA8 is set to 0xFF.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/03_endako/L03_00399230.c, function
 * func_L03_003D1100 (v1.01 address), author commit 91e04732 (llesieur99;
 * `git blame -M -C` names it for all 26 definition lines at fe6adf9b, and
 * `git log --reverse -S'func_L03_003D1100('` agrees). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L03_003D1100 -> func_L03_003D2708 (our split name at the
 *     v2.00 place);
 *   - six callees as L03's copies, the v2.00 ROM's call targets:
 *     func_002A7750 -> func_L03_0031CC98, func_31DD78 -> func_L03_0031F0F0,
 *     func_32AB10 -> func_L03_0032BF38, func_360A80 -> func_L03_00361DC8,
 *     func_311758 -> func_L03_00312AA0, func_L03_003D11E0 ->
 *     func_L03_003D27E8;
 *   - `#include "common.h"` dropped, its three typedefs written out.
 */
void func_L03_003D2708(char *a0)
{
    char *a3 = *(char **)(a0 + 0x68);

    switch (*(u8 *)(a0 + 0x20)) {
    case 0:
        *(s32 *)(a3 + 0x5C) |= 8;
        func_L03_0031CC98(a0, 1, -1);
        break;
    case 1: {
        char *s1 = (char *)func_L03_0031F0F0(a0, 0x10000, 0);
        s32 v;

        if (func_L03_0032BF38(a0) != 0
            || (s1 != 0 && (v = *(s32 *)(s1 + 0x20)) != 0
                && *(s16 *)(v + 0xAA) == 0xB1B)) {
            func_L03_003D27E8(a0);
            func_L03_00361DC8(0, 0, a0);
            func_L03_00312AA0(a0);
        } else {
            *(u8 *)(a0 + 0xA8) = 0xFF;
        }
        break;
    }
    }
}
