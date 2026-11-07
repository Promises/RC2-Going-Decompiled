/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2563, RULING #13291 row L14-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003E0618, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack.
 */

/* L14's copies of boot vector helpers, at their v2.00 places. */
extern void func_L14_002F6AA8(void *dst, void *src, float s);

extern void func_L14_002F6A20(void *dst, void *src, void *src2);

/* This level's graph-link helper (row L14-A too). */
extern void func_L14_004220D0();

/**
 * func_L14_003E0618 - mark the graph links in front of and behind an object.
 *
 * @arg0: the object; its state block is at +0x68, its axis at +0xC0 and its
 *        position at +0x10.
 * @arg1: 0 clears the front link, non-zero sets it.
 *
 * Builds two points two units ahead of and behind the object along its axis
 * (swapped when state +0x7C is set) and calls func_L14_004220D0 for the
 * entity at state +0x64 twice: front point with clear = (arg1 == 0), back
 * point with clear = 0.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190-g8.c,
 * function func_L14_003DF100 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L14_003DF100 -> func_L14_003E0618 (our split name at
 *     the v2.00 place);
 *   - func_2F5CC0 -> func_L14_002F6AA8, func_2F5C38 ->
 *     func_L14_002F6A20, func_420B10 -> func_L14_004220D0 (L14's
 *     copies / v2.00 place);
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L14_003E0618(char *arg0, int arg1) {
    float a[4];
    float b[4];
    char *v;
    char *p;

    v = *(char **)(arg0 + 0x68);
    func_L14_002F6AA8(a, arg0 + 0xC0, 2.0f);
    func_L14_002F6AA8(b, arg0 + 0xC0, -2.0f);
    if (*(int *)(v + 0x7C) != 0) {
        func_L14_002F6AA8(a, a, -1.0f);
        func_L14_002F6AA8(b, b, -1.0f);
    }
    p = arg0 + 0x10;
    func_L14_002F6A20(a, a, p);
    func_L14_002F6A20(b, b, p);
    func_L14_004220D0(*(int *)(v + 0x64), a, p, arg1 == 0);
    func_L14_004220D0(*(int *)(v + 0x64), b, p, 0);
}
