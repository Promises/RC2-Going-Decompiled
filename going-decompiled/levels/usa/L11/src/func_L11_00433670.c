/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x00433670, in place
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

extern void func_L11_00324B40(u32 a0, s32 a1, f32 *a2);
extern f32 func_L11_00301808(f32 a0);
extern f32 func_L11_00301820(f32 a0);
extern void func_L11_003011E0(f32 *a0, f32 *a1, f32 *a2);
extern void func_L11_00433728(u8 *a0, f32 *a1);
extern void func_L11_00433938(u8 *a0, f32 *a1);

/**
 * func_L11_00433670 - build the object's follow vector from the owner
 * renderer's position, offset its height and hand it to the two transform
 * helpers.
 *
 * Reads the owner's (state +0x27C) renderer state, takes its +0x278 class
 * record's position, scales the object's own heading angles by 0.1, offsets
 * the vector's height by -0.05, then submits it to func_L11_00433728 and
 * func_L11_00433938.
 *
 * @arg0: the moby object; reads +0x68 (state), +0xF8 (heading).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_00425F10.c, function func_L11_00431E30 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - func_L11_00431E30 -> func_L11_00433670 (this function's v2.00 split name);
 *   - func_00282CC0 -> func_L11_003011E0, func_00283180 -> func_L11_00301808,
 *   - func_00283198 -> func_L11_00301820, func_0029FCC0 -> func_L11_00324B40,
 *   - func_L11_00431EE8 -> func_L11_00433728, func_L11_004320F8 -> func_L11_00433938 (v1.01-address callees respelled to the v2.00 level split names).
 * No codegen or addressing device is used; the statements are upstream's.
 */
void func_L11_00433670(u8 *arg0) {
    u8 *s0 = arg0;
    u8 *s1 = *(u8 **)(s0 + 0x68);
    u8 *p = *(u8 **)(s1 + 0x27C);
    u8 *q;
    f32 v[3];
    f32 w[3];

    if (p == 0)
        return;
    q = *(u8 **)(p + 0x68);
    func_L11_00324B40(*(u32 *)(q + 0x278), 0, v);
    w[0] = func_L11_00301808(*(f32 *)(s0 + 0xF8)) * 0.1f;
    w[1] = func_L11_00301820(*(f32 *)(s0 + 0xF8)) * 0.1f;
    w[2] = 0.0f;
    func_L11_003011E0(v, v, w);
    v[2] = v[2] - 0.05f;
    func_L11_00433728(s0, v);
    func_L11_00433938(s0, v);
}
