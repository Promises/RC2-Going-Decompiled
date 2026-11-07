/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x003C16D8, in place of its split asm; the row is
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
 * the function differs from the disc in 26 of 34 words (masked compare). */
extern char *D_L20_001B2F1C __attribute__((section(".sdata")));
extern float D_L20_0023E5F0[];

/**
 * func_L20_003C16D8 - lower every entity of a group by 20 units, saving their heights.
 *
 * Walks the group's id list D_L20_001FBAA0[index] (index = the state's first
 * word; ids masked with 0x7FFF index the 0x100-byte records at
 * D_L20_001B2F1C; the last id has its top bit set). For each entity it saves
 * the height +0x18 to the next slot of D_L20_0023E5F0, sets flags 0x41 in
 * +0x34 and lowers +0x18 by 20.
 *
 * @arg0: the controlling moby; its +0x68 points at the state.
 * Return: nothing.
 *
 * Non-obvious: func_L20_003C1758 raises them back from the saved heights.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_0039C448.c, function func_L20_003C02D0 (v1.01
 * address), author commit 7f907424 (ReaperOfVeriod). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the
 * statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_003C02D0 -> func_L20_003C16D8; D_L20_001B2F9C ->
 *     D_L20_001B2F1C; D_L20_001FBB20 -> D_L20_001FBAA0; D_L20_0023E670 ->
 *     D_L20_0023E5F0
 *   - `__attribute__((sda))` written `__attribute__((section(".sdata")))` on
 *     the extern declarations (RULING #8620 addressing device, comment above
 *     them)
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 */
void func_L20_003C16D8(char *arg0) {
    s32 index = *(s32 *)(*(char **)(arg0 + 0x68));
    u16 *rec = (u16 *)D_L20_001FBAA0[index];

    if (rec != 0) {
        char *base = D_L20_001B2F1C;
        f32 twenty = 20.0f;
        f32 *out = D_L20_0023E5F0;

        do {
            u16 word = *rec;
            char *p = base + ((word & 0x7FFF) << 8);

            *out = *(f32 *)(p + 0x18);
            out++;
            *(u16 *)(p + 0x34) |= 0x41;
            *(f32 *)(p + 0x18) = *(f32 *)(p + 0x18) - twenty;
        } while (*(s16 *)rec++ >= 0);
    }
}
