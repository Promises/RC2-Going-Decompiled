/* Level 03 (Endako), USA v2.00: a level-only function built from C
 * (task #2655, RULING #13291 row L03-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003C9048, in place of its split asm; the row is
 * going-decompiled/levels/usa/L03/config/L03_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* Upstream include/common.h's own definitions of the typedefs the body uses. */
typedef unsigned short u16;
typedef signed int s32;

/* L03's copy of a boot state-change helper, at its v2.00 place. */
extern void func_L03_0031CC98(char *a0, s32 a1, s32 a2);

/**
 * func_L03_003C9048 - reset the object's private counters and switch it to
 * state 2.
 *
 * @a0: the object; its private block pointer is at +0x68.
 * Return: nothing.
 *
 * Clears the private block's halfword +0x106 and word +0x100, sets bit 0x400
 * in the object's flags halfword at +0x34, calls
 * func_L03_0031CC98(a0, 2, -1), then clears the object's word at +0x9C.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/03_endako/L03_00399230.c, function
 * func_L03_003C7A40 (v1.01 address), author commit 91e04732 (llesieur99;
 * `git blame -M -C` names it for 9 of the 10 definition lines at fe6adf9b,
 * the tenth being a blank line from upstream's root commit 7fccf735, and
 * `git log --reverse -S'func_L03_003C7A40('` agrees). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L03_003C7A40 -> func_L03_003C9048 (our split name at the
 *     v2.00 place);
 *   - callee func_002A7750 -> func_L03_0031CC98 (L03's copy, the v2.00 ROM's
 *     call target);
 *   - `#include "common.h"` dropped, its two typedefs written out; the
 *     file's other functions and externs not carried.
 */
void func_L03_003C9048(char *a0)
{
    char *v1 = *(char **)(a0 + 0x68);

    *(u16 *)(v1 + 0x106) = 0;
    *(s32 *)(v1 + 0x100) = 0;
    *(u16 *)(a0 + 0x34) |= 0x400;
    func_L03_0031CC98(a0, 2, -1);
    *(s32 *)(a0 + 0x9C) = 0;
}
