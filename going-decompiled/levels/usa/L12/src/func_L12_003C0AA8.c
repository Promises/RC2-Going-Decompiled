/* Level 12 (Todano), USA v2.00: a level-only function built from C (task #2670,
 * row L12-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x003C0AA8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L12/config/L12_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed int s32;
typedef float f32;

/* The core table at 0x00188660, by the boot's own name for that address
   (symbol_addrs/usa, carried in the level's core_externs.txt). */
extern u8 g_listenerPosHistory[];
/* The level's copies of boot code, named by their v2.00 places. */
extern s32 func_L12_00321700(void *arg0, s32 arg1, s32 arg2);
extern s32 func_L12_00365ED8(s32 arg0, s32 arg1, void *arg2);
extern void func_L12_003150E0(void *arg0);
extern f32 func_L12_002F3548(f32 x, f32 y);

/**
 * func_L12_003C0AA8 - per-frame update of a spinning object that keeps a
 * sound slot.
 *
 * On first update (state byte +0x20 == 0) sets the state to 1, +0x30 to
 * 0x20, the halfword +0x32 to 0x20 and the slot index (state word 0) to -1.
 * Every update advances the angle at +0xF8 by 0.017453294 (one degree in
 * radians) through func_L12_002F3548 (the level's copy of WrapAnglePiSum),
 * then restarts the sound with func_L12_00365ED8(0, 4, object) (its copy of
 * PlayMobySound) when the 0x70-byte record the index selects in
 * g_listenerPosHistory no longer names this object (+0x88) or is inactive
 * (+0x74 == 0). When func_L12_00321700(object, 0x10000, 0) is nonzero the
 * object is freed (func_L12_003150E0, its copy of FreeMoby). The copies'
 * identities are the names symbol_addrs/L12_boot_names.txt gives at those
 * addresses.
 *
 * @object: the object; its state block is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/12_todano/L12_00398E50.c, function
 * func_L12_003BF7D0 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; the respellings, each to
 * the target of the v2.00 disc's own relocation at this place (task #2670
 * relverify):
 *   - func_L12_003BF7D0 -> func_L12_003C0AA8 (our split name at the v2.00 place);
 *   - func_2F2950 -> func_L12_002F3548, func_364DF8 -> func_L12_00365ED8,
 *     func_3205F8 -> func_L12_00321700, func_314008 -> func_L12_003150E0
 *     (our split names at the v2.00 targets);
 *   - D_L12_00188660 -> g_listenerPosHistory (the boot's name at 0x00188660,
 *     a core address below the level image);
 *   - `#include "common.h"` dropped: the typedefs used are written here and
 *     only this function's declarations kept; upstream's one-line comment
 *     folded into this one.
 */
void func_L12_003C0AA8(u8 *object) {
    u8 *state = *(u8 **)(object + 0x68);
    u8 *record;
    s32 index;

    if (*(u8 *)(object + 0x20) == 0) {
        *(u8 *)(object + 0x20) = 1;
        *(u8 *)(object + 0x30) = 0x20;
        *(u16 *)(object + 0x32) = 0x20;
        *(s32 *)(state + 0) = -1;
    }
    *(f32 *)(object + 0xF8) = func_L12_002F3548(*(f32 *)(object + 0xF8), 0.017453294f);
    index = *(s32 *)(state + 0);
    record = g_listenerPosHistory + index * 0x70;
    if (*(u8 **)(record + 0x88) != object || *(u8 *)(record + 0x74) == 0)
        *(s32 *)(state + 0) = func_L12_00365ED8(0, 4, object);
    if (func_L12_00321700(object, 0x10000, 0) != 0)
        func_L12_003150E0(object);
}
