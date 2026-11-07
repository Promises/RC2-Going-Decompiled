/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x003C2DA0, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern int D_L23_001B3630 __attribute__((section(".sdata")));
extern void func_L23_003C2DC8(char *arg0);

/**
 * func_L23_003C2DA0 - clears the level global D_L23_001B3630 (func_L23_003C2EF0
 * stores an object there), then releases the object's two handles
 * (func_L23_003C2DC8).
 *
 * @arg0: the object (moby); +0x68 points at its state block.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_0035CE90.c, function
 * func_L23_003C1960 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED; the body's statements unchanged. Edits:
 *   - func_L23_003C1960 -> func_L23_003C2DA0 (our split name at the v2.00
 *     place)
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on
 *     D_L23_001B3630 (ablation: without it, 7 of 9 words differ and the body
 *     is 8 words) (RULING #8620: an addressing-model device on an extern
 *     DECLARATION, EE only; upstream's patched compiler reads sda, ours does
 *     not)
 *   - D_L23_001B36B0 -> D_L23_001B3630 (address-named extern inside a larger
 *     split label, RULING #13291 section 2)
 *   - func_L23_003C1988 -> func_L23_003C2DC8
 *   - upstream declares D_L23_001B36B0 earlier in the file; the declaration is
 *     carried here
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the TU
 *     declares what it uses, and upstream's comments are folded into this one
 */
void func_L23_003C2DA0(char *arg0) {
    D_L23_001B3630 = 0;
    func_L23_003C2DC8(arg0);
}
