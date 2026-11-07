/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x0041C978, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm ee-gcc
 * 2.9-ee-991111b/r4 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, assembled by
 * plain GNU as (the row's `as` column).
 */

typedef signed int s32;
extern void func_L23_003177F8(void);

/**
 * func_L23_0041C978 - run a level helper and report success.
 *
 * Calls func_L23_003177F8() and returns 1.
 *
 * No parameters.
 * Return: 1.
 *
 * Non-obvious: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/us2/levels/23_damosel_orbit/L23_0041C978.c, function
 * func_US2_L23_0041C978 (v2.00 address), author commit cde7e4dd (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v2.00-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_US2_L23_0041C978 -> func_L23_0041C978; func_L00_0030C930 ->
 *     func_L23_003177F8
 *   - `#include "include_asm.h"` dropped; the typedefs it uses written in the
 *     TU (the file's own where it has them, else common.h's); only the
 *     file-level declarations it uses kept, without their comments
 *   - upstream's one-line comment folded into this one
 */
s32 func_L23_0041C978(void) {
    func_L23_003177F8();
    return 1;
}
