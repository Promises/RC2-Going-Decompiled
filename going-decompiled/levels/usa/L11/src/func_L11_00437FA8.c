/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x00437FA8, in place
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

extern void func_L11_003016D8(float *, float *, float *);
extern void func_L11_003395B0(char *, char *, float, int, int);
extern void func_L11_0032F698(char *, char *, int, float, float, float, int, int, int);
extern void func_L11_003011E0(void *, void *, void *);
extern void func_L11_0032F698(char *p, char *q, int a, float b, float c, float d, int e, int f, int g);

/**
 * func_L11_00437FA8 - place the record against the object's basis frame and
 * re-arm its measured stage.
 *
 * Sums the state's +0x100/+0x104 pair into a quadword, transforms it by the
 * object's +0xC0 basis, then composes it with the object's +0x10 position;
 * steps the saturation counter at +0x96 (saturating at 0xFFFF) and runs the
 * two stage calls on the state's +0x290 record (2.5 length; the second with
 * 0x4000000 and unit scales).
 *
 * @obj: the moby object; reads +0x10, +0x68 (state), +0x96, +0xC0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_00432628.c, function func_L11_00436768 (v1.01
 * address), author commit b4b0fe9b (bigbeno37), which moved the text verbatim
 * from Gianni's nonmatching draft in 77639a79 (validator #2618).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - func_L11_00436768 -> func_L11_00437FA8 (this function's v2.00 split name);
 *   - func_00282CC0 -> func_L11_003011E0, func_00283098 -> func_L11_003016D8,
 *   - func_32E480 -> func_L11_0032F698, func_338398 -> func_L11_003395B0 (v1.01-address callees respelled to the v2.00 level split names).
 * No codegen or addressing device is used; the statements are upstream's.
 */
void func_L11_00437FA8(char *obj)
{
    float tmp[3];
    TI128 tmp2[2];
    char *state;
    float f0, f1;

    tmp2[1] = 0;
    state = *(char **)(obj + 0x68);
    f1 = *(float *)(state + 0x100);
    f0 = *(float *)(state + 0x104);
    *(float *)((char *)&tmp2[1] + 8) = f0 + f1;
    tmp2[0] = tmp2[1];
    func_L11_003016D8((float *)&tmp2[0], (float *)&tmp2[0], (float *)((char *)obj + 0xC0));
    func_L11_003011E0((char *)tmp, (char *)obj + 0x10, (char *)&tmp2[0]);
    if (*(unsigned short *)(obj + 0x96) <= 0xFFFE)
        *(unsigned short *)(obj + 0x96) = *(unsigned short *)(obj + 0x96) + 1;
    func_L11_003395B0(obj, state + 0x290, 2.5f, 1, 1);
    func_L11_0032F698(obj, obj + 0x10, 0x4000000, 1.0f, 1.0f, 1.0f, 0, 1, 0);
}
