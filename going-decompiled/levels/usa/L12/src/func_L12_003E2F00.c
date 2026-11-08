/* Level 12 (Todano), USA v2.00: a level-only function built from C (task #2670,
 * row L12-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x003E2F00, in place of its split asm; the row is
 * going-decompiled/levels/usa/L12/config/L12_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;

/* The moby array base, by the name symbol_addrs/L12_boot_names.txt gives at
   exactly 0x001B2E1C. ADDRESSING-MODEL DEVICE (RULING #8620) on its extern
   declaration: upstream's `sda` (ignored by this cc1) is respelled
   `section(".sdata")`. cc1 then prints the load as the one-instruction `lw`
   macro, which the assembler expands at -G0 to the absolute lui/lw pair the
   ROM has. Ablation (task #2670, upstream's own `sda`, same arm and flags):
   99 words against the ROM's 97, 86 of them differing. It moves no data and
   emits nothing. */
extern s32 g_mobyTableBase __attribute__((section(".sdata")));
/* The per-group moby index lists, by the map's name at 0x001FB8E0. */
extern u16 *g_mobyGroupLists[];
/* The zone table at 0x001FEA40. No label starts there in our split (it lies
   inside a larger data label), so this extern is ADDRESS-NAMED and the level
   ELF values it from its name (RULING #13291 §2). */
extern s32 *D_L12_001FEA40[];
/* The level's copies of boot code, named by their v2.00 places. */
extern s32 func_L12_00320968(void *, void *, s32);
extern void func_L12_0031EFF8(void *, s32, s32, s32);
extern void func_L12_0031F2D8(void *, s32, s32);

/**
 * func_L12_003E2F00 - hand an object to the first class-0xFB4 moby of a group
 * whose zone contains it.
 *
 * Walks group @index's moby indices (low 15 bits index the 0x100-byte
 * records at g_mobyTableBase; the entry with bit 15 set is the last). For a
 * class-0xFB4 record (+0xAA), the zone is the D_L12_001FEA40 entry its vars
 * word +0x138 selects; when func_L12_00320968(@obj+0x10, zone+4, zone[0])
 * finds @obj's position inside it:
 *   - a record in state 9 (+0x20) clears flags 0x41 and sets 0x1000 in its
 *     halfword +0x34, copies +0x98 from the +0x10 word of the block its +0x24
 *     points to, switches animation (+0x43) to 5 if needed
 *     (func_L12_0031EFF8(rec, 5, 0, 10)) and moves to state 10
 *     (func_L12_0031F2D8(rec, 10, -1));
 *   - any other live record (state not 0xFE/0xFD) moves to state 11
 *     (func_L12_0031F2D8(rec, 11, 4)) and switches animation to 7 if needed
 *     (func_L12_0031EFF8(rec, 7, 0, 0x14));
 * then stores @obj at vars+0x374 and stops.
 *
 * @index: the group.
 * @obj: the object being handed over.
 * Return: the record that took @obj, or 0 (none, or no list for the group).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/12_todano/L12_00398E50.c, function
 * func_L12_003E1B28 (v1.01 address), author commit 8f02ea08 (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED; the body's statements unchanged, the edits, each name to
 * the target of the v2.00 disc's own relocation at this place (task #2670
 * relverify):
 *   - func_L12_003E1B28 -> func_L12_003E2F00 (our split name at the v2.00 place);
 *   - func_002A7470 -> func_L12_0031EFF8, func_002A7750 -> func_L12_0031F2D8,
 *     func_002A8AF0 -> func_L12_00320968 (our split names at the v2.00
 *     targets);
 *   - D_L12_001B2E9C -> g_mobyTableBase and D_L12_001FB960 ->
 *     g_mobyGroupLists, the names the level's boot-name map gives at the
 *     v2.00 targets;
 *   - D_L12_001FEAC0 -> D_L12_001FEA40, an address-named extern for a target
 *     inside a larger split data label (RULING #13291 §2);
 *   - device: `__attribute__((sda))` -> `__attribute__((section(".sdata")))`
 *     on g_mobyTableBase's extern declaration (RULING #8620; the audit's
 *     `resp` variant, NOTE #13281);
 *   - `#include "common.h"` dropped: the typedefs used are written here and
 *     only this function's declarations kept (upstream's file declares
 *     func_002A7470 and func_002A7750 twice each, with the same prototypes;
 *     one copy of each kept); upstream's file has no comment on this
 *     function.
 */
u8 *func_L12_003E2F00(s32 index, u8 *obj)
{
    u16 *list = g_mobyGroupLists[index];
    u16 *entry;
    u8 *found = 0;

    if (list == 0) {
        return 0;
    }
    entry = list;
    do {
        u8 *rec = (u8 *)g_mobyTableBase + (*entry & 0x7FFF) * 0x100;

        if (*(s16 *)(rec + 0xAA) == 0xFB4) {
            u8 *vars = *(u8 **)(rec + 0x68);
            s32 *zone = D_L12_001FEA40[*(s32 *)(vars + 0x138)];

            if (func_L12_00320968(obj + 0x10, zone + 4, zone[0]) != 0) {
                if (rec[0x20] == 9) {
                    *(u16 *)(rec + 0x34) = (*(u16 *)(rec + 0x34) & 0xFFBE) | 0x1000;
                    *(s32 *)(rec + 0x98) = *(s32 *)(*(u8 **)(rec + 0x24) + 0x10);
                    if (rec[0x43] != 5) {
                        func_L12_0031EFF8(rec, 5, 0, 10);
                    }
                    func_L12_0031F2D8(rec, 10, -1);
                } else if (rec != 0 && rec[0x20] != 0xFE && rec[0x20] != 0xFD) {
                    func_L12_0031F2D8(rec, 11, 4);
                    if (rec[0x43] != 7) {
                        func_L12_0031EFF8(rec, 7, 0, 0x14);
                    }
                }
                *(u8 **)(vars + 0x374) = obj;
                found = rec;
                break;
            }
        }
    } while ((s16)*entry++ >= 0);
    return found;
}
