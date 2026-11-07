/* Level 03 (Endako), USA v2.00: a level-only function built from C
 * (task #2655, RULING #13291 row L03-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003D1138, in place of its split asm; the row is
 * going-decompiled/levels/usa/L03/config/L03_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* Upstream include/common.h's own definitions of the typedefs the body uses. */
typedef unsigned char u8;
typedef signed int s32;

/* L03's copies of boot and level helpers, at their v2.00 places. The second
 * is declared without a prototype upstream: its call passes eleven
 * arguments. */
extern void func_L03_0031CC98(char *a0, s32 a1, s32 a2);
extern void func_L03_0032D438();
extern void func_L03_0032D750(char *a0, char *a1, s32 a2);

/**
 * func_L03_003D1138 - two-state update: set up the moby's path, then follow
 * it.
 *
 * @a0: the object; its moby block pointer is at +0x68, its state byte at
 *      +0x20.
 * Return: nothing.
 *
 * State 0 sets bit 0 of the block's flags at +0x5C, starts the path record at
 * block+0x60 (func_L03_0032D438 with nine zero arguments) and switches to
 * state 1 (func_L03_0031CC98(a0, 1, -1)). State 1 steps that record
 * (func_L03_0032D750(a0, block + 0x60, 0)).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/03_endako/L03_00399230.c, function
 * func_L03_003CFB30 (v1.01 address), author commit 91e04732 (llesieur99;
 * `git blame -M -C` names it for all 15 definition lines at fe6adf9b, and
 * `git log --reverse -S'func_L03_003CFB30('` agrees). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L03_003CFB30 -> func_L03_003D1138 (our split name at the
 *     v2.00 place);
 *   - three callees as L03's copies, the v2.00 ROM's call targets:
 *     func_002A7750 -> func_L03_0031CC98, func_32C010 -> func_L03_0032D438,
 *     func_32C328 -> func_L03_0032D750;
 *   - `#include "common.h"` dropped, its two typedefs written out.
 */
void func_L03_003D1138(char *a0)
{
    char *moby = *(char **)(a0 + 0x68);

    switch (*(u8 *)(a0 + 0x20)) {
    case 0:
        *(s32 *)(moby + 0x5C) |= 1;
        func_L03_0032D438(a0, moby + 0x60, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        func_L03_0031CC98(a0, 1, -1);
        break;
    case 1:
        func_L03_0032D750(a0, moby + 0x60, 0);
        break;
    }
}
