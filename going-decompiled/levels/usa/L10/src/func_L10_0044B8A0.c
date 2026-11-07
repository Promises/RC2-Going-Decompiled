/* Level 10 (Hrugis Cloud), USA v2.00: a level-only function built from C
 * (task #2657, RULING #13291 row L10-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x0044B8A0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L10/config/L10_c.tsv, the arm ee-gcc
 * 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G8, plain as.
 */

/* A level pointer word at 0x001ADE70 (our split label). `sda` is upstream's
   own text, ignored by this cc1: at -G8 the 4-byte word is $gp-relative by
   its size alone, which is what the ROM's one-instruction load needs. */
extern void *D_L10_001ADE70 __attribute__((sda));

/**
 * func_L10_0044B8A0 - return the pointer word at 0x001ADE70.
 *
 * Return: the word.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/10_hrugis_cloud/L10_00424028.c, function
 * func_L10_00449AD0 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Its body equals func_00115200's but for names, a single trivial `return`
 * line that carries no further credit (RULING #13541 rev 2). Adopted with
 * the authors' permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED;
 * the body's statement unchanged, the edits:
 *   - built at -G8 where upstream's cflags give its file -G0 (a gflag
 *     setting in L10_c.tsv, not a device; NOTE #13281 code `G`): at -G0 cc1
 *     splits the load into an absolute %hi/%lo pair where the ROM has the
 *     one $gp-relative lw (ablation, task #2657, upstream's -G0, same arm: 3
 *     words where the disc has 2, all 3 differing);
 *   - func_L10_00449AD0 -> func_L10_0044B8A0 (our split name at the v2.00 place);
 *   - D_L10_001ADE90 -> D_L10_001ADE70 (our split label at the target of the
 *     v2.00 disc's own relocation, task #2657 relverify);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped (the
 *     function uses no typedef) and only its declaration kept.
 */
void *func_L10_0044B8A0(void) {
    return D_L10_001ADE70;
}
