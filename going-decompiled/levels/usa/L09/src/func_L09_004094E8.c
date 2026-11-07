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
 * one place, 0x004094E8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm 2.9-ee-991111
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, plain as.
 */

typedef signed char s8;
typedef signed short s16;
typedef unsigned char u8;
typedef unsigned short u16;
typedef int s32;

/**
 * func_L09_004094E8 - set an entity's motion flags from three signed
 * requests (the shared body func_L09_004095C8 hands entities to).
 *
 * Nothing happens for a null @arg0 or an entity whose state byte at +0x20 is
 * 0xFE or 0xFD. The +0x24 owner check (kind word 0x12 at owner+0x46, a null
 * owner passes) gates the first two requests: @arg1 nonzero sets the u16 at
 * +0x34 to `& 0xFFFD` when positive else `| 2`; @arg2 nonzero (any owner)
 * writes +0x31 = 1 and `& 0xFFFE` when positive, else +0x31 = 0 and `| 1`.
 * @arg3 nonzero copies the owner's word at +0x10 to +0x98 when positive,
 * else stores 0 there.
 *
 * @arg0: the entity record.
 * @arg1: the horizontal-motion request (sign picks the bit direction).
 * @arg2: the vertical-motion request.
 * @arg3: the follow request (positive copies the owner's word, else clears).
 * Return: none.
 *
 * Non-obvious: the second request re-tests `arg0 == 0` inside its owner
 * check even though the outer test already proved it nonzero -- upstream
 * spells it that way and the body keeps it verbatim. No codegen or addressing
 * device is used.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, MIT (the file's own notice,
 * carried verbatim above; `grep -ic lombyte` 0, so not Lombyte-origin), file
 * src/levels/09_dobbo/L09_00408058-rc2.c, function func_L09_00408058 (v1.01
 * address), author commit fe9859d7 (Sergio Rodmauser). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names (BYTE-IDENTICAL): only the function name changes,
 * func_L09_00408058 -> func_L09_004094E8 (our split name at the v2.00 place);
 * the body references no other symbol (0 relocations), and
 * `#include "common.h"` is dropped: the typedefs this function uses are
 * written here and only its declarations kept.
 */
void func_L09_004094E8(void *arg0, s32 arg1, s32 arg2, s32 arg3) {
    u16 temp_v0_2;
    u16 var_v0;
    u16 var_v0_2 ;
    s32 temp_v1 ;
    void *temp_v0;
    void *temp_v0_3;
    void *temp_v0_4;

    if ((arg0 != 0) &&
        (temp_v1 = (*(u8 *)((s8 *)(arg0) + (0x20))), temp_v1 != 0xFE) &&
        (temp_v1 != 0xFD)) {
        if ((arg1 != 0) &&
            ((temp_v0 = (*(void **)((s8 *)(arg0) + (0x24))), temp_v0 == 0) ||
             ((*(s16 *)((s8 *)(temp_v0) + (0x46))) != 0x12))) {
            temp_v0_2 = (*(u16 *)((s8 *)(arg0) + (0x34)));
            if (arg1 > 0) {
                var_v0 = temp_v0_2 & 0xFFFD;
            } else {
                var_v0 = temp_v0_2 | 2;
            }
            (*(u16 *)((s8 *)(arg0) + (0x34))) = var_v0;
        }
        if ((arg2 != 0) &&
            ((arg0 == 0) ||
             ((temp_v0_3 = (*(void **)((s8 *)(arg0) + (0x24))), temp_v0_3 == 0) ||
              ((*(s16 *)((s8 *)(temp_v0_3) + (0x46))) != 0x12)))) {
            temp_v1 = 1;
            if (arg2 > 0) {
                (*(s8 *)((s8 *)(arg0) + (0x31))) = temp_v1;
                var_v0_2 = (*(u16 *)((s8 *)(arg0) + (0x34))) & 0xFFFE;
            } else {
                (*(s8 *)((s8 *)(arg0) + (0x31))) = 0;
                var_v0_2 = (*(u16 *)((s8 *)(arg0) + (0x34))) | 1;
            }
            (*(u16 *)((s8 *)(arg0) + (0x34))) = var_v0_2;
        }
        if (arg3 != 0) {
            temp_v0_4 = (*(void **)((s8 *)(arg0) + (0x24)));
            if (temp_v0_4 != 0) {
                if (arg3 > 0) {
                    (*(s32 *)((s8 *)(arg0) + (0x98))) = (s32)(*(s32 *)((s8 *)(temp_v0_4) + (0x10)));
                    return;
                }
                (*(s32 *)((s8 *)(arg0) + (0x98))) = 0;
            }
        }
    }
}
