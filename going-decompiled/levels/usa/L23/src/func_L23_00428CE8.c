/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00428CE8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef int s32;
extern void func_L23_00325548(void *, s32, s32);
extern void func_L23_00428DD8(u8 *);
extern float D_L23_001B367C __attribute__((sda));

/**
 * func_L23_00428CE8 - two-state setup of a moby, then its per-frame handler.
 *
 * State 0 (byte +0x20): sets the draw distances (+0x32 = 0xFF, +0x30 = 0xFF),
 * flag 0x100 in +0x34, a 2000.0 timer at state +0x20 with its halfword copy
 * at state +0x24, +0x7C = 0x3FF, animation 1 (func_L23_00325548(moby, 1, 0))
 * and flag 0x4000. State 1: when the shared timer D_L23_001B367C is non-zero
 * it is copied to state +0x20 and, as a halfword, to +0x24; then animation 2.
 * Either way it ends with func_L23_00428DD8(moby).
 *
 * @moby: the moby; its +0x68 points at the state.
 * Return: nothing.
 *
 * Non-obvious: D_L23_001B367C lies inside the split's larger label
 * D_L23_001B2AB0 (+0xBCC); the address-named extern is valued from its name
 * by tools/levels/level_elf.py (RULING #13291 section 2). Upstream's
 * `__attribute__((sda))` text on the extern declarations is kept as written;
 * this cc1 ignores the attribute.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/23_damosel_orbit/L23_0041E418-g8.c, function func_L23_004277E8
 * (v1.01 address), author commit d2e11c19 (bigmak94). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_L23_004277E8 -> func_L23_00428CE8; D_L23_001B36FC ->
 *     D_L23_001B367C; func_002A7750 -> func_L23_00325548; func_L23_004278D8 ->
 *     func_L23_00428DD8
 *   - `#include "include_asm.h"` dropped; the typedefs it uses written in the
 *     TU (the file's own where it has them, else common.h's); only the
 *     file-level declarations it uses kept, without their comments
 *   - upstream's comment folded into this one
 */
void func_L23_00428CE8(u8 *moby) {
    u8 *vars = *(u8 **)(moby + 0x68);
    u8 state = moby[0x20];

    if (state == 0) {
        *(u16 *)(moby + 0x32) = 0xFF;
        moby[0x30] = 0xFF;
        *(u16 *)(moby + 0x34) = *(u16 *)(moby + 0x34) | 0x100;
        *(float *)(vars + 0x20) = 2000.0f;
        *(s16 *)(vars + 0x24) = 2000;
        *(u16 *)(moby + 0x7C) = 0x3FF;
        func_L23_00325548(moby, 1, 0);
        *(u16 *)(moby + 0x34) = *(u16 *)(moby + 0x34) | 0x4000;
    } else if (state == 1) {
        float t = D_L23_001B367C;

        if (t != 0.0f) {
            *(float *)(vars + 0x20) = t;
            *(s16 *)(vars + 0x24) = (s16)t;
        }
        func_L23_00325548(moby, 2, 0);
    }
    func_L23_00428DD8(moby);
}
