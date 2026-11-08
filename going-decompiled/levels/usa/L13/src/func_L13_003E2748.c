/* Level 13 (Boldan), USA v2.00: a level-only function built from C (task
 * #2672, RULING #13291 row L13-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003E2748, in place of its split asm; the row is
 * going-decompiled/levels/usa/L13/config/L13_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef float f32;
typedef unsigned char u8;
extern f32 func_L13_002FA528(f32, f32);

/**
 * func_L13_003E2748 - advance the object's angle at +0xF4 by 0.017453296 rad (one degree).
 *
 * @a0: the object.
 * Return: nothing.
 *
 * The float at +0xF4 is replaced by func_L13_002FA528(angle, 0.017453296f).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/us2/levels/13_boldan/L13_003E2710.c, function func_US2_L13_003E2748 (v2.00 address), author commit c4928ea3 (Flavius).
 * Its one-statement body matches llesieur99's func_L13_003E12C0 /
 * func_L13_003E12F8 (91e04732) but for names; a one-statement body carries
 * no rename-copy credit (RULING #13541 rev 4, validator #2678).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_US2_L13_003E2748 -> func_L13_003E2748 (our split name at the v2.00 place);
 *   - func_00283B98 -> func_L13_002FA528 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
void func_L13_003E2748(u8 *a0) {
    *(f32 *)(a0 + 0xF4) = func_L13_002FA528(*(f32 *)(a0 + 0xF4), 0.017453296f);
}
