/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x00444478, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern int func_L20_00335628(char *);
extern void func_L20_003BB6F0(char *);
extern void func_L20_0044AFC0(char *);

/**
 * func_L20_00444478 - call func_L20_003BB6F0 on an object when
 * func_L20_00335628 accepts it, or func_L20_0044AFC0 when its id is 0x12D3.
 *
 * @arg0: the object; +0xAA its signed 16-bit id.
 * Return: none.
 *
 * Non-obvious: The id test only runs when func_L20_00335628(arg0) returns 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_0043ECB0.c (blob c24c6906), function
 * func_L20_00442F30 (v1.01 address), author commit db93b56d (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2566):
 *   - func_L20_00442F30 -> func_L20_00444478 (our split name at the v2.00
 *     place);
 *   - func_002ABB78 -> func_L20_00335628 (the level's own copy at its L20
 *     place);
 *   - func_L02_003A6F78 -> func_L20_003BB6F0 (the level's own copy at its L20
 *     place);
 *   - func_L20_00449A78 -> func_L20_0044AFC0 (its v2.00 split name);
 *   - `#include "common.h"` dropped; the typedefs this body uses are written
 *     here (or are the file's own); upstream file-scope declarations it does
 *     not use are not carried, the rest keep upstream's order and text,
 *     duplicates included.
 */
void func_L20_00444478(char *arg0) {
    if (func_L20_00335628(arg0) != 0) {
        func_L20_003BB6F0(arg0);
    } else if (*(short *)(arg0 + 0xAA) == 0x12D3) {
        func_L20_0044AFC0(arg0);
    }
}
