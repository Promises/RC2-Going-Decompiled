/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x002DC618, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern unsigned char g_bPlayerMode;
extern void func_L20_002D79B0(int a0, int a1);

/**
 * func_L20_002DC618 - start the level's shared loader in the variant the player mode selects.
 *
 * Calls func_L20_002D79B0(first, mode) with mode 2 when @enable is non-zero,
 * else 1, and first = 0, 0x3F or 0x8D for g_bPlayerMode 0, 1 or 4. Any other
 * player mode does nothing.
 *
 * @enable: non-zero selects mode 2, zero mode 1.
 * Return: nothing.
 *
 * Non-obvious: the three cases compile to compare-and-branch, not a jump
 * table, so the TU has no .rodata.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_002D6488.c, function func_L20_002DC390 (v1.01
 * address), author commit ee3fee45 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for
 * names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_002DC390 -> func_L20_002DC618; D_L20_0018C0D4 ->
 *     g_bPlayerMode; func_L20_002D7730 -> func_L20_002D79B0
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 *   - upstream's file comment about func_L20_002D7730 dropped (it is about
 *     that function, not this one); its one-line comment folded into this one
 *   - the opening brace moved onto the signature line (whitespace only:
 *     level_gate's seeded-C selftest arm seeds the first L20_c.tsv row and
 *     needs it there)
 */
void func_L20_002DC618(int a0) {
    int a1 = (a0 != 0) ? 2 : 1;

    switch (g_bPlayerMode) {
    case 0:
        func_L20_002D79B0(0, a1);
        break;
    case 1:
        func_L20_002D79B0(0x3F, a1);
        break;
    case 4:
        func_L20_002D79B0(0x8D, a1);
        break;
    }
}
