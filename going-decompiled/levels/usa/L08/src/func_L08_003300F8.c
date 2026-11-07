/* Level 08 (Tabora), USA v2.00: a level-only function built from C (task
 * #2627, RULING #13291 row L08-G1). Compiled ALONE as its own TU and linked at
 * its one place, 0x003300F8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L08/config/L08_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned int ti_word __attribute__((mode(TI)));
typedef float f32;

/* The level's other shape checkers, by our split names (each its own TU or
 * split asm; func_L08_00330050 is adopted in this same row). */
extern float func_L08_0032FCC0(ti_word *, int, float);
extern float func_L08_0032FDB0(ti_word *, int, float);
extern float func_L08_0032FF10(ti_word *, int, float);
extern float func_L08_00330050(ti_word *, int, float);

/**
 * func_L08_003300F8 - the four-way maximum over the level's shape lists.
 *
 * @input: a 16-byte scratch block whose first word is copied through.
 * @initial/first/second/third: the four entity handles keyed to the four
 *   checkers.
 *
 * Chains the four checkers over the same scratch copy of *input, each one
 * keyed by its own handle and each starting from the previous best:
 * func_L08_00330050 (the 0x10-stride lists), func_L08_0032FCC0 (the corner
 * sweep), func_L08_0032FDB0 (the axis sweeps), func_L08_0032FF10 (the
 * two-axis sweep). Return: the final best.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/08_tabora/L08_002D90C8.c, function
 * func_L08_0032EF80 (v1.01 address), author commit b31a91e7 (MantraAU).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED (byte-identical; two byte-neutral edits beyond names,
 * named by validator #2637); edits:
 *   - name func_L08_0032EF80 -> func_L08_003300F8 (our split name at the
 *     v2.00 place);
 *   - func_L08_0032EED8 -> func_L08_00330050 (adopted here, same row),
 *     func_L08_0032EB48 -> func_L08_0032FCC0, func_L08_0032EC38 ->
 *     func_L08_0032FDB0, func_L08_0032ED98 -> func_L08_0032FF10 (our split
 *     names at the relverify disc-implied places);
 *   - the file's `#include` line, other functions and their externs not
 *     carried (nothing else here uses them); the ti_word/f32 typedefs kept;
 *   - the return type written `f32` where upstream has `float` (the same
 *     type through the kept typedef);
 *   - an `extern float func_L08_00330050(ti_word *, int, float);` prototype
 *     added for the sibling this body calls (upstream defines it earlier in
 *     the same file, so needs none).
 */
f32 func_L08_003300F8(ti_word *input, int initial, int first, int second, int third) {
    ti_word value = *input;
    float result;
    result = func_L08_00330050(&value, initial, 0.0f);
    result = func_L08_0032FCC0(&value, first, result);
    result = func_L08_0032FDB0(&value, second, result);
    return func_L08_0032FF10(&value, third, result);
}
