/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x004419F8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern void func_L20_00302FC0(char *, char *, char *);
extern void func_L20_00303030(char *, char *, float);
extern void func_L20_00332230(char *, char *, int, int, float, float, float, int,
                          int);
extern int func_L20_00302C30(char *);
extern void func_L20_003265E8(char *);
extern void func_L20_00332230(char *, char *, int, int, float, float, float, int,
                          int);

/**
 * func_L20_004419F8 - per-frame step that updates an object's position from
 * its state, then calls func_L20_00332230 and, when func_L20_00302C30 says so,
 * func_L20_003265E8.
 *
 * @arg0: the object; +0x10 its position, +0x68 its state, +0xB8 the pointer
 * passed first to func_L20_00332230.
 * Return: none.
 *
 * Non-obvious: func_L20_00302FC0(pos, pos, state) rewrites the position from
 * the state's 4-vector at +0. The state's float at +8 is reduced by the one at
 * +0x14, and func_L20_00303030 rewrites the state vector in place with the
 * scale state + 0x10. func_L20_00332230 gets 0x10001, 0, the state's +0x18,
 * 3.0, 1.0, 1 and 0. When func_L20_00302C30(state + 0x1C) returns nonzero,
 * func_L20_003265E8(arg0) runs.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_0043ECB0.c (blob c24c6906), function
 * func_L20_004404B8 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2566):
 *   - func_L20_004404B8 -> func_L20_004419F8 (our split name at the v2.00
 *     place);
 *   - func_00282948 -> func_L20_00302C30 (the level's own copy at its L20
 *     place);
 *   - func_00282CC0 -> func_L20_00302FC0 (the level's own copy at its L20
 *     place);
 *   - func_00282D30 -> func_L20_00303030 (the level's own copy at its L20
 *     place);
 *   - func_0029F338 -> func_L20_003265E8 (the level's own copy at its L20
 *     place);
 *   - func_002A8C28 -> func_L20_00332230 (the level's own copy at its L20
 *     place);
 *   - `#include "common.h"` dropped; the typedefs this body uses are written
 *     here (or are the file's own); upstream file-scope declarations it does
 *     not use are not carried, the rest keep upstream's order and text,
 *     duplicates included.
 */
void func_L20_004419F8(char *arg0) {
    char *q = arg0 + 0x10;
    char *state = *(char **)(arg0 + 0x68);

    func_L20_00302FC0(q, q, state);
    *(float *)(state + 8) = *(float *)(state + 8) - *(float *)(state + 0x14);
    func_L20_00303030(state, state, *(float *)(state + 0x10));
    func_L20_00332230(*(char **)(arg0 + 0xB8), q, 0x10001, 0,
                  *(float *)(state + 0x18), 3.0f, 1.0f, 1, 0);
    if (func_L20_00302C30(state + 0x1C) != 0) {
        func_L20_003265E8(arg0);
    }
}
