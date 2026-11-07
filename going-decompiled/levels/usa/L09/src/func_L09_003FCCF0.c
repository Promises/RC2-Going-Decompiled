/* Level 09 (Dobbo), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2580, RULING #13291 row L09-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x003FCCF0, in place of its
 * split asm; the row is going-decompiled/levels/usa/L09/config/L09_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column; the plain-`as` ablation is in the row's FACT, task #2580).
 */

typedef unsigned char u8;

/* Upstream's MACRO_ADDR, from its include/common.h: a section attribute on an
 * extern DECLARATION, an ADDRESSING-MODEL DEVICE (RULING #8620) that is
 * upstream's own text. It moves no data and emits nothing, and on these
 * incomplete arrays it does not make the access $gp-relative: cc1 prints the
 * one-instruction `la` macro for each table's address, and the assembler
 * expands it to an absolute lui/addiu pair (R_MIPS_HI16/LO16), as the ROM has
 * it. Ablation (task #2580): with MACRO_ADDR defined empty in both TUs
 * (func_L09_003FCC18 and func_L09_003FCCF0), cc1 splits %hi/%lo itself and
 * level .text differs in 91 bytes, first at 0x003FCC1C. */
#define MACRO_ADDR __attribute__((section(".sdata")))

/* Three per-slot word tables at 0x001B3560, 0x001B3660 and 0x001B36A0 inside
 * our split label D_L09_001B2B70, and the 16-slot pair table at 0x00255E10
 * inside D_L09_00240000: address-named externs (RULING #13291 section 2),
 * valued from their names by level_elf. */
extern int D_L09_001B3560[] MACRO_ADDR;
extern int D_L09_001B3660[] MACRO_ADDR;
extern int D_L09_001B36A0[] MACRO_ADDR;
extern int D_L09_00255E10[];

/* L09's copy of the boot's func_00291450 (releases a handle). */
extern void func_L09_002F9EC0(int);

/**
 * func_L09_003FCCF0 - free slot @index of the level's pair table.
 *
 * Clears the slot's D_L09_001B3560 word and both of its ids in
 * D_L09_00255E10, then releases each of its two handles in D_L09_001B3660 and
 * D_L09_001B36A0 that is not -1 (setting it to -1). The by-index twin of
 * func_L09_003FCC18.
 *
 * @index: the slot, 0..15.
 *
 * Non-obvious: the second id is cleared through a byte-offset pointer
 * (index * 8 + 4) after the first handle is read, the order upstream writes
 * and the ROM schedules.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_003FB860.c, function func_L09_003FB860 (v1.01
 * address), author commit 88c5f521 (Flavius). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the body's
 * statements unchanged; the edits, each name to the target of the v2.00
 * disc's own relocation at this place (task #2580 relverify):
 *   - func_L09_003FB860 -> func_L09_003FCCF0 (our split name);
 *   - D_L09_00255E90 -> D_L09_00255E10, D_L09_001B35E0 -> D_L09_001B3560,
 *     D_L09_001B36E0 -> D_L09_001B3660, D_L09_001B3720 -> D_L09_001B36A0:
 *     address-named externs (RULING #13291 section 2);
 *   - callee func_00291450 -> func_L09_002F9EC0 (L09's copy);
 *   - built at -G8 against upstream's -G0 (the audit's code G, NOTE #13281);
 *   - kept from upstream: MACRO_ADDR (section(".sdata")) on the three word
 *     tables, a RULING #8620 device in upstream's own text (ablation above);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped; the u8
 *     typedef and the MACRO_ADDR definition copied in; brace moved onto the
 *     signature line (whitespace only).
 */
void func_L09_003FCCF0(int index) {
    int h;

    D_L09_001B3560[index] = 0;
    D_L09_00255E10[index * 2] = 0;
    h = D_L09_001B3660[index];
    *(int *)((u8 *)D_L09_00255E10 + index * 8 + 4) = 0;
    if (h != -1) {
        func_L09_002F9EC0(h);
        D_L09_001B3660[index] = -1;
    }
    if (D_L09_001B36A0[index] != -1) {
        func_L09_002F9EC0(D_L09_001B36A0[index]);
        D_L09_001B36A0[index] = -1;
    }
}
