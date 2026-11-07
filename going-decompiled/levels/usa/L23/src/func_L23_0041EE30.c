/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x0041EE30, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern char *D_L23_001B1434 __attribute__((sda));
extern void func_L23_0031B350(int);
extern void func_L23_0041BB48(char *);
extern void func_L23_0042C170(void);

/**
 * func_L23_0041EE30 - tear down the level object's handles.
 *
 * Runs func_L23_0041BB48 on the level object D_L23_001B1434, then frees
 * (func_L23_0031B350) and clears the handles at its state +0x620, +0x624 and
 * +0x614. Before freeing +0x624 it calls func_L23_0042C170, which releases
 * the two level sound handles.
 *
 * No parameters.
 * Return: nothing.
 *
 * Non-obvious: Upstream's `__attribute__((sda))` text on the extern
 * declarations is kept as written; this cc1 ignores the attribute.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/23_damosel_orbit/L23_00416388-g8.c, function func_L23_0041D930
 * (v1.01 address), author commit adcbd51e (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_L23_0041D930 -> func_L23_0041EE30; D_L23_001B1484 ->
 *     D_L23_001B1434; func_31A108 -> func_L23_0031B350; func_L23_0041A648 ->
 *     func_L23_0041BB48; func_L23_0042AC70 -> func_L23_0042C170
 *   - `#include "include_asm.h"` dropped; the typedefs it uses written in the
 *     TU (the file's own where it has them, else common.h's); only the
 *     file-level declarations it uses kept, without their comments
 *   - upstream's one-line comment folded into this one
 */
void func_L23_0041EE30(void) {
    char *state;

    func_L23_0041BB48(D_L23_001B1434);
    state = *(char **)(D_L23_001B1434 + 0x68);
    if (*(char **)(state + 0x620) != 0) {
        func_L23_0031B350((int)*(char **)(state + 0x620));
        *(char **)(state + 0x620) = 0;
    }
    if (*(char **)(state + 0x624) != 0) {
        func_L23_0042C170();
        func_L23_0031B350((int)*(char **)(state + 0x624));
        *(char **)(state + 0x624) = 0;
    }
    if (*(char **)(state + 0x614) != 0) {
        func_L23_0031B350((int)*(char **)(state + 0x614));
        *(char **)(state + 0x614) = 0;
    }
}
