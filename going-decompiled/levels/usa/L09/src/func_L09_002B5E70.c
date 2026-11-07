/* Level 09 (Dobbo), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2580, RULING #13291 row L09-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x002B5E70, in place of its
 * split asm; the row is going-decompiled/levels/usa/L09/config/L09_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column; the plain-`as` ablation is in the row's FACT, task #2580).
 */

typedef unsigned char u8;
typedef unsigned int u32;

/* The hero/runtime block the boot keeps at 0x00189E20 (the boot's name). */
extern u8 g_soundBankHandlesBlk[];

/* L09's own copies of boot code, at their level-09 places. */
extern float func_L09_002EB2F0(void *, void *);
extern float func_L09_0031AF88(float *, float, float);
extern float func_L09_0031B048(float *, float *, float, float, float, float);

/**
 * func_L09_002B5E70 - per-frame update of the level's three camera/aim
 * channel floats in the runtime block at g_soundBankHandlesBlk.
 *
 * Sets the channel targets (+0x258/+0x25C/+0x260) from the mode byte
 * +0x22B4, the state words +0x229C/+0x2294 and the byte +0x49E, then, unless
 * the aim check passes (flag +0x287 set, neither +0x22A4 nor +0x22B0 is 0x12,
 * +0x254 >= 0.53 in mode 0, state not 0x11, +0x1494 clear, and the distance
 * from +0x240 to the hero position +0x80 is within half of +0x264), ramps the
 * channels at +0x250/+0x254 and +0x264/+0x268 towards their targets by 0.02
 * through the level's copies of the boot's ramp helpers.
 *
 * Takes and returns nothing.
 *
 * Non-obvious: the ramp block pointer is taken first and the runtime block is
 * reached back from it (p - 0x250), as upstream writes it; the code shape
 * depends on that.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_002AFC90.c, function func_L09_002B5CE8 (v1.01
 * address), author commit aac730b3 (Flavius). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the body's
 * statements unchanged; the edits, each name to the target of the v2.00
 * disc's own relocation at this place (task #2580 relverify):
 *   - func_L09_002B5CE8 -> func_L09_002B5E70 (our split name);
 *   - D_00189E20 -> g_soundBankHandlesBlk (the boot's name, same address);
 *   - callees func_00282E80 -> func_L09_002EB2F0, func_002AA2E8 ->
 *     func_L09_0031AF88, func_002AA3A8 -> func_L09_0031B048 (L09's copies);
 *   - built at -G8 against upstream's -G0 (the audit's code G, NOTE #13281);
 *   - `#include "include_asm.h"` dropped (nothing used it); upstream's
 *     comments folded into this one.
 */
void func_L09_002B5E70(void) {
    int a3;
    int v1;
    float r;

    if (g_soundBankHandlesBlk[0x22B4] == 0) {
        *(float *)(g_soundBankHandlesBlk + 0x258) = 0.8f;
        *(float *)(g_soundBankHandlesBlk + 0x25C) = 0.7f;
        *(float *)(g_soundBankHandlesBlk + 0x260) = 0.45f;
    }

    a3 = *(int *)(g_soundBankHandlesBlk + 0x229C);
    if (a3 == 4) {
        if (*(short *)(g_soundBankHandlesBlk + 0x4AC) < *(int *)(g_soundBankHandlesBlk + 0x1B8)) {
            if (g_soundBankHandlesBlk[0x49E] == 0) {
                *(float *)(g_soundBankHandlesBlk + 0x25C) = *(float *)(g_soundBankHandlesBlk + 0x4B8);
            }
        }
    } else {
        v1 = *(int *)(g_soundBankHandlesBlk + 0x2294);
        if (v1 == 6) {
            *(float *)(g_soundBankHandlesBlk + 0x25C) = 0.5f;
        } else if (v1 == 4) {
            *(float *)(g_soundBankHandlesBlk + 0x258) = 0.35000002384185791f;
        } else if ((u32)(a3 - 0x11) < 2) {
            *(int *)(g_soundBankHandlesBlk + 0x258) = 0;
            *(int *)(g_soundBankHandlesBlk + 0x25C) = 0;
        } else if (v1 == 0x7C) {
            *(float *)(g_soundBankHandlesBlk + 0x260) = 0.8f;
        }
    }

    if (g_soundBankHandlesBlk[0x287] == 0)
        goto slow;
    if (*(int *)(g_soundBankHandlesBlk + 0x22A4) == 0x12)
        goto slow;
    if (*(int *)(g_soundBankHandlesBlk + 0x22B0) == 0x12)
        goto slow;
    if (g_soundBankHandlesBlk[0x22B4] == 0) {
        if (*(float *)(g_soundBankHandlesBlk + 0x254) < 0.53f)
            goto slow;
    }
    if (*(int *)(g_soundBankHandlesBlk + 0x229C) == 0x11)
        goto slow;
    if (g_soundBankHandlesBlk[0x1494] != 0)
        goto slow;

    r = func_L09_002EB2F0(g_soundBankHandlesBlk + 0x240, g_soundBankHandlesBlk + 0x80);
    if (r > *(float *)(g_soundBankHandlesBlk + 0x264) * 0.5f)
        goto slow;
    return;

slow:
    {
        u8 *p = g_soundBankHandlesBlk + 0x250;
        u8 *s = p - 0x250;
        float step = 0.02f;
        func_L09_0031AF88((float *)p, *(float *)(s + 0x258), step);
        func_L09_0031AF88((float *)(p + 4), *(float *)(s + 0x25C), step);
        func_L09_0031B048(
            (float *)(p + 0x14),
            (float *)(p + 0x18),
            *(float *)(s + 0x260),
            step,
            0.3f,
            0.06666667f);
    }
}
