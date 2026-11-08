/* Level 15 (Gorn), USA v2.00: a level-only function built from C (task
 * #2677, RULING #13291 row L15-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x004398D8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L15/config/L15_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern void func_L15_00300580(void *, void *);
extern void func_L15_003004B8(void *a0, void *a1);
extern void func_L15_003006C0(void *, int);
extern void func_L15_00439FF8(void);
extern void func_L15_0043A020(void);
extern void func_L15_0043A2D8(void);

/**
 * func_L15_004398D8 - register an object's three hooks.
 *
 * @a0: the object.
 * Return: nothing.
 *
 * Passes func_L15_00439FF8, func_L15_0043A020 and func_L15_0043A2D8 with a0 to
 * func_L15_00300580, func_L15_003004B8 and func_L15_003006C0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/15_gorn/L15_004328E8.c, function func_L15_00437B18 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L15_00437B18 -> func_L15_004398D8 (our split name at the v2.00 place);
 *   - func_L00_002E6CF0 -> func_L15_003004B8 (the v2.00 relocation target, our name there);
 *   - func_L00_002E6DB8 -> func_L15_00300580 (the v2.00 relocation target, our name there);
 *   - func_L00_002E6EF8 -> func_L15_003006C0 (the v2.00 relocation target, our name there);
 *   - func_L15_00438238 -> func_L15_00439FF8 (the v2.00 relocation target, our name there);
 *   - func_L15_00438260 -> func_L15_0043A020 (the v2.00 relocation target, our name there);
 *   - func_L15_00438518 -> func_L15_0043A2D8 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
void func_L15_004398D8(void *a0) {
    func_L15_00300580((void *)func_L15_00439FF8, a0);
    func_L15_003004B8((void *)func_L15_0043A020, a0);
    func_L15_003006C0((void *)func_L15_0043A2D8, a0);
}
