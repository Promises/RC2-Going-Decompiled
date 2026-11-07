/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2563, RULING #13291 row L14-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003E0B68, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* This level's own functions, at their v2.00 places. */
extern void func_L14_003E01F8();
extern void func_L14_003E0BB8();

/**
 * func_L14_003E0B68 - install an object's update routine.
 *
 * @arg0: the object.
 *
 * While its byte +0x20 is 0 it first runs func_L14_003E01F8; then it stores
 * func_L14_003E0BB8's address in the object's update word +0x64.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c,
 * function func_L14_003DF650 (v1.01 address), author commit db93b56d (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L14_003DF650 -> func_L14_003E0B68 (our split name at
 *     the v2.00 place);
 *   - func_3DECE0 -> func_L14_003E01F8 and func_3DF6A0 ->
 *     func_L14_003E0BB8 (v2.00 places);
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L14_003E0B68(unsigned char *arg0) {
    if (arg0[0x20] == 0) {
        func_L14_003E01F8();
    }
    *(int *)(arg0 + 0x64) = (int)func_L14_003E0BB8;
}
