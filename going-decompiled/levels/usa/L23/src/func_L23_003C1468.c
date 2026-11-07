/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x003C1468, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern void func_L23_00325548(int, int, int);
extern void func_L23_003C2DC8(char *arg0);
extern void func_L23_00368398(int);

/**
 * func_L23_003C1468 - resets the object: releases its two handles
 * (func_L23_003C2DC8), clears state +0x178, sets 0x41 and clears 0x1000 in the
 * flags at +0x34 of both the object and the object at state +0x2A0, and zeroes
 * +0x98 of both. Then func_L23_00325548(object, 0xD, 4), clears state +0x25C,
 * and releases the handle at state +0x3B4 through func_L23_00368398 unless it
 * is -1, setting it back to -1.
 *
 * @arg0: the object (moby); +0x68 points at its state block.
 * Return: nothing.
 *
 * Non-obvious: func_L23_00325548 is declared with an int first parameter and
 * called with the char pointer, as upstream does.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_0035CE90.c, function
 * func_L23_003C0028 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED in names only (verbatim but for names; the audit's file-
 * level `long long` code does not apply, since this body's own text has no
 * `long long`, FACT #13321); the body's statements unchanged. Edits:
 *   - func_L23_003C0028 -> func_L23_003C1468 (our split name at the v2.00
 *     place)
 *   - func_L23_003C1988 -> func_L23_003C2DC8, func_3242D0 ->
 *     func_L23_00325548, func_367150 -> func_L23_00368398
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the TU
 *     declares what it uses, and upstream's comments are folded into this one
 */
void func_L23_003C1468(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    int none = -1;

    func_L23_003C2DC8(arg0);
    *(short *)(state + 0x178) = 0;
    *(unsigned short *)(*(char **)(state + 0x2A0) + 0x34) |= 0x41;
    *(unsigned short *)(*(char **)(state + 0x2A0) + 0x34) &= 0xEFFF;
    *(int *)(*(char **)(state + 0x2A0) + 0x98) = 0;
    *(int *)(arg0 + 0x98) = 0;
    *(unsigned short *)(arg0 + 0x34) = (*(unsigned short *)(arg0 + 0x34) | 0x41) & 0xEFFF;
    func_L23_00325548(arg0, 0xD, 4);
    *(int *)(state + 0x25C) = 0;
    if (*(int *)(state + 0x3B4) != none) {
        func_L23_00368398(*(int *)(state + 0x3B4));
        *(int *)(state + 0x3B4) = none;
    }
}
