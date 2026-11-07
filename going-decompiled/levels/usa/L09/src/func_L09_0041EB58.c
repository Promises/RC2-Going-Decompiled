/* Level 09 (Dobbo), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2580, RULING #13291 row L09-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x0041EB58, in place of its
 * split asm; the row is going-decompiled/levels/usa/L09/config/L09_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column; the plain-`as` ablation is in the row's FACT, task #2580).
 */

/* Upstream's 128-bit integer type: a zero store through it is one `sq $0`,
 * which cc1 emits itself (no asm text). */
typedef int DobboQuad __attribute__((mode(TI)));
/* Every callee is L09's copy of boot or shared level code, at its L09 place. */
extern void func_L09_00322558(void *, void *, float, float, float);
extern void func_L09_00429CC0(void *, void *, int, void *, int, int);
extern void func_L09_002EB180(void *, void *, float);
extern void func_L09_0033C290(void *, void *, int, int, int, int, int, int, float, float);
extern void func_L09_00317AF8(void *, int, int);
extern int func_L09_0031A158(void *, int, int);
extern void func_L09_00327E00(void *, void *, void *, void *, int, int, int, int, int, int, int);
extern void func_L09_003F55B0(void *, int, int, int, int, int, int, int, int, int, int, int, float, float);
extern void func_L09_0031C9A8(void *, void *, void *, int, int, int, int, float, float, float, float, int, int, int, int, int, float, float, float);
extern void func_L09_002EB5B8(void *, void *, void *);
extern void func_L09_00323A58(void *, void *, int, int, void *, float, float, float, float, int, int);
extern void func_L09_002E90D8(void *, void *, void *, int, int, float, float, float, float, float);
extern void func_L09_0030D900(void *);
/* The level's tuning words (our labels at 0x001B1AF0..0x001B1B20); upstream's
 * `sda` attribute text is kept, and this cc1 ignores it (a warning), so it
 * emits nothing: each is $gp-relative at -G8 by its size. */
extern int D_L09_001B1B20 __attribute__((sda));
extern float D_L09_001B1AF8[2] __attribute__((sda));
extern float D_L09_001B1AF0[2] __attribute__((sda));
extern float D_L09_001B1B00[2] __attribute__((sda));

/**
 * func_L09_0041EB58 - a Dobbo object's three-state update (state byte +0x20).
 *
 * State 0: takes two points on the object (func_L09_00322558 at the offsets
 * (0, 4, 19) and (0, 10, 13), w set to 0.2 and 1.3) and passes them to
 * func_L09_00429CC0 (colour word 0x28608020); scales the offset (0, 5, 18) by
 * the object's scale +0x2C and passes it to func_L09_0033C290 (same colour,
 * 1470000 * scale); then moves to state 1 (func_L09_00317AF8).
 * State 1: moves to state 2 once func_L09_0031A158(object, 0x4010000, 0) is
 * true and the word at the object's data (+0x68) is 0; writes 0xFF to +0xA8.
 * State 2: spawns the effect burst (func_L09_00327E00 with the
 * D_L09_001B1B00/1AF8/1AF0 tables and D_L09_001B1B20, func_L09_003F55B0,
 * func_L09_0031C9A8, func_L09_00323A58, func_L09_002E90D8, with a velocity
 * (0, 1/15, 0) rotated by the object's matrix +0xC0); then, if D_L09_001B1B20
 * is non-negative, sets +0x7C to (1 << D_L09_001B1B20) | 0x8000 and moves to
 * state 3, else calls func_L09_0030D900(object).
 *
 * @obj: the object.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_0041D650-g8.c, function func_L09_0041D650
 * (v1.01 address), author commit 913f6380 (bigbeno37). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2580 relverify):
 *   - func_L09_0041D650 -> func_L09_0041EB58 (our split name);
 *   - D_L09_001B1B70/1B48/1B40/1B50 -> our labels D_L09_001B1B20/1AF8/1AF0/
 *     1B00 (`sda` text kept);
 *   - callees func_L00_00324F18 -> func_L09_00322558, func_L03_0042B6F8 ->
 *     func_L09_00429CC0, func_00282D30 -> func_L09_002EB180,
 *     func_L00_0033E220 -> func_L09_0033C290, func_002A7750 ->
 *     func_L09_00317AF8, func_002A9640 -> func_L09_0031A158,
 *     func_L00_0032A258 -> func_L09_00327E00, func_0032C700 ->
 *     func_L09_003F55B0, func_002ABBB8 -> func_L09_0031C9A8, func_00283098 ->
 *     func_L09_002EB5B8, func_L00_003263D0 -> func_L09_00323A58,
 *     func_L00_002EC248 -> func_L09_002E90D8, func_0029F338 ->
 *     func_L09_0030D900;
 *   - `#include "common.h"` dropped (nothing else used).
 */
void func_L09_0041EB58(char *obj) {
    float offset[4], first[4], second[4], velocity[4], extra[4];
    char *data = *(char **)(obj+0x68);
    *(DobboQuad *)offset = 0;
    offset[1] = 5.0f;
    offset[2] = 18.0f;
    switch (*(unsigned char *)(obj+0x20)) {
    case 0:
        func_L09_00322558(obj, first, 0.0f,4.0f,19.0f);
        func_L09_00322558(obj, second,0.0f,10.0f,13.0f);
        first[3] = 0.2f; second[3] = 1.3f;
        func_L09_00429CC0(first,second,0x28608020,obj,7,1);
        func_L09_002EB180(offset,offset,*(float *)(obj+0x2C));
        func_L09_0033C290(offset,obj,0x28608020,-1,0,-1,0,0x60,*(float *)(obj+0x2C)*1470000.0f,0.0f);
        func_L09_00317AF8(obj,1,-1);
        break;
    case 1:
        if (func_L09_0031A158(obj,0x4010000,0) && !*(int *)data) func_L09_00317AF8(obj,2,-1);
        *(unsigned char *)(obj+0xA8) = 0xFF;
        break;
    case 2:
        func_L09_00327E00(obj,D_L09_001B1B00,D_L09_001B1AF8,D_L09_001B1AF0,0xFFFF,0,D_L09_001B1B20,0,0x87F,0xFF,0);
        *(DobboQuad *)second = 0;
        func_L09_00322558(obj,first,offset[0],offset[1],offset[2]);
        func_L09_003F55B0(first,0,12,12,12,12,12,0,1,0,0,0,*(float *)(obj+0x2C),20.0f);
        func_L09_0031C9A8(obj,second,first,20,1,1,-1,0.0f,0.0f,0.0f,0.0f,0,0,-1,0,0x810001,9.0f,0.75f,0.0f);
        *(DobboQuad *)velocity = 0;
        velocity[1] = 0.0666666701f;
        func_L09_002EB5B8(velocity,velocity,obj+0xC0);
        func_L09_00323A58(obj,first,10,1,velocity,0.25f,5.0f,0.75f,1.5f,0x7FAAAA40,0);
        *(DobboQuad *)extra = 0;
        extra[1] = 0.200000018f; extra[2] = -0.0833333358f;
        func_L09_002E90D8(obj,offset,extra,60,3,*(float *)(obj+0x2C)*2.0f,0.200000018f,1.0f,5.0f,1.0f);
        if (D_L09_001B1B20 >= 0) {
            *(short *)(obj+0x7C) = (1 << D_L09_001B1B20) | 0x8000;
            func_L09_00317AF8(obj,3,-1);
        } else func_L09_0030D900(obj);
        break;
    }
}
