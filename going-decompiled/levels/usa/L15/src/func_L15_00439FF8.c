/* Level 15 (Gorn), USA v2.00: a level-only function built from C (task
 * #2677, RULING #13291 row L15-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00439FF8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L15/config/L15_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack (upstream builds its file at -G8), plain as.
 */

extern int D_L15_001B182C __attribute__((sda));
extern void func_L15_00439928(int, int);

/**
 * func_L15_00439FF8 - call func_L15_00439928(a0, 1) while D_L15_001B182C is
 * set.
 *
 * @a0: passed through.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/15_gorn/L15_004328E8-g8.c, function func_L15_00438238 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L15_00438238 -> func_L15_00439FF8 (our split name at the v2.00 place);
 *   - D_L15_001B187C -> D_L15_001B182C (our split label at the v2.00 address);
 *   - func_L15_00437B68 -> func_L15_00439928 (the v2.00 relocation target, our name there);
 *   - upstream's `__attribute__((sda))` on D_L15_001B182C kept verbatim:
 *   our cc1 ignores it with a warning and, at -G8, places these 4-byte
 *   objects $gp-relative as retail has them, so it is byte-inert here;
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
void func_L15_00439FF8(int a0) {
    if (D_L15_001B182C != 0) {
        func_L15_00439928(a0, 1);
    }
}
