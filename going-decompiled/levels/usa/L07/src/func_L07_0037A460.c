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
/* Level 07 (Siberius), USA v2.00: a level-only function built from C (row
 * L07-A, task #2656, RULING #13291). Compiled ALONE as its own TU and linked
 * at its one place, 0x0037A460, in place of its split asm; the row is
 * going-decompiled/levels/usa/L07/config/L07_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/* Upstream's cross-reference declaration for the unnamed callee, its
 * prototype verbatim with our name at the implied v2.00 address. */
extern void func_L07_003078E8();

/**
 * func_L07_0037A460 - tail into func_L07_003078E8.
 *
 * The whole body is one call to func_L07_003078E8 with no arguments set up:
 * whatever the caller left in $a0..$a3 passes straight through. The same
 * callee is reached through the D_L07_001B0A90 latch by func_L07_003E74F8.
 *
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, MIT (the file's own notice,
 * carried verbatim above; `grep -ic lombyte` 0, so not Lombyte-origin), file
 * src/levels/07_siberius/L07_003791E8-rc2.c, function func_L07_003791E8
 * (v1.01 address), author commit fe9859d7 (Sergio Rodmauser), by blame -M -C
 * of the definition (all 3 lines). Its 11-token body (one call) is
 * token-identical to 20 other upstream functions, but a trivial line carries
 * no credit (RULING #13541 rev 2), so no rename-copy writer is credited.
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (BYTE-IDENTICAL), each name to the target
 * of the v2.00 disc's own relocation at this place (task #2656 relverify by
 * name):
 *   - func_L07_003791E8 -> func_L07_0037A460 (our split name);
 *   - ref_L07_003791E8_00 -> func_L07_003078E8 (upstream's cross-reference
 *     name for the unnamed callee; ours is the split glabel at the implied
 *     address);
 *   - `#include "common.h"` dropped: this function uses no common.h
 *     typedef. Upstream's cflags row for the file adds -fno-builtin; this
 *     row compiles without it, as task #2539's audit measured it (0
 *     differing words), and the body calls no builtin.
 */
void func_L07_0037A460(void) {
    func_L07_003078E8();
}
