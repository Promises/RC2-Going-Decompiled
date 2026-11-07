/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x004450A8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef float f32;

typedef int TI128 __attribute__((mode(TI)));
typedef union {
    TI128 v;
    struct { f32 x, y, z, w; } f;
} L20_V4;
extern float func_L20_00330988(float, float);
extern void func_L20_00303828(TI128 *, TI128 *);
extern void func_L20_00303B70(TI128 *, TI128 *, char *);
extern void func_L20_00302FC0(char *, char *, char *);
extern void func_L20_00303488(char *, char *, char *);
extern void func_L20_00303030(char *, char *, float);

/**
 * func_L20_004450A8 - build a spawn position and a per-frame velocity for
 * something an object emits, from its matrix and random terms, and scale a
 * caller's float.
 *
 * @arg0: the object; +0x10 its position, +0x68 its state (float +0x298 used),
 * +0xC0 its matrix.
 * @arg1: out, a 4-vector position.
 * @arg2: out, a 4-vector per-frame velocity.
 * @arg3: in/out, a float multiplied by the random y term.
 * Return: none.
 *
 * Non-obvious: A random y in func_L20_00330988(-1.0, 1.0) is drawn. A basis is
 * built by func_L20_00303828 from the angles (0, 0, state float +0x298 * 0.5),
 * and func_L20_00303B70 combines it with the matrix at +0xC0. The offset (5.0,
 * y, 5.5) goes through that basis and is added to the position to give arg1.
 * The velocity (func_L20_00330988(5.0, 10.0), y, func_L20_00330988(4.0, 6.0))
 * goes through the basis and is scaled by 1/60 (0.016666668) into arg2.
 * Finally *arg3 is multiplied by y.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_00443B60-helpers.c (blob b37ec192),
 * function func_L20_00443B60 (v1.01 address), author commit bf60a47f
 * (Louis-Philippe Le Sieur). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082, #13291). VERBATIM but for names (no device).
 * Respellings, each to the target of the v2.00 disc's own relocation at this
 * place (relverify by name on this object, task #2566):
 *   - func_L20_00443B60 -> func_L20_004450A8 (our split name at the v2.00
 *     place);
 *   - func_00282CC0 -> func_L20_00302FC0 (the level's own copy at its L20
 *     place);
 *   - func_00282D30 -> func_L20_00303030 (the level's own copy at its L20
 *     place);
 *   - func_00283098 -> func_L20_00303488 (the level's own copy at its L20
 *     place);
 *   - func_002833F0 -> func_L20_00303828 (the level's own copy at its L20
 *     place);
 *   - func_00283738 -> func_L20_00303B70 (the level's own copy at its L20
 *     place);
 *   - func_002A7878 -> func_L20_00330988 (the level's own copy at its L20
 *     place);
 *   - `#include "common.h"` dropped; the typedefs this body uses are written
 *     here (or are the file's own); upstream file-scope declarations it does
 *     not use are not carried, the rest keep upstream's order and text,
 *     duplicates included.
 */
void func_L20_004450A8(char *arg0, char *arg1, char *arg2, float *arg3) {
    char *state = *(char **)(arg0 + 0x68);
    L20_V4 v1;
    L20_V4 v2;
    L20_V4 v3;
    struct {
        TI128 angles;
        L20_V4 adjustment;
        TI128 basis[3];
    } transform;
    f32 r;

    v1.v = 0;
    v1.f.x = 5.0f;
    v3.v = 0;
    v1.f.z = 5.5f;
    r = func_L20_00330988(-1.0f, 1.0f);
    v3.f.y = r;
    transform.adjustment.v = 0;
    v2.v = v3.v;
    transform.adjustment.f.z = *(float *)(state + 0x298) * 0.5f;
    transform.angles = transform.adjustment.v;
    func_L20_00303828(transform.basis, &transform.angles);
    func_L20_00303B70(transform.basis, transform.basis, arg0 + 0xC0);
    func_L20_00302FC0((char *)&v1, (char *)&v1, (char *)&v2);
    func_L20_00303488((char *)&v1, (char *)&v1, (char *)&transform.basis);
    func_L20_00302FC0(arg1, (char *)&v1, arg0 + 0x10);
    r = func_L20_00330988(5.0f, 10.0f);
    v2.f.x = r;
    r = func_L20_00330988(4.0f, 6.0f);
    v2.f.z = r;
    func_L20_00303488((char *)&v3, (char *)&v2, (char *)&transform.basis);
    func_L20_00303030(arg2, (char *)&v3, 0.016666668f);
    *arg3 = *arg3 * v2.f.y;
}
