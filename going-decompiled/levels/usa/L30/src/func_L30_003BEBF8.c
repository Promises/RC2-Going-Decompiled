/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x003BEBF8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/* L30's functions at their v2.00 places. */
extern void func_L30_00321DA8(char *, int, int, int);
extern void func_L30_0042ECC8(int);

/**
 * func_L30_003BEBF8 - step the actor's open/close animation state.
 * @a0: the actor; its state block is at +0x68.
 *
 * In state 1 (byte +0x20) it starts animation 1 (func_L30_00321DA8 with
 * blend 5) unless already playing (byte +0x43) and moves to state 2. In any
 * other state it starts animation 2 the same way, moves to state 5 and, when
 * state +0xC holds a handle (not -1), passes it to func_L30_0042ECC8.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/30_insomniac_museum/L30_0032AAF0.c, function
 * func_L30_003BE678 (v1.01 address), author commit 91e04732 (llesieur99;
 * `git blame -M -C` gives all 18 definition lines to it, `git log -S`
 * agrees). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names, each name to the target of the
 * v2.00 disc's own relocation at this place (task #2673 relverify by name):
 *   - func_L30_003BE678 -> func_L30_003BEBF8 (our split name);
 *   - func_002A7470 -> func_L30_00321DA8, func_L00_0042A500 ->
 *     func_L30_0042ECC8 (L30's copies at their v2.00 places);
 *   - the file's `#include` lines and other declarations dropped.
 */
void func_L30_003BEBF8(char *a0) {
    char *st = *(char **)(a0 + 0x68);

    if (*(unsigned char *)(a0 + 0x20) == 1) {
        if (*(unsigned char *)(a0 + 0x43) != 1) {
            func_L30_00321DA8(a0, 1, 0, 5);
        }
        *(unsigned char *)(a0 + 0x20) = 2;
    } else {
        if (*(unsigned char *)(a0 + 0x43) != 2) {
            func_L30_00321DA8(a0, 2, 0, 5);
        }
        *(unsigned char *)(a0 + 0x20) = 5;
        if (*(int *)(st + 0xC) != -1) {
            func_L30_0042ECC8(*(int *)(st + 0xC));
        }
    }
}
