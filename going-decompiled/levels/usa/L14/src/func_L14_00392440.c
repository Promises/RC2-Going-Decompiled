/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2563, RULING #13291 row L14-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x00392440, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* This level's own helper, at its v2.00 place. */
extern void func_L14_00392538(char *a0, char *a1, int a2, int a3, int t0, int t1, int t2,
                        float f12, float f13, float f14, float f15, float f16);

/* A table inside L14's larger label D_L14_0023001E, so address-named
 * (RULING #13291 §2). */
extern char D_L14_00237A80[];

/**
 * func_L14_00392440 - refresh four record pairs of an object's beam table.
 *
 * @arg0: the object; its state block is at +0x68.
 *
 * For i = 0..3 it calls func_L14_00392538 twice, on the 256-byte records
 * (state +0xDC) + i of D_L14_00237A80 and of the table 0x400 bytes further
 * on, with the state's first and second sets of parameters (+0x38.. and
 * +0x48..) and the factors 0.9 and 0.1.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c,
 * function func_L14_00390FE8 (v1.01 address), author commit adcbd51e (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L14_00390FE8 -> func_L14_00392440 (our split name at
 *     the v2.00 place);
 *   - D_L14_00237B00 -> D_L14_00237A80 (the v2.00 relocation target;
 *     inside D_L14_0023001E, so address-named, RULING #13291 §2);
 *   - func_3910E0 -> func_L14_00392538 (v2.00 place);
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L14_00392440(char *arg0) {
    char *s0 = *(char **)(arg0 + 0x68);
    char *base = D_L14_00237A80;
    char *base2 = base + 0x400;
    int i;

    for (i = 0; i < 4; i++) {
        func_L14_00392538(arg0, (char *)(((*(int *)(s0 + 0xDC) + i) << 8) + (unsigned int)base),
                    *(int *)(s0 + 0x38), *(int *)(s0 + 0x40),
                    *(int *)(s0 + 0x60), *(int *)(s0 + 0x44), *(int *)(s0 + 0x64),
                    *(float *)(s0 + 0x50), *(float *)(s0 + 0x70), *(float *)(s0 + 0x54),
                    *(float *)(s0 + 0x74), 0.9f);
        func_L14_00392538(arg0, (char *)(((*(int *)(s0 + 0xDC) + i) << 8) + (unsigned int)base2),
                    *(int *)(s0 + 0x38), *(int *)(s0 + 0x48),
                    *(int *)(s0 + 0x68), *(int *)(s0 + 0x4C), *(int *)(s0 + 0x6C),
                    *(float *)(s0 + 0x58), *(float *)(s0 + 0x78), *(float *)(s0 + 0x5C),
                    *(float *)(s0 + 0x7C), 0.1f);
    }
}
