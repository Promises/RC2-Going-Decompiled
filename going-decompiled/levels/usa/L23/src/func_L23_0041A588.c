/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x0041A588, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm ee-gcc
 * 2.9-ee-991111b/r4 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, assembled by
 * plain GNU as (the row's `as` column).
 */

typedef signed short s16;
typedef unsigned short u16;

/**
 * func_L23_0041A588 - push a value into a moby state's three-entry history ring.
 *
 * Advances the ring index at state +0x39E to (index + 1) % 3, stores the
 * previous current value (state +0x394) in that slot of the halfword ring at
 * state +0x398, and makes @arg1 the new current value.
 *
 * @arg0: the moby; its +0x68 points at the state. @arg1: the new value.
 * Return: nothing.
 *
 * Non-obvious: the modulo by 3 is a real signed division (cc1's multiply-high
 * sequence), as the ROM has it.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/23_damosel_orbit/L23_00416388.c, function func_L23_00419088
 * (v1.01 address), author commit ee3fee45 (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_L23_00419088 -> func_L23_0041A588
 *   - `#include "include_asm.h"` dropped; the typedefs it uses written in the
 *     TU (the file's own where it has them, else common.h's); only the
 *     file-level declarations it uses kept, without their comments
 *   - upstream's one-line comment folded into this one
 */
void func_L23_0041A588(char *arg0, s16 arg1) {
    char *p = *(char **)(arg0 + 0x68);
    s16 n = *(s16 *)(p + 0x39E);
    u16 old = *(u16 *)(p + 0x394);
    int off;
    int r;

    r = (n + 1) % 3;
    off = r * 2;
    *(s16 *)(p + 0x39E) = r;
    *(s16 *)(p + off + 0x398) = old;
    *(s16 *)(p + 0x394) = arg1;
}
