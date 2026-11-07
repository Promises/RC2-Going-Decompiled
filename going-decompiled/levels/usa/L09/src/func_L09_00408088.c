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
/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (row L09-B,
 * task #2562, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x00408088, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef signed char s8;
typedef int s32;

/* The hero's position record: the boot's g_heroPos at 0x00189EA0 (upstream's
 * address-named extern spelled D_00189EA0). */
extern s8 g_heroPos[];
extern s32 func_L09_003270C8(s8 *, s32);
extern void func_L09_00408030(s32, s32);

/**
 * func_L09_00408088 - test the record's distance predicate and report it.
 *
 * With @arg0 == -1 nothing is tested and the report value is 0 (var_a1 keeps
 * its initial 0 through the s0 path). Otherwise func_L09_003270C8(g_heroPos,
 * @arg0) decides: a nonzero result latches var_s0 = 1 and leaves the report
 * value at 0; a zero result sets the report value to 2. Either way
 * func_L09_00408030(@arg1, report value) fires, and the return is the latch.
 *
 * @arg0: a record index into g_heroPos's table, or -1 for "none".
 * @arg1: the id the result is reported for.
 * Return: 1 when the predicate held on a real record, else 0.
 *
 * Non-obvious: var_a1's value in the taken path is the OLD var_s0 (0), not a
 * fresh constant -- upstream spells that dataflow `var_a1 = var_s0` before
 * `var_s0 = 1`, and the body keeps it verbatim. No codegen or addressing
 * device is used.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, MIT (the file's own notice,
 * carried verbatim above; `grep -ic lombyte` 0, so not Lombyte-origin), file
 * src/levels/09_dobbo/L09_00406BF8-rc2.c, function func_L09_00406BF8 (v1.01
 * address), author commit fe9859d7 (Sergio Rodmauser). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2562 relverify by name):
 *   - func_L09_00406BF8 -> func_L09_00408088 (our split name);
 *   - D_00189EA0 -> g_heroPos (the boot's name at 0x00189EA0);
 *   - ref_L09_00406BF8_00 -> func_L09_003270C8 and
 *     ref_L09_00406BF8_01 -> func_L09_00408030 (upstream's cross-reference
 *     names for unnamed callees; ours are the split glabels at the implied
 *     addresses);
 *   - `#include "common.h"` dropped: the typedefs this function uses are
 *     written here and only its declarations kept.
 */
s32 func_L09_00408088(s32 arg0, s32 arg1) {
    s32 var_a1;
    s32 var_s0;

    var_a1 = 0;
    var_s0 = 0;
    if (arg0 != -1) {
        if (func_L09_003270C8(g_heroPos, arg0) != 0) {
            var_a1 = var_s0;
            var_s0 = 1;
        } else {
            var_a1 = 2;
        }
    }
    func_L09_00408030(arg1, var_a1);
    return var_s0;
}
