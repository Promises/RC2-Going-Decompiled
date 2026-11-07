/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x004418D8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern char *g_mobyGroupLists[];
/* RULING #8620 addressing device (level code has no native arm): upstream's
 * `__attribute__((sda))` is written `section(".sdata")` on these extern
 * DECLARATIONS. It moves no data and emits nothing: cc1 then prints the
 * one-instruction `lw/sw $r,SYM` macro, which GNU as expands to an adjacent
 * lui + lw/sw pair (absolute %hi/%lo, not $gp-relative at -G0), as retail
 * has it. Ablation (task #2565): with upstream's `sda` (ignored by this cc1)
 * the function differs from the disc in 16 of 49 words (masked compare). */
extern int g_mobyGroupCount __attribute__((section(".sdata")));
extern unsigned int g_mobyTableBase __attribute__((section(".sdata")));

/**
 * func_L20_004418D8 - count the live entities of a group.
 *
 * Counts the entities of the group whose state byte +0x20 is neither 6 nor
 * negative. g_mobyGroupLists holds a list of 16-bit ids per group; the low 15
 * bits of an id index the 0x100-byte records at g_mobyTableBase, and the last
 * id of a list has its top bit set. A group of 0xFF (low byte) or above the
 * bound g_mobyGroupCount, or one with no list, counts 0.
 *
 * @group: the group index.
 * Return: the count.
 *
 * Non-obvious: the guards on the group and its list come before the walk and
 * again after each record. g_mobyGroupCount does not change in the loop, so cc1
 * hoists its repeated test into a register known to be zero, which the loop
 * still tests (retail's `daddu $t0,$zero,$zero` and `bnez $t0`); upstream's
 * note.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_0043ECB0.c, function func_L20_00440398 (v1.01
 * address), author commit 1da2d1f3 (Fromnowhere). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the
 * statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_00440398 -> func_L20_004418D8; D_L20_001B2F9C ->
 *     g_mobyTableBase; D_L20_001B2FB8 -> g_mobyGroupCount; D_L20_001FBB20 ->
 *     g_mobyGroupLists
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
 *   - the duplicate declaration of g_mobyTableBase (upstream declares it twice)
 *     written once
 */
int func_L20_004418D8(int group) {
    unsigned short *rec = (unsigned short *)g_mobyGroupLists[group];
    int count = 0;

    if ((group & 0xFF) == 0xFF) {
        goto end;
    }
    if (g_mobyGroupCount < group) {
        goto end;
    }
    if (rec == 0) {
        goto end;
    }
    for (;;) {
        char *object = (char *)g_mobyTableBase + ((*rec & 0x7FFF) << 8);

        if (object == 0) {
            goto end;
        }
        if (*(unsigned char *)(object + 0x20) != 6 && *(signed char *)(object + 0x20) >= 0) {
            count++;
        }
        rec++;
        if ((group & 0xFF) == 0xFF) {
            goto end;
        }
        if (g_mobyGroupCount < group) {
            goto end;
        }
        if (rec == 0) {
            goto end;
        }
        if (*(short *)(rec - 1) >= 0) {
            continue;
        }
        if (rec == (unsigned short *)g_mobyGroupLists[group]) {
            continue;
        }
        break;
    }
end:
    return count;
}
