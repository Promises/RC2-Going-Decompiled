/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2563, RULING #13291 row L14-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x0032A188, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L14's copies of boot helpers, at their v2.00 places. */
extern char *func_L14_00329A60();
extern void func_L14_0032A0A0();
extern void func_L14_002F6A50();

/**
 * func_L14_0032A188 - push an object's attachment along by a vector.
 *
 * @arg0: the object; its position is at +0x10.
 * @arg1: the vector.
 * @arg2: passed on.
 *
 * When func_L14_00329A60 finds the object's attachment, the vector is taken
 * relative to the object's position (func_L14_002F6A50) and handed with arg2
 * and the object's +0xF0 to func_L14_0032A0A0.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c,
 * function func_L14_00328E90 (v1.01 address), author commit db93b56d (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L14_00328E90 -> func_L14_0032A188 (our split name at
 *     the v2.00 place);
 *   - func_328768 -> func_L14_00329A60, func_2F5C68 ->
 *     func_L14_002F6A50, func_328DA8 -> func_L14_0032A0A0 (L14's
 *     copies);
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L14_0032A188(char *arg0, char *arg1, char *arg2) {
    char buf[16];
    char *s1 = func_L14_00329A60(arg0);

    if (s1 != 0) {
        func_L14_002F6A50(buf, arg0 + 0x10, arg1);
        func_L14_0032A0A0(s1, buf, arg2, arg0 + 0xF0);
    }
}
