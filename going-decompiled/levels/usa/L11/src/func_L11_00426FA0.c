/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x00426FA0, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 104 of
 * 149 .text words differ from the disc, relocated fields masked, and .text is
 * 0x254 bytes, not the place's 0x2C4, and it emits a .lit4 pool of 52 bytes,
 * which level_elf.py cobj refuses (only .text is allowed).
 */

typedef float f32;
extern void func_L11_00301210(f32 *, f32 *, f32 *);
extern f32 func_L11_00301340(f32 *);
extern void func_L11_003014F8(f32 *, f32 *, f32);
extern void func_L11_003011E0(f32 *, f32 *, f32 *);
extern f32 func_L11_00302390(f32, f32);
extern int func_L11_0032E668(char *, char *, char *, f32, f32, f32, f32, int);
extern f32 func_L11_00331048(f32 *, f32, f32);
extern void func_L11_00331348(f32 *, char *, f32, f32, f32, f32);
extern void func_L11_00331800(char *, f32, f32, f32, f32, char *, int);
extern int func_L11_0033CF50(char *, int, f32 *, char *, int, char *, char *, f32, f32);

/**
 * func_L11_00426FA0 - steer the object toward a point and settle on a heading
 * there.
 *
 * @object: the walker (moby); +0x10 position, +0xC0 its forward vector, +0xF8 yaw,
 *          +0x68 its state block (+0x27C, +0x320, +0x520..+0x53C).
 * @point:  the target point.
 * @value:  the heading to settle on once within @limit.
 * @rate:   the walk speed (units per second; per frame it is rate/60, and the
 *          acceleration rate/1200).
 * @limit:  the stop distance.
 * @extra:  the margin passed to the path query func_L11_0033CF50.
 * Return: 1 when it has stopped and faces @value within 5 degrees, or after 30
 * frames in which the move did not succeed; else 0.
 *
 * Non-obvious: Far from the point it asks func_L11_0033CF50 for a path
 * heading, and func_L11_00331800 turns the yaw toward it. It only speeds up
 * (func_L11_00331348) when within 30 degrees of that heading, else its speed
 * (+0x538) is zeroed. Near the point it turns toward @value and brakes
 * (func_L11_00331048). The step is then tried with func_L11_0032E668, and each
 * failure counts in +0x53C. The same body as func_L11_0042B988, from another
 * upstream file; `point` is passed as an int to the path query, as upstream
 * declares it.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_00420C88.c (blob 8360e080), function
 * func_L11_00425788 (v1.01 address), author commit aaa28c3b (Flavius). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED: built at -G8 where upstream builds the file at -G0 (its stem has no
 * -g8; a tsv gflag setting, not a text edit; measured at -G0, task #2578:
 * .text 0x2BC bytes, not 0x2C4 (cobj refuses)); the text is verbatim but for
 * names. Respellings, each to the target of the v2.00 disc's own relocation at
 * this place (relverify by name on this object, task #2578):
 *   - func_L11_00425788 -> func_L11_00426FA0 (our split name at the v2.00
 *     place);
 *   - func_00282CC0 -> func_L11_003011E0 (the level's own copy at its L11
 *     place);
 *   - func_00282CF0 -> func_L11_00301210 (the level's own copy at its L11
 *     place);
 *   - func_00282DF0 -> func_L11_00301340 (the level's own copy at its L11
 *     place);
 *   - func_00282F20 -> func_L11_003014F8 (the level's own copy at its L11
 *     place);
 *   - func_00283C80 -> func_L11_00302390 (the level's own copy at its L11
 *     place);
 *   - func_002A7EA0 -> func_L11_0032E668 (the level's own copy at its L11
 *     place);
 *   - func_002AA2E8 -> func_L11_00331048 (the level's own copy at its L11
 *     place);
 *   - func_002AA548 -> func_L11_00331348 (the level's own copy at its L11
 *     place);
 *   - func_002AAA00 -> func_L11_00331800 (the level's own copy at its L11
 *     place);
 *   - func_L02_0032E998 -> func_L11_0033CF50 (the level's own copy at its L11
 *     place);
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
int func_L11_00426FA0(char *object, char *point, f32 value, f32 rate, f32 limit, f32 extra)
{
    char *st = *(char **)(object + 0x68);
    f32 v[4];
    f32 w[4];
    f32 x[4];
    f32 d = 0.0f;
    f32 out;
    f32 len;
    f32 t;
    int ret = 0;

    func_L11_00301210(v, (f32 *)point, (f32 *)(object + 0x10));
    v[2] = 0.0f;
    len = func_L11_00301340(v);
    t = rate * 0.0008333333535119891f;
    if (limit < len) {
        int c = *(int *)(st + 0x520);

        out = 0.0f;
        func_L11_0033CF50(object + 0x10, (int)point, &out, st + 0x320, c, st + 0x530,
                          st + 0x534, extra, 0.4f);
        func_L11_00331800(object + 0xF8, out, 0.006981317885220051f,
                      0.0034906589426100254f, 0.20943954586982727f, st + 0x27C, 0);
        if (func_L11_00302390(*(f32 *)(object + 0xF8), out) < 0.52359885f) {
            func_L11_00331348(&d, st + 0x538, len, t, t, rate * 0.01666666753590107f);
        } else {
            *(f32 *)(st + 0x538) = 0.0f;
        }
    } else {
        func_L11_00331800(object + 0xF8, value, 0.006981317885220051f, 0.2f,
                      0.20943954586982727f, st + 0x27C, 0);
        func_L11_00331048((f32 *)(st + 0x538), 0.0f, t);
        if (*(f32 *)(st + 0x538) == 0.0f) {
            if (func_L11_00302390(*(f32 *)(object + 0xF8), value) < 0.0872664749622345f) {
                *(f32 *)(st + 0x27C) = 0.0f;
                ret = 1;
            }
        }
    }
    func_L11_003014F8(w, (f32 *)(object + 0xC0), *(f32 *)(st + 0x538));
    func_L11_003011E0(x, w, (f32 *)(object + 0x10));
    if (func_L11_0032E668(object, object + 0x10, (char *)x, 0.75f, 0.75f, 0.4f,
                     0.52359885f, 0) == 0) {
        *(int *)(st + 0x53C) = *(int *)(st + 0x53C) + 1;
    }
    /* The counter at +0x53C wraps at 0x1E, and the object then reports the change. */
    if (*(int *)(st + 0x53C) < 0x1E) {
        goto done;
    }
    *(int *)(st + 0x53C) = 0;
    ret = 1;
done:
    return ret;
}
