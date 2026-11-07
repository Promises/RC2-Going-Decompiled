/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x00448B88, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;

extern u16 *g_mobyGroupLists[];
/* RULING #8620 addressing device (level code has no native arm): upstream's
 * `__attribute__((sda))` is written `section(".sdata")` on these extern
 * DECLARATIONS. It moves no data and emits nothing. cc1 then prints the entity
 * bank's load as the one-instruction `lw $r,SYM` macro, which GNU as expands
 * to an adjacent lui/lw pair (absolute: the bank has no size, so it is not
 * $gp-relative even at -G8); without it cc1 splits %hi/%lo and schedules them
 * apart. The 4-byte bound is $gp-relative by its size either way, and the
 * relocation types are the same in both. Ablation (task #2565): with
 * upstream's `sda` (ignored by this cc1) the function differs from the disc in
 * 2 of 66 words (masked compare). */
extern u8 *g_mobyTableBase[] __attribute__((section(".sdata")));
extern int g_mobyGroupCount __attribute__((section(".sdata")));
extern void func_L20_0032FD10(void *, int, int);
extern void func_L20_003FD3C0(u8 *, int);

/**
 * func_L20_00448B88 - trigger the class-0xB69 entities of a moby's group.
 *
 * For every entity of the object's group (byte +0x21; id list
 * g_mobyGroupLists[group], ids masked with 0x7FFF, 0x100-byte records at
 * g_mobyTableBase) of class 0xB69 (+0xAA) whose state byte is neither 0xFE nor
 * 0xFD, calls func_L20_0032FD10(e, 0, 1) and func_L20_003FD3C0(e, 0). A group
 * of 0xFF does nothing.
 *
 * @self: the moby.
 * Return: nothing.
 *
 * Non-obvious: the doubled p test leaves the exit test at the bottom, so
 * retail's `b test` entry and the %hi copy in its delay slot come out
 * (upstream's note).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_00447640-g8.c, function func_L20_00447640 (v1.01
 * address), author commit 8422a726 (Fromnowhere). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the
 * statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_00447640 -> func_L20_00448B88; D_L20_001B2F9C ->
 *     g_mobyTableBase; D_L20_001B2FB8 -> g_mobyGroupCount; D_L20_001FBB20 ->
 *     g_mobyGroupLists; func_002A6C58 -> func_L20_0032FD10;
 *     func_L18_003F74B8 -> func_L20_003FD3C0
 *   - D_L20_001B2F1C -> g_mobyTableBase, D_L20_001B2F38 -> g_mobyGroupCount,
 *     D_L20_001FBAA0 -> g_mobyGroupLists (task #2646): the names
 *     symbol_addrs/L20_boot_names.txt gives at exactly those addresses; the
 *     address-named spelling was a second name for the same address (A2 class,
 *     NOTE #13409).
 *   - `__attribute__((sda))` written `__attribute__((section(".sdata")))` on
 *     the extern declarations (RULING #8620 addressing device, comment above
 *     them)
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 *   - upstream's comment folded into this one
 */
void func_L20_00448B88(u8 *self) {
    u16 *p;
    u8 *e;

    if (self[0x21] == 0xFF)
        return;
    for (p = g_mobyGroupLists[self[0x21]];
         self[0x21] != 0xFF && self[0x21] <= g_mobyGroupCount && p != 0 &&
         ((p != 0 && (s16)p[-1] >= 0) || p == g_mobyGroupLists[self[0x21]]);
         p++) {
        e = g_mobyTableBase[0] + ((*p & 0x7FFF) << 8);
        if (e == 0)
            break;
        if (*(s16 *)(e + 0xAA) == 0xB69 && e[0x20] != 0xFE && e[0x20] != 0xFD) {
            func_L20_0032FD10(e, 0, 1);
            func_L20_003FD3C0(e, 0);
        }
    }
}
