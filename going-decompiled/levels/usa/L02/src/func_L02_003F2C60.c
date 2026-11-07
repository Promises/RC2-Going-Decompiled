/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x003F2C60, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* func_L02_003FD670 (row L02-C too) is defined as (char *, s32); this unit
 * keeps upstream's own declaration of it, (int, short), unchanged. */
extern int func_L02_003FD670(int, short);
/* L02's copy of a boot helper. */
extern int func_L02_003309B0(char *, int, float, float, int, int);

/**
 * func_L02_003F2C60 - keep the actor's claimed timer slot and its effect alive.
 *
 * @arg0: the actor; its state block is at +0x68.
 *
 * Below state 4 (byte +0x20) it clears the word at state +0x4B0 and the
 * claimed flag at +0x4EA. Already claimed (+0x4EA == 1), it re-runs
 * func_L02_003309B0(arg0, state +0x4E4, 100, 100, 0, 0) into +0x4E8 and copies
 * +0x210 to +0x4B0. Not claimed, it asks func_L02_003FD670 for one of the
 * timers named by the actor's class (+0xAA) on the object at +0xB8; if it gets
 * one it sets the flag and does the same, else it clears the flag.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003DB310.c,
 * function func_L02_003F16F0 (v1.01 address), author commit e80a585a (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_003F16F0 -> func_L02_003F2C60 (our split name at
 *     the v2.00 place);
 *   - func_L02_003FC0D0 -> func_L02_003FD670 and func_002B3BA8 ->
 *     func_L02_003309B0 (L02's v2.00 places);
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L02_003F2C60(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(unsigned char *)(arg0 + 0x20) < 4) {
        *(int *)(state + 0x4B0) = 0;
        goto done;
    }

    if (*(short *)(state + 0x4EA) == 1) {
        *(short *)(state + 0x4E8) =
            func_L02_003309B0(arg0, *(int *)(state + 0x4E4), 100.0f, 100.0f, 0, 0);
        *(int *)(state + 0x4B0) = *(int *)(state + 0x210);
    } else {
        if (func_L02_003FD670(*(int *)(arg0 + 0xB8), *(short *)(arg0 + 0xAA)) == 0)
            goto done;
        *(short *)(state + 0x4EA) = 1;
        *(short *)(state + 0x4E8) =
            func_L02_003309B0(arg0, *(int *)(state + 0x4E4), 100.0f, 100.0f, 0, 0);
        *(int *)(state + 0x4B0) = *(int *)(state + 0x210);
    }
    return;

done:
    *(short *)(state + 0x4EA) = 0;
}
