/* Level 15 (Gorn), USA v2.00: a level-only function built from C (task
 * #2677, RULING #13291 row L15-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00433F68, in place of its split asm; the row is
 * going-decompiled/levels/usa/L15/config/L15_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern unsigned char func_L15_004346A8[];
extern void func_L15_00433FD0(void *a0);
extern void func_L15_003006C0(void *a0, void *a1);
extern int func_L15_00307608(void *a0);
extern void func_L15_0032A2A0(void *a0);

/**
 * func_L15_00433F68 - per-frame step of an armed object: register the
 * func_L15_004346A8 hook and end the object when its timer runs out.
 *
 * @p: the object; its state block is at +0x68.
 * Return: nothing.
 *
 * Only in state 1 (byte +0x20): func_L15_00433FD0(p), then
 * func_L15_003006C0(func_L15_004346A8, p), and when the timer at state +0x54
 * expires (func_L15_00307608) the object is passed to func_L15_0032A2A0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/15_gorn/L15_00315428.c, function func_L15_004321A8 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L15_004321A8 -> func_L15_00433F68 (our split name at the v2.00 place);
 *   - D_L15_004328E8 -> func_L15_004346A8 (the v2.00 relocation target, our name there);
 *   - func_802FFC98 -> func_L15_003006C0 (the v2.00 relocation target, our name there);
 *   - func_803068A0 -> func_L15_00307608 (the v2.00 relocation target, our name there);
 *   - func_80329048 -> func_L15_0032A2A0 (the v2.00 relocation target, our name there);
 *   - func_L15_00432210 -> func_L15_00433FD0 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
void func_L15_00433F68(char *p)
{
    char *q = *(char **)(p + 0x68);

    if (*(unsigned char *)(p + 0x20) == 1) {
        func_L15_00433FD0(p);
        func_L15_003006C0(func_L15_004346A8, p);
        if (func_L15_00307608(q + 0x54) != 0) {
            func_L15_0032A2A0(p);
        }
    }
}
