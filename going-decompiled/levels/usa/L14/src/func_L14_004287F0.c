/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x004287F0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm Sony 2.9-ee-991111
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

/* SPDX-License-Identifier: MIT
 * MIT License
 *
 * Copyright (c) 2026 RC2 decomp contributions
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
typedef unsigned char      u8;
typedef signed char        s8;
typedef signed int         s32;

/**
 * func_L14_004287F0 - should this entity be skipped by the lift's guard
 * logic?
 *
 * @arg0: the entity (may be null).
 * Return: 1 when arg0 is null, its stage byte is 0xFE or 0xFD, or its state
 *         block's word at +0x04 is set; else 0.
 *
 * Non-obvious: the comma expression inside the condition is upstream's own
 * rc2 spelling of the short-circuit chain: the stage byte is loaded only
 * after the null check, and the +0x04 test only after both stage tests fail.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, MIT (the file's own notice above,
 * "RC2 decomp contributions"; kept verbatim per RULING #12108; `grep -ic
 * lombyte` on it = 0, so not Lombyte-origin), file
 * src/levels/14_aranos_prison/L14_00427210-rc2.c, function func_L14_00427210
 * (v1.01 address), author commit fe9859d7 (Sergio Rodmauser; the file's
 * import commit; the definition line blames fe9859d7 too). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L14_00427210 -> func_L14_004287F0 (our split name at the
 *     v2.00 place);
 *   - the typedefs u8/s8/s32 written out with upstream include/common.h's
 *     own definitions (the body uses them; `#include "common.h"` dropped);
 *   - no externs: the body calls nothing.
 */
s32 func_L14_004287F0(s8 *arg0) {
    s32 var_a1;
    s8 *temp_v1;
    u8 temp_a0;

    var_a1 = 0;
    temp_v1 = (*(s8 **)((s8 *)(arg0) + (0x68)));
    if ((arg0 == 0) || (temp_a0 = (*(u8 *)((s8 *)(arg0) + (0x20))), (temp_a0 == 0xFE)) || (temp_a0 == 0xFD) || ((*(s32 *)((s8 *)(temp_v1) + (4))) != 0)) {
        var_a1 = 1;
    }
    return var_a1;
}
