/* Level 18 (Damosel), USA v2.00: a level-only function built from C (task
 * #2662, RULING #13291 row L18-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x00433A38, in place of its split asm; the row is
 * going-decompiled/levels/usa/L18/config/L18_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef int s32;
extern float func_L18_00309D00(float, float);

/**
 * func_L18_00433A38 - face a target point and set the turn mode.
 *
 * @a0: the object; its state block is at +0x68.
 * @a1: the target point (x, y).
 * @a2: non-zero selects mode 2, zero mode 1.
 *
 * Stores the mode in the halfword +0x808 of the state and the heading
 * func_L18_00309D00(dx, dy) from the object's position (+0x10/+0x14) to the
 * target in +0x7A4.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/18_damosel/L18_003F80A0.c,
 * function func_L18_00432548 (v1.01 address), author commit fa8d6672 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L18_00432548 -> func_L18_00433A38 (our split name at the v2.00 place);
 *   - func_00283248 -> func_L18_00309D00 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept (the common.h typedefs it uses written out as common.h defines them).
 */
void func_L18_00433A38(u8 *a0, float *a1, s32 a2) {
    u8 *s0 = *(u8 **)(a0 + 0x68);
    *(u16 *)(s0 + 0x808) = (a2 != 0) ? 2 : 1;
    *(float *)(s0 + 0x7A4) = func_L18_00309D00(a1[0] - *(float *)(a0 + 0x10),
                                           a1[1] - *(float *)(a0 + 0x14));
}
