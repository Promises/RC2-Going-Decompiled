/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x003F30E0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L02's copy of a boot state-change helper. */
extern void func_L02_0031F998(char *, int, int);

/**
 * func_L02_003F30E0 - remember the actor's state and enter state 6.
 *
 * @arg0: the actor; its state block is at +0x68.
 *
 * Saves the actor's byte +0x20 and byte +0x95 as halfwords at state +0x4EC
 * and +0x4EE, then calls func_L02_0031F998(arg0, 6, 4).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003DB310.c,
 * function func_L02_003F1B70 (v1.01 address), author commit 773eafc2 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_003F1B70 -> func_L02_003F30E0 (our split name at
 *     the v2.00 place);
 *   - func_002A7750 -> func_L02_0031F998 (L02's copy);
 *   - the file's second, unprototyped `extern void func_002A7750();`
 *     not carried (the prototype it composes with is);
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L02_003F30E0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    *(short *)(state + 0x4EC) = *(unsigned char *)(arg0 + 0x20);
    *(short *)(state + 0x4EE) = *(unsigned char *)(arg0 + 0x95);
    func_L02_0031F998(arg0, 6, 4);
}
