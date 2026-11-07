/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x003E3750, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

extern float func_L14_003E37C0();
extern void func_L14_002F7280();
extern void func_L14_002F6AA8(void *dst, void *src, float scale);
extern void func_L14_0031B8E0();

/**
 * func_L14_003E3750 - scale the object's matrix axes by its stage's factor.
 *
 * @arg0: the object; its matrix and two axes are at +0xC0/+0xD0/+0xF0.
 * Return: nothing.
 *
 * Non-obvious: the factor is func_L14_003E37C0's reading of the stage byte
 * (a ratio of state-block counters, or a constant). The matrix is first
 * rebuilt from the axis (func_L14_002F7280), then the matrix and the other
 * axis are scaled in place by the factor (func_L14_002F6AA8), and the object
 * is queued (func_L14_0031B8E0).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c, function
 * func_L14_003E2238 (v1.01 address), author commit db93b56d (llesieur99; the
 * definition line blames db93b56d too). Adopted with the authors' permission
 * (RULINGs #10405, #12108, #13082, #13291). BYTE-IDENTICAL: VERBATIM but for
 * names; edits:
 *   - name func_L14_003E2238 -> func_L14_003E3750 (our split name at the
 *     v2.00 place);
 *   - func_3E22A8 -> func_L14_003E37C0 (the level's own copy at its L14
 *     place);
 *   - func_2F6498 -> func_L14_002F7280 (the level's own copy of boot
 *     func_002833F0 at its L14 place);
 *   - func_00282D30 -> func_L14_002F6AA8 (the level's own copy of the boot
 *     function at its L14 place);
 *   - func_31A618 -> func_L14_0031B8E0 (the level's own copy of boot
 *     func_002A10E8 at its L14 place);
 *   - the file's `#include` lines, other functions and their externs not
 *     carried (nothing else here uses them); upstream's declaration forms
 *     kept as written (func_00282D30 prototyped, the rest unprototyped).
 */
void func_L14_003E3750(char *arg0) {
    float f = func_L14_003E37C0(arg0);

    func_L14_002F7280(arg0 + 0xC0, arg0 + 0xF0);
    func_L14_002F6AA8(arg0 + 0xC0, arg0 + 0xC0, f);
    func_L14_002F6AA8(arg0 + 0xD0, arg0 + 0xD0, f);
    func_L14_0031B8E0(arg0);
}
