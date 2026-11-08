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
 * linked at its one place, 0x00385AC8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, plain as.
 */

typedef signed char s8;
typedef signed int s32;

/* L30's played-lines bitmask, an .sdata word at its v2.00 label (at -G8 cc1
 * reaches it $gp-relative by size; upstream's `sda` marker is ignored by our
 * cc1, kept verbatim), and the per-volume line-kind table. */
extern s32 D_L30_001AEC40 __attribute__((sda));
extern s32 D_L30_00295B60[];
/* The boot's file-load / dialog-voice manager block (core, symbol_addrs). */
extern s8 g_fileLoadVoiceState[];

/**
 * func_L30_00385AC8 - start the voice line for the actor's current volume,
 * choosing a first-time or a repeat line.
 * @arg0: the actor; its state block is at +0x68 (the chosen volume index at
 *        +0x144).
 *
 * Only when the voice manager is idle (g_fileLoadVoiceState +0x68 is 0 and
 * +0x24 is -1) and the volume index is not -1. The volume's kind
 * k = D_L30_00295B60[index] picks the line: for k != 6, the first time (bit
 * k of D_L30_001AEC40 clear) the line at state +0xE0 + 4k, setting the bit,
 * afterwards the one at +0x10C + 4k; for k == 6, state +0xF8 when the index
 * is 1, else +0x124. The manager's +0x24 becomes line & 0x3FFF plus 0xBB8
 * and +0x28 is cleared.
 * Return: 1 when a line was started, else 0.
 *
 * Arm: upstream's config/cflags.txt builds this file at -G8 (with
 * -fno-builtin, which changes nothing here: the body calls no function), and
 * NOTE #13281's record is -G8; the task table's -G0 was derived from the
 * file stem (the #2667 trap). Measured (task #2673): 0 of 49 words differ at
 * -G8, 35 of 50 at -G0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, MIT (the file's own notice, kept
 * verbatim above per RULING #12108; `grep -ic lombyte` 0, so not Lombyte-
 * origin), file src/levels/30_insomniac_museum/L30_003848B8-rc2.c, function
 * func_L30_003848B8 (v1.01 address), author commit fe9859d7 (Sergio
 * Rodmauser; `git blame -M -C` gives all 41 definition lines to it, `git log
 * -S` agrees). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names, each name to the target of the
 * v2.00 disc's own relocation at this place (task #2673 relverify by name):
 *   - func_L30_003848B8 -> func_L30_00385AC8 (our split name);
 *   - ref_L30_003848B8_00 -> D_L30_001AEC40 and ref_L30_003848B8_01 ->
 *     D_L30_00295B60 (upstream's cross-reference names to our split labels at
 *     the v2.00 targets);
 *   - D_001A63A8 -> g_fileLoadVoiceState (the boot's symbol_addrs name at
 *     exactly 0x001A63A8);
 *   - `#include "common.h"` dropped, the typedefs s8/s32 written out with
 *     upstream include/common.h's own definitions.
 */
s32 func_L30_00385AC8(void *arg0) {
    s8 *temp_a3;
    s8 *t0;
    s8 *t1;
    s32 temp_a1;
    s32 temp_a2;
    s32 temp_v1;
    s32 var_v1;
    s32 o1;
    s32 o2;

    t0 = g_fileLoadVoiceState;
    temp_a3 = *(s8 **)((s8 *)arg0 + 0x68);
    temp_a2 = *(s32 *)(temp_a3 + 0x144);
    temp_a1 = D_L30_00295B60[temp_a2];
    if (*(s32 *)(t0 + 0x68) == 0) {
        temp_v1 = *(s32 *)(t0 + 0x24);
        if ((temp_v1 == -1) && (temp_a2 != temp_v1)) {
            if (temp_a1 != 6) {
                if ((((s32) D_L30_001AEC40 >> temp_a1) ^ 1) & 1) {
                    o1 = temp_a1 * 4;
                    var_v1 = *(s32 *)(temp_a3 + o1 + 0xE0);
                    D_L30_001AEC40 |= 1 << temp_a1;
                } else {
                    o1 = temp_a1 * 4;
                    o2 = temp_a1 * 4;
                    var_v1 = *(s32 *)(temp_a3 + o2 + 0x10C);
                }
            } else if (temp_a2 == 1) {
                var_v1 = *(s32 *)(temp_a3 + 0xF8);
            } else {
                var_v1 = *(s32 *)(temp_a3 + 0x124);
            }
            t1 = g_fileLoadVoiceState;
            *(s32 *)(t1 + 0x24) = (var_v1 & 0x3FFF) + 0xBB8;
            *(s32 *)(t1 + 0x28) = 0;
            return 1;
        }
    }
    return 0;
}
