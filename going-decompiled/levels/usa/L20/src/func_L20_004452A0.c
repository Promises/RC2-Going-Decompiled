/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x004452A0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern int func_L20_00330930(int, int);
typedef int TI128 __attribute__((mode(TI)));
extern void func_L20_004450A8(char *, char *, char *, float *);
extern char *func_L20_0044B518(char *, char *, char *, float, int, float);

/**
 * func_L20_004452A0 - emit one item for an object through func_L20_0044B518,
 * with a spawn computed by func_L20_004450A8 and a random count.
 *
 * @arg0: the object; +0x68 its state, whose byte at +0x36 becomes the last
 * (float) argument.
 * Return: func_L20_0044B518's result (a pointer).
 *
 * Non-obvious: The float starts at 1.0471977 (pi/3) and is scaled by
 * func_L20_004450A8. The int argument is func_L20_00330930(0x12C, 0x258).
 * Upstream's copy of the float into a second local (g = f) is kept as written.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_0043ECB0.c (blob c24c6906), function
 * func_L20_00443D58 (v1.01 address), author commit c17a465e (Fromnowhere).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2566):
 *   - func_L20_00443D58 -> func_L20_004452A0 (our split name at the v2.00
 *     place);
 *   - func_002A7820 -> func_L20_00330930 (the level's own copy at its L20
 *     place);
 *   - func_L20_00443B60 -> func_L20_004450A8 (its v2.00 split name);
 *   - func_L20_00449FD0 -> func_L20_0044B518 (its v2.00 split name);
 *   - `#include "common.h"` dropped; the typedefs this body uses are written
 *     here (or are the file's own); upstream file-scope declarations it does
 *     not use are not carried, the rest keep upstream's order and text,
 *     duplicates included.
 */
char *func_L20_004452A0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    TI128 a;
    TI128 b;
    float f;
    float g;
    int r;

    f = 1.0471977f;
    func_L20_004450A8(arg0, (char *)&a, (char *)&b, &f);
    g = f;
    r = func_L20_00330930(0x12C, 0x258);
    return func_L20_0044B518(arg0, (char *)&a, (char *)&b, g, r, (float)*(unsigned char *)(state + 0x36));
}
