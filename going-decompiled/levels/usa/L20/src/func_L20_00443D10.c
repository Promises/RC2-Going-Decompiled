/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x00443D10, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern void func_L20_00332230(char *, char *, int, int, float, float, float, int,
                          int);
extern void func_L20_00332230(char *, char *, int, int, float, float, float, int,
                          int);
extern float func_L20_003031A0(char *, char *);
extern void func_L20_00341D88(char *, char *, float);
extern void func_L20_00341B68(char *, char *);

/**
 * func_L20_00443D10 - call func_L20_00332230 for an object, then call
 * func_L20_00341D88 or func_L20_00341B68 for the target, by a distance test.
 *
 * @arg0: the object; +0x10 its position, +0x68 its state.
 * @arg1: the target, passed on to both branches.
 * @f: a float passed to func_L20_00341D88.
 * Return: none.
 *
 * Non-obvious: func_L20_00332230 gets 0x04010000, 0, the state's +0x50 plus
 * 0.5, 50.0, 1.0, 1 and 0. If func_L20_003031A0(arg0 + 0x10, arg1) <= the
 * state's float at +0x84, func_L20_00341D88(arg0, arg1, f) runs; otherwise
 * func_L20_00341B68(arg0, arg1) runs.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_0043ECB0.c (blob c24c6906), function
 * func_L20_004427C8 (v1.01 address), author commit adcbd51e (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2566):
 *   - func_L20_004427C8 -> func_L20_00443D10 (our split name at the v2.00
 *     place);
 *   - func_00282E80 -> func_L20_003031A0 (the level's own copy at its L20
 *     place);
 *   - func_002A8C28 -> func_L20_00332230 (the level's own copy at its L20
 *     place);
 *   - func_002B50E0 -> func_L20_00341D88 (the level's own copy at its L20
 *     place);
 *   - func_L01_00321178 -> func_L20_00341B68 (the level's own copy at its L20
 *     place);
 *   - `#include "common.h"` dropped; the typedefs this body uses are written
 *     here (or are the file's own); upstream file-scope declarations it does
 *     not use are not carried, the rest keep upstream's order and text,
 *     duplicates included.
 */
void func_L20_00443D10(char *arg0, char *arg1, float f) {
    char *state = *(char **)(arg0 + 0x68);

    func_L20_00332230(arg0, arg0 + 0x10, 0x04010000, 0,
                  *(float *)(state + 0x50) + 0.5f, 50.0f, 1.0f, 1, 0);
    if (func_L20_003031A0(arg0 + 0x10, arg1) <= *(float *)(state + 0x84)) {
        func_L20_00341D88(arg0, arg1, f);
    } else {
        func_L20_00341B68(arg0, arg1);
    }
}
