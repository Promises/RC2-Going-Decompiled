/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (task #2561,
 * RULING #13291 row L09-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x00408120, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm ee-gcc
 * 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G8.
 */

typedef unsigned char u8;
typedef signed int s32;

/* The level's entity table base, at 0x001B27DC inside our split label
   D_L09_001B2000: an address-named extern (RULING #13291 §2), valued from its
   address by level_elf. `sda` is upstream's own text, ignored by this cc1
   ("attribute directive ignored"): at -G8 the 4-byte pointer is
   $gp-relative by its size alone. */
extern char *D_L09_001B27DC __attribute__((sda));

/**
 * func_L09_00408120 - release the entity the object's state points at.
 *
 * Reads the entity index at state+0x5C; unless it is -1, sets byte +0xBC of
 * that 0x100-byte entity slot (from D_L09_001B27DC) to 1 and resets the
 * index to -1.
 *
 * @a0: the object; its state is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/09_dobbo/L09_003FEA48.c, function func_L09_00406C90 (v1.01
 * address), author commit 264cb2be (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the body's
 * statements unchanged, the edits, each name to the target of the v2.00
 * disc's own relocation at this place (task #2561 relverify):
 *   - func_L09_00406C90 -> func_L09_00408120 (our split name at the v2.00 place);
 *   - D_L09_001B285C -> D_L09_001B27DC: an address-named extern inside
 *     D_L09_001B2000 (RULING #13291 §2);
 *   - built at -G8 (the L09_c.tsv gflag), opposite to upstream's -G0 for
 *     this file: it is what makes the 4-byte table base $gp-relative as in
 *     the ROM (NOTE #13281 code G);
 *   - `#include "common.h"` dropped: its u8/s32 typedefs are written here and
 *     only the declarations this function uses are kept.
 */
void func_L09_00408120(char *a0) {
    char *a1 = *(char **)(a0 + 0x68);
    s32 v1 = *(s32 *)(a1 + 0x5C);
    if (v1 == -1) {
        return;
    }
    *(u8 *)((char *)((v1 << 8) + (s32)D_L09_001B27DC) + 0xBC) = 1;
    *(s32 *)(a1 + 0x5C) = -1;
}
