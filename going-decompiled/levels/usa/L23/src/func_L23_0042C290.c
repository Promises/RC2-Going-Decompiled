/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x0042C290, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm ee-gcc
 * 2.9-ee-991111b/r4 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, assembled by
 * plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef unsigned short u16;

/**
 * func_L23_0042C290 - count a moby's frames, saturating at 0xFFFF.
 *
 * Increments the halfword at +0x96 while it is at most 0xFFFE.
 *
 * @arg0: the moby.
 * Return: nothing.
 *
 * Non-obvious: the same body is at 0x00428DB0 and 0x0042B4F8 (separate rows).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/23_damosel_orbit/L23_0041E418.c, function func_L23_0042AD90
 * (v1.01 address), author commit 264cb2be (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_L23_0042AD90 -> func_L23_0042C290
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped; the
 *     typedefs it uses written in the TU (the file's own where it has them,
 *     else common.h's); only the file-level declarations it uses kept, without
 *     their comments
 *   - upstream's one-line comment folded into this one
 */
void func_L23_0042C290(u8 *arg0) {
    u16 *counter = (u16 *)(arg0 + 0x96);
    u16 value = *counter;

    if (value <= 0xFFFE) {
        *counter = value + 1;
    }
}
