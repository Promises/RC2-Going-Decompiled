/* Level 18 (Damosel), USA v2.00: a level-only function built from C (task
 * #2662, RULING #13291 row L18-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x0035CB48, in place of its split asm; the row is
 * going-decompiled/levels/usa/L18/config/L18_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef signed int s32;

extern float func_L18_0030A650(float, float);
extern float func_L18_00309C38(float);
extern float func_L18_00309C50(float);
extern float func_L18_0030A738(float, float);
extern float func_L18_003095B0(float);
extern void func_L18_0034C4E0(s32);

/**
 * func_L18_0035CB48 - advance an orbiting record one frame.
 *
 * @a0: the record func_L18_0035C9C8 filled; its orbit block is at +0x20.
 *
 * Adds the step byte (+0x34) to byte 8, advances the angle (+0x28) by the
 * rate (+0x2C) through func_L18_0030A650, adds +0x30 to +0xC, and re-places
 * the record on its circle round the centre (*(+0x20)) as the spawner does.
 * When func_L18_0030A738 of the angle and the centre's +0x10 angle falls
 * below func_L18_003095B0 of twice the rate, the record is released through
 * func_L18_0034C4E0.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/18_damosel/L18_002E68B0.c,
 * function func_L18_0035B838 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L18_0035B838 -> func_L18_0035CB48 (our split name at the v2.00 place);
 *   - func_308860 -> func_L18_003095B0 (the v2.00 relocation target, our name there);
 *   - func_308EE8 -> func_L18_00309C38 (the v2.00 relocation target, our name there);
 *   - func_308F00 -> func_L18_00309C50 (the v2.00 relocation target, our name there);
 *   - func_309900 -> func_L18_0030A650 (the v2.00 relocation target, our name there);
 *   - func_3099E8 -> func_L18_0030A738 (the v2.00 relocation target, our name there);
 *   - func_34B1D0 -> func_L18_0034C4E0 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept (the common.h typedefs it uses written out as common.h defines them).
 */
void func_L18_0035CB48(char *a0)
{
    char *s1 = a0;
    char *s0 = s1 + 0x20;
    float t;

    *(u8 *)(s1 + 8) = *(u8 *)(s1 + 8) + *(u8 *)(s0 + 0x14);
    *(float *)(s0 + 8) = func_L18_0030A650(*(float *)(s0 + 8), *(float *)(s0 + 0xC));
    *(float *)(s1 + 0xC) = *(float *)(s1 + 0xC) + *(float *)(s0 + 0x10);
    *(float *)(s1 + 0x10) = *(float *)(*(char **)(s1 + 0x20)) +
                            *(float *)(s0 + 4) * func_L18_00309C38(*(float *)(s0 + 8));
    *(float *)(s1 + 0x18) = *(float *)(*(char **)(s1 + 0x20) + 8) +
                            *(float *)(s0 + 4) * func_L18_00309C50(*(float *)(s0 + 8));
    t = func_L18_0030A738(*(float *)(s0 + 8), *(float *)(*(char **)(s1 + 0x20) + 0x10));
    if (t < func_L18_003095B0(*(float *)(s0 + 0xC) + *(float *)(s0 + 0xC)))
        func_L18_0034C4E0(s1);
}
