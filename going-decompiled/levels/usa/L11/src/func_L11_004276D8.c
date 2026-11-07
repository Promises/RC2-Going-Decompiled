/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x004276D8, in place
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

extern char *func_L11_00330218(char *p, int a1, int a2);

/**
 * func_L11_004276D8 - tick the object's clock down by the rate the linked
 * node states.
 *
 * Looks the state's +0x280 resource id up with class mask 0x10000; when found,
 * subtracts the node's +0x2C rate from the state's +0x20 timer and answers 1,
 * else 0.
 *
 * @object: the moby object; reads +0x68 (state).
 * Return: 1 when the node was found (timer ticked), 0 otherwise.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_00420C88.c, function func_L11_00425EC0 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "include_asm.h"` dropped (nothing in the function used it);
 *   - func_L11_00425EC0 -> func_L11_004276D8 (this function's v2.00 split name);
 *   - func_0032F000 -> func_L11_00330218 (v1.01-address callee respelled to the v2.00 level split name).
 * No codegen or addressing device is used; the statements are upstream's.
 */
int func_L11_004276D8(char *object)
{
    char *p = *(char **)(object + 0x68);
    char *r = func_L11_00330218(*(char **)(p + 0x280), 0x10000, 0);

    if (r != 0) {
        *(float *)(p + 0x20) = *(float *)(p + 0x20) - *(float *)(r + 0x2C);
        return 1;
    }
    return 0;
}
