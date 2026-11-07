/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x004307C0, in place
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

extern void func_L11_0032D918(u8 *a0, u32 a1, u32 a2, u32 a3);

/**
 * func_L11_004307C0 - apply the class setup to this object and its owner, or
 * to this object alone when they are the same.
 *
 * When the state's +0x27C owner pointer is non-null, the setup runs twice
 * (this object with arg1, the owner with arg2, both with arg3/arg4);
 * otherwise it runs once, and only when +0x43 differs from arg1.
 *
 * @arg0: the moby object; reads +0x43, +0x68 (state).
 * @arg1..arg4: the class-setup arguments, handed through unchanged.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_00425F10.c, function func_L11_0042EF90 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - func_L11_0042EF90 -> func_L11_004307C0 (this function's v2.00 split name);
 *   - func_002A7470 -> func_L11_0032D918 (v1.01-address callee respelled to the v2.00 level split name).
 * No codegen or addressing device is used; the statements are upstream's.
 */
void func_L11_004307C0(u8 *arg0, u32 arg1, u32 arg2, u32 arg3, u32 arg4) {
    u8 *s2 = *(u8 **)(arg0 + 0x68);

    if (*(u32 *)(s2 + 0x27C) != 0) {
        func_L11_0032D918(arg0, arg1, arg3, arg4);
        func_L11_0032D918(*(u8 **)(s2 + 0x27C), arg2, arg3, arg4);
    } else if (arg0[0x43] != arg1) {
        func_L11_0032D918(arg0, arg1, arg3, arg4);
    }
}
