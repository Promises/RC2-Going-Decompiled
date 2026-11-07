/* Level 10 (Hrugis Cloud), USA v2.00: a level-only function built from C
 * (task #2657, RULING #13291 row L10-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x004185E0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L10/config/L10_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

/* The level's copies of boot code, named by their v2.00 places. */
extern u32 *func_L10_0037CCF8(u8 *arg0);
extern void func_L10_0037DF20(u8 *arg0, s32 a1);
extern f32 func_L10_003365A8(void);

/**
 * func_L10_004185E0 - set an object up as active with a 30-unit timer and a
 * random start angle.
 *
 * Stores 0x1E at +0x148 of the record func_L10_0037CCF8 returns, calls
 * func_L10_0037DF20(@arg0, 1), sets the state's (+0x68) float at +0x180 to
 * 30.0, the state byte (+0x20) to 1, the alpha byte (+0x30) to 0xFF and the
 * halfword at +0x32 to 0x1FF. The state's +0x2BC takes func_L10_003365A8()'s
 * float and +0x2B8 the object's +0x18; +0x2C0..+0x2C8, +0x2CC and +0x2D0 are
 * cleared and +0x2E4 set to 0x21C. The scale (+0x2C) becomes 6.4 times the
 * class header's (+0x24) float at +0x24, and the halfword at +0x7C 0xFF.
 *
 * @arg0: the object.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/10_hrugis_cloud/L10_003170D0.c, function
 * func_L10_00417040 (v1.01 address), author commit 431ac503 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names (no device); the
 * respellings, each to the target of the v2.00 disc's own relocation at this
 * place (task #2657 relverify):
 *   - func_L10_00417040 -> func_L10_004185E0 (our split name at the v2.00 place);
 *   - func_0037BA88 -> func_L10_0037CCF8, func_0037CCB0 -> func_L10_0037DF20,
 *     func_00335320 -> func_L10_003365A8 (our split names at the v2.00
 *     targets);
 *   - `#include "common.h"` dropped: the typedefs used are written here and
 *     only this function's declarations kept.
 */
void func_L10_004185E0(u8 *arg0) {
    u8 *s1 = *(u8 **)(arg0 + 0x68);

    *(s32 *)((u8 *)func_L10_0037CCF8(arg0) + 0x148) = 0x1E;
    func_L10_0037DF20(arg0, 1);
    *(f32 *)(s1 + 0x180) = 30.0f;
    arg0[0x20] = 1;
    arg0[0x30] = 0xFF;
    *(u16 *)(arg0 + 0x32) = 0x1FF;
    *(f32 *)(s1 + 0x2BC) = func_L10_003365A8();
    *(f32 *)(s1 + 0x2B8) = *(f32 *)(arg0 + 0x18);
    *(s32 *)(s1 + 0x2C0) = 0;
    *(s32 *)(s1 + 0x2C4) = 0;
    *(s32 *)(s1 + 0x2C8) = 0;
    *(f32 *)(arg0 + 0x2C) = *(f32 *)(*(u8 **)(arg0 + 0x24) + 0x24) * 6.4f;
    *(s32 *)(s1 + 0x2E4) = 0x21C;
    *(s32 *)(s1 + 0x2D0) = 0;
    *(s32 *)(s1 + 0x2CC) = 0;
    *(u16 *)(arg0 + 0x7C) = 0xFF;
}
