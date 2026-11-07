/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x004112C8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G0.
 */

typedef unsigned char u8;
typedef signed int s32;
extern void func_L02_0031F998();
/* The boot's own D_1A7BD3 (core, below 0x1A7C80): a dlabel of its own in the
 * boot split, asm/usa/data/core.lit/0A7400.data.s:1150 (also the operand of
 * func_002CDEB0 and func_002CDF48). Not a claim of containment: symbol_addrs
 * puts g_abLevelAvailableFlags at 0x1A7BD0, but the split has the separate
 * dlabels D_1A7BD1 and D_1A7BD2 in between. Address-named (RULING #13291 §2).
 * ADDRESSING-MODEL DEVICE (RULING #8620,
 * EE arm only): `.sdata` on the extern declaration makes cc1 print the
 * one-instruction `lbu` macro, which GNU as expands absolute at -G0, as
 * retail has it. It moves no data and emits nothing. */
extern u8 D_1A7BD3 __attribute__((section(".sdata")));
/* The boot's own D_1395B8 (core; its own label in the boot split, not inside
 * g_quickSelectMap): address-named, a second spelling of D_1395B8. */
extern u8 D_001395B8[];
/* Inside the larger split label D_L02_001B3930: address-named. */
extern u8 D_L02_001C0700[];
extern char g_heroPos[];
extern int func_L02_00325C50(void *, int);
extern s32 func_L02_002F8E70(u8 *, s32);
extern s32 func_L02_002F8FB8(u8 *);
extern int func_L02_00379338(void);
extern void func_L02_00326B28(int);
extern void func_L02_003296D0(int);
extern void func_L02_003157A0(u8 *);

/**
 * func_L02_004112C8 - step an actor by its kind byte.
 * @a0: the actor; +0x20 is its kind, +0x68 its record.
 *
 * Kind 0: hands the actor to func_L02_003157A0 when it holds no record, else
 * calls func_L02_0031F998(@a0, 1, -1). Kind 1: when D_1A7BD3 and the byte
 * D_001395B8[4] are clear, func_L02_00325C50 accepts g_heroPos with the
 * record's index at +0xC, and func_L02_002F8E70 accepts message 0x2D on
 * D_L02_001C0700, it queues 3 and 0x2E too, commits them
 * (func_L02_002F8FB8), sets D_001395B8[4] and calls func_L02_0031F998(@a0,
 * 2, -1). Kind 2: unless func_L02_00379338 answers yes, runs
 * func_L02_00326B28 and func_L02_003296D0 with 3 and calls
 * func_L02_0031F998(@a0, 3, -1).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/us2/levels/02_maktar_nebula/L02_004112C8.c, function
 * func_US2_L02_004112C8 (v2.00 address), author commit 50c79a7f (Flavius).
 * Lombyte mentions in the file: 0. Adopted with the authors' permission
 * (RULINGs #10405, #12108, #13082, #13291). ADAPTED, the body's statements
 * unchanged; edits:
 *   - name func_US2_L02_004112C8 -> func_L02_004112C8 (our split name at the
 *     same v2.00 place);
 *   - D_00189EA0 -> g_heroPos (the boot's name, same address);
 *   - D_001A7BCB -> D_1A7BD3: the address the ROM's own relocations imply
 *     is 0x1A7BD3, 8 bytes past upstream's name (relverify, task #2557);
 *     address-named, the boot's existing spelling of that address (its own
 *     dlabel, core.lit/0A7400.data.s:1150). Provenance of the respelling:
 *     D_001A7BD3 -> D_1A7BD3 (task #2621, census NOTE #13409);
 *   - D_L02_001395B8 -> D_001395B8: the same address, a core one (below
 *     0x1A7C80), spelled without the level prefix; address-named, the boot's
 *     own D_1395B8 (not inside g_quickSelectMap, validator #2598);
 *   - func_002A7750 -> func_L02_0031F998, func_002ACBC8 -> func_L02_00325C50,
 *     func_00288AA0 -> func_L02_002F8E70, func_00288BE8 -> func_L02_002F8FB8,
 *     func_002F5D10 -> func_L02_00379338, func_002AD7B8 -> func_L02_00326B28,
 *     func_002AF0A8 -> func_L02_003296D0, func_0029F338 -> func_L02_003157A0:
 *     our split names at the v2.00 addresses the ROM's own relocations imply
 *     (relverify, task #2557);
 *   - D_L02_001C0700 keeps its upstream name, which already states its v2.00
 *     address (address-named, RULING #13291 §2);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     D_1A7BD3 declaration (RULING #8620; upstream's `sda` is a marker for
 *     its own patched compiler, which our cc1 ignores);
 *   - `#include "include_asm.h"` dropped (nothing here used it) and the
 *     upstream comment folded into this one.
 */
void func_L02_004112C8(u8 *a0) {
    u8 *state = *(u8 **)(a0 + 0x68);

    switch (a0[0x20]) {
    case 0:
        if (state == 0)
            func_L02_003157A0(a0);
        else
            func_L02_0031F998(a0, 1, -1);
        break;
    case 1:
        if (D_1A7BD3 == 0 && D_001395B8[4] == 0 &&
            func_L02_00325C50(g_heroPos, *(s32 *)(state + 0xC)) != 0 &&
            func_L02_002F8E70(D_L02_001C0700, 0x2D) != 0) {
            func_L02_002F8E70(D_L02_001C0700, 3);
            func_L02_002F8E70(D_L02_001C0700, 0x2E);
            func_L02_002F8FB8(D_L02_001C0700);
            D_001395B8[4] = 1;
            func_L02_0031F998(a0, 2, -1);
        }
        break;
    case 2:
        if (func_L02_00379338() == 0) {
            func_L02_00326B28(3);
            func_L02_003296D0(3);
            func_L02_0031F998(a0, 3, -1);
        }
        break;
    }
}
