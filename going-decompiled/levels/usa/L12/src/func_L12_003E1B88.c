/* Level 12 (Todano), USA v2.00: a level-only function built from C (task #2670,
 * row L12-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x003E1B88, in place of its split asm; the row is
 * going-decompiled/levels/usa/L12/config/L12_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed int s32;
typedef float f32;

/* The level's copies of boot code, named by their v2.00 places. */
extern void func_L12_00323C00(void *arg0, void *arg1);
extern void func_L12_00327A50(void *arg0, void *arg1, s32 arg2, f32 x, f32 y);

/**
 * func_L12_003E1B88 - re-arm an object's two-slot primitive after its update.
 *
 * Calls func_L12_00323C00(object, state+0x50), then func_L12_00327A50 on the
 * slots at state+0x6C0 (flag 1) and state+0x640 (flag 0), each with 0.03 and
 * 0.3, and bumps the object's halfword counter (+0x96), saturating at 0xFFFF.
 *
 * @object: the object; its state block is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/12_todano/L12_00398E50.c, function
 * func_L12_003E07B0 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; the respellings, each to
 * the target of the v2.00 disc's own relocation at this place (task #2670
 * relverify):
 *   - func_L12_003E07B0 -> func_L12_003E1B88 (our split name at the v2.00 place);
 *   - func_322AF8 -> func_L12_00323C00, func_326948 -> func_L12_00327A50
 *     (our split names at the v2.00 targets);
 *   - `#include "common.h"` dropped: the typedefs used are written here and
 *     only this function's declarations kept; upstream's one-line comment
 *     folded into this one.
 */
void func_L12_003E1B88(u8 *object) {
    u8 *state = *(u8 **)(object + 0x68);
    u16 counter;

    func_L12_00323C00(object, state + 0x50);
    func_L12_00327A50(object, state + 0x6C0, 1, 0.03f, 0.3f);
    func_L12_00327A50(object, state + 0x640, 0, 0.03f, 0.3f);
    counter = *(u16 *)(object + 0x96);
    if (counter <= 0xFFFE)
        *(u16 *)(object + 0x96) = counter + 1;
}
