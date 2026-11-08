/* Level 12 (Todano), USA v2.00: a level-only function built from C (task #2670,
 * row L12-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x003E1DA0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L12/config/L12_c.tsv, the arm ee-gcc
 * 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G8, assembled by plain
 * GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;

/* The moby array base and the per-group moby index lists, by the names
   symbol_addrs/L12_boot_names.txt gives at exactly 0x001B2E1C and
   0x001FB8E0. The base is read through $gp at -G8 (upstream's `sda` text is
   ignored by this cc1). */
extern u8 *g_mobyTableBase __attribute__((sda));
extern u16 *g_mobyGroupLists[];

/**
 * func_L12_003E1DA0 - whether every class-0xBB8 moby of a group is in state 1.
 *
 * Walks group @index's list of moby indices (each entry's low 15 bits index
 * the 0x100-byte records at g_mobyTableBase; the entry with bit 15 set is the
 * last). Any record of class 0xBB8 (+0xAA) whose state byte (+0x20) is not 1
 * ends the walk.
 *
 * @index: the group.
 * Return: 0 if such a moby was found; 1 otherwise, including when the group
 * has no list.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/12_todano/L12_00398E50-g8.c, function
 * func_L12_003E09C8 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; the respellings, each to
 * the target of the v2.00 disc's own relocation at this place (task #2670
 * relverify):
 *   - func_L12_003E09C8 -> func_L12_003E1DA0 (our split name at the v2.00 place);
 *   - D_L12_001B2E9C -> g_mobyTableBase and D_L12_001FB960 ->
 *     g_mobyGroupLists, the names the level's boot-name map gives at the
 *     v2.00 targets (both lie inside larger split data labels, so no
 *     address-named extern is needed);
 *   - `#include "common.h"` dropped: the typedefs used are written here and
 *     only this function's declarations kept; upstream's one-line comment
 *     folded into this one.
 */
s32 func_L12_003E1DA0(s32 index) {
    u16 *entry = g_mobyGroupLists[index];
    u16 packed;
    u8 *base;

    if (entry == 0)
        return 1;
    base = g_mobyTableBase;
    do {
        u8 *record;
        packed = *entry;
        record = base + (packed & 0x7FFF) * 0x100;
        if (*(s16 *)(record + 0xAA) == 0xBB8) {
            if (*(u8 *)(record + 0x20) != 1)
                return 0;
        }
        entry++;
    } while ((s16)packed >= 0);
    return 1;
}
