/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x0044AFC0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned short u16;
extern void func_L20_00330820(char *, int, int);

/**
 * func_L20_0044AFC0 - move an object to state 1 through func_L20_00330820 and
 * reset its flags.
 *
 * @arg0: the object; +0x34 its u16 flags, +0x98 a word that is cleared.
 * Return: none.
 *
 * Non-obvious: Flag 0x1000 is cleared before the call. Then +0x98 is cleared,
 * and the flags gain 0x41 and lose 0x1000 again in one store.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_00449178.c (blob deb0cb90), function
 * func_L20_00449A78 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2566):
 *   - func_L20_00449A78 -> func_L20_0044AFC0 (our split name at the v2.00
 *     place);
 *   - func_002A7750 -> func_L20_00330820 (the level's own copy at its L20
 *     place);
 *   - `#include "include_asm.h"` dropped; the typedefs this body uses are
 *     written here (or are the file's own); upstream file-scope declarations
 *     it does not use are not carried, the rest keep upstream's order and
 *     text, duplicates included.
 */
void func_L20_0044AFC0(char *arg0) {
    *(u16 *)(arg0 + 0x34) &= 0xEFFF;
    func_L20_00330820(arg0, 1, -1);
    *(int *)(arg0 + 0x98) = 0;
    *(u16 *)(arg0 + 0x34) = (*(u16 *)(arg0 + 0x34) | 0x41) & 0xEFFF;
}
