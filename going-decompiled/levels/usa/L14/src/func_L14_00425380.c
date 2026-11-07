/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2563, RULING #13291 row L14-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x00425380, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L14's copies of boot helpers, at their v2.00 places. */
extern void func_L14_0031A198();
extern void func_L14_002F6A50();
extern float func_L14_002F70D8(float a, float b);
extern float func_L14_002F6B98(float *p);

/**
 * func_L14_00425380 - aim the object's body at a point.
 *
 * @a0: the object; its state block is at +0x68.
 * @a1: the point.
 *
 * Sets the state halfword +0x776 to 1, takes the object's bone 2 position
 * (func_L14_0031A198) and the vector from it to the point
 * (func_L14_002F6A50). The heading func_L14_002F70D8(v[0], v[1]) goes to
 * state +0x744; the pitch, -func_L14_002F70D8(|v.xy| from func_L14_002F6B98,
 * v[2]) * 0.5, goes to both +0x6E4 and +0x664.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_003F4F98.c,
 * function func_L14_00423DC0 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED in the audit (code 1l); as carried, VERBATIM but for names; edits:
 *   - name func_L14_00423DC0 -> func_L14_00425380 (our split name at
 *     the v2.00 place);
 *   - func_318ED0 -> func_L14_0031A198, func_2F5C68 ->
 *     func_L14_002F6A50, func_2F62F0 -> func_L14_002F70D8,
 *     func_2F5DB0 -> func_L14_002F6B98 (L14's copies);
 *   - the audit's `long long`->`long` respelling (RULING #13197) is
 *     needed only to compile the upstream FILE whole on s136; no line
 *     carried into this TU contains `long long` or `long`, so it
 *     applies to nothing here;
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L14_00425380(char *a0, int a1)
{
    float v[4];
    char *s0;
    float f;

    s0 = *(char **)(a0 + 0x68);
    *(unsigned short *)(s0 + 0x776) = 1;
    func_L14_0031A198(a0, 2, v);
    func_L14_002F6A50(v, a1, v);
    f = func_L14_002F70D8(v[0], v[1]);
    *(float *)(s0 + 0x744) = f;
    f = func_L14_002F6B98(v);
    f = -func_L14_002F70D8(f, v[2]) * 0.5f;
    *(float *)(s0 + 0x6E4) = f;
    *(float *)(s0 + 0x664) = f;
}
