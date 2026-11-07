/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x00444E50, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern void func_L20_00302FC0(char *, char *, char *);
extern int func_L20_00302C30(char *);
extern void func_L20_003265E8(char *);
typedef int TI128 __attribute__((mode(TI)));
typedef union {
    TI128 q;
    float f[4];
} Vec4;
extern void func_L20_00303488(void *, void *, void *);
extern void func_L20_00303D90(void *, void *);
extern void func_L20_00303B70(void *, void *, void *);
extern void func_L20_00302FF0(void *, void *, void *);

/**
 * func_L20_00444E50 - per-frame step that updates an object's position from
 * its state, applies a rotated 0.5 offset through its matrix at +0xC0, and
 * calls func_L20_003265E8 when func_L20_00302C30 says so.
 *
 * @arg0: the object; +0x10 its position, +0x68 its state, +0xC0 its matrix
 * (rewritten).
 * Return: none.
 *
 * Non-obvious: func_L20_00302FC0(pos, pos, state) rewrites the position first,
 * and the state's float at +8 is reduced by the one at +0x20. A local 4-vector
 * (0, 0, 0.5, 0) goes through func_L20_00303488 with +0xC0; func_L20_00303D90
 * reads state + 0x10, and func_L20_00303B70 rewrites +0xC0 in place. A second
 * func_L20_00303488 pass follows, func_L20_00302FF0 combines the two results,
 * and func_L20_00302FC0 applies that to the position. The locals are 128-bit
 * unions (TI mode), as upstream writes them. func_L20_003265E8(arg0) runs when
 * func_L20_00302C30(state + 0x24) returns nonzero.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_0043ECB0.c (blob c24c6906), function
 * func_L20_00443908 (v1.01 address), author commit 3b12a35b (Louis-Philippe Le
 * Sieur). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names (no device). Respellings, each to
 * the target of the v2.00 disc's own relocation at this place (relverify by
 * name on this object, task #2566):
 *   - func_L20_00443908 -> func_L20_00444E50 (our split name at the v2.00
 *     place);
 *   - func_00282948 -> func_L20_00302C30 (the level's own copy at its L20
 *     place);
 *   - func_00282CC0 -> func_L20_00302FC0 (the level's own copy at its L20
 *     place);
 *   - func_00282CF0 -> func_L20_00302FF0 (the level's own copy at its L20
 *     place);
 *   - func_00283098 -> func_L20_00303488 (the level's own copy at its L20
 *     place);
 *   - func_00283738 -> func_L20_00303B70 (the level's own copy at its L20
 *     place);
 *   - func_00283958 -> func_L20_00303D90 (the level's own copy at its L20
 *     place);
 *   - func_0029F338 -> func_L20_003265E8 (the level's own copy at its L20
 *     place);
 *   - `#include "common.h"` dropped; the typedefs this body uses are written
 *     here (or are the file's own); upstream file-scope declarations it does
 *     not use are not carried, the rest keep upstream's order and text,
 *     duplicates included.
 */
void func_L20_00444E50(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    Vec4 v[6];

    func_L20_00302FC0(arg0 + 0x10, arg0 + 0x10, state);
    *(float *)(state + 8) = *(float *)(state + 8) - *(float *)(state + 0x20);
    v[2].q = 0;
    v[2].f[2] = 0.5f;
    func_L20_00303488(&v[0], &v[2], arg0 + 0xC0);
    func_L20_00303D90(state + 0x10, &v[3]);
    func_L20_00303B70(arg0 + 0xC0, &v[3], arg0 + 0xC0);
    func_L20_00303488(&v[1], &v[2], arg0 + 0xC0);
    func_L20_00302FF0(&v[2], &v[0], &v[1]);
    func_L20_00302FC0(arg0 + 0x10, arg0 + 0x10, (char *)&v[2]);
    if (func_L20_00302C30(state + 0x24) != 0) {
        func_L20_003265E8(arg0);
    }
}
