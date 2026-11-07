/* Level 03 (Endako), USA v2.00: a level-only function built from C
 * (task #2655, RULING #13291 row L03-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003D0DC0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L03/config/L03_c.tsv, the arm Sony 2.9-ee-991111
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

/* Upstream include/common.h's own definitions of the typedefs the body uses. */
typedef unsigned char u8;
typedef signed int s32;

/**
 * func_L03_003D0DC0 - test whether the object's state byte is 2, 3 or 4.
 *
 * @a0: the object; its state byte is at +0x20.
 * Return: 1 when the byte is 2..4, else 0.
 *
 * Non-obvious: one unsigned compare, (u8)(state - 2) < 3.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/03_endako/L03_00399230.c, function
 * func_L03_003CF7B8 (v1.01 address), author commit 91e04732 (llesieur99;
 * `git blame -M -C` names it for all 4 definition lines at fe6adf9b, and
 * `git log --reverse -S'func_L03_003CF7B8('` agrees). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L03_003CF7B8 -> func_L03_003D0DC0 (our split name at the
 *     v2.00 place); no other symbol is referenced (0 relocations);
 *   - `#include "common.h"` dropped, its two typedefs written out.
 */
s32 func_L03_003D0DC0(u8 *a0)
{
    return (u8)(a0[0x20] - 2) < 3;
}
