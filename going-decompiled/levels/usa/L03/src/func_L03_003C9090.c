/* Level 03 (Endako), USA v2.00: a level-only function built from C
 * (task #2655, RULING #13291 row L03-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003C9090, in place of its split asm; the row is
 * going-decompiled/levels/usa/L03/config/L03_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* Upstream include/common.h's own definitions of the typedefs the body uses. */
typedef unsigned short u16;
typedef signed int s32;

/**
 * func_L03_003C9090 - mark the object and its private block as set.
 *
 * @a0: the object; its private block pointer is at +0x68.
 * Return: nothing.
 *
 * Sets the private block's halfword +0x106 to 1 and the object's word +0x9C
 * to 1 (the counterpart of func_L03_003C9048, which clears both).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/03_endako/L03_00399230.c, function
 * func_L03_003C7A88 (v1.01 address), author commit 91e04732 (llesieur99;
 * `git blame -M -C` names it for all 7 definition lines at fe6adf9b, and
 * `git log --reverse -S'func_L03_003C7A88('` agrees). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L03_003C7A88 -> func_L03_003C9090 (our split name at the
 *     v2.00 place); no other symbol is referenced (0 relocations);
 *   - `#include "common.h"` dropped, its two typedefs written out.
 */
void func_L03_003C9090(char *a0)
{
    char *v0 = *(char **)(a0 + 0x68);

    *(u16 *)(v0 + 0x106) = 1;
    *(s32 *)(a0 + 0x9C) = 1;
}
