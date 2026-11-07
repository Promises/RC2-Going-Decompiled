/* Level 01 (Oozla), USA v2.00: a level-only function built from C (row L01-G1,
 * task #2626, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003BF330, in place of its split asm; the row is
 * going-decompiled/levels/usa/L01/config/L01_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef struct L01_003BDF28_BUF {
    float x;
    float y;
    int z;
} L01_003BDF28_BUF;

/* The hero's position record: the boot's g_heroPos at 0x00189EA0 (upstream's
 * address-named extern spelled D_00189EA0). */
extern char g_heroPos[];

extern float func_L01_002E4750(float);

extern float func_L01_002E4768(float);

extern float func_L01_003117A0(float, float);

extern void func_L01_002E4178(void *, void *, void *);

extern void func_L01_002E41A8(void *, void *, void *);

extern void func_L01_002E4440(void *, void *, float);

extern void func_L01_003BEFF0(void *);

/**
 * func_L01_003BF330 - scatter the Oozla plant's spawn point on a disc of the
 * state's radius byte, lift it clear of the hero when the state asks, then
 * run the shared placement.
 *
 * The two floats at the state's +0x1E0/+0x1E4 become cos/sin
 * (func_L01_002E4750/func_L01_002E4768) of a random angle
 * (func_L01_003117A0(-pi, pi)) scaled by the radius byte at +0x201; +0x1E8 is
 * zeroed, the point is translated by the state's origin at +0x1D0
 * (func_L01_002E4178) and lifted by 1.7. When the state's +0x23C word is set
 * the point is additionally measured against g_heroPos (func_L01_002E41A8),
 * its z field zeroed, rescaled by 8.0f (func_L01_002E4440) and translated
 * back. func_L01_003BEFF0 (the shared placement, upstream's
 * func_L01_003BDBE8) finishes the move.
 *
 * @a0: the object record (its state is at *(char **)(a0 + 0x68)).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/01_oozla/L01_003BB9D0.c, function func_L01_003BDF28 (v1.01
 * address), author commit ee3fee45 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2626 relverify by name):
 *   - func_L01_003BDF28 -> func_L01_003BF330 (our split name);
 *   - D_00189EA0 -> g_heroPos (the boot's name at 0x00189EA0);
 *   - func_00283180 -> func_L01_002E4750, func_00283198 ->
 *     func_L01_002E4768, func_002A7878 -> func_L01_003117A0,
 *     func_00282CC0 -> func_L01_002E4178, func_00282CF0 ->
 *     func_L01_002E41A8, func_00282F20 -> func_L01_002E4440 and
 *     func_L01_003BDBE8 -> func_L01_003BEFF0 (L01's own copies at the v2.00
 *     places);
 *   - `#include "common.h"` and the file's other declarations dropped: this
 *     function references no other symbol.
 */
void func_L01_003BF330(char *a0)
{
    char *s2 = a0;
    char *s0 = *(char **)(s2 + 0x68);
    char *s1 = s0 + 0x1E0;
    L01_003BDF28_BUF buf;

    *(float *)(s0 + 0x1E0) = func_L01_002E4750(func_L01_003117A0(-3.1415927f, 3.1415927f)) * (float)*(unsigned char *)(s0 + 0x201);
    *(float *)(s0 + 0x1E4) = func_L01_002E4768(func_L01_003117A0(-3.1415927f, 3.1415927f)) * (float)*(unsigned char *)(s0 + 0x201);
    *(int *)(s0 + 0x1E8) = 0;
    func_L01_002E4178(s1, s1, s0 + 0x1D0);
    *(float *)(s0 + 0x1E8) = *(float *)(s0 + 0x1E8) + 1.7f;
    if (*(int *)(s0 + 0x23C) != 0) {
        func_L01_002E41A8(&buf, s1, g_heroPos);
        buf.z = 0;
        func_L01_002E4440(&buf, &buf, 8.0f);
        func_L01_002E4178(s1, s1, &buf);
    }
    func_L01_003BEFF0(s2);
}
