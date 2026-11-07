/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x00444158, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern void func_L20_00330820(int, int, int);
extern int func_L20_00330930(int, int);
extern int func_L20_004448F8(char *);
extern int func_L20_003308F0(int);

/**
 * func_L20_00444158 - choose an object's next state from the state word at
 * +0x17F4, then copy the object's byte +0x95 into that word.
 *
 * @arg0: the object; +0x68 its state, +0x95 a byte.
 * Return: none.
 *
 * Non-obvious: When the word is 1: the result of func_L20_004448F8(arg0) is
 * stored at state + 0x17FC, and the object goes to state 0xC (sub 4) if it is
 * 0, else state 4 (sub 2), through func_L20_00330820. Any other value: state +
 * 0x17F8 = func_L20_003308F0(2) + 2, state + 0x1810 = func_L20_00330930(0xB4,
 * 0x168), and func_L20_00330820(arg0, 3, 1) runs. Upstream's goto chain (== 2,
 * >= 3, != 1) is kept as written; it is what gives the ROM's compare order.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_0043ECB0.c (blob c24c6906), function
 * func_L20_00442C10 (v1.01 address), author commit adcbd51e (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2566):
 *   - func_L20_00442C10 -> func_L20_00444158 (our split name at the v2.00
 *     place);
 *   - func_002A7750 -> func_L20_00330820 (the level's own copy at its L20
 *     place);
 *   - func_002A77E0 -> func_L20_003308F0 (the level's own copy at its L20
 *     place);
 *   - func_002A7820 -> func_L20_00330930 (the level's own copy at its L20
 *     place);
 *   - func_L20_004433B0 -> func_L20_004448F8 (its v2.00 split name);
 *   - `#include "common.h"` dropped; the typedefs this body uses are written
 *     here (or are the file's own); upstream file-scope declarations it does
 *     not use are not carried, the rest keep upstream's order and text,
 *     duplicates included.
 */
void func_L20_00444158(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    int v = *(int *)(state + 0x17F4);
    int r;

    if (v == 2) goto other;
    if (v >= 3) goto other;
    if (v != 1) goto other;
    r = func_L20_004448F8(arg0);
    *(int *)(state + 0x17FC) = r;
    if (r == 0) {
        func_L20_00330820(arg0, 0xC, 4);
    } else {
        func_L20_00330820(arg0, 4, 2);
    }
    goto done;
other:
    r = func_L20_003308F0(2) + 2;
    *(int *)(state + 0x17F8) = r;
    *(int *)(state + 0x1810) = func_L20_00330930(0xB4, 0x168);
    func_L20_00330820(arg0, 3, 1);
done:
    *(int *)(state + 0x17F4) = *(unsigned char *)(arg0 + 0x95);
}
