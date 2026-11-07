/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x0042C170, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef short s16;
typedef int s32;
extern s16 D_L23_001B1B44 __attribute__((sda));
extern s16 D_L23_001B1B46 __attribute__((sda));
extern void func_L23_00307910(s32);

/**
 * func_L23_0042C170 - release the level's two sound handles.
 *
 * For each of the halfword handles D_L23_001B1B44 and D_L23_001B1B46 that is
 * not -1, calls func_L23_00307910(handle) and sets it to -1.
 *
 * No parameters.
 * Return: nothing.
 *
 * Non-obvious: D_L23_001B1B44 lies inside the split's larger label
 * D_L23_001B1B40 (+0x4); the address-named extern is valued from its name by
 * tools/levels/level_elf.py (RULING #13291 section 2). Upstream's
 * `__attribute__((sda))` text on the extern declarations is kept as written;
 * this cc1 ignores the attribute.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/23_damosel_orbit/L23_0041E418-g8.c, function func_L23_0042AC70
 * (v1.01 address), author commit 264cb2be (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_L23_0042AC70 -> func_L23_0042C170; D_L23_001B1B94 ->
 *     D_L23_001B1B44; D_L23_001B1B96 -> D_L23_001B1B46; func_00291450 ->
 *     func_L23_00307910
 *   - `#include "include_asm.h"` dropped; the typedefs it uses written in the
 *     TU (the file's own where it has them, else common.h's); only the
 *     file-level declarations it uses kept, without their comments
 *   - upstream's one-line comment folded into this one
 */
void func_L23_0042C170(void) {
    s16 handle = D_L23_001B1B44;

    if (handle != -1) {
        func_L23_00307910(handle);
        D_L23_001B1B44 = -1;
    }
    handle = D_L23_001B1B46;
    if (handle != -1) {
        func_L23_00307910(handle);
        D_L23_001B1B46 = -1;
    }
}
