/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x0045A840, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L11's copy of the boot's vector scale helper (func_00282D30), at its v2.00
 * place in this level. */
extern void func_L11_00301280(char *dst, char *src, float scale);

/**
 * func_L11_0045A840 - scale four consecutive 16-byte XYZ vectors in place.
 *
 * @p:     the first of four 16-byte vectors.
 * @scale: the scalar.
 *
 * Calls func_L11_00301280(v, v, scale) on p, p + 16, p + 32 and p + 48.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_00453358.c, function
 * func_L11_00458D88 (v1.01 address), author commit 7fccf735 (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged (verbatim but for
 * names; no device); edits:
 *   - name func_L11_00458D88 -> func_L11_0045A840 (our split name at the v2.00 place);
 *   - func_00282D30_2 -> func_L11_00301280: the target of the v2.00 ROM's
 *     own relocations (relverify, task #2553). Upstream's _2 suffix only gave
 *     the prototype its own name;
 *   - the audit's `long long` -> `long` respelling (RULING #13197 class,
 *     needed because cc1 s136 refuses the file's other `long long` uses) is
 *     NOT applied: this one-function TU carries no `long long`;
 *   - `#include "common.h"` dropped (no typedef of it is used here); the
 *     upstream one-line comment folded into this one; the file's other
 *     functions and their externs not carried.
 */
void func_L11_0045A840(char *p, float scale)
{
    func_L11_00301280(p, p, scale);
    func_L11_00301280(p + 16, p + 16, scale);
    func_L11_00301280(p + 32, p + 32, scale);
    func_L11_00301280(p + 48, p + 48, scale);
}
