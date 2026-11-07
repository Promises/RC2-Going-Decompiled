/* Level 03 (Endako), USA v2.00: a level-only function built from C
 * (task #2655, RULING #13291 row L03-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003D35B8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L03/config/L03_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* Upstream include/common.h's own definitions of the typedefs the body uses. */
typedef unsigned short u16;
typedef signed int s32;

/* L03's copies of boot helpers, at their v2.00 places. */
extern void func_L03_003254E0(s32 a0, s32 a1);
extern void func_L03_0030EA10(s32 a0, s32 a1);

/**
 * func_L03_003D35B8 - finish the object's sequence and hand its linked moby
 * back.
 *
 * @a0: the object; its block pointer is at +0x68.
 * Return: nothing.
 *
 * Calls func_L03_003254E0 with the block's words +0x18 and +0x1C and
 * func_L03_0030EA10(0, -1). Then, on the moby at block+0x2C: sets bits 0x41
 * and clears bit 0x1000 of its flags halfword +0x34, and clears its word
 * +0x98 and halfword +0x96.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/03_endako/L03_00399230.c, function
 * func_L03_003D1FB0 (v1.01 address), author commit 91e04732 (llesieur99;
 * `git blame -M -C` names it for all 11 definition lines at fe6adf9b, and
 * `git log --reverse -S'func_L03_003D1FB0('` agrees). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L03_003D1FB0 -> func_L03_003D35B8 (our split name at the
 *     v2.00 place);
 *   - two callees as L03's copies, the v2.00 ROM's call targets:
 *     func_324168 -> func_L03_003254E0, func_30D710 -> func_L03_0030EA10
 *     (upstream declares the latter twice in the file; one declaration is
 *     carried);
 *   - `#include "common.h"` dropped, its two typedefs written out.
 */
void func_L03_003D35B8(char *a0)
{
    char *s0 = *(char **)(a0 + 0x68);

    func_L03_003254E0(*(s32 *)(s0 + 0x18), *(s32 *)(s0 + 0x1C));
    func_L03_0030EA10(0, -1);
    *(u16 *)(*(s32 *)(s0 + 0x2C) + 0x34) |= 0x41;
    *(u16 *)(*(s32 *)(s0 + 0x2C) + 0x34) &= ~0x1000;
    *(s32 *)(*(s32 *)(s0 + 0x2C) + 0x98) = 0;
    *(u16 *)(*(s32 *)(s0 + 0x2C) + 0x96) = 0;
}
