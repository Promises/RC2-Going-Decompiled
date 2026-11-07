/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x0039F250, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern void func_L20_003265E8(int);
extern void func_L20_00330820(char *, int, int);

/**
 * func_L20_0039F250 - release a moby's helper objects and make it inactive.
 *
 * Sets animation 6 (func_L20_00330820(moby, 6, 2)), sets flags 0x41 and
 * clears 0x1000 in +0x34, clears +0x98, and frees (func_L20_003265E8) and
 * clears the helper at state +0x254 and the two at state +0x258 / +0x25C.
 *
 * @arg0: the moby; its +0x68 points at the state.
 * Return: nothing.
 *
 * Non-obvious: upstream passes a `char *` to func_L20_003265E8's `int`
 * parameter in the loop (an implicit conversion cc1 warns about); kept as
 * written.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_0039C448.c, function func_L20_0039DE88 (v1.01
 * address), author commit 431ac503 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for
 * names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_0039DE88 -> func_L20_0039F250; func_0029F338 ->
 *     func_L20_003265E8; func_002A7750 -> func_L20_00330820
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 */
void func_L20_0039F250(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char **p;
    int i;

    func_L20_00330820(arg0, 6, 2);
    *(unsigned short *)(arg0 + 0x34) = (*(unsigned short *)(arg0 + 0x34) | 0x41) & 0xEFFF;
    *(int *)(arg0 + 0x98) = 0;
    if (*(int *)(state + 0x254) != 0) {
        func_L20_003265E8(*(int *)(state + 0x254));
        *(int *)(state + 0x254) = 0;
    }
    p = (char **)(state + 0x258);
    for (i = 1; i >= 0; i--) {
        if (*p != 0) {
            func_L20_003265E8(*p);
            *p = 0;
        }
        p++;
    }
}
