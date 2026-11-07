/* Level 10 (Hrugis Cloud), USA v2.00: a level-only function built from C
 * (task #2657, RULING #13291 row L10-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x00418C40, in place of its split asm; the row is
 * going-decompiled/levels/usa/L10/config/L10_c.tsv, the arm ee-gcc
 * 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G8, plain as.
 */

typedef signed int s32;

/* A one-shot level flag word at 0x001B0EE4 (our split label). `sda` is
   upstream's own text, ignored by this cc1 ("attribute directive ignored"):
   at -G8 the 4-byte word is $gp-relative by its size alone. */
extern s32 D_L10_001B0EE4 __attribute__((sda));

/**
 * func_L10_00418C40 - read and clear the flag word at 0x001B0EE4.
 *
 * Return: the word's value before it was cleared to 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/10_hrugis_cloud/L10_003170D0-g8.c,
 * function func_L10_004176A0 (v1.01 address), author commit 431ac503
 * (llesieur99). Adopted with the authors' permission (RULINGs #10405,
 * #12108, #13082, #13291). BYTE-IDENTICAL: VERBATIM but for names (no
 * device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2657 relverify):
 *   - func_L10_004176A0 -> func_L10_00418C40 (our split name at the v2.00 place);
 *   - D_L10_001B0F44 -> D_L10_001B0EE4 (our split label at the v2.00 target);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the s32
 *     typedef is written here and only this function's declaration kept.
 */
s32 func_L10_00418C40(void) {
    s32 v = D_L10_001B0EE4;
    D_L10_001B0EE4 = 0;
    return v;
}
