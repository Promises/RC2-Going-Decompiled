/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (task #2561,
 * RULING #13291 row L09-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x00408030, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm ee-gcc
 * 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

typedef unsigned char u8;
typedef signed short s16;
typedef signed int s32;

/* The level's entity table base, g_mobyTableBase: the name L09's boot-name
   map (symbol_addrs/L09_boot_names.txt) gives 0x001B27DC (task #2646).
   ADDRESSING-MODEL DEVICE (RULING #8620): upstream's
   `sda` (ignored by this cc1) is respelled `section(".sdata")` on this extern
   declaration. cc1 then prints the load as the one-instruction `lw` macro,
   which the assembler expands to an ADJACENT absolute lui/lw pair
   (R_MIPS_HI16/LO16, not $gp-relative), as the ROM has it. With upstream's
   `sda` instead, cc1 splits %hi/%lo itself and the function keeps its length
   but is scheduled and allocated differently (ablation, task #2561: 24 changed
   lines in the disassembly diff). It moves no data and emits nothing. */
extern char *g_mobyTableBase __attribute__((section(".sdata")));

/**
 * func_L09_00408030 - set the colour key of one entity of class 0x102 or
 * 0xDE0.
 *
 * Index -1 does nothing. Otherwise takes the 0x100-byte entity slot @a0 from
 * g_mobyTableBase; a slot whose state byte (+0x20) is 0xFE or 0xFD is left
 * alone, and one of class (+0xAA) 0x102 or 0xDE0 gets byte +0x95 set to @a1.
 *
 * @a0: the entity index, or -1.
 * @a1: the key byte.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/09_dobbo/L09_003FEA48.c, function func_L09_00406BA0 (v1.01
 * address), author commit 264cb2be (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the body's
 * statements unchanged, the edits, each name to the target of the v2.00
 * disc's own relocation at this place (task #2561 relverify):
 *   - func_L09_00406BA0 -> func_L09_00408030 (our split name at the v2.00 place);
 *   - D_L09_001B285C -> g_mobyTableBase (L09's boot-map name at the v2.00
 *     target 0x001B27DC);
 *   - D_L09_001B27DC -> g_mobyTableBase (task #2646): the name
 *     symbol_addrs/L09_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2 class,
 *     NOTE #13409).
 *   - device: `__attribute__((sda))` -> `__attribute__((section(".sdata")))`
 *     on g_mobyTableBase's extern declaration (RULING #8620; the audit's
 *     `resp` variant, NOTE #13281);
 *   - `#include "common.h"` dropped: its u8/s16/s32 typedefs are written here
 *     and only the declarations this function uses are kept.
 */
void func_L09_00408030(s32 a0, u8 a1) {
    char *v1;
    if (a0 == -1) {
        return;
    }
    v1 = g_mobyTableBase;
    v1 += a0 << 8;
    if (v1 == 0) {
        return;
    }
    if (*(u8 *)(v1 + 0x20) == 0xFE) {
        return;
    }
    if (*(u8 *)(v1 + 0x20) == 0xFD) {
        return;
    }
    if (*(s16 *)(v1 + 0xAA) == 0x102 || *(s16 *)(v1 + 0xAA) == 0xDE0) {
        *(u8 *)(v1 + 0x95) = a1;
    }
}
