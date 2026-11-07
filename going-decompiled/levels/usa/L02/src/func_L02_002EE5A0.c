/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x002EE5A0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G0.
 */

typedef struct {
    float x, y, z, w;
    float pad[4];
} L02_Record20;

typedef struct {
    float x, y, z, w;
    float pad[12];
} L02_Record40;

/* The four table words sit inside the larger split label D_L02_001B3930, so
 * they are address-named externs valued from their names (RULING #13291 §2).
 * ADDRESSING-MODEL DEVICE (RULING #8620, EE arm only; this TU has no other
 * arm): the .sdata section on these extern declarations makes cc1 print the
 * one-instruction `lw` macro, which GNU as expands absolute at -G0
 * (`lui $r` / `lw $r,%lo($r)`, one register), as retail has it. Without it
 * cc1 splits each load into two registers and interleaves them. It moves no
 * data and emits nothing.
 */
extern L02_Record20 *D_L02_001B3C14 __attribute__((section(".sdata")));
extern L02_Record20 *D_L02_001B3C18 __attribute__((section(".sdata")));
extern L02_Record40 *D_L02_001B3BD0 __attribute__((section(".sdata")));
extern int D_L02_001B3BD4 __attribute__((section(".sdata")));

/**
 * func_L02_002EE5A0 - shift every record of two tables by one period, toward
 * the side of the limit the flag selects.
 *
 * The first table runs from D_L02_001B3C14 to D_L02_001B3C18 (0x20-byte
 * records). A record whose z is below @limit takes @flag as its side, the
 * others take !@flag. On the selected side x is moved down by 2000 while it
 * is positive; on the other side it is moved up by 2000 while it is negative.
 * The second table, D_L02_001B3BD0 with D_L02_001B3BD4 entries (0x40-byte
 * records), does the same against @limit * 1024 with a period of 2048000.
 *
 * @flag:  which side of the limit counts as "selected".
 * @limit: the z threshold for the first table (scaled by 1024 for the second).
 *
 * Each record moves by at most one period per call: the tests are `if`, not
 * `while`.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_002ED7B0.c, function func_L02_002ED7B0
 * (v1.01 address), author commit 6e138d9a (bigmak94). Lombyte mentions in
 * the file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L02_002ED7B0 -> func_L02_002EE5A0 (our split name at the
 *     v2.00 place);
 *   - the four externs keep their upstream names, which already state their
 *     v2.00 addresses; they are address-named (RULING #13291 §2);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     four extern declarations (RULING #8620; upstream's `sda` is a marker
 *     for its own patched compiler, which our cc1 ignores);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped (nothing
 *     here used them), every other declaration of the file dropped, and the
 *     upstream comments folded into this one.
 */
void func_L02_002EE5A0(int flag, float limit) {
    L02_Record20 *p;
    int i;

    for (p = D_L02_001B3C14; p != D_L02_001B3C18; p++) {
        int sel = p->z < limit ? flag : !flag;

        if (sel) {
            if (p->x > 0.0f)
                p->x -= 2000.0f;
        } else {
            if (p->x < 0.0f)
                p->x += 2000.0f;
        }
    }
    limit *= 1024.0f;
    for (i = 0; i < D_L02_001B3BD4; i++) {
        L02_Record40 *q = &D_L02_001B3BD0[i];
        int sel = q->z < limit ? flag : !flag;

        if (sel) {
            if (q->x > 0.0f)
                q->x -= 2048000.0f;
        } else {
            if (q->x < 0.0f)
                q->x += 2048000.0f;
        }
    }
}
