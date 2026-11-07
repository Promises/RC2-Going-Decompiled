/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x004566D0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L11's own function and its copies of boot code, at their v2.00 places. */
extern void func_L11_0032DBF8(void *a0, int a1, int a2);
extern void func_L11_00325678(void *a0, int a1, int a2, int a3);
extern float func_L11_0032DD60(float a, float b);
extern void func_L11_0032D918(void *a0, int a1, int a2, int a3);
extern void func_L11_003011A8(void *);
extern void func_L11_00398770(char *, int, float, float);

/**
 * func_L11_004566D0 - aim the actor's stage, re-arm its effect and store the
 * new heading.
 *
 * @p:  the actor; its state block q is at +0x68.
 * @a1: the heading id, stored at q+0x1EE.
 * @a2: the mode, stored at q+0x1FC; nonzero picks the 7/3 state with
 *      func_L11_00398770(p, 0, 2.0f, 2.0f), zero the 3/1 state.
 *
 * It seeds q+0x20 from the halfword q+0x24, clears q+0x1EC, sets q+0x2E,
 * drops the record state with func_L11_0032D918(p, 0, 0, 10) when p+0x43 is
 * set, and rewrites the +0x34 flags and +0x98 (from the definition at +0x24).
 * After func_L11_003011A8(q + 0x1B0), q+0x1CC gets
 * func_L11_0032DD60(20.0f, 30.0f) times pi/180. When q+0x50 is set,
 * it clears it, calls func_L11_00325678 with q's three words +0x1F0..+0x1F8,
 * and sets q+0x52.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_00453358.c, function
 * func_L11_00454C78 (v1.01 address), author commit daf0e4a1 (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged (verbatim but for
 * names; no device); edits:
 *   - name func_L11_00454C78 -> func_L11_004566D0 (our split name at the
 *     v2.00 place);
 *   - func_002A7470 -> func_L11_0032D918, func_002A7750 -> func_L11_0032DBF8,
 *     func_L11_003973F8 -> func_L11_00398770, func_00282C88 ->
 *     func_L11_003011A8, func_002A7878 -> func_L11_0032DD60, func_002A0488 ->
 *     func_L11_00325678: the targets of the v2.00 ROM's own relocations
 *     (relverify, task #2553);
 *   - the audit's `long long` -> `long` respelling (RULING #13197 class,
 *     needed because cc1 s136 refuses the file's other `long long` uses) is
 *     NOT applied: this one-function TU carries no `long long`;
 *   - `#include "common.h"` dropped (no typedef of it is used here); the
 *     upstream one-line comment folded into this one; the file's other
 *     functions and their externs not carried.
 */
void func_L11_004566D0(char *p, int a1, int a2)
{
    char *q = *(char **)(p + 0x68);
    char *definition;
    unsigned short flags;
    int cache;
    float f;

    f = (float)*(short *)(q + 0x24);
    *(short *)(q + 0x1FC) = a2;
    *(short *)(q + 0x1EC) = 0;
    *(unsigned char *)(q + 0x2E) = 1;
    *(float *)(q + 0x20) = f;
    if (*(unsigned char *)(p + 0x43) != 0)
        func_L11_0032D918(p, 0, 0, 10);
    flags = *(unsigned short *)(p + 0x34);
    *(unsigned char *)(p + 0x7F) = 0;
    *(unsigned short *)(p + 0x34) = (flags | 0x1000) & 0xFFB9;
    definition = *(char **)(p + 0x24);
    cache = *(int *)(definition + 0x10);
    *(int *)(p + 0x98) = cache;
    if (*(short *)(q + 0x1FC) != 0) {
        func_L11_0032DBF8(p, 7, 3);
        *(unsigned short *)(p + 0x34) |= 0x100;
        func_L11_00398770(p, 0, 2.0f, 2.0f);
    } else {
        func_L11_0032DBF8(p, 3, 1);
        *(unsigned char *)(p + 0x31) = 1;
        *(unsigned short *)(p + 0x34) &= 0xFEFE;
    }
    func_L11_003011A8(q + 0x1B0);
    f = func_L11_0032DD60(20.0f, 30.0f) * 0.017453294f;
    *(short *)(q + 0x1EE) = a1;
    *(float *)(q + 0x1CC) = f;
    if (*(short *)(q + 0x50) != 0) {
        *(short *)(q + 0x50) = 0;
        func_L11_00325678(p, *(int *)(q + 0x1F0), *(int *)(q + 0x1F4), *(int *)(q + 0x1F8));
        *(short *)(q + 0x52) = 1;
    }
}
