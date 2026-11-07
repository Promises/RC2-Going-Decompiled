/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2563, RULING #13291 row L14-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x00424560, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* This level's own functions, at their v2.00 places. */
extern void func_L14_004245C0();
extern void func_L14_00424800();
extern void func_L14_00424918();
extern void func_L14_004250F0();
extern void func_L14_00425198();

/**
 * func_L14_00424560 - per-frame update of an animated prison object.
 *
 * @a0: the object.
 *
 * While its byte +0x20 is 0 it only runs the set-up step
 * (func_L14_004245C0); afterwards it runs func_L14_00424800,
 * func_L14_00424918, func_L14_004250F0 and func_L14_00425198 in turn.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_003F4F98.c,
 * function func_L14_00422FA0 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED in the audit (code 1l); as carried, VERBATIM but for names; edits:
 *   - name func_L14_00422FA0 -> func_L14_00424560 (our split name at
 *     the v2.00 place);
 *   - func_423000 -> func_L14_004245C0, func_423240 ->
 *     func_L14_00424800, func_423358 -> func_L14_00424918,
 *     func_423B30 -> func_L14_004250F0, func_423BD8 ->
 *     func_L14_00425198 (v2.00 places);
 *   - the audit's `long long`->`long` respelling (RULING #13197) is
 *     needed only to compile the upstream FILE whole on s136; no line
 *     carried into this TU contains `long long` or `long`, so it
 *     applies to nothing here;
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L14_00424560(char *a0)
{
    if (*(unsigned char *)(a0 + 0x20) == 0) {
        func_L14_004245C0(a0);
    } else {
        func_L14_00424800(a0);
        func_L14_00424918(a0);
        func_L14_004250F0(a0);
        func_L14_00425198(a0);
    }
}
