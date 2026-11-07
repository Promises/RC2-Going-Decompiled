/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x003E0F28, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

extern void func_L14_002F6A20();
extern void func_L14_00421F60();

/**
 * func_L14_003E0F28 - turn a world point into the object's frame and hand it
 * to the object's class handler.
 *
 * @arg0: the object; its state block pointer is at +0x68, its matrix +0xC0
 *        and its position +0x10.
 * @arg1: a world point, passed on unchanged.
 * Return: nothing.
 *
 * Non-obvious: the point is first expressed in the object's frame
 * (func_L14_002F6A20 with the matrix and position as upstream spells it),
 * then dispatched with the state block's first word and that local point
 * (func_L14_00421F60, the level's copy of a shared class handler).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c, function
 * func_L14_003DFA10 (v1.01 address), author commit adcbd51e (llesieur99; the
 * definition line blames adcbd51e too). Adopted with the authors' permission
 * (RULINGs #10405, #12108, #13082, #13291). BYTE-IDENTICAL: VERBATIM but for
 * names; edits:
 *   - name func_L14_003DFA10 -> func_L14_003E0F28 (our split name at the
 *     v2.00 place);
 *   - func_2F5C38 -> func_L14_002F6A20 (the level's own copy of boot
 *     func_00282CC0 at its L14 place);
 *   - func_4209A0 -> func_L14_00421F60 (the level's own copy of shared
 *     func_L03_00419A60 at its L14 place);
 *   - the file's `#include` lines, other functions and their externs not
 *     carried (nothing else here uses them); upstream's unprototyped
 *     `extern void func_2F5C38();` / `extern void func_4209A0();` forms kept.
 */
void func_L14_003E0F28(char *arg0, char *arg1) {
    char buf[16];
    char *v = *(char **)(arg0 + 0x68);

    func_L14_002F6A20(buf, arg0 + 0xC0, arg0 + 0x10);
    func_L14_00421F60(*(int *)(v + 0x0), buf, arg1);
}
