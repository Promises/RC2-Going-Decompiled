/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00440DF0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef signed int s32;

extern void func_L23_00440728(void);
extern void func_L23_00440C20(s32);

/**
 * func_L23_00440DF0 - prepare a level object, then start it.
 *
 * Calls func_L23_00440728(), then func_L23_00440C20(@arg0).
 *
 * @arg0: passed through to func_L23_00440C20.
 * Return: nothing.
 *
 * Non-obvious: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/23_damosel_orbit/L23_0041E418.c, function func_L23_0043F930
 * (v1.01 address), author commit 264cb2be (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_L23_0043F930 -> func_L23_00440DF0; func_L00_004287E8 ->
 *     func_L23_00440728; func_L00_00428CE0 -> func_L23_00440C20
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped; the
 *     typedefs it uses written in the TU (the file's own where it has them,
 *     else common.h's); only the file-level declarations it uses kept, without
 *     their comments
 *   - upstream's one-line comment folded into this one
 */
void func_L23_00440DF0(s32 arg0) {
    func_L23_00440728();
    func_L23_00440C20(arg0);
}
