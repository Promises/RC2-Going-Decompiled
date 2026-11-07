/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x003F2AE8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L02's copies of boot helpers, at their v2.00 places in this level. */
extern void func_L02_0032C1D0(char *, char *, float, int, int);
extern void func_L02_003214D8(char *, char *, int, float, float, float, int, int, int);

/**
 * func_L02_003F2AE8 - per-frame update of a simple actor.
 *
 * @arg0: the actor; its state block is at +0x68.
 *
 * Bumps the halfword frame count at +0x96 (held at 0xFFFF), runs
 * func_L02_0032C1D0(arg0, state + 0x220, 2.5, 7, 1), then
 * func_L02_003214D8(arg0, arg0 + 0x10, 0x4000000, 1.3, 1.0, 1.0, 0, 1, 0).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003DB310.c,
 * function func_L02_003F1578 (v1.01 address), author commit adcbd51e (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_003F1578 -> func_L02_003F2AE8 (our split name at
 *     the v2.00 place);
 *   - func_002B0990 -> func_L02_0032C1D0 and func_002A8C28 ->
 *     func_L02_003214D8 (L02's copies);
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L02_003F2AE8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(unsigned short *)(arg0 + 0x96) < 0xFFFF)
        (*(unsigned short *)(arg0 + 0x96))++;
    func_L02_0032C1D0(arg0, state + 0x220, 2.5f, 7, 1);
    func_L02_003214D8(arg0, arg0 + 0x10, 0x4000000, 1.3f, 1.0f, 1.0f, 0, 1, 0);
}
