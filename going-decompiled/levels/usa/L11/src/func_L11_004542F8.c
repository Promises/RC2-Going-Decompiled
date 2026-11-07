/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2556,
 * RULING #13291 row L11-D). Compiled ALONE as its own TU and linked at its one
 * place, 0x004542F8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the SN 2.95.3 v1.36 cc1 with -fopt-stack
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;
extern void func_L11_00454430(char *a0, char *a1, float a2, int a3, int a4, int a5);

/**
 * func_L11_004542F8 - bind the three state tables while the counter is live.
 *
 * When the object's counter byte (+0x95) is nonzero, the three tables at
 * state+0x270/+0x280/+0x290 are each handed to the shared dispatch helper
 * func_L11_00454430: the first at scale 0.125 with id 0x36F6, then two at
 * 0.15 with id 0x1050B0 and different priority masks.
 *
 * @a0: the object; its state is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own
 * and no Lombyte mention), file
 * src/levels/11_joba/L11_004465F8.c, function func_L11_004528A0 (v1.01
 * address), author commit fa8d6672 (llesieur99). Adopted with
 * the authors’ permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device):
 *   - func_L11_004528A0 -> func_L11_004542F8 (our split name at the v2.00
 *     place);
 *   - func_L00_003D7250 -> func_L11_00454430;
 *   - `#include "common.h"` and `#include "include_asm.h"` dropped: the
 *     u8/u16/u32/s16/s32/f32 typedefs common.h supplied are written here;
 *   - the opening brace moved to the signature line (whitespace only).
 *
 */
void func_L11_004542F8(char *a0) {
    char *s1 = *(char **)(a0 + 0x68);
    float f;

    if (*(u8 *)(a0 + 0x95) != 0) {
        func_L11_00454430(a0, s1 + 0x270, 0.125f, 0x36F6, 0x20000000, 0x20000000);
        f = 0.15f;
        func_L11_00454430(a0, s1 + 0x280, f, 0x1050B0, 0x30000000, 0x20000000);
        func_L11_00454430(a0, s1 + 0x290, f, 0x1050B0, 0x10000000, 0x30000000);
    }
}
