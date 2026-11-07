/* Level 01 (Oozla), USA v2.00: a level-only function built from C (row L01-G1,
 * task #2626, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003BC3B8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L01/config/L01_c.tsv, the arm 2.9-ee-991111b/r4
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G8, plain as.
 */

extern float D_L01_001AFDD8 __attribute__((sda));

/**
 * func_L01_003BC3B8 - return the .lit float constant D_L01_001AFDD8.
 *
 * The value is a level height: its sibling consumer func_L01_003B53C8
 * (upstream func_L01_003B4480's group) reads it as a base water height
 * (func_L01_003BAFB0()+0.3f upstream). The whole body is the one $gp load
 * the -G8 file exists for.
 *
 * Return: the float at D_L01_001AFDD8.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/01_oozla/L01_003835D8-g8.c, function func_L01_003BAFB0
 * (v1.01 address), author commit ee3fee45 (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2626 relverify by name):
 *   - func_L01_003BAFB0 -> func_L01_003BC3B8 (our split name);
 *   - D_L01_001AFE18 -> D_L01_001AFDD8 (our committed .lit label at the
 *     v2.00 target);
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped: this function references no other symbol.
 */
float func_L01_003BC3B8(void) {
    return D_L01_001AFDD8;
}
