/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x003C2238, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned long u64;
extern int func_L23_0041B1A0(int);

/**
 * func_L23_003C2238 - collects the indices (0..63) of the set bits of a 64-bit
 * mask into an array, keeping only indices func_L23_0041B1A0 accepts when a
 * check is asked for.
 *
 * @arg0: unused.  @v: the mask.  @arg2: the output array (up to 64 ints).
 * @arg3: nonzero to filter through func_L23_0041B1A0.
 * Return: the number of indices written.
 *
 * Non-obvious: func_L23_0041A430 is a second copy of the same code (same
 * catalogue fingerprint), at another place in this level.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_0035CE90.c, function
 * func_L23_003C0DF8 (v1.01 address), author commit 91e04732 (llesieur99), last
 * touched fa31bfc3 (bigmak94). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082, #13291). ADAPTED; the body's statements unchanged.
 * Edits:
 *   - func_L23_003C0DF8 -> func_L23_003C2238 (our split name at the v2.00
 *     place)
 *   - `unsigned long long` -> `u64` (RULING #13197: a 64-bit EE integer
 *     spelled s64/u64, local typedefs as common.h's; ablation with `long
 *     long`: the s136 cc1 fails, `unsupported wide integer operation`)
 *   - func_419CA0 -> func_L23_0041B1A0
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the TU
 *     declares what it uses, and upstream's comments are folded into this one
 */
int func_L23_003C2238(int arg0, u64 v, int *arg2, int arg3) {
    int *out = arg2;
    int i = 0;
    int n = 0;

    while (i < 0x40) {
        if (v & 1) {
            if (arg3 == 0 || func_L23_0041B1A0(i) != 0) {
                *out = i;
                n++;
                out++;
            }
        }
        i++;
        v >>= 1;
    }
    return n;
}
