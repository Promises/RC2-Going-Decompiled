/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00428CB8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;

extern void func_L23_00428CE8(u8 *);
extern void func_L23_00428DB0(u8 *);

/**
 * func_L23_00428CB8 - per-frame update of a moby: run its state step, then count the frame.
 *
 * Calls func_L23_00428CE8(moby), then func_L23_00428DB0(moby).
 *
 * @arg0: the moby.
 * Return: nothing.
 *
 * Non-obvious: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/23_damosel_orbit/L23_0041E418.c, function func_L23_004277B8
 * (v1.01 address), author commit 97da1aea (Louis-Philippe Le Sieur). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_L23_004277B8 -> func_L23_00428CB8; func_L23_004277E8 ->
 *     func_L23_00428CE8; func_L23_004278B0 -> func_L23_00428DB0
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped; the
 *     typedefs it uses written in the TU (the file's own where it has them,
 *     else common.h's); only the file-level declarations it uses kept, without
 *     their comments
 */
void func_L23_00428CB8(u8 *arg0) {
    func_L23_00428CE8(arg0);
    func_L23_00428DB0(arg0);
}
