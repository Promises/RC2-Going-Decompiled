/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x00445238, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;

typedef int TI128 __attribute__((mode(TI)));
extern void func_L20_004450A8(char *, char *, char *, float *);
extern u8 *func_L20_004416D8(int, const float *, const float *, float);

/**
 * func_L20_00445238 - emit one item for an object through func_L20_004416D8,
 * with a spawn computed by func_L20_004450A8.
 *
 * @arg0: the object; +0x68 its state, whose word at +0x21C is passed first.
 * Return: func_L20_004416D8's result (a pointer).
 *
 * Non-obvious: The float starts at 1.0471977 (pi/3) and func_L20_004450A8
 * scales it before it is passed on. The position and velocity are two 128-bit
 * locals.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_0043ECB0.c (blob c24c6906), function
 * func_L20_00443CF0 (v1.01 address), author commit c17a465e (Fromnowhere).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2566):
 *   - func_L20_00443CF0 -> func_L20_00445238 (our split name at the v2.00
 *     place);
 *   - func_L20_00440198 -> func_L20_004416D8 (its v2.00 split name);
 *   - func_L20_00443B60 -> func_L20_004450A8 (its v2.00 split name);
 *   - `#include "common.h"` dropped; the typedefs this body uses are written
 *     here (or are the file's own); upstream file-scope declarations it does
 *     not use are not carried, the rest keep upstream's order and text,
 *     duplicates included.
 */
u8 *func_L20_00445238(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    TI128 v0;
    TI128 v1;
    float f;

    f = 1.0471977f;
    func_L20_004450A8(arg0, (char *)&v0, (char *)&v1, &f);
    return func_L20_004416D8(*(int *)(state + 0x21C), (float *)&v0, (float *)&v1, f);
}
