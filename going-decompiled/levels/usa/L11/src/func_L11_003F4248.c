/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x003F4248, in place
 * of its split asm; the row is config/L11_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack.
 */

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;
typedef int TI128 __attribute__((mode(TI)));

extern void func_L11_0032DBF8(u8 *arg0, s32 arg1, s32 arg2);
extern s32 func_L11_0032DCC8(s32 arg0);
extern s32 func_L11_004424D0(s32 arg0);
extern s32 D_L11_001B051C __attribute__((sda));
extern s32 D_L11_001B053C __attribute__((sda));

/**
 * func_L11_003F4248 - pick this object's retirement path.
 *
 * Does nothing while either twin record halfword (state +0x278 / +0x27A) is
 * 2 or more; otherwise retires the object with code 7 when the level gate
 * helper answers non-zero for D_L11_001B053C and this class instance's count
 * is below the .lit threshold D_L11_001B051C, else with code 6.
 *
 * @arg0: the moby object; reads +0xB8 (class instance), +0x68 (state).
 *
 * Non-obvious: both thresholds live in the level .lit and are reached through
 * $gp at this -G8 arm.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_003D6270-g8.c, function func_L11_003F2D38 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "include_asm.h"` dropped (nothing in the function used it);
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - `__attribute__((sda))` kept verbatim (this toolchain ignores it, warning
 *     only; measured inert at this arm and -G flag);
 *   - func_L11_003F2D38 -> func_L11_003F4248 (this function's v2.00 split name);
 *   - D_L11_001B056C -> D_L11_001B051C, D_L11_001B058C -> D_L11_001B053C (exact v2.00 split dlabels);
 *   - func_002A7750 -> func_L11_0032DBF8, func_002A77E0 -> func_L11_0032DCC8,
 *   - func_L11_00440C80 -> func_L11_004424D0 (v1.01-address callees respelled to the v2.00 level split names).
 * No codegen or addressing device is used; the statements are upstream's.
 */
void func_L11_003F4248(u8 *arg0) {
    u8 *s0 = arg0;
    s32 v1 = *(s32 *)(s0 + 0x68);
    if (*(u16 *)(v1 + 0x278) >= 2)
        return;
    if (*(u16 *)(v1 + 0x27A) >= 2)
        return;
    if (func_L11_0032DCC8(D_L11_001B053C) != 0
        && func_L11_004424D0(*(s32 *)(s0 + 0xB8)) < D_L11_001B051C) {
        func_L11_0032DBF8(s0, 7, -1);
    } else {
        func_L11_0032DBF8(s0, 6, -1);
    }
}
