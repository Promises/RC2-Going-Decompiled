/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x003C2388, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm ee-gcc
 * 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, assembled by plain
 * GNU as (the row's `as` column).
 */

/**
 * func_L23_003C2388 - records the current node id (state +0x2B4) in a three-
 * entry history at state +0x2B8 (its index at +0x2BE advances mod 3), then
 * makes arg1 the current node id.
 *
 * @arg0: the object (moby); +0x68 points at its state block.
 * @arg1: the new node id.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_0035CE90.c, function
 * func_L23_003C0F48 (v1.01 address), author commit 91e04732 (llesieur99), last
 * touched fe5602ab (bigbeno37). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082, #13291). BYTE-IDENTICAL (the audit's verdict):
 * verbatim but for names, plus any edit listed below; the body's statements
 * unchanged. Edits:
 *   - func_L23_003C0F48 -> func_L23_003C2388 (our split name at the v2.00
 *     place)
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the TU
 *     declares what it uses, and upstream's comments are folded into this one
 */
void func_L23_003C2388(char *arg0, int arg1) {
    char *p = *(char **)(arg0 + 0x68);
    short n = *(short *)(p + 0x2BE);
    unsigned short old = *(unsigned short *)(p + 0x2B4);
    int off;
    int r;

    r = (n + 1) % 3;
    off = r * 2;
    *(short *)(p + 0x2BE) = r;
    *(short *)(p + off + 0x2B8) = old;
    *(short *)(p + 0x2B4) = arg1;
}
