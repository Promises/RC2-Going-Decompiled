/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2556,
 * RULING #13291 row L11-D). Compiled ALONE as its own TU and linked at its one
 * place, 0x0044DCA0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the SN 2.95.3 v1.36 cc1 with -fopt-stack
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;
extern f32 func_00282E80(u8 *, u8 *);
extern s32 func_L11_004430B8(u8 *, s32);
extern s32 func_L11_0033DAC8(u8 *, s32, f32, f32, s32, s32);

/**
 * func_L11_0044DCA0 - state handler of the gate class: gate the +0x490 word on
 * the frame window and the class counter.
 *
 * While the object's state byte (+0x20) is below 4 the state's +0x490 word is
 * cleared and its gate halfword (+0x4CA) dropped. From state 4, an armed gate
 * (+0x4CA == 1) runs the 100-unit smash helper func_L11_0033DAC8 on the +0x4C4
 * id and records the +0x220 target at +0x490; an unarmed gate is armed first
 * when func_L11_004430B8 grants a counter of the object's class from the +0xB8
 * handle, with the same smash and record steps. Same body as func_L02_003F16F0
 * with this function's own callees.
 *
 * @arg0: the object; its state is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own
 * and no Lombyte mention), file
 * src/levels/11_joba/L11_004465F8.c, function func_L11_0044C268 (v1.01
 * address), author commit a2d495f1 (Flavius). Adopted with
 * the authors’ permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device):
 *   - func_L11_0044C268 -> func_L11_0044DCA0 (our split name at the v2.00
 *     place);
 *   - func_002B3BA8 -> func_L11_0033DAC8;
 *   - func_L11_00441868 -> func_L11_004430B8;
 *   - `#include "common.h"` and `#include "include_asm.h"` dropped: the
 *     u8/u16/u32/s16/s32/f32 typedefs common.h supplied are written here;
 *   - upstream’s two comments above the function (the state-handler summary
 *     and “Same body as func_L02_003F16F0, with this function’s own callees”)
 *     are folded into this one.
 *
 */
void func_L11_0044DCA0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(unsigned char *)(arg0 + 0x20) < 4) {
        *(int *)(state + 0x490) = 0;
        goto done;
    }

    if (*(short *)(state + 0x4CA) == 1) {
        *(short *)(state + 0x4C8) =
            func_L11_0033DAC8(arg0, *(int *)(state + 0x4C4), 100.0f, 100.0f, 0, 0);
        *(int *)(state + 0x490) = *(int *)(state + 0x220);
    } else {
        if (func_L11_004430B8(*(int *)(arg0 + 0xB8), *(short *)(arg0 + 0xAA)) == 0)
            goto done;
        *(short *)(state + 0x4CA) = 1;
        *(short *)(state + 0x4C8) =
            func_L11_0033DAC8(arg0, *(int *)(state + 0x4C4), 100.0f, 100.0f, 0, 0);
        *(int *)(state + 0x490) = *(int *)(state + 0x220);
    }
    return;

done:
    *(short *)(state + 0x4CA) = 0;
}
