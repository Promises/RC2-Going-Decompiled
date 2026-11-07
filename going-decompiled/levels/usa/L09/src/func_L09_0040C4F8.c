/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (row L09-B,
 * task #2562, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x0040C4F8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef int s32;
typedef float f32;

/* The level's scale reference datum: upstream's D_L09_001B9C30, whose v2.00
 * target 0x001B9BB0 lies 0x4F0 inside the larger boot label g_uiTextureCache,
 * so it stays an address-named extern (RULING #13291 section 2; the level ELF
 * values it from its name). */
extern char D_L09_001B9BB0[];

extern f32 func_L09_002EB200(char *a0, char *a1);
extern f32 func_L09_002EB068(f32 x);
extern void func_L09_002EB958(char *a0, char *a1);

/**
 * func_L09_0040C4F8 - zero the state's first two floats and record which of
 * two scaled views is nearer.
 *
 * The state is at *(char **)(a0+0x68): its words at +8 and +0xC are cleared
 * and its floats at +0 and +4 set to 0.2. The scratch view at a0+0xC0 is
 * rebuilt from a0+0xF0 by func_L09_002EB958, then both it and the a0+0xD0
 * view are scaled against D_L09_001B9BB0 through func_L09_002EB200 and
 * func_L09_002EB068; the state's word at +0x14 becomes 1 when the a0+0xD0
 * view's scale is the smaller of the two, else 0.
 *
 * @a0: the object.
 * Return: none.
 *
 * Non-obvious: `b < a` stores 1 -- the comparison keeps the NEARER view, and
 * an exact tie stores 0. No codegen or addressing device is used.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_003FEA48.c, function func_L09_0040B008 (v1.01
 * address), author commit 431ac503 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2562 relverify by name):
 *   - func_L09_0040B008 -> func_L09_0040C4F8 (our split name);
 *   - D_L09_001B9C30 -> D_L09_001B9BB0 (address-named extern,
 *     g_uiTextureCache+0x4F0: RULING #13291 section 2);
 *   - func_2EA578 -> func_L09_002EB200, func_2EA3E0 -> func_L09_002EB068,
 *     func_2EACD0 -> func_L09_002EB958 (L09's own copies at the v2.00
 *     places);
 *   - `#include "common.h"` dropped: the typedefs this function uses are
 *     written here and only its declarations kept.
 */
void func_L09_0040C4F8(char *a0) {
    char *s1 = a0;
    char *s2 = s1 + 0xC0;
    char *s3 = *(char **)(s1 + 0x68);
    f32 a, b;
    *(s32 *)(s3 + 0x8) = 0;
    *(s32 *)(s3 + 0xC) = 0;
    *(f32 *)(s3 + 0x0) = 0.2f;
    *(f32 *)(s3 + 0x4) = 0.2f;
    func_L09_002EB958(s2, s1 + 0xF0);
    a = func_L09_002EB068(func_L09_002EB200(s2, D_L09_001B9BB0));
    b = func_L09_002EB068(func_L09_002EB200(s1 + 0xD0, D_L09_001B9BB0));
    if (b < a) {
        *(s32 *)(s3 + 0x14) = 1;
    } else {
        *(s32 *)(s3 + 0x14) = 0;
    }
}
