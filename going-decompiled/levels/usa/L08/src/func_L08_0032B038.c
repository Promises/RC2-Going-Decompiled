/* Level 08 (Tabora), USA v2.00: a level-only function built from C (task
 * #2627, RULING #13291 row L08-G1). Compiled ALONE as its own TU and linked at
 * its one place, 0x0032B038, in place of its split asm; the row is
 * going-decompiled/levels/usa/L08/config/L08_c.tsv, the arm sdk29
 * (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

/**
 * func_L08_0032B038 - record a race object's two floats.
 *
 * @value: the value stored.
 * @object: the object as a float array; only slots 28 and 29 are written.
 *
 * Sets object[29] (+0x70) to 0.2f and object[28] (+0x6C) to value minus the
 * object's own slot 6 (+0x18). Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/08_tabora/L08_002D90C8.c, function
 * func_L08_00329EC0 (v1.01 address), author commit b31a91e7 (MantraAU).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L08_00329EC0 -> func_L08_0032B038 (our split name at the
 *     v2.00 place);
 *   - the file's `#include` line, other functions and their externs not
 *     carried (nothing here uses them; the body has no external references).
 */
void func_L08_0032B038(float value, float *object) {
    object[29] = 0.2f;
    object[28] = value - object[6];
}
