/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (row L09-B,
 * task #2562, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x0040A6B8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef int Quad_Dobbo __attribute__((mode(TI)));

typedef float f32;

extern float func_L09_00317C60(float, float);
extern void func_L09_0030EA40(char *, int, void *);
extern float func_L09_002EC248(int);
extern char *func_L09_0032F7C0(void *, void *, int, int, int, int, int, int);

/**
 * func_L09_0040A6B8 - throw the alternating two-colour spark burst plus the
 * white centre spark around the object.
 *
 * The velocity vector starts as a zeroed Quad_Dobbo with element 2 set to
 * -(func_L09_00317C60(3.5f, 4.5f) * 0.0166666675f). For i = 0..1 the
 * position is refreshed from the object by func_L09_0030EA40(@obj, 2, pos)
 * (mode 2), pos[2] advances by func_L09_002EC248(i) * 0.015f, and two sparks
 * are thrown through func_L09_0032F7C0: an alternating pair (i odd:
 * 0x77FF0000/0xFF0060 kind 17 at 150, i even: 0x77FF0060/0xFF0000 kind 19 at
 * 220, both y-offset -100) and a white 0x77FFFFFF spark of kind 10 at 80 with
 * y-offset -80, all mode 1.
 *
 * @obj: the object.
 * Return: none.
 *
 * Non-obvious: the velocity is computed once, outside the loop, and every
 * spark's return value is ignored (no early-out on failure here). No codegen
 * or addressing device is used.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_003FEA48.c, function func_L09_004091C8 (v1.01
 * address), author commit 913f6380 (bigbeno37). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2562 relverify by name):
 *   - func_L09_004091C8 -> func_L09_0040A6B8 (our split name);
 *   - func_002A7878 -> func_L09_00317C60, func_0029FCC0 -> func_L09_0030EA40,
 *     func_00283CE0 -> func_L09_002EC248, func_002BAE50 -> func_L09_0032F7C0
 *     (L09's own copies at the v2.00 places);
 *   - `#include "common.h"` dropped: the typedefs this function uses are
 *     written here and only its declarations kept (Quad_Dobbo is upstream's
 *     own file-scope typedef, kept verbatim).
 */
void func_L09_0040A6B8(char *obj)
{
    float pos[4];
    float vel[4];
    float initial[4];
    int i;
    *(Quad_Dobbo *)initial = 0;
    initial[2] = -(func_L09_00317C60(3.5f, 4.5f) * 0.0166666675f);
    *(Quad_Dobbo *)vel = *(Quad_Dobbo *)initial;
    for (i = 0; i < 2; i++) {
        func_L09_0030EA40(obj, 2, pos);
        pos[2] += func_L09_002EC248(i) * 0.015f;
        if (i & 1)
            func_L09_0032F7C0(pos, vel, 0x77FF0000, 0xFF0060, 17, 150, -100, 1);
        else
            func_L09_0032F7C0(pos, vel, 0x77FF0060, 0xFF0000, 19, 220, -100, 1);
        func_L09_0032F7C0(pos, vel, 0x77FFFFFF, 0xFF0000, 10, 80, -80, 1);
    }
}
