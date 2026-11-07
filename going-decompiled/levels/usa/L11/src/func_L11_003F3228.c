/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x003F3228, in place
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

extern s32 func_L11_00302400(f32 arg0);
extern void func_L11_003256A8(u8 *arg0, u8 *arg1, u8 *arg2, u8 *arg3);
extern void func_L11_003F45A8(u8 *arg0);

/**
 * func_L11_003F3228 - first-frame initialiser of the object's state block.
 *
 * Sets the two animation bytes from frame indices 15 and 16, hands the four
 * state pointers at +0x280..+0x288 to the frame helper, sets mode 3, speed
 * 1.5, marks flag 0x100, runs the deactivate path func_L11_003F45A8 and
 * clears the record slot at state +0x290.
 *
 * @arg0: the moby object; reads +0x68 (state), writes +0x28/+0x29/+0x34.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_003D6270.c, function func_L11_003F1D18 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - func_L11_003F1D18 -> func_L11_003F3228 (this function's v2.00 split name);
 *   - func_00283CF0 -> func_L11_00302400, func_002A04B8 -> func_L11_003256A8,
 *   - func_L11_003F3098 -> func_L11_003F45A8 (v1.01-address callees respelled to the v2.00 level split names).
 * No codegen or addressing device is used; the statements are upstream's.
 */
void func_L11_003F3228(u8 *arg0) {
    u8 *s1 = arg0;
    u8 *s0 = *(u8 **)(s1 + 0x68);
    *(u8 *)(s0 + 0x48) = func_L11_00302400(15.0f);
    *(u8 *)(s0 + 0x4A) = func_L11_00302400(16.0f);
    func_L11_003256A8(s1, s0 + 0x280, s0 + 0x284, s0 + 0x288);
    *(u8 *)(s0 + 0x28) = 3;
    *(f32 *)(s0 + 0x30) = 1.5f;
    *(u8 *)(s0 + 0x29) = 1;
    *(u16 *)(s1 + 0x34) |= 0x100;
    func_L11_003F45A8(s1);
    *(s32 *)(s0 + 0x290) = -1;
}
