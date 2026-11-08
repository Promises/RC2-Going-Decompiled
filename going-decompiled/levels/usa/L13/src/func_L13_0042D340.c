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

/* Level 13 (Boldan), USA v2.00: a level-only function built from C (task
 * #2672, RULING #13291 row L13-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x0042D340, in place of its split asm; the row is
 * going-decompiled/levels/usa/L13/config/L13_c.tsv, the arm ee-gcc 2.9-ee-991111b
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, plain as.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;

/**
 * func_L13_0042D340 - one-shot start: on state 0, go to state 1 and set
 * flag 0x800.
 *
 * @arg0: the object (state byte +0x20, flags halfword +0x34).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, MIT, the file's own notice (reproduced
 * above this TU's declarations, RULING #12108; `grep -ic lombyte` on it = 0,
 * so not Lombyte-origin),
 * file src/levels/13_boldan/L13_0042BE60-rc2.c, function func_L13_0042BE60 (v1.01 address), author commit fe9859d7 (Sergio Rodmauser).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L13_0042BE60 -> func_L13_0042D340 (our split name at the v2.00 place);
 *   - the file's MIT notice carried verbatim above (RULING #12108); upstream's
 *   macro-expanded field accesses kept as written;
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
void func_L13_0042D340(s8 *arg0) {
    if ((*(u8 *)((char *)(arg0) + (0x20))) == 0) {
        (*(u8 *)((char *)(arg0) + (0x20))) = 1;
        (*(u16 *)((char *)(arg0) + (0x34))) = (u16) ((*(u16 *)((char *)(arg0) + (0x34))) | 0x800);
    }
}
