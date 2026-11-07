/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (task #2561,
 * RULING #13291 row L09-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x00409680, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm ee-gcc
 * 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

typedef signed char s8;
typedef signed int s32;

/* The level's entity table base, at 0x001B27DC inside our split label
   D_L09_001B2000: an address-named extern (RULING #13291 §2), valued from its
   address by level_elf. ADDRESSING-MODEL DEVICE (RULING #8620): upstream's
   `sda` (ignored by this cc1) is respelled `section(".sdata")` on this extern
   declaration. cc1 then prints the load as the one-instruction `lw` macro,
   which the assembler expands to an ADJACENT absolute lui/lw pair
   (R_MIPS_HI16/LO16, not $gp-relative), as the ROM has it. With upstream's
   `sda` instead, cc1 splits %hi/%lo itself and the function keeps its length
   but is scheduled and allocated differently (ablation, task #2561: 16 changed
   lines in the disassembly diff). It moves no data and emits nothing. */
extern char *D_L09_001B27DC __attribute__((section(".sdata")));

/* The level's entity colour-key setter (still split asm at its place). */
extern void func_L09_004095C8(char *a0, char *a1, s32 a2, s32 a3, s32 a4);

/**
 * func_L09_00409680 - hand the record's colour key to one entity, negated
 * unless the caller asks for it as is.
 *
 * Takes the 0x100-byte entity slot @a1 from D_L09_001B27DC (a null slot does
 * nothing) and calls func_L09_004095C8(a0, slot, key...) with the three
 * signed key bytes of @a2 at +0xBC, +0xBD and +0xBF; each is negated when
 * @a3 is 0.
 *
 * @a0: the record (passed through).
 * @a1: the entity index.
 * @a2: the key source.
 * @a3: nonzero to pass the key unchanged, 0 to negate it.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/09_dobbo/L09_003FEA48.c, function func_L09_004081F0 (v1.01
 * address), author commit 96fa46bb (Louis-Philippe Le Sieur), which
 * introduced the body (validator #2585; at 264cb2be, cited here before, the
 * function was still INCLUDE_ASM). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the body's
 * statements unchanged, the edits, each name to the target of the v2.00
 * disc's own relocation at this place (task #2561 relverify):
 *   - func_L09_004081F0 -> func_L09_00409680 (our split name at the v2.00 place);
 *   - func_L09_00408138 -> func_L09_004095C8;
 *   - D_L09_001B285C -> D_L09_001B27DC: an address-named extern inside
 *     D_L09_001B2000 (RULING #13291 §2);
 *   - device: `__attribute__((sda))` -> `__attribute__((section(".sdata")))`
 *     on D_L09_001B27DC's extern declaration (RULING #8620; the audit's
 *     `resp` variant, NOTE #13281);
 *   - `#include "common.h"` dropped: its s8/s32 typedefs are written here and
 *     only the declarations this function uses are kept, and upstream's
 *     function comment is folded into this one.
 */
void func_L09_00409680(char *a0, s32 a1, char *a2, s32 a3) {
    char *v1 = a2;
    char *p = D_L09_001B27DC + (a1 << 8);

    if (p == 0) {
        return;
    }
    if (a3 != 0) {
        func_L09_004095C8(a0, p, *(s8 *)(v1 + 0xBC), *(s8 *)(v1 + 0xBD),
                          *(s8 *)(v1 + 0xBF));
    } else {
        func_L09_004095C8(a0, p, -*(s8 *)(v1 + 0xBC), -*(s8 *)(v1 + 0xBD),
                          -*(s8 *)(v1 + 0xBF));
    }
}
