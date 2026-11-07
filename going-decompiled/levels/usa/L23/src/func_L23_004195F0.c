/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x004195F0, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned short u16;
typedef signed int s32;
extern void func_L23_002F8B38(char *);
extern void func_L23_002F8B68(char *);

/**
 * func_L23_004195F0 - steps the two values at state +0x348 and +0x34C through
 * the level's functions at 0x002F8B38 and 0x002F8B68, and advances two
 * counters, holding each at 0xFFFF: the 16-bit one at +0x96 and the 32-bit one
 * at state +0x5AC. (Upstream's comment: "Refreshes two measured state pairs and
 * advances the two counters under their limits.")
 *
 * @arg0: the object (moby); +0x68 points at its state block.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_00416388.c, function
 * func_L23_004180F0 (v1.01 address), author commit 773eafc2 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED in names only (verbatim but for names; the audit's file-
 * level `long long` code does not apply, since this body's own text has no
 * `long long`, FACT #13321); the body's statements unchanged. Edits:
 *   - func_L23_004180F0 -> func_L23_004195F0 (our split name at the v2.00
 *     place)
 *   - func_00282948 -> func_L23_002F8B38, func_00282978 -> func_L23_002F8B68
 *   - the file's u16/s32 typedefs carried
 *   - `#include "include_asm.h"` dropped: the TU declares what it uses, and
 *     upstream's comments are folded into this one
 */
void func_L23_004195F0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    func_L23_002F8B38(state + 0x348);
    func_L23_002F8B68(state + 0x34C);
    if (*(u16 *)(arg0 + 0x96) <= 0xFFFE)
        *(u16 *)(arg0 + 0x96) = *(u16 *)(arg0 + 0x96) + 1;
    if (*(s32 *)(state + 0x5AC) <= 0xFFFE)
        *(s32 *)(state + 0x5AC) = *(s32 *)(state + 0x5AC) + 1;
}
