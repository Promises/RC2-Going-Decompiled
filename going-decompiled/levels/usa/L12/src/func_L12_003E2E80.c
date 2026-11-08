/* Level 12 (Todano), USA v2.00: a level-only function built from C (task #2670,
 * row L12-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x003E2E80, in place of its split asm; the row is
 * going-decompiled/levels/usa/L12/config/L12_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column). -G8 is the row's gflag setting,
 * opposite to upstream's -G0 for this file: at -G8 the moby array base is
 * read through $gp as the ROM does. Ablation (task #2670, the same TU at
 * -G0): 33 words against the ROM's 31, 26 of them differing.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;

/* The moby array base and the per-group moby index lists, by the names
   symbol_addrs/L12_boot_names.txt gives at exactly 0x001B2E1C and
   0x001FB8E0 (upstream's `sda` text is kept; this cc1 ignores it). */
extern s32 g_mobyTableBase __attribute__((sda));
extern u16 *g_mobyGroupLists[];

/**
 * func_L12_003E2E80 - flag every live class-0xFB4 moby of an object's group.
 *
 * Does nothing when the object's group byte (+0x21) is 0xFF or the group has
 * no list. Otherwise walks the group's moby indices (low 15 bits index the
 * 0x100-byte records at g_mobyTableBase; the entry with bit 15 set is the
 * last) and, for each record whose state byte (+0x20) is not negative and
 * whose class (+0xAA) is 0xFB4, sets the halfword at +0x366 of its vars
 * block to 1.
 *
 * @a0: the object, as an address.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/12_todano/L12_00398E50.c, function
 * func_L12_003E1AA8 (v1.01 address), author commit ee3fee45 (llesieur99),
 * which carried 15 of its 21 lines from the same author's
 * func_L12_00427D50 in src/levels/shared/L12_003E1CB0.c (91e04732; a
 * near-twin that tests class 0x11EB and sets +0x336) and wrote the class
 * test, the store and the loop tail (`git blame -M -C -C -C` at the pin).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED; the body's statements unchanged, the edits, each name to
 * the target of the v2.00 disc's own relocation at this place (task #2670
 * relverify):
 *   - func_L12_003E1AA8 -> func_L12_003E2E80 (our split name at the v2.00 place);
 *   - D_L12_001B2E9C -> g_mobyTableBase and D_L12_001FB960 ->
 *     g_mobyGroupLists, the names the level's boot-name map gives at the
 *     v2.00 targets (both lie inside larger split data labels, so no
 *     address-named extern is needed);
 *   - gflag -G8 against upstream's -G0 (the audit's code G, NOTE #13281; a
 *     tsv setting, no source device);
 *   - `#include "common.h"` dropped: the typedefs used are written here and
 *     only this function's declarations kept; upstream's one-line comment
 *     folded into this one.
 */
void func_L12_003E2E80(s32 a0) {
    s32 base;
    s16 *p;
    s16 *q;

    if (*(u8 *)(a0 + 0x21) == 0xFF)
        return;
    q = g_mobyGroupLists[*(u8 *)(a0 + 0x21)];
    if (q == 0)
        return;
    p = q;
    base = g_mobyTableBase;
    do {
        u32 v = (*(u16 *)p & 0x7FFF) << 8;

        if (*(s8 *)(base + v + 0x20) >= 0) {
            if (*(s16 *)(base + v + 0xAA) == 0xFB4)
                *(u16 *)(*(u32 *)(base + v + 0x68) + 0x366) = 1;
        }
    } while (*p++ >= 0);
}
