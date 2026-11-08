/* Level 12 (Todano), USA v2.00: a level-only function built from C (task #2670,
 * row L12-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x00405BC0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L12/config/L12_c.tsv, the arm ee-gcc
 * 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G8, assembled by plain
 * GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef signed char s8;
typedef signed int s32;

/* The tracked object, read through $gp at -G8 (upstream's `sda` text is
   ignored by this cc1). */
extern u8 *D_L12_001B1320 __attribute__((sda));

/**
 * func_L12_00405BC0 - whether the tracked object is in a state that still
 * counts as open (the func_L12_003DF858 test, with one more state closed).
 *
 * Return: 0 when there is no tracked object (D_L12_001B1320 is null) or its
 * state byte (+0x20) is 6, 5, 8 or negative (bit 7 set); otherwise 1.
 *
 * Non-obvious: the separate `== 6` test and the `== 5 || == 8 || < 0` chain
 * are upstream's shape; its nonmatching notes record that merging the
 * adjacent 6/5 compares makes cc1 emit a range test the ROM lacks.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/12_todano/L12_00398E50-g8.c, function
 * func_L12_00404778 (v1.01 address), author commit 448f05b9 (Louis-Philippe
 * Le Sieur), from his nonmatching draft 45fddeea
 * (nonmatching/levels/func_L12_00404778.c; its signature and first line
 * survive), with five lines in the shape of func_L12_003DE4B0 by llesieur99
 * (ee3fee45). Attributions by `git blame -M -C -C -C` at the pin. Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * BYTE-IDENTICAL: VERBATIM but for names; the respellings, each to the target
 * of the v2.00 disc's own relocation at this place (task #2670 relverify):
 *   - func_L12_00404778 -> func_L12_00405BC0 (our split name at the v2.00 place);
 *   - D_L12_001B1360 -> D_L12_001B1320 (our split label);
 *   - `#include "common.h"` dropped: the typedefs used are written here and
 *     only this function's declaration kept; upstream's one-line comment
 *     folded into this one.
 */
s32 func_L12_00405BC0(void) {
    u8 *object = D_L12_001B1320;
    s32 result = 1;

    if (object == 0) {
        result = 0;
    } else {
        if (*(u8 *)(object + 0x20) == 6) {
            result = 0;
        } else if (*(u8 *)(object + 0x20) == 5 || *(u8 *)(object + 0x20) == 8
                 || *(s8 *)(object + 0x20) < 0) {
            result = 0;
        }
    }
    return result;
}
