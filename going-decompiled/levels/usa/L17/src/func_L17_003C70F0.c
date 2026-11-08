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

/* Level 17 (Smolg), USA v2.00: a level-only function built from C (task
 * #2667, RULING #13291 row L17-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003C70F0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L17/config/L17_c.tsv, the arm ee-gcc 2.9-ee-991111b
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G8 (upstream builds its file at -G8), plain as.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

#ifndef NULL
#define NULL 0
#endif
extern struct Gl { s32 w0; s32 w1; s32 w2; } D_0018A160;
extern s32 g_mobyTableBase __attribute__((sda));

/**
 * func_L17_003C70F0 - decide whether an object of class 0x920/0x929 may
 * act now.
 *
 * @arg0: the object (class halfword +0xAA, state block at +0x68).
 * Return: 1 or 0.
 *
 * 0 when the first word of D_0018A160 is 0. Class 0x920 returns 1; any
 * class other than 0x929 returns 0. For 0x929: 0 when the state's linked
 * object (+0x9C) exists and is of class 0x920, 0x921 or 0x929, or when the
 * state's float +0x98 is non-zero. Otherwise 1 when the state's moby index
 * (+0xA0) is -1, when its moby (g_mobyTableBase + index * 0x100) is NULL, or
 * when that moby's state byte +0x20 is 0xFE or 0xFD; else 0.
 *
 * Non-obvious: g_mobyTableBase is read as a word (the table's base), so the
 * "moby is NULL" test is on base + offset. Upstream's
 * `__attribute__((sda))` on it is not an attribute our cc1 knows (it warns
 * "directive ignored"); the 4-byte extern is $gp-addressed at -G8, as retail
 * reads it.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, MIT, the file's own notice (reproduced
 * above this TU's declarations, RULING #12108; `grep -ic lombyte` on it = 0,
 * so not Lombyte-origin),
 * file src/levels/17_smolg/L17_003C5B68-rc2.c, function func_L17_003C5B68 (v1.01 address), author commit fe9859d7 (Sergio Rodmauser).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L17_003C5B68 -> func_L17_003C70F0 (our split name at the v2.00 place);
 *   - ref_L17_003C5B68_00 -> g_mobyTableBase: the name symbol_addrs/L17_boot_names.txt gives at
 *   exactly 0x001B2C1C, where the split has no label of its own (NAMES rule,
 *   task #2646 class A2);
 *   - D_0018A160 kept: upstream's name already states the v2.00 address
 *   (0x0018A160, inside g_heroFacingDir (+0x80)); no name exists at that exact address
 *   (same grep); address-named (RULING #13291 section 2);
 *   - upstream's `__attribute__((sda))` kept verbatim (our cc1 ignores it with a
 *   warning; the measured bytes are with it in place);
 *   - the file's MIT notice carried verbatim above (RULING #12108); its
 *   `#ifndef NULL` block kept as upstream wrote it;
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
s32 func_L17_003C70F0(void *arg0) {
    void *temp_a0;
    void *temp_v0;
    s32 temp_a1;
    s32 temp_v1;
    s32 temp_a0_2;
    s32 temp_v1_2;
    void *temp_v0_2;

    if (D_0018A160.w0 == 0) {
        return 0;
    }
    temp_a1 = (*(s16 *)((char *)(arg0) + (0xAA)));
    if (temp_a1 == 0x920) {
success:
        return 1;
    }
    if (temp_a1 != 0x929) {
        goto fail;
    }
    temp_a0 = (*(void **)((char *)(arg0) + (0x68)));
    temp_v0 = (*(void **)((char *)(temp_a0) + (0x9C)));
    if (temp_v0 != NULL) {
        temp_v1 = (*(u16 *)((char *)(temp_v0) + (0xAA)));
        if ((u32) (temp_v1 - 0x920) < 2U) {
            goto fail;
        }
        if ((s16) temp_v1 == temp_a1) {
            goto fail;
        }
    }
    if ((*(f32 *)((char *)(temp_a0) + (0x98))) != 0.0f) {
        goto fail;
    }
    temp_a0_2 = (*(s32 *)((char *)(temp_a0) + (0xA0)));
    if (temp_a0_2 == -1) {
        goto success;
    }
    temp_v0_2 = (void *) (g_mobyTableBase + (temp_a0_2 << 8));
    if (temp_v0_2 == NULL) {
        goto success;
    }
    temp_v1_2 = (*(u8 *)((char *)(temp_v0_2) + (0x20)));
    if (temp_v1_2 == 0xFE) {
        goto success;
    }
    if (temp_v1_2 == 0xFD) {
        goto success;
    }
fail:
    return 0;
}
