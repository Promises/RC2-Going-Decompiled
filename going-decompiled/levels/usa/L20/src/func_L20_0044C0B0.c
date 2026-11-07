/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x0044C0B0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef int TI128 __attribute__((mode(TI)));
extern void func_L20_0041B550(void *a0, int a1, int a2, int a3, int a4, int a5, int a6,
                          int a7, float f12, float f13, int s1, int s2, int s3,
                          int s4);
extern void func_L20_00303488(void *, void *, void *);
extern void func_L20_00302FC0(void *, void *, void *);
extern void func_L20_003763C0(int);

/**
 * func_L20_0044C0B0 - call func_L20_0041B550 at a point 0.3 along an object's
 * matrix from its position, then release the handle in the state's +0x1B8.
 *
 * @arg0: the object; +0x10 its position, +0x68 its state, +0xC0 its matrix.
 * Return: none.
 *
 * Non-obvious: The point is (0, 0, 0.3, 0) through func_L20_00303488 with
 * +0xC0, plus the position (func_L20_00302FC0). func_L20_0041B550 gets (point,
 * 0, 0x19, 0x14, 0x14, 0x14, 0x2D, 0, 0.666, 20.0, 1, 0, 0, 0). If the state's
 * word at +0x1B8 is not -1, func_L20_003763C0 is called with it and it is set
 * to -1. The state pointer is loaded after the calls, as upstream writes it.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_00449178.c (blob deb0cb90), function
 * func_L20_0044AB68 (v1.01 address), author commit 3b12a35b (Louis-Philippe Le
 * Sieur). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names (no device). Respellings, each to
 * the target of the v2.00 disc's own relocation at this place (relverify by
 * name on this object, task #2566):
 *   - func_L20_0044AB68 -> func_L20_0044C0B0 (our split name at the v2.00
 *     place);
 *   - func_00282CC0 -> func_L20_00302FC0 (the level's own copy at its L20
 *     place);
 *   - func_00283098 -> func_L20_00303488 (the level's own copy at its L20
 *     place);
 *   - func_002E59F8 -> func_L20_003763C0 (the level's own copy at its L20
 *     place);
 *   - func_0032C700 -> func_L20_0041B550 (the level's own copy at its L20
 *     place);
 *   - `#include "include_asm.h"` dropped; the typedefs this body uses are
 *     written here (or are the file's own); upstream file-scope declarations
 *     it does not use are not carried, the rest keep upstream's order and
 *     text, duplicates included.
 */
void func_L20_0044C0B0(char *arg0) {
    union {
        TI128 q;
        float f[4];
    } v;
    char *state;

    v.q = 0;
    v.f[2] = 0.3f;
    func_L20_00303488(&v, &v, arg0 + 0xC0);
    func_L20_00302FC0(&v, &v, arg0 + 0x10);
    func_L20_0041B550(&v, 0, 0x19, 0x14, 0x14, 0x14, 0x2D, 0, 0.666f, 20.0f, 1, 0,
                  0, 0);
    state = *(char **)(arg0 + 0x68);
    if (*(int *)(state + 0x1B8) != -1) {
        func_L20_003763C0(*(int *)(state + 0x1B8));
        *(int *)(state + 0x1B8) = -1;
    }
}
