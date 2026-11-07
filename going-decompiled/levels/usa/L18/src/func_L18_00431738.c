/* Level 18 (Damosel), USA v2.00: a level-only function built from C (task
 * #2662, RULING #13291 row L18-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00431738, in place of its split asm; the row is
 * going-decompiled/levels/usa/L18/config/L18_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
extern void func_L18_004317A0(u8 *a0);
extern void func_L18_00431DE0(u8 *a0);
extern void func_L18_00432348(u8 *a0);
extern void func_L18_00433560(u8 *a0);
extern void func_L18_00433800(u8 *a0);

/**
 * func_L18_00431738 - per-frame dispatcher of one Damosel object.
 *
 * @a0: the object.
 *
 * State byte (+0x20) 0 runs the set-up func_L18_004317A0 and returns; state
 * 1 does nothing; any other state runs func_L18_00431DE0,
 * func_L18_00432348, func_L18_00433560 and func_L18_00433800 in that order.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/18_damosel/L18_003F80A0.c,
 * function func_L18_00430248 (v1.01 address), author commit fa8d6672 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L18_00430248 -> func_L18_00431738 (our split name at the v2.00 place);
 *   - func_L18_004302B0 -> func_L18_004317A0 (the v2.00 relocation target, our name there);
 *   - func_L18_004308F0 -> func_L18_00431DE0 (the v2.00 relocation target, our name there);
 *   - func_L18_00430E58 -> func_L18_00432348 (the v2.00 relocation target, our name there);
 *   - func_L18_00432070 -> func_L18_00433560 (the v2.00 relocation target, our name there);
 *   - func_L18_00432310 -> func_L18_00433800 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept (the common.h typedefs it uses written out as common.h defines them).
 */
void func_L18_00431738(u8 *a0) {
    u8 v = *(u8 *)(a0 + 0x20);
    if (v == 0) {
        func_L18_004317A0(a0);
        return;
    }
    if (v == 1) return;
    func_L18_00431DE0(a0);
    func_L18_00432348(a0);
    func_L18_00433560(a0);
    func_L18_00433800(a0);
}
