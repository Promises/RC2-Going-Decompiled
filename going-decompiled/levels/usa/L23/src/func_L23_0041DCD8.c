/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x0041DCD8, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern char *D_L23_001B1434;
extern unsigned char *g_pHeroMoby __attribute__((section(".data")));
extern int D_L23_00258FD0[3][60];
extern int func_L23_00325618(int);
extern int func_L23_0041DDC0(char *, void *, int, int);

/**
 * func_L23_0041DCD8 - draws a row (0, 1 or 2; three choices only when the byte
 * at +0x43 of the hero moby is not 6) and hands the position to
 * func_L23_0041DDC0 with the level object D_L23_001B1434 and that row.
 *
 * @pos: the position.  @arg1: passed on.
 * Return: nothing.
 *
 * Non-obvious (upstream's comment): the search for a free slot in
 * D_L23_00258FD0 is retail's, and its result is never used. The three cases
 * make cc1 build the decision tree rooted on 1; with two, it emits a comparison
 * chain from 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_00416388-g8.c,
 * function func_L23_0041C7D8 (v1.01 address), author commit babfe411
 * (bigmak94). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED; the body's statements unchanged. Edits:
 *   - func_L23_0041C7D8 -> func_L23_0041DCD8 (our split name at the v2.00
 *     place)
 *   - `__attribute__((nosda))` -> `__attribute__((section(".data")))` on
 *     g_pHeroMoby (RULING #8620 addressing-model device on an extern
 *     DECLARATION, EE only: it keeps the ROM's lui/lw pair at -G8; ablation
 *     without it: 47 of 55 words differ and the body is 2 words short)
 *   - D_0018C0B0 -> g_pHeroMoby (the boot's name, same address)
 *   - D_L23_00259050 -> D_L23_00258FD0 (address-named extern inside a larger
 *     split label, RULING #13291 section 2); D_L23_001B1484 -> D_L23_001B1434
 *   - upstream's `__attribute__((sda))` on D_L23_001B1484 dropped: inert at
 *     -G8 (measured: removing it leaves 0 of 57 words differing)
 *   - func_002A77E0 -> func_L23_00325618, func_L23_0041C8C0 ->
 *     func_L23_0041DDC0
 *   - `#include "include_asm.h"` dropped: the TU declares what it uses, and
 *     upstream's comments are folded into this one
 */
void func_L23_0041DCD8(void *pos, int arg1) {
    int type = 0;
    int i;

    switch (func_L23_00325618(g_pHeroMoby[0x43] == 6 ? 2 : 3)) {
    case 0:
        type = 1;
        break;
    case 1:
        type = 2;
        break;
    case 2:
        type = 0;
        break;
    }
    for (i = 0; i < 50; i++) {
        if (D_L23_00258FD0[type][i] == 0)
            break;
        if (i == 49) {
            i = -1;
            break;
        }
    }
    func_L23_0041DDC0(D_L23_001B1434, pos, type, arg1);
}
