/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2563, RULING #13291 row L14-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003DF838, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L14's copies of boot helpers, at their v2.00 places. */
extern void func_L14_002F6A20();
extern void func_L14_002F6A50();
extern void func_L14_002F6D00(void *dst, void *src, float s);
extern void func_L14_002F6B50(void *dst, void *src, void *mtx);
extern void func_L14_00329638(void *dst, void *src, void *mtx, float f);
extern float func_L14_00326270(float a, float b, float c);

/**
 * func_L14_003DF838 - turn an object about its group's frame and rebuild its
 * matrix.
 *
 * @self: the object; its state block is at +0x68.
 * @f:    the angle step.
 *
 * Takes the object's position relative to the group's matrix at state +0xB0
 * into a buffer, turns it by f (func_L14_00329638), adds f to the angle at
 * state +0x80 and scales the buffer by func_L14_00326270(+0x84, +0x88,
 * angle * 2/pi). The object's position, its two axes (+0xD0, +0xE0) and the
 * matrix at +0xC0 are rebuilt from it, and the three floats +0xCC, +0xDC and
 * +0xEC cleared. The first call is given the buffer, not the object's own
 * matrix, as the ROM's code does.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190.c,
 * function func_L14_003DE360 (v1.01 address), author commit 913bd1bc (Gianni).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L14_003DE360 -> func_L14_003DF838 (our split name at
 *     the v2.00 place);
 *   - func_2F5C68 -> func_L14_002F6A50, func_328340 ->
 *     func_L14_00329638, func_2F5F18 -> func_L14_002F6D00,
 *     func_324F78 -> func_L14_00326270, func_2F5C38 ->
 *     func_L14_002F6A20, func_2F5D68 -> func_L14_002F6B50 (L14's
 *     copies);
 *   - the file's `#include` lines, other functions and their externs
 *     not carried (nothing here uses them).
 */
void func_L14_003DF838(char *self, float f) {
    char buf[16];
    char *s0 = *(char **)(self + 0x68);
    char *mB0 = s0 + 0xB0;
    char *mC0 = s0 + 0xC0;
    char *m10 = self + 0x10;
    char *mD0 = self + 0xD0;
    char *mE0 = self + 0xE0;
    float f0;
    float zero;

    func_L14_002F6A50(buf, m10, mB0);
    func_L14_00329638(buf, buf, mC0, f);
    f0 = *(float *)(s0 + 0x80) + f;
    *(float *)(s0 + 0x80) = f0;
    func_L14_002F6D00(buf, buf,
                func_L14_00326270(*(float *)(s0 + 0x84), *(float *)(s0 + 0x88), f0 * 0.63661977f));
    func_L14_002F6A20(m10, mB0, buf);
    func_L14_002F6D00(mD0, mC0, 1.0f);
    func_L14_002F6D00(mE0, buf, -1.0f);
    func_L14_002F6B50(self + 0xC0, mE0, mD0);
    zero = 0.0f;
    *(float *)(self + 0xEC) = zero;
    *(float *)(self + 0xDC) = zero;
    *(float *)(self + 0xCC) = zero;
}
