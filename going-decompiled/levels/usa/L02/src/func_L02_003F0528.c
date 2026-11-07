/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x003F0528, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef int TI128 __attribute__((mode(TI)));
/* The level's path table, a word inside L02's larger .bss label D_L02_001F0000,
 * so address-named (RULING #13291 §2). */
extern char *D_L02_001FF1C0[];
/* The hero's position (boot core, below 0x1A7C80). */
extern char g_heroPos[];
/* L02's copy of a boot distance helper (unprototyped, as upstream declares it). */
extern float func_L02_002F2F18();
extern int func_L02_003497D0(void *, char *);

/**
 * func_L02_003F0528 - test whether the hero is in reach of the actor and of one
 * of the points of its path.
 *
 * @arg0: the actor; its state block is at +0x68.
 *
 * Returns 0 when the hero is farther than the radius at state +0x2C from the
 * state's position (+0x20). Otherwise it walks the 16-byte points of the
 * state's D_L02_001FF1C0 entry, taking every (state +0x40)-th one; at the first
 * point within the radius it copies the hero's position as one 128-bit value
 * to a local, stores func_L02_003497D0(&copy, entry) at state +0x30 and
 * returns 1.
 * Return: 1 when a point was found, else 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003DB310.c,
 * function func_L02_003EEFB8 (v1.01 address), author commit 246b78db (bigmak94).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_003EEFB8 -> func_L02_003F0528 (our split name at
 *     the v2.00 place);
 *   - D_00189EA0 -> g_heroPos (our name at 0x00189EA0);
 *   - func_00282E48 -> func_L02_002F2F18 and func_L02_00348420 ->
 *     func_L02_003497D0 (L02's v2.00 places);
 *   - D_L02_001FF1C0 unchanged (same address in v2.00, kept address-
 *     named, RULING #13291 §2);
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
int func_L02_003F0528(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *entry = D_L02_001FF1C0[*(int *)state];
    TI128 buf;
    int i;

    if (*(float *)(state + 0x2C) < func_L02_002F2F18(g_heroPos, state + 0x20))
        return 0;
    for (i = 0; i < *(int *)entry; i += *(int *)(state + 0x40)) {
        if (func_L02_002F2F18(g_heroPos, entry + (i * 16 + 0x10)) < *(float *)(state + 0x2C)) {
            buf = *(TI128 *)g_heroPos;
            *(int *)(state + 0x30) = func_L02_003497D0(&buf, entry);
            return 1;
        }
    }
    return 0;
}
