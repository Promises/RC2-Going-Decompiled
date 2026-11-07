/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x0041B1E0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm ee-gcc
 * 2.9-ee-991111b/r4 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, assembled by
 * plain GNU as (the row's `as` column).
 */

typedef signed int s32;
typedef unsigned long long u64;
extern s32 func_L23_0041B1A0(s32 arg0);

/**
 * func_L23_0041B1E0 - test that a 64-bit mask has exactly one bit set and that bit is not taken.
 *
 * Counts the set bits of @arg0 over all 64 positions, giving up as soon as a
 * second one is found. With exactly one set bit at index i it returns 1 when
 * func_L23_0041B1A0(i) returns 0.
 *
 * @arg0: the 64-bit mask.
 * Return: 1 or 0.
 *
 * Non-obvious: u64 is `unsigned long long` (a 64-bit EE register). The row is
 * on the sdk29 arm, so no `long long` respelling applies (the audit's `l`
 * code is file-level and not set for this function).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/23_damosel_orbit/L23_00416388.c, function func_L23_00419CE0
 * (v1.01 address), author commit 86ff805f (bigmak94). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_L23_00419CE0 -> func_L23_0041B1E0; func_L23_00419CA0 ->
 *     func_L23_0041B1A0
 *   - `#include "include_asm.h"` dropped; the typedefs it uses written in the
 *     TU (the file's own where it has them, else common.h's); only the
 *     file-level declarations it uses kept, without their comments
 *   - upstream's one-line comment folded into this one
 */
s32 func_L23_0041B1E0(u64 arg0) {
    s32 count = 0;
    s32 found = 0;
    s32 i = 0;

    do {
        if (arg0 & 1) {
            count++;
            if (count >= 2)
                return 0;
            found = i;
        }
        i++;
        arg0 >>= 1;
    } while (i < 0x40);

    if (count == 0)
        return 0;
    if (count == 1) {
        if (func_L23_0041B1A0(found) == 0)
            return 1;
    }
    return 0;
}
