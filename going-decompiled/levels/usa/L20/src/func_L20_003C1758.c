/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x003C1758, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

extern char *D_L20_001FBAA0[];
/* RULING #8620 addressing device (level code has no native arm): upstream's
 * `__attribute__((sda))` is written `section(".sdata")` on these extern
 * DECLARATIONS. It moves no data and emits nothing: cc1 then prints the
 * one-instruction `lw/sw $r,SYM` macro, which GNU as expands to an adjacent
 * lui + lw/sw pair (absolute %hi/%lo, not $gp-relative at -G0), as retail
 * has it. Ablation (task #2565): with upstream's `sda` (ignored by this cc1)
 * the function differs from the disc in 6 of 54 words (masked compare). */
extern char *D_L20_001B2F1C __attribute__((section(".sdata")));
extern float D_L20_0023E5F0[];
extern float func_L20_00304118(int);
extern float func_L20_00330BB8(float, float, float, float, float);

/**
 * func_L20_003C1758 - raise a group's entities back towards their saved heights.
 *
 * The inverse of func_L20_003C16D8, eased: scale = func_L20_00330BB8(-1, 0,
 * 1, 0, 1 - func_L20_00304118(state[1]) * state[2]); each entity's height
 * becomes (saved - 20) + 20 * scale, and flags 0x41 in +0x34 are cleared.
 *
 * @arg0: the controlling moby; its +0x68 points at the state (group index,
 * timer, rate).
 * Return: nothing.
 *
 * Non-obvious: the walk returns at once when the group's list pointer is
 * null.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_0039C448.c, function func_L20_003C0350 (v1.01
 * address), author commit 7f907424 (ReaperOfVeriod). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the
 * statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_003C0350 -> func_L20_003C1758; D_L20_001B2F9C ->
 *     D_L20_001B2F1C; D_L20_001FBB20 -> D_L20_001FBAA0; D_L20_0023E670 ->
 *     D_L20_0023E5F0; func_00283CE0 -> func_L20_00304118; func_002A7AA8 ->
 *     func_L20_00330BB8
 *   - `__attribute__((sda))` written `__attribute__((section(".sdata")))` on
 *     the extern declarations (RULING #8620 addressing device, comment above
 *     them)
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 */
void func_L20_003C1758(char *arg0) {
    char *s1 = *(char **)(arg0 + 0x68);
    u16 *s0 = (u16 *)D_L20_001FBAA0[*(s32 *)s1];
    f32 *out;
    char *base;
    f32 scale;

    if (s0 == 0) {
        return;
    }
    {
        f32 v = func_L20_00304118(*(s32 *)(s1 + 4)) * *(f32 *)(s1 + 8);
        scale = func_L20_00330BB8(-1.0f, 0.0f, 1.0f, 0.0f, 1.0f - v);
    }
    out = D_L20_0023E5F0;
    base = D_L20_001B2F1C;
    do {
        f32 v = *out;
        u16 w = *s0;
        char *p = base + ((w & 0x7FFF) << 8);
        f32 d = v - 20.0f;

        out++;
        *(u16 *)(p + 0x34) &= 0xFFBE;
        *(f32 *)(p + 0x18) = d + (v - d) * scale;
    } while (*(s16 *)s0++ >= 0);
}
