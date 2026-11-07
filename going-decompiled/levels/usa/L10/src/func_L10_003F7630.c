/* Level 10 (Hrugis Cloud), USA v2.00: a level-only function built from C
 * (task #2657, RULING #13291 row L10-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003F7630, in place of its split asm; the row is
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

/**
 * func_L10_003F7630 - set an object up as active with a 2-unit timer.
 *
 * Stores 2 at +0x148 of the record func_L10_0037CCF8 returns and clears its
 * +0x14C, calls func_L10_0037DF20(@arg0, 1), sets the state's (+0x68) float
 * at +0x180 to 2.0, the state byte (+0x20) to 1, the alpha byte (+0x30) to
 * 0xFF, the halfword at +0x32 to 0x1FF, sets flag 0x100 and clears flag
 * 0x1000 of the halfword at +0x34, and sets the scale (+0x2C) to 4.1 times
 * the class header's (+0x24) float at +0x24.
 *
 * @arg0: the object.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/10_hrugis_cloud/L10_003170D0.c, function
 * func_L10_003F6110 (v1.01 address), author commit 431ac503 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names (no device); the
 * respellings, each to the target of the v2.00 disc's own relocation at this
 * place (task #2657 relverify):
 *   - func_L10_003F6110 -> func_L10_003F7630 (our split name at the v2.00 place);
 *   - func_0037BA88 -> func_L10_0037CCF8, func_0037CCB0 -> func_L10_0037DF20
 *     (our split names at the v2.00 targets);
 *   - `#include "common.h"` dropped: the typedefs used are written here and
 *     only this function's declarations kept.
 */
void func_L10_003F7630(u8 *arg0) {
    u8 *s1 = *(u8 **)(arg0 + 0x68);
    u8 *s2 = (u8 *)func_L10_0037CCF8(arg0);

    *(s32 *)(s2 + 0x148) = 2;
    func_L10_0037DF20(arg0, 1);
    *(f32 *)(s1 + 0x180) = 2.0f;
    arg0[0x20] = 1;
    arg0[0x30] = 0xFF;
    *(u16 *)(arg0 + 0x32) = 0x1FF;
    *(u16 *)(arg0 + 0x34) = *(u16 *)(arg0 + 0x34) | 0x100;
    *(f32 *)(arg0 + 0x2C) = *(f32 *)(*(u8 **)(arg0 + 0x24) + 0x24) * 4.1f;
    *(s32 *)(s2 + 0x14C) = 0;
    *(u16 *)(arg0 + 0x34) = *(u16 *)(arg0 + 0x34) & 0xEFFF;
}
