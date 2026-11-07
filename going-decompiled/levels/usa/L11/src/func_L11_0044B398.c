/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2556,
 * RULING #13291 row L11-D). Compiled ALONE as its own TU and linked at its one
 * place, 0x0044B398, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the SN 2.95.3 v1.36 cc1 with -fopt-stack
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;
typedef int TI128 __attribute__((mode(TI)));
extern void func_L11_004431C8(s32 a0, int a1);
extern void func_L11_0044AD58(char *self, TI128 *pos);
extern void func_L11_0044B2A0(char *a0);
extern void func_L11_00300F68(char *a0, int a1, int a2);
extern void func_L11_003247F8(char *a0, int a1, char *a2);
extern void func_L11_0032DFB0(void *a0, float f12, float f13);
extern void func_L11_00339740(char *a0, int a1, void *a2, float f12, float f13, float f14,
                        float f15, int a3);
extern void func_L11_00443288(char *a0, int a1, int a2);
extern void func_L11_004148D0(char *a0, int a1, char *a2);

/**
 * func_L11_0044B398 - the death path of a paired-moby (class 0xD98).
 *
 * The USA v2.00 body of upstream's func_L11_00449970 (the same C, plus
 * clearing bit 0x1000 of the +0x34 halfword): zeroes the state's +0x20 word,
 * hands the class counter back when the phase (+0x366) is 1
 * (func_L11_004431C8 with 0xD98), clears the phase and the partner pointer,
 * flushes a live sub-moby's position through func_L11_0044AD58 and, when the
 * +0x2B1 byte is clear, plays the 0x20 impulse packet (1/60 s rise, 0.05
 * falloff, 3/6/3/6) over the reset rig (func_L11_00300F68, func_L11_003247F8,
 * func_L11_0032DFB0, func_L11_00339740). The object is then stopped (+0x98 =
 * 0, bit 0x1000 of +0x34 dropped) and the arguments select the tail: @a1 == 0
 * runs the func_L11_00443288 teardown, and @a2 == 0 re-arms
 * func_L11_0044B2A0 as the callback through func_L11_004148D0 instead of
 * calling it directly.
 *
 * @a0: the moby.
 * @a1: 0 = run the func_L11_00443288 teardown.
 * @a2: 0 = re-arm func_L11_0044B2A0 as the callback; else call it now.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own
 * and no Lombyte mention), file
 * src/us2/levels/11_joba/L11_0044B398.c, function func_US2_L11_0044B398 (v2.00
 * address), author commit ac7e1a2e (bigbeno37), which
 * carried the text from func_L11_00449970 in
 * src/levels/11_joba/L11_004465F8.c (upstream's v1.01 body: written in
 * 7119fabe by Gianni, seven callee spellings changed in 1d296136 by
 * Louis-Philippe Le Sieur), renaming it and adding the one line
 * `*(u16 *)(a0 + 0x34) &= 0xEFFF;` (RULING #13427). Adopted with
 * the authors’ permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device):
 *   - func_US2_L11_0044B398 -> func_L11_0044B398 (our split name at the v2.00
 *     place);
 *   - func_00282A60 -> func_L11_00300F68;
 *   - func_0029F978 -> func_L11_003247F8;
 *   - func_002A7A00 -> func_L11_0032DFB0;
 *   - func_002B0A68 -> func_L11_00339740;
 *   - func_L01_003E9CF8 -> func_L11_004148D0;
 *   - func_L02_003FC248 -> func_L11_00443288;
 *   - func_L11_00441978 -> func_L11_004431C8;
 *   - func_L11_00449330 -> func_L11_0044AD58;
 *   - func_L11_00449878 -> func_L11_0044B2A0;
 *   - `#include "common.h"` and `#include "include_asm.h"` dropped: the
 *     u8/u16/u32/s16/s32/f32 typedefs common.h supplied are written here;
 *   - the opening brace moved to the signature line (whitespace only);
 *   - the 128-bit typedef `TI128` upstream defines at file scope is written
 *     here; upstream’s comment above the function is folded into this one;
 *   - the body’s `(char *)(long)` cast is upstream’s own spelling, kept
 *     verbatim: EE `long` is 64-bit on this arm (RULING #13197’s native -m32
 *     concern does not arise - level TUs build only the EE arms).
 *
 */
void func_L11_0044B398(char *a0, int a1, int a2) {
    char *s1 = *(char **)(a0 + 0x68);
    char *s0;
    TI128 tmp;
    float f0;

    *(s32 *)(s1 + 0x20) = 0;
    if (*(s16 *)(s1 + 0x366) == 1)
        func_L11_004431C8(*(s32 *)(a0 + 0xB8), 0xD98);
    *(s16 *)(s1 + 0x366) = 0;
    *(s32 *)(s1 + 0x350) = 0;
    if (*(s32 *)(s1 + 0x354) != 0) {
        tmp = *(TI128 *)((char *)(long)*(s32 *)(s1 + 0x354) + 0x10);
        func_L11_0044AD58(a0, &tmp);
    }
    if (*(u8 *)(s1 + 0x2B1) == 0) {
        s0 = s1 + 0x2B0;
        func_L11_00300F68(s0, 0, 0x40);
        func_L11_003247F8(a0, 3, s0);
        f0 = 1.0e-4f;
        *(u8 *)(s1 + 0x2B2) = 1;
        *(f32 *)(s1 + 0x2D8) = f0;
        *(f32 *)(s1 + 0x2D4) = f0;
        *(f32 *)(s1 + 0x2D0) = f0;
        func_L11_0032DFB0((char *)&tmp, 1.0f / 60.0f, 0.0500000045f);
        func_L11_00339740(a0, 0x20, (char *)&tmp, 3.0f, 6.0f, 3.0f, 6.0f, 0xFF);
    }
    *(s32 *)(a0 + 0x98) = 0;
    *(u16 *)(a0 + 0x34) &= 0xEFFF;
    if (a1 == 0)
        func_L11_00443288(a0, 0, -1);
    if (a2 == 0)
        func_L11_004148D0(a0, 0, (char *)func_L11_0044B2A0);
    else
        func_L11_0044B2A0(a0);
}
