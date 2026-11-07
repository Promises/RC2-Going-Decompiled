/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x003FCFF8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef int s32;

extern s32 func_L02_003D8D60(char *);
extern void func_L02_003D8E10(char *);

/**
 * func_L02_003FCFF8 - release every taken slot of the actor's list.
 *
 * @a0: the actor; its state block is at +0x68.
 *
 * For each slot 0..(state +0xD8) of the list at state +0xA8 that holds an
 * entry, calls func_L02_003D8E10 on it when func_L02_003D8D60 says so.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003F5430.c,
 * function func_L02_003FBA58 (v1.01 address), author commit adcbd51e (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_003FBA58 -> func_L02_003FCFF8 (our split name at
 *     the v2.00 place);
 *   - func_003D77F8 -> func_L02_003D8D60 and func_003D78A8 ->
 *     func_L02_003D8E10 (v2.00 places);
 *   - s32 typedef carried from the file's own;
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L02_003FCFF8(char *a0) {
    char **p;
    char *s1 = *(char **)(a0 + 0x68);
    s32 i;
    for (i = 0; i <= *(s32 *)(s1 + 0xD8); i++) {
        p = (char **)(s1 + 0xA8 + i * 4);
        if (*p != 0) {
            if (func_L02_003D8D60(*p) != 0)
                func_L02_003D8E10(*p);
        }
    }
}
