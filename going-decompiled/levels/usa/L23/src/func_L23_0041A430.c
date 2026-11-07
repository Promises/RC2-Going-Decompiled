/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x0041A430, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef signed int s32;
typedef unsigned long u64;
extern s32 func_L23_0041B1A0(s32 arg0);

/**
 * func_L23_0041A430 - collects the indices (0..63) of the set bits of a 64-bit
 * mask into an array, keeping only indices func_L23_0041B1A0 accepts when a
 * check is asked for. (Upstream's comment: "Collects the set bits of the mask
 * whose index the shared predicate accepts.")
 *
 * @arg0: unused.  @arg1: the mask.  @arg2: the output array (up to 64 ints).
 * @arg3: nonzero to filter through func_L23_0041B1A0.
 * Return: the number of indices written.
 *
 * Non-obvious: func_L23_003C2238 is a second copy of the same code (same
 * catalogue fingerprint), at another place in this level.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_00416388.c, function
 * func_L23_00418F30 (v1.01 address), author commit adcbd51e (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED; the body's statements unchanged. Edits:
 *   - func_L23_00418F30 -> func_L23_0041A430 (our split name at the v2.00
 *     place)
 *   - the file's typedef `u64` = `unsigned long long` -> `unsigned long`
 *     (RULING #13197: s64/u64 as common.h spells them; ablation with `unsigned
 *     long long`: the s136 cc1 fails, `unsupported wide integer operation`);
 *     the body text is unchanged
 *   - func_L23_00419CA0 -> func_L23_0041B1A0
 *   - the file's s32 typedef carried
 *   - `#include "include_asm.h"` dropped: the TU declares what it uses, and
 *     upstream's comments are folded into this one
 */
s32 func_L23_0041A430(char *arg0, u64 arg1, s32 *arg2, s32 arg3) {
    s32 count = 0;
    s32 *out = arg2;
    s32 i;

    for (i = 0; i < 0x40; i++) {
        if ((arg1 & 1) != 0 && (arg3 == 0 || func_L23_0041B1A0(i) != 0)) {
            *out = i;
            count++;
            out++;
        }
        arg1 >>= 1;
    }
    return count;
}
