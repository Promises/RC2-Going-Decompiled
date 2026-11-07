/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2563, RULING #13291 row L14-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x00415288, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* A vector inside L14's larger .bss label D_L14_001B28B0, so address-named
 * (RULING #13291 §2). */
extern char D_L14_001BF900[];

/* L14's copies of boot vector helpers, at their v2.00 places in this level. */
extern void func_L14_002F6D00(void *out, void *in, float scale);

extern void func_L14_002F6A20(void *out, void *in1, void *in2);

extern int func_L14_002E61F8(void *a, void *b, int mode, void *self, int flag);

extern void func_L14_002F6AA8(void *out, void *in, float scale);

/**
 * func_L14_00415288 - pick the direction the actor should face from a probe
 * along its axis.
 *
 * @self: the actor; its axis vector is at +0xE0, its position at +0x10.
 * @p1:   the output vector.
 * @p2:   0 negates the output at the end.
 *
 * Builds two probe points, the axis scaled by 0.1 and by -3.0, each moved to
 * the actor's position (func_L14_002F6D00, func_L14_002F6A20). When
 * func_L14_002E61F8(a, b, 2, self, 0) reports a hit between them, the output
 * is the level vector D_L14_001BF900 (scaled by 1.0); otherwise it is the
 * axis scaled by -1.0. With p2 == 0 it is then scaled by -1.0
 * (func_L14_002F6AA8).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_003F4F98.c,
 * function func_L14_00413CD8 (v1.01 address), author commit 7fccf735 (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED in the audit (code 1lo); as carried, VERBATIM but for names; edits:
 *   - name func_L14_00413CD8 -> func_L14_00415288 (our split name at
 *     the v2.00 place);
 *   - func_00282F20_2 -> func_L14_002F6D00, func_00282CC0_6 ->
 *     func_L14_002F6A20, func_002761F8 -> func_L14_002E61F8,
 *     func_00282D30_6 -> func_L14_002F6AA8 (L14's copies, the targets
 *     of the v2.00 relocations);
 *   - D_L14_001BF980 -> D_L14_001BF900 (the v2.00 relocation target;
 *     inside D_L14_001B28B0, so address-named, RULING #13291 §2);
 *   - the audit's `long long`->`long` respelling (RULING #13197) is
 *     needed only to compile the upstream FILE whole on s136; no line
 *     carried into this TU contains `long long` or `long`, so it
 *     applies to nothing here;
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L14_00415288(char *self, char *p1, int p2)
{
    float a[4];
    float b[4];

    func_L14_002F6D00(a, self + 224, 0.1f);
    func_L14_002F6D00(b, self + 224, -3.0f);
    func_L14_002F6A20(a, a, self + 16);
    func_L14_002F6A20(b, b, self + 16);

    if (func_L14_002E61F8(a, b, 2, self, 0))
        func_L14_002F6D00(p1, D_L14_001BF900, 1.0f);
    else
        func_L14_002F6D00(p1, self + 224, -1.0f);

    if (p2 == 0)
        func_L14_002F6AA8(p1, p1, -1.0f);
}
