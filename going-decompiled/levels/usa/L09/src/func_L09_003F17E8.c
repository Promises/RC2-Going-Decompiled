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
 * one place, 0x003F17E8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef signed char s8;
typedef signed short s16;
typedef int s32;
typedef unsigned char u8;
typedef float f32;

/* Upstream's cross-reference declarations for the unnamed callees, their
 * prototypes verbatim with our names at the implied v2.00 addresses. */
extern void func_L09_003F1740(void);
extern s32 func_L09_002EC258(f32);
extern void func_L09_00317818(void *, s32, s32, s32);
extern void func_L09_00317AF8(void *, s32, s32);
extern s32 func_L09_0031A158(void *, s32, s32);
extern f32 func_L09_0031A1B8(void *, s32);
extern void func_L09_0031C598(void *, void *);
extern void func_L09_0031C658(void *, s32);
extern void func_L09_003275B8(void *, s32, s32, s32, f32, f32);

/**
 * func_L09_003F17E8 - drain a timed effect's remaining budget, then retune it.
 *
 * The object's state lives at *(s8 **)(a0+0x68). When its byte +0x95 is 0 the
 * object only runs its idle tick func_L09_003F1740. Otherwise the effect's
 * per-step magnitude is queried from func_L09_0031A158(@a0, 0x10000, 0) (kept
 * only when the state's s16 at +0x26 is below 1) and scaled by
 * func_L09_0031A1B8(@a0, that value). While a nonzero budget remains and the
 * scaled amount is positive it is subtracted from the state's float at +0x20;
 * when that hits zero the effect is closed via func_L09_00317AF8(@a0, 6, 4),
 * and while it is still positive the state's byte at +0x77 is retuned to
 * func_L09_002EC258(amount * 20) + 0x64, the update pair
 * func_L09_0031C598(@a0, state+0x70) runs, func_L09_00317818(@a0, 4, 0, 1)
 * fires unless the object's byte +0x43 is already 4, and
 * func_L09_00317AF8(@a0, 5, -1) marks the step. Either way the object's byte
 * +0xA8 is set to 0xFF, and every path ends in func_L09_0031C658(@a0, the
 * state+0x70 view) and func_L09_003275B8(@a0, state's word +0x8C, 0, 0, 0, 0).
 *
 * @arg0: the object.
 * Return: none.
 *
 * Non-obvious: the two tail calls run for BOTH the +0x95 == 0 and != 0 arms;
 * only the drain work between them is conditional. No codegen or addressing
 * device is used.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, MIT (the file's own notice,
 * carried verbatim above; `grep -ic lombyte` 0, so not Lombyte-origin), file
 * src/levels/09_dobbo/L09_003F0378-rc2.c, function func_L09_003F0378 (v1.01
 * address), author commit fe9859d7 (Sergio Rodmauser). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2562 relverify by name):
 *   - func_L09_003F0378 -> func_L09_003F17E8 (our split name);
 *   - ref_L09_003F0378_00 -> func_L09_003F1740, _01 -> func_L09_002EC258,
 *     _02 -> func_L09_00317818, _03 -> func_L09_00317AF8,
 *     _04 -> func_L09_0031A158, _05 -> func_L09_0031A1B8,
 *     _06 -> func_L09_0031C598, _07 -> func_L09_0031C658,
 *     _08 -> func_L09_003275B8 (upstream's cross-reference names for
 *     unnamed callees; ours are the split glabels at the implied addresses);
 *   - `#include "common.h"` dropped: the typedefs this function uses are
 *     written here and only its declarations kept.
 */
void func_L09_003F17E8(void *arg0) {
    void *temp_s2;
    s32 temp_s0;
    s32 func_result;
    s32 temp_cond ;
    void *var_s3;
    f32 temp_f0;
    f32 temp_f1;

    temp_s2 = *(void **)((s8 *)arg0 + 0x68);
    if (*(u8 *)((s8 *)arg0 + 0x95) == 0) {
        var_s3 = (s8 *)temp_s2 + 0x70;
        func_L09_003F1740();
    } else {
        var_s3 = (s8 *)temp_s2 + 0x70;
        temp_s0 = 0;
        func_result = func_L09_0031A158(arg0, 0x10000, 0);
        temp_cond = *(s16 *)((s8 *)temp_s2 + 0x26) < 1;
        temp_s0 = temp_cond ? func_result : temp_s0;
        temp_f1 = func_L09_0031A1B8(arg0, temp_s0);
        if (temp_s0 != 0) {
            if (temp_f1 > 0.0f) {
                temp_f0 = *(f32 *)((s8 *)temp_s2 + 0x20) - temp_f1;
                *(f32 *)((s8 *)temp_s2 + 0x20) = temp_f0;
                if (temp_f0 <= 0.0f) {
                    func_L09_00317AF8(arg0, 6, 4);
                } else {
                    *(s8 *)((s8 *)temp_s2 + 0x77) = (s8)(func_L09_002EC258(temp_f1 * 20.0f) + 0x64);
                    func_L09_0031C598(arg0, var_s3);
                    if (*(u8 *)((s8 *)arg0 + 0x43) != 4) {
                        func_L09_00317818(arg0, 4, 0, 1);
                    }
                    func_L09_00317AF8(arg0, 5, -1);
                }
            }
        }
        *(u8 *)((s8 *)arg0 + 0xA8) = 0xFF;
    }
    func_L09_0031C658(arg0, (s32)var_s3);
    func_L09_003275B8(arg0, *(s32 *)((s8 *)temp_s2 + 0x8C), 0, 0, 0.0f, 0.0f);
}
