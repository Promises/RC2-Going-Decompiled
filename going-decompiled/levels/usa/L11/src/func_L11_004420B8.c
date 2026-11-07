/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x004420B8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;
typedef unsigned int u32;

/* The level's id lists and its 256-byte record bank. Both are the
   names L11's boot-name map gives at those addresses (0x001FC4A0,
   0x001B361C; task #2646). The record-bank pointer keeps upstream's section(".sdata"). */
extern u8 *g_mobyGroupLists[];

extern u8 *g_mobyTableBase __attribute__((section(".sdata")));

extern void func_L11_003F4448(u8 *);

extern void func_L11_0043B190(u8 *);

/**
 * func_L11_004420B8 - find the free record of a class in the object's group
 * and start it.
 *
 * The state's group word (+0xC0) names an id list in g_mobyGroupLists (0xFF or
 * a null list: return 0). Each halfword's low 15 bits index a 256-byte record
 * of the bank at g_mobyTableBase; the walk ends after the halfword whose sign
 * bit is set. The first record that is live (state byte +0x20 neither 0xFE
 * nor 0xFD), of class @id (+0xAA) and in state 1 gets @obj as its owner
 * (+0xB8). Class 0xD4A is started by func_L11_0043B190 and counted at
 * state+0x118; class 0xB3A by func_L11_003F4448, counted at state+0x11C.
 *
 * @obj: the group's owner; its state is at +0x68.
 * @id: the class to look for.
 * Return: the started record, or 0 when none is found.
 *
 * Non-obvious: a matching record of any other class gets its owner set but is
 * not started, and the walk goes on.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_0043E1A0.c, function func_L11_00440868 (v1.01
 * address), author commit 96f56db3 (bigbeno37). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device; the section(".sdata") attribute is upstream's own text); the
 * respellings, each to the target of the v2.00 disc's own relocation at this
 * place (task #2555 relverify):
 *   - func_L11_00440868 -> func_L11_004420B8 (our split name at the v2.00 place);
 *   - D_L11_001FC420 -> g_mobyGroupLists (L11's boot-map name at 0x001FC4A0);
 *   - D_L11_001B359C -> g_mobyTableBase (L11's boot-map name at 0x001B361C);
 *   - func_L11_003F2F38 -> func_L11_003F4448, func_L11_00439950 -> func_L11_0043B190;
 *   - D_L11_001B361C -> g_mobyTableBase, D_L11_001FC4A0 -> g_mobyGroupLists
 *     (task #2646): the names symbol_addrs/L11_boot_names.txt gives at exactly
 *     those addresses; the address-named spelling was a second name for the
 *     same address (A2 class, NOTE #13409).
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the
 *     u8/u16/s16/s32/u32 typedefs common.h supplied are written here.
 */
u8 *func_L11_004420B8(u8 *obj, s32 id) {
    u8 *state = *(u8 **)(obj + 0x68);
    s32 index = *(s32 *)(state + 0xC0);
    u16 *cursor;
    u8 *record;
    u8 *list;

    if (index == 0xFF) {
        return 0;
    }
    list = g_mobyGroupLists[index];
    if (list == 0) {
        return 0;
    }
    cursor = (u16 *)list;
    do {
        u16 value = *cursor & 0x7FFF;
        record = g_mobyTableBase + (value << 8);

        if (record != 0 && record[0x20] != 0xFE && record[0x20] != 0xFD &&
            *(s16 *)(record + 0xAA) == id && record[0x20] == 1) {
            *(u32 *)(record + 0xB8) = (u32)obj;
            switch (id) {
            case 0xD4A:
                func_L11_0043B190(record);
                *(s32 *)(state + 0x118) += 1;
                return record;
            case 0xB3A:
                func_L11_003F4448(record);
                *(s32 *)(state + 0x11C) += 1;
                return record;
            }
        }
    } while (*(s16 *)cursor++ >= 0);
    return 0;
}
