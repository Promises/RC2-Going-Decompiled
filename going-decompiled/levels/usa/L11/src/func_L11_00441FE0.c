/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x00441FE0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack.
 */

typedef signed int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef float f32;

/* The level's 256-byte record bank and its id lists. Both are the
   names L11's boot-name map gives at those addresses (0x001B361C,
   0x001FC4A0; task #2646). The `sda` attribute is upstream's own text; this cc1 ignores
   it, and at -G8 the 4-byte pointer is $gp-relative by its size alone. */
extern u8 *g_mobyTableBase __attribute__((sda));
extern u8 *g_mobyGroupLists[];

/* The two class handlers: the level function at 0x004421C0 and our C
   func_L11_004424E0 (src/func_L11_004424E0.c). */
extern s32 func_L11_004421C0(u8 *obj, u8 *record);
extern s32 func_L11_004424E0(u8 *obj, u8 *record);

/**
 * func_L11_00441FE0 - find a free record of a class in the object's group and
 * hand it to the group handler.
 *
 * Same walk as func_L11_004420B8: the state's group word (+0xC0) names an id
 * list in g_mobyGroupLists, whose halfwords index 256-byte records of the bank
 * at g_mobyTableBase until one has its sign bit set. The first live record
 * (+0x20 neither 0xFE nor 0xFD) of class @id (+0xAA) in state 1 gets @obj as
 * its owner (+0xB8) and goes to func_L11_004421C0 when @flag is non-zero, else
 * to func_L11_004424E0.
 *
 * @obj: the group's owner; its state is at +0x68.
 * @id: the class to look for.
 * @flag: selects the handler.
 * Return: the handler's result, or 0 when no record matched.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_00440238-g8.c, function func_L11_00440790 (v1.01
 * address), author commit 68795b5b (Gianni). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2555 relverify):
 *   - func_L11_00440790 -> func_L11_00441FE0 (our split name at the v2.00 place);
 *   - D_L11_001B359C -> g_mobyTableBase (L11's boot-map name at 0x001B361C);
 *   - D_L11_001FC420 -> g_mobyGroupLists (L11's boot-map name at 0x001FC4A0);
 *   - func_L11_00440970 -> func_L11_004421C0, func_L11_00440C90 -> func_L11_004424E0;
 *   - D_L11_001B361C -> g_mobyTableBase, D_L11_001FC4A0 -> g_mobyGroupLists
 *     (task #2646): the names symbol_addrs/L11_boot_names.txt gives at exactly
 *     those addresses; the address-named spelling was a second name for the
 *     same address (A2 class, NOTE #13409).
 *   - `#include "include_asm.h"` dropped (nothing used it); the file's own
 *     typedefs are kept, and its `nosda` D_00189E20 declaration (used only by
 *     the file's other function) is omitted.
 */
s32 func_L11_00441FE0(u8 *obj, s32 id, s32 flag) {
    u8 *state = *(u8 **)(obj + 0x68);
    s32 index = *(s32 *)(state + 0xC0);
    u16 *cursor;
    u8 *record;
    u8 *base;
    u8 *list;

    if (index == 0xFF) {
        return 0;
    }
    list = g_mobyGroupLists[index];
    if (list == 0) {
        return 0;
    }
    cursor = (u16 *)list;
    base = g_mobyTableBase;
    do {
        record = base + ((*cursor & 0x7FFF) << 8);

        if (record != 0 && record[0x20] != 0xFE && record[0x20] != 0xFD &&
            *(s16 *)(record + 0xAA) == id && record[0x20] == 1) {
            *(u32 *)(record + 0xB8) = (u32)obj;
            if (flag != 0) {
                return func_L11_004421C0(obj, record);
            }
            return func_L11_004424E0(obj, record);
        }
    } while (*(s16 *)cursor++ >= 0);
    return 0;
}
