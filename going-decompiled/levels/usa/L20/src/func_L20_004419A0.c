/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x004419A0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern void func_L20_00330820(int, int, int);

/**
 * func_L20_004419A0 - reset an object's state fields and move it to state 6
 * through func_L20_00330820.
 *
 * @arg0: the object; +0x68 its state, +0x34 its u16 flags, +0x98 a word that
 * is cleared.
 * Return: none.
 *
 * Non-obvious: The state's u16 at +0x178 is cleared, and its float at +0x20 is
 * set from its signed 16-bit value at +0x24. The flags gain 0x41 and lose
 * 0x1000 in one store. Then func_L20_00330820(arg0, 6, 3) runs.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_0043ECB0.c (blob c24c6906), function
 * func_L20_00440460 (v1.01 address), author commit db93b56d (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2566):
 *   - func_L20_00440460 -> func_L20_004419A0 (our split name at the v2.00
 *     place);
 *   - func_002A7750 -> func_L20_00330820 (the level's own copy at its L20
 *     place);
 *   - `#include "common.h"` dropped; the typedefs this body uses are written
 *     here (or are the file's own); upstream file-scope declarations it does
 *     not use are not carried, the rest keep upstream's order and text,
 *     duplicates included.
 */
void func_L20_004419A0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    *(unsigned short *)(state + 0x178) = 0;
    *(float *)(state + 0x20) = (float)*(short *)(state + 0x24);
    *(int *)(arg0 + 0x98) = 0;
    *(unsigned short *)(arg0 + 0x34) = (*(unsigned short *)(arg0 + 0x34) | 0x41) & 0xEFFF;
    func_L20_00330820(arg0, 6, 3);
}
