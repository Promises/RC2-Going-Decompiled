/* Level 03 (Endako), USA v2.00: a level-only function built from C
 * (task #2655, RULING #13291 row L03-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x0038BF08, in place of its split asm; the row is
 * going-decompiled/levels/usa/L03/config/L03_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
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
typedef signed char s8;
typedef signed int s32;
typedef float f32;

/* L03's helper at its v2.00 place (upstream's cross-reference name
 * ref_L03_0038AA20_00). */
extern void func_L03_0038C000(s8 *, s32, s32, s32, s32, s32, s32, f32, f32, f32, f32, f32);
/* A 0x800-byte region inside the level's .bss: no name exists at 0x00238040
 * (it is D_00220000+0x6BC0 by L03's boot-name map, D_L03_0023001E+0x8022 by
 * our split's labels), so it is address-named (RULING #13291 section 2;
 * upstream's cross-reference name ref_L03_0038AA20_01). */
extern s8 D_L03_00238040[];

/**
 * func_L03_0038BF08 - emit the object's eight paired strips, four rows of two.
 *
 * @arg0: the object; its state block pointer is at +0x68.
 * Return: nothing.
 *
 * For i = 0..3, calls func_L03_0038C000 twice: once into the 256-byte slot
 * (state+0xDC + i) of the first 0x400-byte half of D_L03_00238040, with the
 * state's ints +0x38/+0x40/+0x60/+0x44/+0x64, floats +0x50/+0x70/+0x54/+0x74
 * and the constant 0.9; once into the same slot of the second half, with
 * +0x38/+0x48/+0x68/+0x4C/+0x6C, +0x58/+0x78/+0x5C/+0x7C and 0.1.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, MIT (the file's own notice above,
 * "Forja contributors"; kept verbatim per RULING #12108; `grep -ic lombyte`
 * on it = 0, so not Lombyte-origin), file
 * src/levels/03_endako/L03_0038AA20-rc2.c, function func_L03_0038AA20
 * (v1.01 address), author commit fe9859d7 (Sergio Rodmauser; the file's
 * import commit; `git blame -M -C` names it for all 16 definition lines at
 * fe6adf9b, and `git log --reverse -S'func_L03_0038AA20('` agrees).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L03_0038AA20 -> func_L03_0038BF08 (our split name at the
 *     v2.00 place);
 *   - ref_L03_0038AA20_00 -> func_L03_0038C000 (our split name at the v2.00
 *     ROM's call target) and ref_L03_0038AA20_01 -> D_L03_00238040
 *     (address-named at the v2.00 ROM's relocation target);
 *   - `#include "common.h"` dropped, its three typedefs written out.
 */
void func_L03_0038BF08(s8 *arg0) {
    s8 *s0;
    s32 i;
    s32 b1;
    s32 b2;

    i = 0;
    b1 = (s32)D_L03_00238040;
    s0 = *(s8 **)(arg0 + 0x68);
    b2 = b1 + 0x400;
    do {
        func_L03_0038C000(arg0, ((*(s32 *)(s0 + 0xDC) + i) << 8) + b1, *(s32 *)(s0 + 0x38), *(s32 *)(s0 + 0x40), *(s32 *)(s0 + 0x60), *(s32 *)(s0 + 0x44), *(s32 *)(s0 + 0x64), *(f32 *)(s0 + 0x50), *(f32 *)(s0 + 0x70), *(f32 *)(s0 + 0x54), *(f32 *)(s0 + 0x74), 0.89999997615814208984375f);
        func_L03_0038C000(arg0, ((*(s32 *)(s0 + 0xDC) + i) << 8) + b2, *(s32 *)(s0 + 0x38), *(s32 *)(s0 + 0x48), *(s32 *)(s0 + 0x68), *(s32 *)(s0 + 0x4C), *(s32 *)(s0 + 0x6C), *(f32 *)(s0 + 0x58), *(f32 *)(s0 + 0x78), *(f32 *)(s0 + 0x5C), *(f32 *)(s0 + 0x7C), 0.100000001490116119384765625f);
        i += 1;
    } while (i < 4);
}
