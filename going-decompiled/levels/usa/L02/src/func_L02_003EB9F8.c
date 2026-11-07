/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * (task #2559, RULING #13291 row L02-C). Compiled ALONE as its own TU and
 * linked at its one place, 0x003EB9F8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L02's copy of a boot state-change helper, at its v2.00 place in this level. */
extern void func_L02_0031F998(char *, int, int);

/**
 * func_L02_003EB9F8 - re-arm the actor and show it again.
 *
 * @arg0: the actor; its state block is at +0x68.
 *
 * Only when the actor's byte +0x20 is 0: sets bit 2 of the state's word at
 * +0x5C, changes state through func_L02_0031F998(arg0, 1, -1), clears the
 * word at +0x98 and rewrites the flags halfword at +0x34 as
 * (flags & 0xEFFF) | 0x43.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/02_maktar_nebula/L02_003DB310.c,
 * function func_L02_003EA488 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L02_003EA488 -> func_L02_003EB9F8 (our split name at
 *     the v2.00 place);
 *   - func_31E6A0 -> func_L02_0031F998 (L02's copy of the boot's
 *     func_002A7750, the target of the v2.00 relocation);
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L02_003EB9F8(char *arg0) {
    char *p = *(char **)(arg0 + 0x68);

    if (*(unsigned char *)(arg0 + 0x20) != 0)
        return;
    *(int *)(p + 0x5C) |= 4;
    func_L02_0031F998(arg0, 1, -1);
    *(int *)(arg0 + 0x98) = 0;
    *(unsigned short *)(arg0 + 0x34) =
        (*(unsigned short *)(arg0 + 0x34) & 0xEFFF) | 0x43;
}
