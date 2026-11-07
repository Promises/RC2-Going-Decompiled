/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x00414290, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed int s32;

/* L11's own copies of boot code (callees at their v2.00 places in this level). */
extern void func_L11_00308A80(s32, s32, s32, s32, s32);
extern s32 func_L11_00333E28(void *, s32);
extern s32 func_L11_00300E50(void *);
extern s32 func_L11_003076B8(s32, s32);

/* Three halfwords inside the boot's larger g_health block, so they stay
 * address-named (RULING #13291 §2). `section(".data")` keeps cc1 from
 * addressing these 2-byte objects $gp-relative at -G8: upstream's NOT_SDA
 * macro, spelled out (RULING #8620 addressing-model device). */
extern u16 D_0018CD60 __attribute__((section(".data")));
extern u16 D_00194158 __attribute__((section(".data")));
extern u16 D_0018D048 __attribute__((section(".data")));
/* The next three carry `section(".sdata")`, an ADDRESSING-MODEL DEVICE on the
 * declaration only (RULING #8620): cc1 prints a one-instruction macro load
 * for each, and GNU as at -G8 expands it $gp-relative for the 4-byte
 * g_gameTime and as an absolute lui/lw pair for the two arrays, as the
 * ROM has them. It moves no data and emits nothing.
 * g_gameTime is L11's boot-name map name at 0x001B3148 (task #2646). */
extern s32 g_gameTime __attribute__((section(".sdata")));
extern u8 g_heroPos[];
/* Flag bytes. The v2.00 ROM's relocations put byte 0 at 0x001A7B08, 8 bytes
 * past the boot's g_inventoryOwned (0x001A7B00). Upstream calls this
 * g_inventoryOwned at the v1.01 address; that identity is not asserted
 * here. */
extern u8 D_001A7B08[0x38] __attribute__((section(".sdata")));
extern s32 D_L11_001A9010[] __attribute__((section(".sdata")));

/**
 * func_L11_00414290 - queue Joba subtitles when the inventory and cuboid
 * gates allow, with retry timers.
 *
 * @object: the actor. Its state block is at +0x68, its stage byte at +0x20
 *          and a byte at +0x30.
 *
 * On the first call (stage 0) it sets the stage to 1 and the +0x30 byte to
 * 0xFF, and arms the two timers at state+0x8 (0x1A4) and state+0x10 (0x12C).
 * It does nothing more unless the stage is 1. Then four checks run in turn:
 *   - if flag byte 0 is set, it queues line 0x9FF;
 *   - if both level gates hold, it sets D_0018CD60 to 0xFFFF;
 *   - when func_L11_00333E28(g_heroPos, state+0x0) (upstream: the cuboid
 *     gate) and func_L11_00300E50(state+0x8) (upstream: the retry timer) are
 *     both nonzero and D_00194158 is clear, it plays 0xA00 and re-arms
 *     state+0x8;
 *   - flag byte 0x2E queues line 0xA03, and flag byte 0xB plays 0xA3D when
 *     D_0018D048 is clear and func_L11_00300E50(state+0x10) is nonzero.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_00412B20-g8.c, function
 * func_L11_00412B20 (v1.01 address), author commit c7d9e30a (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L11_00412B20 -> func_L11_00414290 (our split name at the
 *     v2.00 place);
 *   - callees respelled to the targets of the v2.00 ROM's own relocations
 *     (relverify, task #2553): func_L01_002EABA8 -> func_L11_00308A80,
 *     func_002ACBC8 -> func_L11_00333E28, func_00282948 -> func_L11_00300E50,
 *     func_00288E18 -> func_L11_003076B8 (L11's copies of that code);
 *   - data: D_00189EA0 -> g_heroPos (the boot's name, same address);
 *     D_L11_001B30C8 -> g_gameTime and D_001A7B00 -> D_001A7B08 (upstream
 *     names state v1.01 addresses; ours are the v2.00 ROM's targets);
 *     D_0018CD60, D_00194158, D_0018D048 and D_L11_001A9010 unchanged;
 *   - D_L11_001B3148 -> g_gameTime (task #2646): the name
 *     symbol_addrs/L11_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2 class,
 *     NOTE #13409).
 *   - `__attribute__((sda))` -> `section(".sdata")` (RULING #8620) on three
 *     externs, and the NOT_SDA macro written out as `section(".data")`;
 *   - `#include "common.h"` dropped; its u8/u16/s32 typedefs written out;
 *     the upstream g_inventoryOwned comment and the one-line function comment
 *     folded into this one.
 */
void func_L11_00414290(u8 *object) {
    u8 *state = *(u8 **)(object + 0x68);

    if (*(u8 *)(object + 0x20) == 0) {
        *(u8 *)(object + 0x20) = 1;
        *(u8 *)(object + 0x30) = 0xFF;
        *(s32 *)(state + 0x8) = 0x1A4;
        *(s32 *)(state + 0x10) = 0x12C;
    }
    if (*(u8 *)(object + 0x20) != 1) {
        return;
    }
    if (D_001A7B08[0]) {
        func_L11_00308A80(0x9FF, 0x56, 3, *(s32 *)(state + 0x4), -1);
    }
    if (D_L11_001A9010[0] != 0 && g_gameTime >= 3) {
        D_0018CD60 = 0xFFFF;
    }
    if (func_L11_00333E28(g_heroPos, *(s32 *)(state + 0x0)) != 0) {
        if (func_L11_00300E50(state + 0x8) != 0) {
            if (D_00194158 == 0) {
                func_L11_003076B8(0xA00, 0x57);
                *(s32 *)(state + 0x8) = 0x1A4;
            }
        }
    }
    if (D_001A7B08[0x2E]) {
        func_L11_00308A80(0xA03, 0x5A, 0, *(s32 *)(state + 0xC), -1);
    }
    if (D_001A7B08[0xB]) {
        if (D_0018D048 == 0) {
            if (func_L11_00300E50(state + 0x10) != 0) {
                func_L11_003076B8(0xA3D, 0x94);
            }
        }
    }
}
