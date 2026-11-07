/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x00427268, in place
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

extern void func_L11_003327B8(char *object, char *p);
extern void func_L11_00300E80(char *p);
extern void func_L11_003369F8(char *object, char *p, int kind, float f12, float f13);
extern void func_L11_003395B0(char *object, char *p, int kind, int a3, float f12);

/**
 * func_L11_00427268 - re-arm the object's two guard nodes and restart its
 * measurement window.
 *
 * Runs the class setup on the state's +0x50 node and both +0x260 and +0x26
 * records, increments the saturation counter at +0x96 (saturating at 0xFFFF),
 * then seeds the two-stage animation on +0x2A0 with the 0.03/0.3 pair and the
 * 3.5-length stage-two call.
 *
 * @arg0: the moby object; reads +0x68 (state), writes +0x96.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_00420C88.c, function func_L11_00425A50 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "include_asm.h"` dropped (nothing in the function used it);
 *   - func_L11_00425A50 -> func_L11_00427268 (this function's v2.00 split name);
 *   - func_00300180 -> func_L11_00300E80, func_003315A0 -> func_L11_003327B8,
 *   - func_003357E0 -> func_L11_003369F8, func_00338398 -> func_L11_003395B0 (v1.01-address callees respelled to the v2.00 level split names).
 * No codegen or addressing device is used; the statements are upstream's.
 */
void func_L11_00427268(char *object)
{
    char *p = *(char **)(object + 0x68);

    func_L11_003327B8(object, p + 0x50);
    func_L11_00300E80(p + 0x260);
    func_L11_00300E80(p + 0x26);
    if (*(u16 *)(object + 0x96) <= 0xFFFE)
        *(u16 *)(object + 0x96) = *(u16 *)(object + 0x96) + 1;
    func_L11_003369F8(object, p + 0x2A0, 4, 0.03f, 0.3f);
    func_L11_003395B0(object, p + 0x2A0, 4, 1, 3.5f);
}
