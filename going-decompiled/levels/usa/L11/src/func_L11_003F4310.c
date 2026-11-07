/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x003F4310, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the SDK 2.9 cc1
 * (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

typedef unsigned char u8;
typedef signed int s32;

/* Two word gates. The section attribute is an ADDRESSING-MODEL DEVICE on these
 * extern declarations only (RULING #8620): cc1 prints a one-instruction macro
 * load instead of splitting %hi/%lo itself, and GNU as expands it at -G0 into
 * the lui/lw pair the ROM has. It moves no data and emits nothing. */
extern s32 D_L11_001A8FF0 __attribute__((section(".sdata")));
extern s32 D_L11_001A8FF4 __attribute__((section(".sdata")));

/**
 * func_L11_003F4310 - is the object's stage open while either gate is clear?
 *
 * @arg0: the object; only its stage byte at +0x20 is read.
 * Return: 1 when D_L11_001A8FF0 or D_L11_001A8FF4 is 0 and the stage is not
 * 9; otherwise 0. When both gates are set, it returns 0 without reading the
 * stage.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_003D6270.c, function
 * func_L11_003F2E00 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L11_003F2E00 -> func_L11_003F4310 (our split name at the
 *     v2.00 place);
 *   - `__attribute__((sda))` on both externs -> `section(".sdata")`
 *     (RULING #8620 respelling; the two names are unchanged, the v2.00 ROM's
 *     relocations target the same addresses);
 *   - `#include "common.h"` dropped; its u8/s32 typedefs written out; the
 *     upstream one-line comment folded into this one.
 */
s32 func_L11_003F4310(u8 *arg0) {
    s32 v1 = 0;

    if (D_L11_001A8FF0 == 0 || D_L11_001A8FF4 == 0)
        v1 = *(u8 *)(arg0 + 0x20) != 9;
    return v1;
}
