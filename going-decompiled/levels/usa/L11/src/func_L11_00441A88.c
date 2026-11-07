/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x00441A88, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the boot's
 * ee-gcc 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G8.
 */

typedef signed int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef float f32;

/* The level's 256-byte record bank and its id lists. Both are the
   names L11's boot-name map gives at those addresses (0x001B361C,
   0x001FC4A0; task #2646). The `sda`/`nosda` attributes are upstream's own text; this
   cc1 ignores both, and at -G8 the 4-byte pointer is $gp-relative by its
   size alone. */
extern u8 *g_mobyTableBase __attribute__((sda));
extern u8 *g_mobyGroupLists[];
extern u8 g_soundBankHandlesBlk[] __attribute__((nosda));

/**
 * func_L11_00441A88 - total up the live members of the object's group.
 *
 * Clears the count (state+0x140) and the weight sum (state+0x144). The
 * state's group word (+0xC0) names an id list in g_mobyGroupLists (0xFF or a
 * null list: return now); each halfword's low 15 bits index a 256-byte record
 * of the bank at g_mobyTableBase, and the walk ends after the halfword whose
 * sign bit is set. Every live record (state byte +0x20 neither 0xFE, 0xFD nor
 * 1) of one of the eight known classes (0xD98, 0xDCF, 0xDF4, 0xDF7, 0xDF9,
 * 0xD37, 0xD4A, 0xB3A) adds 1 to the count and its state's +0x20 float to the
 * sum. Finally state+0x148 = (float) the boot word at
 * g_soundBankHandlesBlk+0x24CC.
 *
 * @obj: the group's owner; its state is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_00440238-g8.c, function func_L11_00440238 (v1.01
 * address), author commit 68795b5b (Gianni). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2555 relverify):
 *   - func_L11_00440238 -> func_L11_00441A88 (our split name at the v2.00 place);
 *   - D_L11_001B359C -> g_mobyTableBase (L11's boot-map name at 0x001B361C);
 *   - D_L11_001FC420 -> g_mobyGroupLists (L11's boot-map name at 0x001FC4A0);
 *   - D_00189E20 -> g_soundBankHandlesBlk (the boot's name, same address);
 *   - D_L11_001B361C -> g_mobyTableBase, D_L11_001FC4A0 -> g_mobyGroupLists
 *     (task #2646): the names symbol_addrs/L11_boot_names.txt gives at exactly
 *     those addresses; the address-named spelling was a second name for the
 *     same address (A2 class, NOTE #13409).
 *   - `#include "include_asm.h"` dropped (nothing used it); the file's own
 *     typedefs are kept.
 */
void func_L11_00441A88(u8 *obj) {
    u8 *state = *(u8 **)(obj + 0x68);
    s32 index = *(s32 *)(state + 0xC0);
    u16 *cursor;
    u8 *base;
    u8 *list;

    *(s32 *)(state + 0x140) = 0;
    *(f32 *)(state + 0x144) = 0.0f;
    if (index == 0xFF) {
        return;
    }
    list = g_mobyGroupLists[index];
    if (list == 0) {
        return;
    }
    cursor = (u16 *)list;
    base = g_mobyTableBase;
    do {
        u16 value = *cursor;
        u8 *record = base + ((value & 0x7FFF) << 8);

        if (record != 0 && record[0x20] != 0xFE && record[0x20] != 0xFD && record[0x20] != 1) {
            s16 id = *(s16 *)(record + 0xAA);

            if (id == 0xD98 || id == 0xDCF || id == 0xDF4 || id == 0xDF7 ||
                id == 0xDF9 || id == 0xD37 || id == 0xD4A || id == 0xB3A) {
                *(s32 *)(state + 0x140) += 1;
                *(f32 *)(state + 0x144) += *(f32 *)(*(u32 *)(record + 0x68) + 0x20);
            }
        }
    } while (*(s16 *)cursor++ >= 0);
    list = g_soundBankHandlesBlk;
    *(f32 *)(state + 0x148) = (f32)*(s32 *)(list + 0x24CC);
}
