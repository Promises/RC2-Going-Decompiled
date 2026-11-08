/* Level 13 (Boldan), USA v2.00: a level-only function built from C (task
 * #2672, RULING #13291 row L13-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003E27F0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L13/config/L13_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef float f32;
typedef unsigned char u8;
extern f32 func_L13_002FA528(f32, f32);

/**
 * func_L13_003E27F0 - advance the object's angle at +0xF4 by 0.017453296 rad (one degree).
 *
 * @a0: the object.
 * Return: nothing.
 *
 * The float at +0xF4 is replaced by func_L13_002FA528(angle, 0.017453296f).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/13_boldan/L13_00306F38.c, function func_L13_003E12F8 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L13_003E12F8 -> func_L13_003E27F0 (our split name at the v2.00 place);
 *   - func_2F9748 -> func_L13_002FA528 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
void func_L13_003E27F0(u8 *a0) {
    *(f32 *)(a0 + 0xF4) = func_L13_002FA528(*(f32 *)(a0 + 0xF4), 0.017453296f);
}
