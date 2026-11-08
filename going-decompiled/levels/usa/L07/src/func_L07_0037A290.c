/* Level 07 (Siberius), USA v2.00: a level-only function built from C (row
 * L07-A, task #2656, RULING #13291). Compiled ALONE as its own TU and linked
 * at its one place, 0x0037A290, in place of its split asm; the row is
 * going-decompiled/levels/usa/L07/config/L07_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

extern char g_cameraPos[];
extern void func_L07_002E4F18(void *, void *, void *);
extern void func_L07_002E4FA0(void *, void *, float);
extern float func_L07_00311C48(float, float);
extern void func_L07_00311E28(float *, float);
extern void func_L07_00337A30(float *, float *, s32, s32);

/**
 * func_L07_0037A290 - scatter 200 particles around the camera target, then
 * set the moby's position 4 ahead of it.
 *
 * Each of the 200 steps draws a radius-15 random direction
 * (func_L07_00311E28) with a random height in [-7, 7], offsets it from the
 * camera block g_cameraPos and 4 units along the block's +0x230 vector
 * (func_L07_002E4FA0 scale, func_L07_002E4F18 add), draws a velocity from the
 * moby's two speeds (per 60 Hz frame, the vertical one scaled by a random
 * [0.8, 1.2] and negated), and spawns the particle with func_L07_00337A30.
 * Finally the point 4 ahead of the camera is stored at the moby's vars+0x60.
 *
 * @m: the moby; its +0x68 vars hold the two speeds at [0] and [1].
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/07_siberius/L07_00379020.c, function func_L07_00379020
 * (v1.01 address), author commit 5e61bf0b (Flavius), by blame -M -C of the
 * definition (all 21 lines; no rename-copy found).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (BYTE-IDENTICAL), each name to the target
 * of the v2.00 disc's own relocation at this place (task #2656 relverify by
 * name):
 *   - func_L07_00379020 -> func_L07_0037A290 (our split name);
 *   - D_L07_001B9200 -> g_cameraPos: the v2.00 target 0x1B9180 is the
 *     address L07_boot_names.txt names g_cameraPos (split-operand), so that
 *     name is used rather than a second, address-named spelling;
 *   - func_00282CC0 -> func_L07_002E4F18, func_00282D30 -> func_L07_002E4FA0,
 *     func_002A7878 -> func_L07_00311C48, func_L02_0031E9E8 ->
 *     func_L07_00311E28, func_L07_00336860 -> func_L07_00337A30 (L07's own
 *     copies at the v2.00 places);
 *   - `#include "common.h"` dropped, the three typedefs this body uses
 *     written out in the TU instead, and upstream's one-line comment folded
 *     into this one.
 */
void func_L07_0037A290(u8 *m)
{
    f32 pos[4] __attribute__((aligned(16)));
    f32 vel[4] __attribute__((aligned(16)));
    f32 ahead[4] __attribute__((aligned(16)));
    f32 *vars = *(f32 **)(m + 0x68);
    s32 i;

    for (i = 0; i < 200; i++) {
        func_L07_00311E28(pos, 15.0f);
        pos[2] = func_L07_00311C48(-7.0f, 7.0f);
        func_L07_002E4F18(pos, pos, g_cameraPos);
        func_L07_002E4FA0(ahead, g_cameraPos + 0x230, 4.0f);
        func_L07_002E4F18(pos, ahead, pos);
        func_L07_00311E28(vel, vars[0] * 0.0166666675f);
        vel[2] = -(vars[1] * 0.0166666675f) * func_L07_00311C48(0.8f, 1.2f);
        func_L07_00337A30(pos, vel, 0, 1);
    }
    func_L07_002E4FA0(ahead, g_cameraPos + 0x230, 4.0f);
    func_L07_002E4F18(vars + 0x60 / 4, ahead, g_cameraPos);
}
