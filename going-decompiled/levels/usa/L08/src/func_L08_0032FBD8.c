/* Level 08 (Tabora), USA v2.00: a level-only function built from C (task
 * #2627, RULING #13291 row L08-G1). Compiled ALONE as its own TU and linked at
 * its one place, 0x0032FBD8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L08/config/L08_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef float f32;

/* L08's copies of the shared vector helpers, at their v2.00 places. */
extern void func_L08_002FAAB0(void *, void *, f32);
extern void func_L08_002FAA40(void *, void *, void *);
extern f32 func_L08_002FAC20(void *, void *);

/**
 * func_L08_0032FBD8 - the projection of a transformed point triple, floored.
 *
 * @a0: the reference point for the projection.
 * @a1: three consecutive 0x10-stride source rows.
 * @a2: a matrix or row composed against the rotated rows.
 * @f0: the floor the result may not undercut.
 * @f1..f3: the three rotation amounts.
 *
 * Each source row in turn is rotated by func_L08_002FAAB0 and composed onto
 * `out` by func_L08_002FAA40; the finished triple is projected against a0 by
 * func_L08_002FAC20, and the return is max(f0, that value).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/08_tabora/L08_002D90C8.c, function
 * func_L08_0032EA60 (v1.01 address), author commit ee3fee45 (llesieur99, the
 * same account as Louis-Philippe Le Sieur). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). BYTE-IDENTICAL:
 * VERBATIM but for names; edits:
 *   - name func_L08_0032EA60 -> func_L08_0032FBD8 (our split name at the
 *     v2.00 place);
 *   - func_2F9E60 -> func_L08_002FAAB0, func_2F9DF0 -> func_L08_002FAA40,
 *     func_2F9FD0 -> func_L08_002FAC20 (L08's copies of the shared helpers,
 *     named by the relverify disc-implied targets);
 *   - the file's `#include` line, other functions and their externs not
 *     carried (nothing here uses them).
 */
f32 func_L08_0032FBD8(char *a0, char *a1, char *a2, f32 f0, f32 f1, f32 f2, f32 f3) {
    f32 out[4];
    f32 t[4];
    f32 r;

    func_L08_002FAAB0(t, a1, f1);
    func_L08_002FAA40(out, a2, t);
    func_L08_002FAAB0(t, a1 + 0x10, f2);
    func_L08_002FAA40(out, out, t);
    func_L08_002FAAB0(t, a1 + 0x20, f3);
    func_L08_002FAA40(out, out, t);
    r = func_L08_002FAC20(a0, out);
    if (f0 < r) f0 = r;
    return f0;
}
