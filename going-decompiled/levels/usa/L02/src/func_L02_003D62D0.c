/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * through the boot's assembler model (task #2579, RULING #13291 row L02-ASM).
 * Compiled ALONE as its own TU and linked at its one place, 0x003D62D0, in
 * place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by tools/ee/asm_unit.sh (the row's `asm_unit` column; the plain-`as`
 * ablation is in the row's FACT, task #2579).
 */

extern float func_L02_002F2F50(char *, void *);
extern int func_L02_0031FA68(int);
extern int D_L02_001B0170 __attribute__((sda));

/**
 * func_L02_003D62D0 - has the actor waited long enough near D_L02_001B0170 to
 * act?
 *
 * @arg0: the actor (moby); +0x68 its state block, whose +0x32E is a timer and
 *        +0x32A a second counter.
 * Return: 1, with both cleared, when it may act; else 0.
 *
 * Non-obvious: it returns 0 when farther than 18 units from D_L02_001B0170
 * (func_L02_002F2F50) or when the timer is below 500. Below 901 it computes
 * 900 - +0x32A; if that is positive and func_L02_0031FA68 of it is nonzero, it
 * returns 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/02_maktar_nebula/L02_003D26B0-g8.c, function
 * func_L02_003D4D68 (v1.01 address), author commit 773eafc2 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: verbatim but for names (no device). Respellings,
 * each to the target of the v2.00 disc's own relocation at this place
 * (relverify by name, task #2579):
 *   - func_L02_003D4D68 -> func_L02_003D62D0 (our split name at the v2.00
 *     place);
 *   - D_L02_001B01B0 -> D_L02_001B0170 (our label at that address);
 *   - func_00282E80 -> func_L02_002F2F50 (the level's own copy at its L02
 *     place);
 *   - func_002A77E0 -> func_L02_0031FA68 (the level's own copy at its L02
 *     place);
 *   - upstream's `__attribute__((sda))` on D_L02_001B0170 kept as its text
 *     (this cc1 ignores it with a warning; the member closes with it as
 *     written);
 *   - `#include "common.h"` dropped (this body uses nothing from it); upstream
 *     file-scope declarations it does not use are not carried; the rest keep
 *     upstream's order and text.
 */
int func_L02_003D62D0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    short timer;
    int delta;

    if (func_L02_002F2F50(arg0 + 0x10, &D_L02_001B0170) > 18.0f) {
        return 0;
    }
    timer = *(short *)(state + 0x32E);
    if (timer < 0x1F4) {
        return 0;
    }
    if (timer < 0x385) {
        delta = 0x384 - *(short *)(state + 0x32A);
        if (delta > 0) {
            if (func_L02_0031FA68(delta) != 0) {
                return 0;
            }
        }
    }
    *(short *)(state + 0x32A) = 0;
    *(short *)(state + 0x32E) = 0;
    return 1;
}
