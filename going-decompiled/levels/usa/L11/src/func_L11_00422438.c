/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x00422438, in place
 * of its split asm; the row is config/L11_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;
typedef int TI128 __attribute__((mode(TI)));

extern u8 *func_L11_00330218(s32 arg0, s32 arg1, s32 arg2);

/**
 * func_L11_00422438 - report whether the resource lookup found the measured
 * resource, and tick the object's timer down by its rate when it did.
 *
 * Looks the state's +0x280 resource id up with class mask 0x10000; when found,
 * subtracts the found node's +0x2C rate from the state's +0x20 timer and
 * answers 1, else 0.
 *
 * @arg0: the moby object; reads +0x68 (state).
 * Return: 1 when the resource was found (timer ticked), 0 otherwise.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_003D6270.c, function func_L11_00420C38 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - func_L11_00420C38 -> func_L11_00422438 (this function's v2.00 split name);
 *   - func_002A9640 -> func_L11_00330218 (v1.01-address callee respelled to the v2.00 level split name).
 * No codegen or addressing device is used; the statements are upstream's.
 */
s32 func_L11_00422438(u8 *arg0) {
    u8 *s0 = *(u8 **)(arg0 + 0x68);
    u8 *v0 = func_L11_00330218(*(s32 *)(s0 + 0x280), 0x10000, 0);
    if (v0 != 0) {
        *(f32 *)(s0 + 0x20) = *(f32 *)(s0 + 0x20) - *(f32 *)(v0 + 0x2C);
        return 1;
    }
    return 0;
}
