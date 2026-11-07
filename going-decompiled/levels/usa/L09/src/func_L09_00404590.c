/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (task #2561,
 * RULING #13291 row L09-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x00404590, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
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

/* The per-group tables of entity ids, at 0x001FB4E0 inside our split label
   D_L09_001F0000 (an address-named extern, RULING #13291 §2). Absolute. */
extern char *D_L09_001FB4E0[];

/* The level's copy of a boot helper, and this row's C peer (upstream
   defined it above this function in the same file). */
extern s32 func_L09_00318B08(s32 a0, s32 a1);
extern void func_L09_00404510(char *a0);

/**
 * func_L09_00404590 - on an event, wake the 0xD57-class peers of the object's
 * group and claim the first free one.
 *
 * Nothing happens for event 0. Otherwise runs func_L09_00404510 (the wake
 * pass). Only for events 4 and 5, a group byte (+0x21) other than 0xFF and a
 * group of at least two members (func_L09_00318B08(group, -1) >= 2): walks
 * the group's id list D_L09_001FB4E0[group] up to the first negative id, and
 * the first 0xD57-class (+0xAA) entity whose state halfword +0x26A is clear
 * gets it set to 1, ending the walk.
 *
 * @a0: the object.
 * @a1: the event code.
 *
 * Non-obvious: the 4..5 range test is one unsigned compare,
 * `(u32)(a1 - 4) >= 2`.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/09_dobbo/L09_003FEA48.c, function func_L09_00403100 (v1.01
 * address), author commit bcc9f7db (bigbeno37). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the body's
 * statements unchanged, the edits, each name to the target of the v2.00
 * disc's own relocation at this place (task #2561 relverify):
 *   - func_L09_00403100 -> func_L09_00404590 (our split name at the v2.00 place);
 *   - func_L09_00403080 -> func_L09_00404510 (this row's C, its own TU here,
 *     so it is declared extern with upstream's definition's prototype);
 *   - func_002A8508 -> func_L09_00318B08;
 *   - D_L09_001B285C -> D_L09_001B27DC: an address-named extern inside
 *     D_L09_001B2000 (RULING #13291 §2);
 *   - D_L09_001FFFB560 -> D_L09_001FB4E0: an address-named extern inside
 *     D_L09_001F0000 (RULING #13291 §2; upstream's name carries a stray `FF`);
 *   - device: `__attribute__((sda))` -> `__attribute__((section(".sdata")))`
 *     on D_L09_001B27DC's extern declaration (RULING #8620; the audit's
 *     `resp` variant, NOTE #13281);
 *   - `#include "common.h"` dropped: its u8/u16/u32/s16/s32 typedefs are
 *     written here and only the declarations this function uses are kept.
 */
void func_L09_00404590(char *a0, s32 a1) {
    char *s1 = a0;
    char *base;
    u16 *p;
    char *rec;
    char *sub;

    if (a1 == 0) {
        return;
    }
    func_L09_00404510(a0);
    if ((u32)(a1 - 4) >= 2) {
        return;
    }
    if (*(u8 *)(s1 + 0x21) == 0xFF) {
        return;
    }
    if (func_L09_00318B08(*(u8 *)(s1 + 0x21), -1) < 2) {
        return;
    }
    base = D_L09_001B27DC;
    p = (u16 *)D_L09_001FB4E0[*(u8 *)(s1 + 0x21)];
    do {
        rec = base + ((*p & 0x7FFF) << 8);
        if (*(s16 *)(rec + 0xAA) == 0xD57) {
            sub = *(char **)(rec + 0x68);
            if (*(s16 *)(sub + 0x26A) == 0) {
                *(s16 *)(sub + 0x26A) = 1;
                return;
            }
        }
    } while ((s16)*p++ >= 0);
}
