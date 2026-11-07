/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (row L09-B,
 * task #2562, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x00404808, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm 2.9-ee-991111
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, plain as.
 */

typedef unsigned char u8;
typedef float f32;

/**
 * func_L09_00404808 - copy the state's float at +0x288 to the object's +0x18.
 *
 * A two-instruction bridge: unless the object's state byte at +0x20 is 9, the
 * float at state+0x288 (the state is at *(char **)(a0+0x68)) becomes the
 * object's float at +0x18.
 *
 * @a0: the object.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_003FEA48.c, function func_L09_00403378 (v1.01
 * address), author commit 264cb2be (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (BYTE-IDENTICAL): only the function name changes,
 * func_L09_00403378 -> func_L09_00404808 (our split name at the v2.00 place);
 * the body references no other symbol (0 relocations), and
 * `#include "common.h"` is dropped: the typedefs this function uses are
 * written here and only its declarations kept.
 */
void func_L09_00404808(char *a0) {
    char *p = *(char **)(a0 + 0x68);
    if (*(u8 *)(a0 + 0x20) == 9) {
        return;
    }
    *(f32 *)(a0 + 0x18) = *(f32 *)(p + 0x288);
}
