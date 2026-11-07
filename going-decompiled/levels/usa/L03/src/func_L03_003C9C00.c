/* Level 03 (Endako), USA v2.00: a level-only function built from C
 * (task #2655, RULING #13291 row L03-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003C9C00, in place of its split asm; the row is
 * going-decompiled/levels/usa/L03/config/L03_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* Upstream include/common.h's own definition of the typedef the body uses. */
typedef signed int s32;

/* L03's copies of boot helpers, at their v2.00 places. */
extern void func_L03_0031CC98(char *a0, s32 a1, s32 a2);
extern s32  func_L03_00313BE0(s32 a0, s32 a1, void *a2);
extern void func_L03_002EFE40(char *a0, char *a1, void *a2);

/**
 * func_L03_003C9C00 - build the object's mode record and release it.
 *
 * @a0: the object; its private block pointer is at +0x68, its position at
 *      +0x10.
 * @a1: the mode.
 * Return: nothing.
 *
 * Upstream's note: the private block gets the mode at +0xA0, the mode record
 * is made from the caller's +0x10 and the object is released. The mode is
 * kept in $v0 across the call, so the store lands in the call's delay slot.
 * In detail: func_L03_00313BE0(a1, 2, buf) fills a 16-byte stack buffer,
 * func_L03_002EFE40(block + 0x90, a0 + 0x10, buf) builds the record, and
 * func_L03_0031CC98(a0, 2, -1) switches the object to state 2.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/03_endako/L03_00399230.c, function
 * func_L03_003C85F8 (v1.01 address), author commit 4541c4ce
 * (ReaperOfVeriod; `git blame -M -C` names it for all 10 definition lines at
 * fe6adf9b, and `git log --reverse -S'func_L03_003C85F8('` agrees).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L03_003C85F8 -> func_L03_003C9C00 (our split name at the
 *     v2.00 place);
 *   - three callees as L03's copies, the v2.00 ROM's call targets:
 *     func_31B920 -> func_L03_0031CC98, func_312898 -> func_L03_00313BE0,
 *     func_2EEFE0 -> func_L03_002EFE40;
 *   - `#include "common.h"` dropped, its typedef written out; upstream's
 *     comment folded into this one.
 */
void func_L03_003C9C00(char *a0, s32 a1)
{
    char buf[0x10];
    char *s1 = *(char **)(a0 + 0x68);

    *(s32 *)(s1 + 0xA0) = a1;
    func_L03_00313BE0(a1, 2, buf);
    func_L03_002EFE40(s1 + 0x90, a0 + 0x10, buf);
    func_L03_0031CC98(a0, 2, -1);
}
