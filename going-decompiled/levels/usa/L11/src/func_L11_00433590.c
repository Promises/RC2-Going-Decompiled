/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x00433590, in place
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

extern void func_L11_003327B8(u8 *a0, void *a1);
extern void func_L11_00300E80(void *a0);

/**
 * func_L11_00433590 - run the class setup on the state's sub-record, unmark
 * its two state halfwords and step the saturation counter.
 *
 * The class setup takes the state's +0x50 node; the halfwords at +0x270 and
 * +0x272 are cleared; +0x96 increments, saturating at 0xFFFF.
 *
 * @arg0: the moby object; reads +0x68 (state), writes +0x96.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_00425F10.c, function func_L11_00431D50 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - func_L11_00431D50 -> func_L11_00433590 (this function's v2.00 split name);
 *   - func_00282978 -> func_L11_00300E80, func_002AB8C0 -> func_L11_003327B8 (v1.01-address callees respelled to the v2.00 level split names).
 * No codegen or addressing device is used; the statements are upstream's.
 */
void func_L11_00433590(u8 *arg0) {
    u8 *s0 = *(u8 **)(arg0 + 0x68);

    func_L11_003327B8(arg0, s0 + 0x50);
    func_L11_00300E80(s0 + 0x270);
    func_L11_00300E80(s0 + 0x272);
    if (*(u16 *)(arg0 + 0x96) <= 0xFFFE)
        (*(u16 *)(arg0 + 0x96))++;
}
