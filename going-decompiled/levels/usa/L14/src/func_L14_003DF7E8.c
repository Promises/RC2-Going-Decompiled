/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x003DF7E8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm Sony 2.9-ee-991111
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

/* SPDX-License-Identifier: MIT
 * MIT License
 *
 * Copyright (c) 2026 Forja contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Upstream include/common.h's own definitions of the three typedefs the body
 * uses (their `#include "common.h"` is not carried; nothing else of it is
 * used here). */
typedef signed char        s8;
typedef signed int         s32;
typedef float              f32;

/**
 * func_L14_003DF7E8 - clear the state pointers of a group's four slots whose
 * owner is dead or spent.
 *
 * @arg0: the object; its state block pointer is at +0x68.
 * Return: nothing.
 *
 * Non-obvious: the block's four slots at +0xE0 are walked backwards
 * (var_a1 = 3 down to 0). A non-null slot pointer is cleared when its record's
 * stage byte (+0x20) is negative, or when the record's own state block's
 * float at +0x40 has run out (<= 0.0). The rc2 spelling (do-while with the
 * goto) is upstream's own text.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, MIT (the file's own notice above,
 * "Forja contributors"; kept verbatim per RULING #12108; `grep -ic lombyte`
 * on it = 0, so not Lombyte-origin), file
 * src/levels/14_aranos_prison/L14_003DE310-rc2.c, function func_L14_003DE310
 * (v1.01 address), author commit fe9859d7 (Sergio Rodmauser; the file's
 * import commit; the definition line blames fe9859d7 too). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L14_003DE310 -> func_L14_003DF7E8 (our split name at the
 *     v2.00 place);
 *   - the typedefs s8/s32/f32 written out with upstream include/common.h's
 *     own definitions (the body uses them; `#include "common.h"` dropped);
 *   - no externs: the body calls nothing.
 */
void func_L14_003DF7E8(s8 *arg0) {
    s32 var_a1;
    s8 **var_a0;
    s8 *temp_v1;
    s8 *t2;

    var_a1 = 3;
    var_a0 = (s8 **)((*(s8 **)((char *)(arg0) + (0x68))) + 0xE0);
    do {
        temp_v1 = *var_a0;
        var_a1 -= 1;
        if (temp_v1 != 0) {
            t2 = (*(s8 **)((char *)(temp_v1) + (0x68)));
            if ((*(s8 *)((char *)(temp_v1) + (0x20))) < 0) {
                goto store;
            }
            if ((*(f32 *)((char *)(t2) + (0x40))) <= 0.0f) {
            store:
                *var_a0 = 0;
            }
        }
        var_a0 = var_a0 + 1;
    } while (var_a1 >= 0);
}
