/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (row L09-B,
 * task #2562, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x00423260, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef int Quadword __attribute__((mode(TI)));
typedef union EffectVector { Quadword bits; float value[4]; } EffectVector;

typedef unsigned char u8;
typedef int s32;

extern int func_L09_00317C08(int a, int b);
extern void func_L09_00317AF8(char *a0, int a1, int a2);
extern int func_L09_002EAD68(char *p);
extern void func_L09_002E90D8(char *a0, float *a1, float *a2, int a3, int a4, float f12, float f13, float f14, float f15, float f16);

/**
 * func_L09_00423260 - the object's two-state randomiser: pick a fresh window,
 * then on each opening throw one drifting effect and re-pick.
 *
 * The state is at *(char **)(a0+0x68); the switch is the object's state byte
 * at +0x20. State 0 stores func_L09_00317C08(0, 0xF0) into the state's first
 * word and moves the object on via func_L09_00317AF8(@a0, 1, -1). State 1,
 * once func_L09_002EAD68(state) says the window has elapsed: if the object's
 * byte at +0x31 is set, one effect vector pair is thrown through
 * func_L09_002E90D8 -- the drift vector a holds (-, -0.033333335f,
 * 0.016666668f), the base vector b holds 2.0f at [2], with the random spread
 * func_L09_00317C08(3, 7) and the float tail (0, 0.0f, 0.033333335f, 0.7f,
 * 15.0f, 1.0f) -- and the state's first word is re-picked as
 * func_L09_00317C08(0x78, 0xF0).
 *
 * @a0: the object.
 * Return: none.
 *
 * Non-obvious: the effect only fires while +0x31 is nonzero, but the re-pick
 * at the end of state 1 happens either way. No codegen or addressing device
 * is used.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_003FEA48.c, function func_L09_00421D60 (v1.01
 * address), author commit 27b929ce (Lynder063). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2562 relverify by name):
 *   - func_L09_00421D60 -> func_L09_00423260 (our split name);
 *   - func_316A68 -> func_L09_00317C08, func_316958 -> func_L09_00317AF8,
 *     func_2EA0E0 -> func_L09_002EAD68, func_2E8450 -> func_L09_002E90D8
 *     (L09's own copies at the v2.00 places);
 *   - `#include "common.h"` dropped: the typedefs this function uses are
 *     written here and only its declarations kept (Quadword and EffectVector
 *     are upstream's own file-scope typedefs, kept verbatim).
 */
void func_L09_00423260(char *a0) {
    char *s2 = *(char **)(a0 + 0x68);
    switch (*(u8 *)(a0 + 0x20)) {
    case 0:
        *(int *)s2 = func_L09_00317C08(0, 0xF0);
        func_L09_00317AF8(a0, 1, -1);
        break;
    case 1:
        if (func_L09_002EAD68(s2)) {
            if (*(u8 *)(a0 + 0x31)) {
                EffectVector a;
                EffectVector b;
                a.bits = 0;
                b.bits = 0;
                a.value[1] = -0.033333335f;
                a.value[2] = 0.016666668f;
                b.value[2] = 2.0f;
                func_L09_002E90D8(a0, b.value, a.value, func_L09_00317C08(3, 7), 0, 0.0f, 0.033333335f, 0.7f, 15.0f, 1.0f);
            }
            *(int *)s2 = func_L09_00317C08(0x78, 0xF0);
        }
        break;
    }
}
