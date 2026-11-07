/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x003C1278, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern void func_L23_002F8B38(char *);
extern void func_L23_002F8B68(char *);

/**
 * func_L23_003C1278 - steps the two values of the object's state block at
 * +0x250 and +0x254 through the level's functions at 0x002F8B38 and 0x002F8B68,
 * then advances the 16-bit counter at +0x96, holding it at 0xFFFF.
 *
 * @arg0: the object (moby); +0x68 points at its state block.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_0035CE90.c, function
 * func_L23_003BFE38 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED in names only (verbatim but for names; the audit's file-
 * level `long long` code does not apply, since this body's own text has no
 * `long long`, FACT #13321); the body's statements unchanged. Edits:
 *   - func_L23_003BFE38 -> func_L23_003C1278 (our split name at the v2.00
 *     place)
 *   - func_2F7DD0 -> func_L23_002F8B38, func_2F7E00 -> func_L23_002F8B68 (the
 *     v2.00 disc's call targets at this place)
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the TU
 *     declares what it uses, and upstream's comments are folded into this one
 */
void func_L23_003C1278(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    func_L23_002F8B38(state + 0x250);
    func_L23_002F8B68(state + 0x254);
    if (*(unsigned short *)(arg0 + 0x96) <= 0xFFFE)
        *(unsigned short *)(arg0 + 0x96) += 1;
}
