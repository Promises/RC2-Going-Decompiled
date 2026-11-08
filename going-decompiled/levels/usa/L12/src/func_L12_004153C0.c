/* Level 12 (Todano), USA v2.00: a level-only function built from C (task #2670,
 * row L12-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x004153C0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L12/config/L12_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

/* The level's tuning words at 0x001B17E4..0x001B1808, read through $gp at
   -G8 (upstream's `sda` text is ignored by this cc1). */
extern s32 D_L12_001B17E8 __attribute__((sda));
extern s32 D_L12_001B17EC __attribute__((sda));
extern s32 D_L12_001B17F0 __attribute__((sda));
extern u16 D_L12_001B17E4 __attribute__((sda));
extern f32 D_L12_001B1808 __attribute__((sda));
/* The level's copies of boot code, named by their v2.00 places. */
extern s32 func_L12_002F21C0(void *arg0);
extern s32 func_L12_0031F3E8(s32 arg0, s32 arg1);
extern void func_L12_0032A7B0(void *arg0, void *arg1, s32 arg2, f32 x, s32 arg3);

/**
 * func_L12_004153C0 - step an object's slot timers and restart its tracked
 * sub-object.
 *
 * When the state's re-arm byte (+0x2EA) is set, clears it and, if the timer
 * at +0x2E0 is below D_L12_001B17E8, reloads it with a random value in
 * [D_L12_001B17EC, D_L12_001B17F0] (func_L12_0031F3E8). Steps the four
 * halfword timers at +0x2E0, +0x2E2, +0x2E6 and +0x2E8 with
 * func_L12_002F21C0; when the first one runs out, +0x2E8 is reloaded from
 * D_L12_001B17E4 before it is stepped. Bumps the object's halfword counter
 * (+0x96), saturating at 0xFFFF, then calls func_L12_0032A7B0 on the
 * sub-object at state+0x2F8 with state+0x420, 3, D_L12_001B1808 and 1.
 *
 * @object: the object; its state block is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/12_todano/L12_00398E50-g8.c, function
 * func_L12_00413F68 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; the respellings, each to
 * the target of the v2.00 disc's own relocation at this place (task #2670
 * relverify):
 *   - func_L12_00413F68 -> func_L12_004153C0 (our split name at the v2.00 place);
 *   - func_2F15C8 -> func_L12_002F21C0, func_31E2E0 -> func_L12_0031F3E8,
 *     func_3296A8 -> func_L12_0032A7B0 (our split names at the v2.00 targets);
 *   - D_L12_001B1834 -> D_L12_001B17E4, D_L12_001B1838 -> D_L12_001B17E8,
 *     D_L12_001B183C -> D_L12_001B17EC, D_L12_001B1840 -> D_L12_001B17F0,
 *     D_L12_001B1858 -> D_L12_001B1808 (our split labels);
 *   - `#include "common.h"` dropped: the typedefs used are written here and
 *     only this function's declarations kept; upstream's one-line comment
 *     folded into this one.
 */
void func_L12_004153C0(u8 *object) {
    u8 *state = *(u8 **)(object + 0x68);
    u16 counter;

    if (*(u8 *)(state + 0x2EA) != 0) {
        *(u8 *)(state + 0x2EA) = 0;
        if (*(s16 *)(state + 0x2E0) < D_L12_001B17E8)
            *(u16 *)(state + 0x2E0) = func_L12_0031F3E8(D_L12_001B17EC, D_L12_001B17F0);
    }
    if (func_L12_002F21C0(state + 0x2E0) == 0)
        *(u16 *)(state + 0x2E8) = D_L12_001B17E4;
    func_L12_002F21C0(state + 0x2E2);
    func_L12_002F21C0(state + 0x2E6);
    func_L12_002F21C0(state + 0x2E8);
    counter = *(u16 *)(object + 0x96);
    if (counter <= 0xFFFE)
        *(u16 *)(object + 0x96) = counter + 1;
    func_L12_0032A7B0(*(void **)(state + 0x2F8), state + 0x420, 3, D_L12_001B1808, 1);
}
