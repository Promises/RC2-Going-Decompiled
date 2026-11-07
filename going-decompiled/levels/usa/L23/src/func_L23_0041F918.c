/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x0041F918, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef short s16;
typedef int s32;
extern u8 *D_L23_001B1434 __attribute__((sda));
typedef struct {
    s32 v[3];
} L23Table3;
extern L23Table3 D_L23_001B1760;
typedef struct {
    u8 pad[0x18];
    s32 f18;
    s32 f1C;
} L23Block1F90;
extern L23Block1F90 D_L23_001B1F10;
typedef struct {
    u8 pad[0x1874];
    s32 f1874;
} D00189E20_View;
extern D00189E20_View g_soundBankHandlesBlk;
extern s32 func_L23_00325618(s32);

/**
 * func_L23_0041F918 - pick one of the timing entries that apply, at random.
 *
 * Starts from a copy of the three-word table D_L23_001B1760. Entry 0 applies
 * when g_soundBankHandlesBlk's word +0x1874 is below the level object's state
 * halfword +0x2F4 minus 5. Unless entry 0 applied and that word is also below
 * half of +0x2F4, entry 1 applies when D_L23_001B1F10's +0x18 is below +0x2FA
 * and entry 2 when its +0x1C is below +0x2FE. The applying entries are packed
 * to the front of the table; with none it returns -1, else the entry at the
 * index func_L23_00325618(count) returns.
 *
 * No parameters.
 * Return: the chosen entry, or -1.
 *
 * Non-obvious: D_L23_001B1F10 lies inside the split's larger label
 * D_L23_001B1F00 (+0x10); the address-named extern is valued from its name by
 * tools/levels/level_elf.py (RULING #13291 section 2). g_soundBankHandlesBlk
 * is read through upstream's view struct (type name D00189E20_View, kept as
 * written). The three-label switch, with the third sharing the default, gives
 * the ROM's decision tree. Upstream's `__attribute__((sda))` text on the
 * extern declarations is kept as written; this cc1 ignores the attribute.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/23_damosel_orbit/L23_0041E418-g8.c, function func_L23_0041E418
 * (v1.01 address), author commit 7a91db15 (bigmak94). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_L23_0041E418 -> func_L23_0041F918; D_00189E20 ->
 *     g_soundBankHandlesBlk; D_L23_001B1484 -> D_L23_001B1434; D_L23_001B17B0
 *     -> D_L23_001B1760; D_L23_001B1F90 -> D_L23_001B1F10; func_002A77E0 ->
 *     func_L23_00325618
 *   - `#include "include_asm.h"` dropped; the typedefs it uses written in the
 *     TU (the file's own where it has them, else common.h's); only the
 *     file-level declarations it uses kept, without their comments
 *   - upstream's one-line comment folded into this one
 */
s32 func_L23_0041F918(void) {
    u8 *obj = *(u8 **)(D_L23_001B1434 + 0x68);
    L23Table3 table = D_L23_001B1760;
    s32 count = 0;

    if (g_soundBankHandlesBlk.f1874 < *(s16 *)(obj + 0x2F4) - 5) {
        table.v[0] = 0;
        count = 1;
    }
    if (count == 0 || !(g_soundBankHandlesBlk.f1874 < *(s16 *)(obj + 0x2F4) / 2)) {
        if (D_L23_001B1F10.f18 < *(s16 *)(obj + 0x2FA)) {
            table.v[count++] = 1;
        }
        if (D_L23_001B1F10.f1C < *(s16 *)(obj + 0x2FE)) {
            table.v[count++] = 2;
        }
    }
    if (count == 0) {
        return -1;
    }
    switch (func_L23_00325618(count)) {
    case 0:
        return table.v[0];
    case 1:
        return table.v[1];
    case 2:
    default:
        return table.v[2];
    }
}
