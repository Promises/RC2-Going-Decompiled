/* Level 09 (Dobbo), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2580, RULING #13291 row L09-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x00406020, in place of its
 * split asm; the row is going-decompiled/levels/usa/L09/config/L09_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column; the plain-`as` ablation is in the row's FACT, task #2580).
 */

typedef unsigned char u8;
typedef int s32;
typedef float f32;

/* The level's speed coefficient for this state (our label at 0x001B13A0),
 * $gp-relative at -G8 by its size. Upstream's `sda` attribute text is kept;
 * this cc1 ignores it (a warning), so it emits nothing. */
extern f32 D_L09_001B13A0 __attribute__((sda));
/* L09's copy of the boot's func_002A7470 (sets an object's state). */
extern void func_L09_00317818(char *a, s32 b, s32 c, s32 d);

/**
 * func_L09_00406020 - put an object into state 7 with its speed parameters.
 *
 * Writes 3.0 to +0x1D4 and D_L09_001B13A0 / 60 to +0x1D0 of the object's
 * state (+0x68); then, unless the object's state byte +0x43 is already 7,
 * calls func_L09_00317818(@a0, 7, 0, @a1). Twin of func_L09_00406080
 * (state 6).
 *
 * @a0: the object.
 * @a1: passed through to the state change.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_003FEA48-g8.c, function func_L09_00404B90
 * (v1.01 address), author commit 264cb2be (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2580 relverify):
 *   - func_L09_00404B90 -> func_L09_00406020 (our split name);
 *   - D_L09_001B13F0 -> D_L09_001B13A0 (our label; `sda` text kept);
 *   - callee func_L08_00316678 -> func_L09_00317818 (L09's copy);
 *   - `#include "common.h"` dropped: the typedefs this function uses are
 *     written here and only its declarations kept.
 */
void func_L09_00406020(char *a0, s32 a1) {
    char *v = a0;
    f32 f = D_L09_001B13A0 * 0.0166666675f;
    char *p = *(char **)(v + 0x68);
    *(f32 *)(p + 0x1D4) = 3.0f;
    *(f32 *)(p + 0x1D0) = f;
    if (*(u8 *)(v + 0x43) == 7) {
        return;
    }
    func_L09_00317818(a0, 7, 0, a1);
}
