/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x0044B020, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm ee-gcc
 * 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, assembled by plain
 * GNU as (the row's `as` column).
 */

typedef unsigned int u32;

/**
 * func_L20_0044B020 - read the word at +0xAC of an object's state.
 *
 * @arg0: the object; +0x68 its state.
 * Return: the state's u32 at +0xAC.
 *
 * Non-obvious: A three-instruction leaf on the sdk29 arm; it has no
 * relocations.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_00449178.c (blob deb0cb90), function
 * func_L20_00449AD8 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). The body has no relocations;
 * the only respelling is the function's own name:
 *   - func_L20_00449AD8 -> func_L20_0044B020 (our split name at the v2.00
 *     place);
 *   - `#include "include_asm.h"` dropped; the typedefs this body uses are
 *     written here (or are the file's own); upstream file-scope declarations
 *     it does not use are not carried, the rest keep upstream's order and
 *     text, duplicates included.
 */
u32 func_L20_0044B020(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    return *(u32 *)(state + 0xAC);
}
