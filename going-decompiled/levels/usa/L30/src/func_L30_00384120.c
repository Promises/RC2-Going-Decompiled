/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00384120, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/* The boot's memset (core name at 0x00115484), with upstream's prototype. */
extern void memset(char *, int, int);

/* Two blocks inside L30's larger label D_00220000, so address-named (RULING
 * #13291 section 2): the 0x1B00-byte block cleared here, and the 24x24 grid
 * of two-float cells. */
extern char D_002338E0[];
extern char D_L30_002353E0[];

typedef struct {
    float x;
    float y;
} L30Pair2f;

/**
 * func_L30_00384120 - clear the work block and lay out a 24 by 24 grid of
 * two-float cells.
 *
 * memset clears 0x1B00 bytes at D_002338E0. Then each cell (row i, column
 * j) of the grid at D_L30_002353E0 (rows 0xC0 bytes apart) gets
 * x = i / 6 - 2 and y = j / 6 - 2, so both run from -2 in steps of one
 * sixth.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/30_insomniac_museum/L30_0032AAF0.c, function
 * func_L30_00382F10 (v1.01 address), author commit 6b21de6b (Louis-Philippe
 * Le Sieur; `git blame -M -C` gives all 14 definition lines to it, `git log
 * -S` agrees). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names, each name to the target of the
 * v2.00 disc's own relocation at this place (task #2673 relverify by name):
 *   - func_L30_00382F10 -> func_L30_00384120 (our split name);
 *   - func_00115484 -> memset (the boot's core name at 0x00115484;
 *     upstream's own prototype kept);
 *   - D_00233A60 -> D_002338E0 and D_L30_00235560 -> D_L30_002353E0 (the
 *     v2.00 targets, inside L30's label D_00220000, each kept in upstream's
 *     own prefix style);
 *   - the file's `#include` lines and other declarations dropped; its
 *     L30Pair2f typedef and upstream's comment (folded into this one) kept.
 */
void func_L30_00384120(void) {
    int i;
    int j;

    memset(D_002338E0, 0, 0x1B00);
    for (i = 0; i < 24; i++) {
        L30Pair2f *row = (L30Pair2f *)(D_L30_002353E0 + i * 0xC0);

        for (j = 0; j < 24; j++) {
            row[j].x = i * (1.0f / 6.0f) + -2.0f;
            row[j].y = j * (1.0f / 6.0f) + -2.0f;
        }
    }
}
