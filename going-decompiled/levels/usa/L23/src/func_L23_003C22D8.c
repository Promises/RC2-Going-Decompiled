/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x003C22D8, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef long s64;
extern int *D_L23_001B3628 __attribute__((section(".sdata")));
extern int *D_L23_001FF280[];
extern char D_L23_00259548[];
extern int func_L23_0041A570(s64 v, char *arg);

/**
 * func_L23_003C22D8 - looks up path `index` in the level's tables: v =
 * D_L23_001B3628[index]. If func_L23_0041A570 finds bit `arg` set in the first
 * of v's two 64-bit masks (16 bytes each at D_L23_00259548), returns 0.
 * Otherwise returns the count at *D_L23_001FF280[v] minus one, after testing
 * bit `arg` of the second mask too.
 *
 * @arg0: unused.  @index: the path index.  @arg: the bit number.
 * Return: 0 if the first mask has the bit, else the path's count minus one.
 *
 * Non-obvious: the second test's result is discarded, and the table entry is
 * read twice, as upstream (and the ROM) do.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_0035CE90.c, function
 * func_L23_003C0E98 (v1.01 address), author commit ee3fee45 (llesieur99), last
 * touched fa31bfc3 (bigmak94). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082, #13291). ADAPTED; the body's statements unchanged.
 * Edits:
 *   - func_L23_003C0E98 -> func_L23_003C22D8 (our split name at the v2.00
 *     place)
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on
 *     D_L23_001B3628 (ablation: without it, the body is 44 words, not 43, and
 *     25 differ) (RULING #8620: an addressing-model device on an extern
 *     DECLARATION, EE only; upstream's patched compiler reads sda, ours does
 *     not)
 *   - `long long` -> `s64` (RULING #13197: a 64-bit EE integer spelled
 *     s64/u64, local typedefs as common.h's; ablation with `long long`: 2 of
 *     43 words differ), in the cast and in the callee prototype
 *   - D_L23_001B36A8 -> D_L23_001B3628, D_L23_001FF300 -> D_L23_001FF280,
 *     D_L23_002595C8 -> D_L23_00259548: address-named externs for addresses
 *     inside larger split labels (RULING #13291 section 2)
 *   - func_419070 -> func_L23_0041A570
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the TU
 *     declares what it uses, and upstream's comments are folded into this one
 */
int func_L23_003C22D8(int arg0, int index, char *arg) {
    int v;
    int w;
    int n;

    v = D_L23_001B3628[index];
    if (func_L23_0041A570(*(s64 *)(D_L23_00259548 + (v << 4)), arg) != 0)
        return 0;
    w = D_L23_001B3628[index];
    n = *D_L23_001FF280[w] - 1;
    func_L23_0041A570(*(s64 *)(D_L23_00259548 + (w << 4) + 8), arg);
    return n;
}
