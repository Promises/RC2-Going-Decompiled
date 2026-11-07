/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x0039F5B0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef float f32;

extern void func_L20_00302FC0(char *, char *, char *);
extern f32 func_L20_00330988(f32, f32);
extern f32 func_L20_00303FD0(f32, f32);
extern f32 func_L20_003035B8(f32);
extern f32 func_L20_003035D0(f32);
extern void func_L20_00302FF0(void *, void *, void *);
extern f32 func_L20_003030F0(void *);
extern void func_L20_003032F8(char *, char *, f32);
extern f32 func_L20_003031A0(void *, void *);
extern char g_soundBankHandlesBlk[];
extern float D_L20_001B9F58;

/**
 * func_L20_0039F5B0 - pick the point a moby moves to next, 8 to 12 units from its target.
 *
 * When the target at state +0x200 is the hero's moby (g_soundBankHandlesBlk +
 * 0x2290), the point (state +0x260) is placed around the hero's position
 * (g_soundBankHandlesBlk + 0x80) at an angle within 45 degrees of
 * D_L20_001B9F58. Otherwise it is the offset from the target to this moby,
 * shortened to that distance when longer, added back to the target's
 * position. Finally state +0x11C gets the distance from the moby to the
 * point.
 *
 * @arg0: the moby; its +0x68 points at the state.
 * Return: nothing.
 *
 * Non-obvious: state + 0x260 and arg0 + 0x10 are written at each use rather
 * than kept in variables; upstream notes that this gives retail's copy of the
 * pointer in a register live after the if.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_0039C448.c, function func_L20_0039E1E8 (v1.01
 * address), author commit 1da2d1f3 (Fromnowhere). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for
 * names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_0039E1E8 -> func_L20_0039F5B0; D_00189E20 ->
 *     g_soundBankHandlesBlk; D_L20_001B9FD8 -> D_L20_001B9F58; func_00282CC0
 *     -> func_L20_00302FC0; func_00282CF0 -> func_L20_00302FF0; func_00282DF0
 *     -> func_L20_003030F0; func_00282E80 -> func_L20_003031A0; func_00282F70
 *     -> func_L20_003032F8; func_00283180 -> func_L20_003035B8; func_00283198
 *     -> func_L20_003035D0; func_00283B98 -> func_L20_00303FD0; func_002A7878
 *     -> func_L20_00330988
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 *   - upstream's comment folded into this one
 */
void func_L20_0039F5B0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    f32 dist;
    f32 angle;
    char *hero;

    dist = func_L20_00330988(8.0f, 12.0f);
    hero = g_soundBankHandlesBlk;
    if (*(char **)(state + 0x200) == *(char **)(hero + 0x2290)) {
        angle = func_L20_00330988(-45.0f, 45.0f);
        angle = func_L20_00303FD0(angle * 0.017453294f, D_L20_001B9F58);
        *(f32 *)(state + 0x260) = func_L20_003035B8(angle) * dist;
        *(f32 *)(state + 0x264) = func_L20_003035D0(angle) * dist;
        *(int *)(state + 0x268) = 0;
        func_L20_00302FC0(state + 0x260, state + 0x260, hero + 0x80);
    } else {
        func_L20_00302FF0(state + 0x260, arg0 + 0x10, *(char **)(state + 0x200) + 0x10);
        if (dist < func_L20_003030F0(state + 0x260)) {
            func_L20_003032F8(state + 0x260, state + 0x260, dist);
        }
        func_L20_00302FC0(state + 0x260, state + 0x260, *(char **)(state + 0x200) + 0x10);
    }
    *(f32 *)(state + 0x11C) = func_L20_003031A0(arg0 + 0x10, state + 0x260);
}
