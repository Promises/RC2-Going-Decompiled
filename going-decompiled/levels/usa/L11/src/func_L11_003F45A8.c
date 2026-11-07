/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x003F45A8, in place
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

extern void func_L11_0032DBF8(u8 *arg0, s32 arg1, s32 arg2);
extern void func_L11_0032D918(u8 *arg0, s32 arg1, s32 arg2, s32 arg3);
extern void func_L11_00325748(u8 *arg0);
extern void func_L11_003F46D0(u8 *arg0);

/**
 * func_L11_003F45A8 - deactivate the object: clear its record list, run the
 * class setup, retire it with code 1 and drop the measured flags.
 *
 * Resets the record list head through func_L11_003F46D0, applies the class
 * setup (0, 0, 3), resets and retires (+1, 0), clears +0x31 and +0x98, and
 * rewrites +0x34 as (flags | 1) & ~0x1000.
 *
 * @arg0: the moby object; writes +0x31, +0x34, +0x98.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_003D6270.c, function func_L11_003F3098 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - func_L11_003F3098 -> func_L11_003F45A8 (this function's v2.00 split name);
 *   - func_002A05A8 -> func_L11_00325748, func_002A7470 -> func_L11_0032D918,
 *   - func_002A7750 -> func_L11_0032DBF8, func_L11_003F31C0 -> func_L11_003F46D0 (v1.01-address callees respelled to the v2.00 level split names).
 * No codegen or addressing device is used; the statements are upstream's.
 */
void func_L11_003F45A8(u8 *arg0) {
    func_L11_003F46D0(arg0);
    func_L11_0032D918(arg0, 0, 0, 3);
    func_L11_00325748(arg0);
    func_L11_0032DBF8(arg0, 1, 0);
    *(u8 *)(arg0 + 0x31) = 0;
    *(s32 *)(arg0 + 0x98) = 0;
    *(u16 *)(arg0 + 0x34) = (*(u16 *)(arg0 + 0x34) | 1) & 0xEFFF;
}
