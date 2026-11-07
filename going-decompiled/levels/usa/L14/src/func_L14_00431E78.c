/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x00431E78, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm Sony 2.9-ee-991111
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

extern float func_L14_002F7B88(float x);

/**
 * func_L14_00431E78 - wrap an angle fraction into turns.
 *
 * @x: an angle in radians.
 * Return: x/2pi, passed through the level's own frac helper and scaled back
 *         to radians.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_003F4F98.c, function
 * func_L14_00430898 (v1.01 address), author commit 91e04732 (llesieur99; the
 * definition line blames 91e04732 too). Adopted with the authors' permission
 * (RULINGs #10405, #12108, #13082, #13291). BYTE-IDENTICAL: VERBATIM but for
 * names; edits:
 *   - name func_L14_00430898 -> func_L14_00431E78 (our split name at the
 *     v2.00 place);
 *   - func_2F6DA0 -> func_L14_002F7B88 (the level's own copy of boot
 *     func_00283CB8 at its L14 place);
 *   - the file's `#include` lines, other functions and their externs not
 *     carried (nothing else here uses them).
 */
float func_L14_00431E78(float x)
{
    return func_L14_002F7B88(x * 0.15915494f) * 6.2831855f;
}
