/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x0039EF18, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern void func_L20_00330540(void *, int, int, int);

/**
 * func_L20_0039EF18 - set the animation of a moby and of its helper object.
 *
 * Calls func_L20_00330540(moby, anim, (int)blend, flags) when the moby's
 * current animation +0x43 differs from @arg1, and always the same for the
 * helper object at state +0x254.
 *
 * @arg0: the moby. @arg1: animation id. @arg2: flags. @f12: blend time
 * (converted to int).
 * Return: nothing.
 *
 * Non-obvious: the float argument arrives in $f12 and is truncated before
 * both calls.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_0039C448.c, function func_L20_0039DB50 (v1.01
 * address), author commit fa8d6672 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for
 * names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_0039DB50 -> func_L20_0039EF18; func_002A7470 ->
 *     func_L20_00330540
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 */
void func_L20_0039EF18(char *arg0, int arg1, int arg2, float f12) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(unsigned char *)(arg0 + 0x43) != arg1) {
        func_L20_00330540(arg0, arg1, (int)f12, arg2);
    }
    func_L20_00330540(*(char **)(state + 0x254), arg1, (int)f12, arg2);
}
