/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x0044B650, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern void func_L20_0041B550(void *a0, int a1, int a2, int a3, int a4, int a5, int a6,
                          int a7, float f12, float f13, int s1, int s2, int s3,
                          int s4);
extern void func_L20_00332230(char *, char *, int, int, float, float, float, int, int);
extern void func_L20_003265E8(char *object);

/**
 * func_L20_0044B650 - call func_L20_0041B550 at an object's position,
 * optionally call func_L20_00332230, then finish the object with
 * func_L20_003265E8.
 *
 * @arg0: the object; +0x10 its position, +0xB8 the pointer passed first to
 * func_L20_00332230.
 * @arg1: when nonzero, the third (int) argument of func_L20_00332230; when 0,
 * that call is skipped.
 * Return: none.
 *
 * Non-obvious: func_L20_0041B550 takes 14 arguments: (pos, 0, 5, 4, 4, 4, 7,
 * 0, 0.333, 20.0, 0, 0, 0, 0). The last four go on the stack ($s-named in
 * upstream's prototype). func_L20_00332230 gets 0, 1.5, 4.0, 1.0, 1 and 0
 * after arg1.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_00449178.c (blob deb0cb90), function
 * func_L20_0044A108 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2566):
 *   - func_L20_0044A108 -> func_L20_0044B650 (our split name at the v2.00
 *     place);
 *   - func_0029F338 -> func_L20_003265E8 (the level's own copy at its L20
 *     place);
 *   - func_002A8C28 -> func_L20_00332230 (the level's own copy at its L20
 *     place);
 *   - func_0032C700 -> func_L20_0041B550 (the level's own copy at its L20
 *     place);
 *   - `#include "include_asm.h"` dropped; the typedefs this body uses are
 *     written here (or are the file's own); upstream file-scope declarations
 *     it does not use are not carried, the rest keep upstream's order and
 *     text, duplicates included.
 */
void func_L20_0044B650(char *arg0, char *arg1) {
    func_L20_0041B550(arg0 + 0x10, 0, 5, 4, 4, 4, 7, 0, 0.333f, 20.0f, 0, 0, 0, 0);
    if (arg1 != 0) {
        func_L20_00332230(*(char **)(arg0 + 0xB8), arg0 + 0x10, (int)arg1, 0, 1.5f,
                      4.0f, 1.0f, 1, 0);
    }
    func_L20_003265E8(arg0);
}
