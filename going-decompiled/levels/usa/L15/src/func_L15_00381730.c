/* Level 15 (Gorn), USA v2.00: a level-only function built from C (task
 * #2677, RULING #13291 row L15-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00381730, in place of its split asm; the row is
 * going-decompiled/levels/usa/L15/config/L15_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern void *func_L15_00381760(int a0);

/**
 * func_L15_00381730 - store a value into the record func_L15_00381760
 * returns for an object.
 *
 * @a0: the object (passed to the lookup as an int).
 * @a1: the value stored.
 * Return: nothing.
 *
 * When func_L15_00381760(a0) returns a record, a1 is written to its +0x158;
 * with no record nothing is written.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/15_gorn/L15_00315428.c, function func_L15_003804B8 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L15_003804B8 -> func_L15_00381730 (our split name at the v2.00 place);
 *   - func_803804E8 -> func_L15_00381760 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
void func_L15_00381730(int a0, void *a1)
{
    void *p = func_L15_00381760(a0);

    if (p != 0) {
        *(void **)((char *)p + 0x158) = a1;
    }
}
