/* Level 17 (Smolg), USA v2.00: a level-only function built from C (task
 * #2667, RULING #13291 row L17-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003C91E0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L17/config/L17_c.tsv, the arm ee-gcc 2.9-ee-991111b
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, plain as.
 */

/**
 * func_L17_003C91E0 - is the object's state word +0xBC equal to 2?
 *
 * @a0: the object; its state block is at +0x68.
 * Return: 1 when it is, else 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/17_smolg/L17_0032C070.c, function func_L17_003C7C58 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L17_003C7C58 -> func_L17_003C91E0 (our split name at the v2.00 place);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
int func_L17_003C91E0(int a0)
{
    return *(int *)(*(int *)(a0 + 0x68) + 0xBC) == 2;
}
