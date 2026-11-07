/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x003FD658, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm the SDK 2.9 cc1
 * (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

typedef int s32;

/**
 * func_L02_003FD658 - read the actor's state word at +0x84.
 *
 * @a0: the actor; its state block is at +0x68.
 *
 * Return: the state's word at +0x84.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003F5430.c,
 * function func_L02_003FC0B8 (v1.01 address), author commit adcbd51e (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_003FC0B8 -> func_L02_003FD658 (our split name at
 *     the v2.00 place);
 *   - s32 typedef carried from the file's own;
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
s32 func_L02_003FD658(char *a0) {
    char *v1 = *(char **)(a0 + 0x68);
    return *(s32 *)(v1 + 0x84);
}
