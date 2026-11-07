/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (task #2561,
 * RULING #13291 row L09-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x003FFE08, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack.
 */

/* The level's group count, at 0x001B27F8 inside our split label
   D_L09_001B2000: an address-named extern (RULING #13291 §2), valued from its
   address by level_elf. Upstream's `sda` is respelled `section(".sdata")`
   with the declaration below (RULING #8620); for this 4-byte int it changes
   nothing: at -G8 it is $gp-relative by its size either way. */
extern int D_L09_001B27F8 __attribute__((section(".sdata")));

/* The level's entity table base, at 0x001B27DC inside D_L09_001B2000 (an
   address-named extern, RULING #13291 §2). ADDRESSING-MODEL DEVICE (RULING
   #8620): upstream's `sda` (ignored by this cc1) is respelled
   `section(".sdata")` on this extern declaration. Declared 12 bytes, it is
   absolute at -G8 either way; with the attribute cc1 prints the load as the
   one-instruction `lw` macro, which the assembler expands to an ADJACENT
   absolute lui/lw pair (R_MIPS_HI16/LO16), as the ROM has it. With upstream's
   `sda` instead, cc1 splits %hi/%lo itself and the function keeps its length
   but is scheduled and allocated differently (ablation, task #2561: 8 changed
   lines in the disassembly diff). It moves no data and emits nothing. */
extern unsigned char *D_L09_001B27DC[3] __attribute__((section(".sdata")));

/* The per-group tables of entity ids, at 0x001FB4E0 inside our split label
   D_L09_001F0000 (an address-named extern, RULING #13291 §2). Absolute. */
extern unsigned short *D_L09_001FB4E0[];

/**
 * func_L09_003FFE08 - flag the object's peers: set byte +0x440 in the state
 * of every entity of the object's group whose word +0x310 equals the
 * object's own.
 *
 * Nothing happens when the object's group byte (+0x21) or the word +0x310 of
 * its state is 0xFF. Otherwise walks the group's id list
 * D_L09_001FB4E0[group] (each id masked 0x7FFF indexes a 0x100-byte entity
 * slot from D_L09_001B27DC[0]) while the group is at most D_L09_001B27F8,
 * stopping after the first negative id or at a null entity.
 *
 * @a0: the object; its state is at +0x68.
 *
 * Non-obvious: the state pointer is loaded ahead of the first test, as the
 * ROM does; the loop condition re-tests the group and the list head on every
 * step exactly as written upstream.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/09_dobbo/L09_002F8BA0-g8.c, function func_L09_003FE978 (v1.01
 * address), author commit 132e23b5 (bigmak94). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the body's
 * statements unchanged, the edits, each name to the target of the v2.00
 * disc's own relocation at this place (task #2561 relverify):
 *   - func_L09_003FE978 -> func_L09_003FFE08 (our split name at the v2.00 place);
 *   - D_L09_001B285C_1 -> D_L09_001B27DC and D_L09_001B2878 -> D_L09_001B27F8:
 *     address-named externs inside D_L09_001B2000 (RULING #13291 §2);
 *   - D_L09_001FB560 -> D_L09_001FB4E0: an address-named extern inside
 *     D_L09_001F0000 (RULING #13291 §2);
 *   - device: `__attribute__((sda))` -> `__attribute__((section(".sdata")))`
 *     on both data extern declarations (RULING #8620; the audit's `resp`
 *     variant, NOTE #13281); it changes the code only through
 *     D_L09_001B27DC (see its declaration);
 *   - `#include "common.h"` dropped: only the declarations this function uses
 *     are kept, and upstream's comments are folded into this one.
 */
void func_L09_003FFE08(unsigned char *a0) {
    unsigned char *state;
    unsigned short *p;
    unsigned char *e;
    unsigned char *s;

    state = *(unsigned char **)(a0 + 0x68);
    if (a0[0x21] == 0xFF)
        return;
    if (*(int *)(state + 0x310) == 0xFF)
        return;
    for (p = D_L09_001FB4E0[a0[0x21]];
         a0[0x21] != 0xFF && a0[0x21] <= D_L09_001B27F8 && p != 0 &&
         ((p != 0 && (short)p[-1] >= 0) || p == D_L09_001FB4E0[a0[0x21]]);
         p++) {
        e = D_L09_001B27DC[0] + ((*p & 0x7FFF) << 8);
        if (e == 0)
            break;
        s = *(unsigned char **)(e + 0x68);
        if (s != 0) {
            if (*(int *)(s + 0x310) == *(int *)(state + 0x310))
                s[0x440] = 1;
        }
    }
}
