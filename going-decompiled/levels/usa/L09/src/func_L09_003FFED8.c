/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (row L09-B,
 * task #2562, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003FFED8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, plain as.
 */

typedef signed short s16;
typedef int s32;
typedef float f32;

/* The resident mode word: upstream's D_L09_001A79F0, our g_playerProgress at
 * the v2.00 address 0x001A79F8. $gp-relative at -G8 by its size alone; the
 * `sda` attribute text is kept verbatim (this cc1 ignores it, a warning). */
extern s32 g_playerProgress __attribute__((sda));
/* The resident release word, at the same address in both builds. */
extern s32 D_L09_001A9040 __attribute__((sda));
/* The level's record table: upstream's D_L09_001B285C, whose v2.00 target
 * 0x001B27DC L09's boot-name map (symbol_addrs/L09_boot_names.txt) names
 * g_mobyTableBase (task #2646). */
extern char *g_mobyTableBase __attribute__((sda));

extern s32 func_L09_00324590(char *a0, char *a1, s32 a2);
extern char *func_L09_003280E0(char *a0);
extern void func_L09_003FFE08(char *a0);
extern void func_L09_003275B8(char *a0, s32 a1, s32 a2, s32 a3, f32 f12, f32 f13);

/**
 * func_L09_003FFED8 - drop the record the sub-object points at, once, then
 * rebuild its effect with both blend weights at zero.
 *
 * When func_L09_00324590(@a0, state+0x160, 0) reports the drop point and
 * g_playerProgress is 9, the state's s16 index at +0x1D0 is used once into
 * the record table g_mobyTableBase (records are 0x100 bytes): if
 * func_L09_003280E0(record) returns 0 the release word D_L09_001A9040 is
 * cleared, and the index is reset to -1 either way. Then func_L09_003FFE08
 * runs the rebuild, and every path ends in
 * func_L09_003275B8(@a0, state's word +0x448, 0, 0, 0, 0).
 *
 * @a0: the object (its state is at *(char **)(a0+0x68)).
 * Return: none.
 *
 * Non-obvious: the release word is cleared only when the record's own release
 * helper returns 0, but the index is invalidated unconditionally. No codegen
 * or addressing device is used.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_003FEA48-g8.c, function func_L09_003FEA48
 * (v1.01 address), author commit 09484b1f (Louis-Philippe Le Sieur). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (BYTE-IDENTICAL), each name to the target of the
 * v2.00 disc's own relocation at this place (task #2562 relverify by name):
 *   - func_L09_003FEA48 -> func_L09_003FFED8 (our split name);
 *   - D_L09_001A79F0 -> g_playerProgress (the boot's name at 0x001A79F8);
 *   - D_L09_001A9040 -> D_L09_001A9040 (same address in both builds);
 *   - D_L09_001B285C -> g_mobyTableBase (L09's boot-map name at the v2.00
 *     target 0x001B27DC);
 *   - D_L09_001B27DC -> g_mobyTableBase (task #2646): the name
 *     symbol_addrs/L09_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2 class,
 *     NOTE #13409).
 *   - func_323340 -> func_L09_00324590, func_326368 -> func_L09_003275B8,
 *     func_L09_00326E90 -> func_L09_003280E0,
 *     func_L09_003FE978 -> func_L09_003FFE08 (L09's own functions at the
 *     v2.00 places);
 *   - `#include "common.h"` dropped: the typedefs this function uses are
 *     written here and only its declarations kept.
 */
void func_L09_003FFED8(char *a0) {
    char *s0 = *(char **)(a0 + 0x68);
    s16 idx;

    if (func_L09_00324590(a0, s0 + 0x160, 0) != 0) {
        if (g_playerProgress == 9) {
            idx = *(s16 *)(s0 + 0x1D0);
            if (idx != -1) {
                if (func_L09_003280E0(g_mobyTableBase + (idx << 8)) == 0) {
                    D_L09_001A9040 = 0;
                }
                *(s16 *)(s0 + 0x1D0) = -1;
            }
        }
        func_L09_003FFE08(a0);
    }
    func_L09_003275B8(a0, *(s32 *)(s0 + 0x448), 0, 0, 0.0f, 0.0f);
}
