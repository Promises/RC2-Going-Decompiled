/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x0039EB78, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

/* RULING #8620 addressing device (level code has no native arm): upstream's
 * `__attribute__((sda))` is written `section(".sdata")` on these extern
 * DECLARATIONS. It moves no data and emits nothing: cc1 then prints the
 * one-instruction `lw/sw $r,SYM` macro, which GNU as expands to an adjacent
 * lui + lw/sw pair (absolute %hi/%lo, not $gp-relative at -G0), as retail
 * has it. Ablation (task #2565): with upstream's `sda` (ignored by this cc1)
 * the function differs from the disc in 6 of 51 words (masked compare). */
extern int g_gameTime __attribute__((section(".sdata")));
extern int func_L20_0039F2E0(char *, int);
extern void func_L20_0039EC48(char *);
extern void func_L20_0039EA48(char *);
extern float func_L20_00304018(float, float);
extern float func_L20_00334370(char *, char *, float, float, float, float, int);

/**
 * func_L20_0039EB78 - per-frame update of a Yeedil moby that drives a helper object.
 *
 * Refreshes the helper (func_L20_0039F2E0(moby, 0)) and runs
 * func_L20_0039EC48. Every eighth frame, staggered by the moby's own address
 * (g_gameTime & 7 == (moby >> 8) & 7), it also runs func_L20_0039EA48.
 * Then it eases two angles of its state (+0x288, +0x28C) towards the change
 * of the moby's heading +0xF8 since last frame (func_L20_00304018,
 * func_L20_00334370 with rates 0.0097 and 0.58), stores the heading at state
 * +0x284, and counts +0x96 up, saturating at 0xFFFF.
 *
 * @arg0: the moby; its +0x68 points at the state.
 * Return: nothing.
 *
 * Non-obvious: the constant 0.0097 is passed twice through two locals
 * (upstream's `c`/`c2`), which gives the ROM's register use.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_0039C448.c, function func_L20_0039D7B0 (v1.01
 * address), author commit 7f907424 (ReaperOfVeriod). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the
 * statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_0039D7B0 -> func_L20_0039EB78; D_L20_001B2AC8 ->
 *     g_gameTime; func_00283BE0 -> func_L20_00304018; func_002AAA00 ->
 *     func_L20_00334370; func_L20_0039D680 -> func_L20_0039EA48;
 *     func_L20_0039D880 -> func_L20_0039EC48; func_L20_0039DF18 ->
 *     func_L20_0039F2E0
 *   - D_L20_001B2A48 -> g_gameTime (task #2646): the name
 *     symbol_addrs/L20_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2 class,
 *     NOTE #13409).
 *   - `__attribute__((sda))` written `__attribute__((section(".sdata")))` on
 *     the extern declarations (RULING #8620 addressing device, comment above
 *     them)
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 */
void func_L20_0039EB78(char *arg0) {
    char *s0 = arg0;
    char *s1;
    f32 c;
    f32 r;
    u32 v1;
    u16 v2;

    s1 = *(char **)(s0 + 0x68);
    func_L20_0039F2E0(s0, 0);
    func_L20_0039EC48(s0);
    if ((g_gameTime & 7) == (((s32)s0 >> 8) & 7)) {
        func_L20_0039EA48(s0);
    }
    c = 0.009696274995803833f;
    r = func_L20_00304018(*(f32 *)(s1 + 0x284), *(f32 *)(s0 + 0xF8));
    {
        f32 c2 = c;
        func_L20_00334370(s1 + 0x288, s1 + 0x28C, r, c, c2, 0.58177649974823f, 0);
    }
    *(f32 *)(s1 + 0x284) = *(f32 *)(s0 + 0xF8);
    v2 = *(u16 *)(s0 + 0x96);
    if ((u32)v2 <= 0xFFFEu) {
        *(u16 *)(s0 + 0x96) = v2 + 1;
    }
}
