/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * through the boot's assembler model (task #2579, RULING #13291 row L02-ASM).
 * Compiled ALONE as its own TU and linked at its one place, 0x003D7F58, in
 * place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by tools/ee/asm_unit.sh (the row's `asm_unit` column; the plain-`as`
 * ablation is in the row's FACT, task #2579).
 */

extern void func_L02_002F2D20(void *);
extern int D_L02_001B0304 __attribute__((sda));
extern int D_L02_001B0310 __attribute__((sda));
extern int D_L02_001B0320 __attribute__((sda));

/**
 * func_L02_003D7F58 - reset the level's three small-data globals.
 *
 * Return: none.
 *
 * Non-obvious: it clears the word D_L02_001B0304, then passes D_L02_001B0320
 * and D_L02_001B0310 to func_L02_002F2D20, the level's copy of the boot's
 * vector clear.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/02_maktar_nebula/L02_003D26B0-g8.c, function
 * func_L02_003D69F0 (v1.01 address), author commit 431ac503 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: verbatim but for names (no device). Respellings,
 * each to the target of the v2.00 disc's own relocation at this place
 * (relverify by name, task #2579):
 *   - func_L02_003D69F0 -> func_L02_003D7F58 (our split name at the v2.00
 *     place);
 *   - D_L02_001B0344 -> D_L02_001B0304 (our label at that address);
 *   - D_L02_001B0350 -> D_L02_001B0310 (our label at that address);
 *   - D_L02_001B0360 -> D_L02_001B0320 (our label at that address);
 *   - func_00282C88 -> func_L02_002F2D20 (the level's own copy at its L02
 *     place);
 *   - upstream's `__attribute__((sda))` on D_L02_001B0304, D_L02_001B0310 and
 *     D_L02_001B0320 kept as its text (this cc1 ignores it with a warning; the
 *     member closes with it as written);
 *   - `#include "common.h"` dropped (this body uses nothing from it); upstream
 *     file-scope declarations it does not use are not carried; the rest keep
 *     upstream's order and text.
 */
void func_L02_003D7F58(void) {
    D_L02_001B0304 = 0;
    func_L02_002F2D20(&D_L02_001B0320);
    func_L02_002F2D20(&D_L02_001B0310);
}
