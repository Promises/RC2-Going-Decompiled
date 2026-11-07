/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x0042BC50, in place
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
extern void func_L11_003369F8(u8 *a0, void *a1, f32 f12, f32 f13, s32 a2);
extern void func_L11_003395B0(u8 *a0, void *a1, f32 f12, s32 a2, s32 a3);

/**
 * func_L11_0042BC50 - same preparation as func_L11_00433590, with the class
 * setup on record +0x260 and a two-stage animation seeded on +0x2A0.
 *
 * Runs the class setup on the state's +0x50 node and the +0x260 and +0x26
 * records, increments the saturation counter at +0x96 (saturating at 0xFFFF),
 * then seeds the two-stage animation on +0x2A0 (0.03/0.3) and its stage-two
 * call (length 3.5).
 *
 * @arg0: the moby object; reads +0x68 (state), writes +0x96.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_00425F10.c, function func_L11_0042A420 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - func_L11_0042A420 -> func_L11_0042BC50 (this function's v2.00 split name);
 *   - func_00282978 -> func_L11_00300E80, func_002AB8C0 -> func_L11_003327B8,
 *   - func_002AEC48 -> func_L11_003369F8, func_002B0990 -> func_L11_003395B0 (v1.01-address callees respelled to the v2.00 level split names).
 * No codegen or addressing device is used; the statements are upstream's.
 */
void func_L11_0042BC50(u8 *arg0) {
    u8 *s0 = *(u8 **)(arg0 + 0x68);

    func_L11_003327B8(arg0, s0 + 0x50);
    func_L11_00300E80(s0 + 0x260);
    func_L11_00300E80(s0 + 0x26);
    if (*(u16 *)(arg0 + 0x96) <= 0xFFFE)
        (*(u16 *)(arg0 + 0x96))++;
    func_L11_003369F8(arg0, s0 + 0x2A0, 0.03f, 0.3f, 4);
    func_L11_003395B0(arg0, s0 + 0x2A0, 3.5f, 4, 1);
}
