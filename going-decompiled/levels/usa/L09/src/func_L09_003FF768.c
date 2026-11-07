/* Level 09 (Dobbo), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2580, RULING #13291 row L09-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x003FF768, in place of its
 * split asm; the row is going-decompiled/levels/usa/L09/config/L09_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column; the plain-`as` ablation is in the row's FACT, task #2580).
 */

/* The pulse table's entry count (our label at 0x001B1188), $gp-relative at
 * -G8 by its size. Upstream's `sda` attribute text is kept; this cc1 ignores
 * it (a warning), so it emits nothing. */
extern int D_L09_001B1188 __attribute__((sda));
/* The colour table and the phase table (our labels). */
extern int D_L09_002A2658[];
extern int D_L09_002A27A0[];
/* L09's copy of the boot's FloatToInt (func_00283CF0). */
extern int func_L09_002EC258(float);

/**
 * func_L09_003FF768 - advance an object's colour-pulse animation.
 *
 * Adds 0.5 to the timer at +0x384 of the object's state (+0x68), wrapping it
 * to 0.0 once it reaches 1.0, and adds its integer part (so one step every
 * other frame) to every non-zero phase in D_L09_002A27A0. A phase above 0x90
 * falls back by 0x80; the phase, folded back above 0x50 (0xA0 - phase),
 * becomes the alpha byte of the matching D_L09_002A2658 colour.
 *
 * @a0: the object; its state is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_002F8BA0-g8.c, function func_L09_003FE2D8
 * (v1.01 address), author commit ba383701 (bigbeno37). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2580 relverify):
 *   - func_L09_003FE2D8 -> func_L09_003FF768 (our split name);
 *   - D_L09_001B11D8 -> D_L09_001B1188 (our label; `sda` text kept);
 *   - callee func_00283CF0 -> func_L09_002EC258 (L09's copy);
 *   - `#include "common.h"` dropped: only the declarations this function uses
 *     are kept; upstream's comments folded into this one.
 */
void func_L09_003FF768(unsigned char *a0) {
    unsigned char *state = *(unsigned char **)(a0 + 0x68);
    float *timer = (float *)(state + 0x384);
    float t = *timer + 0.5f;
    int step;
    int i;
    *timer = t;
    step = func_L09_002EC258(t);
    if (*timer >= 1.0f)
        *timer = 0.0f;
    for (i = 0; i < D_L09_001B1188; i++) {
        int phase = D_L09_002A27A0[i];
        if (phase != 0) {
            phase += step;
            if (phase >= 0x91)
                phase -= 0x80;
            D_L09_002A27A0[i] = phase;
            if (phase >= 0x51)
                phase = 0xA0 - phase;
            D_L09_002A2658[i] = (D_L09_002A2658[i] & 0xFFFFFF) | (phase << 24);
        }
    }
}
