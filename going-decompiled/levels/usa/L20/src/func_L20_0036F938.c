/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x0036F938, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern void func_L20_003083C8(void);
extern void func_L20_00341528(int a, int b, int c, int d, int e);
extern int D_L20_001FFDC0[];
/* RULING #8620 addressing device (level code has no native arm): upstream's
 * `__attribute__((sda))` is written `section(".sdata")` on these extern
 * DECLARATIONS. It moves no data and emits nothing: cc1 then prints the
 * one-instruction `lw/sw $r,SYM` macro, which GNU as expands to an adjacent
 * lui + lw/sw pair (absolute %hi/%lo, not $gp-relative at -G0), as retail
 * has it. Ablation (task #2565): with upstream's `sda` (ignored by this cc1)
 * the function differs from the disc in 11 of 23 words (masked compare). */
extern int D_L20_001A8FDC __attribute__((section(".sdata")));
extern int D_L20_00294F78;

/**
 * func_L20_0036F938 - register this level's mission record and announce it.
 *
 * Calls func_L20_003083C8, then in the level's table D_L20_001FFDC0 stores
 * the address of D_L20_00294F78 at word 0x40 and 0xB at word 0x41, sets
 * D_L20_001A8FDC to 1, and calls func_L20_00341528(3, 1, 0xB, 0, 0).
 *
 * No parameters.
 * Return: nothing.
 *
 * Non-obvious: D_L20_001FFDC0 lies inside the split's larger label
 * D_L20_001F0000; the address-named extern is valued from its name by
 * tools/levels/level_elf.py undef (RULING #13291 section 2).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_002DEBE0.c, function func_L20_0036E720 (v1.01
 * address), author commit 71f1dc28 (ReaperOfVeriod). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the
 * statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_0036E720 -> func_L20_0036F938; D_L20_001FFE40 ->
 *     D_L20_001FFDC0; D_L20_00294F98 -> D_L20_00294F78; func_002B49E0 ->
 *     func_L20_00341528; func_L00_002F3478 -> func_L20_003083C8
 *   - `__attribute__((sda))` written `__attribute__((section(".sdata")))` on
 *     the extern declarations (RULING #8620 addressing device, comment above
 *     them)
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 */
void func_L20_0036F938(void)
{
    int *base;
    int *other;

    func_L20_003083C8();
    base = D_L20_001FFDC0;
    other = &D_L20_00294F78;
    *(int *)(base + 0x40) = (int)other;
    *(int *)(base + 0x41) = 0xB;
    D_L20_001A8FDC = 1;
    func_L20_00341528(3, 1, 0xB, 0, 0);
}
