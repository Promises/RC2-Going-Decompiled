/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x003F4410, in place
 * of its split asm; the row is config/L11_c.tsv, the arm the SDK compiler
 * (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;
typedef int TI128 __attribute__((mode(TI)));

/**
 * func_L11_003F4410 - report the signed area of the triangle (arg0, arg1, arg2).
 *
 * @arg0/@arg1/@arg2: xy pairs; each is read at [0] and [1].
 * Return: (a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x), twice the
 * signed area of the triangle, positive when arg0->arg1->arg2 turns left.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_003D6270.c, function func_L11_003F2F00 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - func_L11_003F2F00 -> func_L11_003F4410 (this function's v2.00 split name). No other symbol appears.
 * No codegen or addressing device is used; the statements are upstream's.
 */
f32 func_L11_003F4410(f32 *arg0, f32 *arg1, f32 *arg2) {
    return (arg0[0] - arg2[0]) * (arg1[1] - arg2[1])
         - (arg0[1] - arg2[1]) * (arg1[0] - arg2[0]);
}
