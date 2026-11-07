/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x0041A4D0, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef signed int s32;
typedef unsigned long u64;
extern s32 func_L23_0041A570(u64, s32);
extern int *D_L23_001FF280[];
extern char D_L23_00259548[];

/**
 * func_L23_0041A4D0 - looks up slot `index` of the object's state (+0x230). If
 * func_L23_0041A570 finds bit `arg` set in the first of that slot's two 64-bit
 * masks (16 bytes each at D_L23_00259548), returns 0. Otherwise returns the
 * slot's count at *D_L23_001FF280[v] minus one, after testing the second mask
 * too. (Upstream's comment: "Returns 0 when bit arg of the slot's first mask is
 * set; otherwise tests the second mask and returns the slot's count minus
 * one.")
 *
 * @arg0: the object (moby); +0x68 points at its state block.
 * @index: the slot index.  @arg: the bit number.
 * Return: 0 or the count minus one.
 *
 * Non-obvious: the second test's result is discarded.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_00416388.c, function
 * func_L23_00418FD0 (v1.01 address), author commit d2e11c19 (bigmak94).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED; the body's statements unchanged. Edits:
 *   - func_L23_00418FD0 -> func_L23_0041A4D0 (our split name at the v2.00
 *     place)
 *   - the file's typedef `u64` = `unsigned long long` -> `unsigned long`
 *     (RULING #13197; ablation with `unsigned long long`: 2 of 40 words
 *     differ); the body text is unchanged
 *   - D_L23_001FF300 -> D_L23_001FF280, D_L23_002595C8 -> D_L23_00259548
 *     (address-named externs inside larger split labels, RULING #13291 section
 *     2)
 *   - func_L23_00419070 -> func_L23_0041A570
 *   - the file's s32 typedef carried
 *   - `#include "include_asm.h"` dropped: the TU declares what it uses, and
 *     upstream's comments are folded into this one
 */
s32 func_L23_0041A4D0(char *arg0, s32 index, s32 arg) {
    s32 *slots = (s32 *)(*(char **)(arg0 + 0x68) + 0x230);
    s32 *slot = &slots[index];
    s32 v;
    s32 n;

    if (func_L23_0041A570(*(u64 *)(D_L23_00259548 + (*slot << 4)), arg) != 0)
        return 0;
    v = *slot;
    n = *D_L23_001FF280[v] - 1;
    func_L23_0041A570(*(u64 *)(D_L23_00259548 + (v << 4) + 8), arg);
    return n;
}
