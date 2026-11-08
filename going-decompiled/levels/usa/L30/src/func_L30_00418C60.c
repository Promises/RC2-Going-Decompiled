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
/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00418C60, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, plain as.
 */

typedef signed char s8;
typedef signed int s32;
typedef float f32;

/* L30's .sdata float at its v2.00 label (at -G8 cc1 reaches it $gp-relative
 * by size; upstream's `sda` marker is ignored by our cc1, kept verbatim). */
extern f32 D_L30_001B1FB0 __attribute__((sda));
/* The camera matrix, the name L30's boot-name map gives exactly 0x001B9E70. */
extern s8 g_cameraMatrix[];
/* L30's functions at their v2.00 places. */
extern void func_L30_002F5CD8(s32, s8 *, f32);
extern void func_L30_002F5A10(s32, s32, s8 *);
extern void func_L30_0031A578(s8 *);

/**
 * func_L30_00418C60 - place the actor's companion moby in front of the
 * camera, run its callback and update it.
 * @arg0: the actor; its state block is at +0x68 (the companion moby at
 *        +0x50).
 *
 * The companion's position (+0x10) is set from g_cameraMatrix at distance
 * D_L30_001B1FB0 (func_L30_002F5CD8), then moved by the 0x230-byte-earlier
 * camera data (func_L30_002F5A10). A non-null callback at the companion's
 * +0x64 is called with it, and func_L30_0031A578 updates the companion.
 * Return: none.
 *
 * Arm: upstream's config/cflags.txt builds this file at -G8 (with
 * -fno-builtin, which changes nothing here: no builtin is called), and NOTE
 * #13281's record is -G8; the task table's -G0 was derived from the file
 * stem (the #2667 trap). Measured (task #2673): 0 of 35 words differ at -G8,
 * 25 of 36 at -G0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, MIT (the file's own notice, kept
 * verbatim above per RULING #12108; `grep -ic lombyte` 0, so not Lombyte-
 * origin), file src/levels/30_insomniac_museum/L30_004185E0-rc2.c, function
 * func_L30_004185E0 (v1.01 address), author commit fe9859d7 (Sergio
 * Rodmauser; `git blame -M -C` gives all 19 definition lines to it, `git log
 * -S` agrees). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names, each name to the target of the
 * v2.00 disc's own relocation at this place (task #2673 relverify by name):
 *   - func_L30_004185E0 -> func_L30_00418C60 (our split name);
 *   - ref_L30_004185E0_00 -> D_L30_001B1FB0 (our split label at the v2.00
 *     target); ref_L30_004185E0_01 -> g_cameraMatrix (the name L30's
 *     boot-name map gives exactly 0x001B9E70); ref_L30_004185E0_02 ->
 *     func_L30_002F5CD8, ref_L30_004185E0_03 -> func_L30_002F5A10,
 *     ref_L30_004185E0_04 -> func_L30_0031A578 (upstream's cross-reference
 *     names to L30's functions at the v2.00 targets);
 *   - `#include "common.h"` dropped, the typedefs s8/s32/f32 written out with
 *     upstream include/common.h's own definitions.
 */
void func_L30_00418C60(s8 *arg0) {
    s8 *temp_s3;
    s8 *temp_s2;
    s8 *temp_v1;
    s32 temp_s1;
    s8 *s0;

    s0 = g_cameraMatrix;
    temp_s3 = *(s8 **)(arg0 + 0x68);
    temp_s2 = *(s8 **)(temp_s3 + 0x50);
    temp_s1 = (s32)(temp_s2 + 0x10);
    func_L30_002F5CD8(temp_s1, s0, D_L30_001B1FB0);
    func_L30_002F5A10(temp_s1, temp_s1, s0 - 0x230);
    if (*(void **)(temp_s2 + 0x64) != 0) {
        temp_v1 = *(s8 **)(temp_s3 + 0x50);
        (*(void (**)(s8 *))(temp_v1 + 0x64))(temp_s2);
    }
    func_L30_0031A578(*(s8 **)(temp_s3 + 0x50));
}
