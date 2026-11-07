/* Level 03 (Endako), USA v2.00: a level-only function built from C
 * (task #2655, RULING #13291 row L03-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x0032E408, in place of its split asm; the row is
 * going-decompiled/levels/usa/L03/config/L03_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L03's copies of boot and level-00/02 helpers, at their v2.00 places. */
extern void *func_L03_0032F168(char *a0);
extern float func_L03_002F04D0(float a, float b);
extern float func_L03_002F0E60(float a, float b);
extern int func_L03_002EFA80(short *p);
extern int func_L03_0031CDA8(int a0, int a1);
/* Upstream: the callee's parameters are listed in the order the retail code
 * sets the registers up in: int, int, float, int, float, float. */
extern int func_L03_0032E328(char *a0, char *a1, float f12, char *a2, float f13, float f14);

/**
 * func_L03_0032E408 - aim the record at the shape the caller found.
 *
 * @self:   the record; its position is at +0x10/+0x14.
 * @target: the point to aim at (x, y).
 * @third:  passed through to func_L03_0032E328.
 * @amount: an angle added to the heading.
 * @f13, @f14: passed through to func_L03_0032E328.
 *
 * Fetches the record's data block (func_L03_0032F168). The heading is
 * func_L03_002F04D0 of the target's offset from the record, plus @amount
 * (func_L03_002F0E60). A non-zero wobble at data+0xDC is added too, and when
 * the timer at data+0xE0 runs out (func_L03_002EFA80) the wobble's sign flips
 * and the timer is reloaded with func_L03_0031CDA8(data+0xE2, data+0xE4).
 * Return: 8 when the record has no data block, else func_L03_0032E328's
 * result for the final heading.
 *
 * Non-obvious: @target is passed as func_L03_0032E328's `char *` argument
 * without a cast, so cc1 warns "incompatible pointer type"; that is
 * upstream's own statement, kept.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/03_endako/L03_002CCB70.c, function
 * func_L03_0032CFD8 (v1.01 address), author commit a6fb1b20 (Flavius;
 * `git blame -M -C` names it for all 19 definition lines at fe6adf9b, and
 * `git log --reverse -S'func_L03_0032CFD8('` agrees). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L03_0032CFD8 -> func_L03_0032E408 (our split name at the
 *     v2.00 place);
 *   - six callees as L03's copies, the targets of the v2.00 ROM's own
 *     relocations: func_L00_0032C638 -> func_L03_0032F168, func_00283248 ->
 *     func_L03_002F04D0, func_00283B98 -> func_L03_002F0E60, func_00282978 ->
 *     func_L03_002EFA80, func_002A7820 -> func_L03_0031CDA8,
 *     func_L02_00331440 -> func_L03_0032E328;
 *   - the file's `#include` and other functions and externs not carried.
 */
int func_L03_0032E408(char *self, float *target, char *third, float amount, float f13, float f14)
{
    char *data = (char *)func_L03_0032F168(self);
    float angle;

    if (data == 0)
        return 8;
    angle = func_L03_002F04D0(target[0] - *(float *)(self + 0x10),
                         target[1] - *(float *)(self + 0x14));
    angle = func_L03_002F0E60(angle, amount);
    if (*(float *)(data + 0xDC) != 0.0f) {
        angle = func_L03_002F0E60(angle, *(float *)(data + 0xDC));
        if (func_L03_002EFA80((short *)(data + 0xE0))) {
            *(float *)(data + 0xDC) = -*(float *)(data + 0xDC);
            *(short *)(data + 0xE0) = func_L03_0031CDA8(*(short *)(data + 0xE2), *(short *)(data + 0xE4));
        }
    }
    return func_L03_0032E328(self, target, angle, third, f13, f14);
}
