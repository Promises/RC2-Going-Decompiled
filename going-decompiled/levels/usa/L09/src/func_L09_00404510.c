/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (task #2561,
 * RULING #13291 row L09-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x00404510, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

/* The level's entity table base, at 0x001B27DC inside our split label
   D_L09_001B2000: an address-named extern (RULING #13291 §2), valued from its
   address by level_elf. `sda` is upstream's own text, ignored by this cc1
   ("attribute directive ignored"): at -G8 the 4-byte pointer is
   $gp-relative by its size alone. */
extern char *D_L09_001B27DC __attribute__((sda));

/* The per-group tables of entity ids, at 0x001FB4E0 inside our split label
   D_L09_001F0000 (an address-named extern, RULING #13291 §2). Absolute. */
extern char *D_L09_001FB4E0[];

/**
 * func_L09_00404510 - wake the 0xD57-class peers of the object's group.
 *
 * Unless the object's group byte (+0x21) is 0xFF or the group has no id list
 * (D_L09_001FB4E0[group] == 0), walks the list up to and including the first
 * negative id: each id (masked 0x7FFF) selects a 0x100-byte entity slot from
 * D_L09_001B27DC, and a live slot (signed byte +0x20 >= 0) of class 0xD57
 * (+0xAA) gets halfword +0x270 of its state (+0x68) set to 1.
 *
 * @a0: the object.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/09_dobbo/L09_003FEA48.c, function func_L09_00403080 (v1.01
 * address), author commit 264cb2be (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the body's
 * statements unchanged, the edits, each name to the target of the v2.00
 * disc's own relocation at this place (task #2561 relverify):
 *   - func_L09_00403080 -> func_L09_00404510 (our split name at the v2.00 place);
 *   - D_L09_001B285C -> D_L09_001B27DC: an address-named extern inside
 *     D_L09_001B2000 (RULING #13291 §2);
 *   - D_L09_001FFFB560 -> D_L09_001FB4E0: an address-named extern inside
 *     D_L09_001F0000 (RULING #13291 §2; upstream's name carries a stray
 *     `FF`, its relocation targets 0x001FB4E0);
 *   - built at -G8 (the L09_c.tsv gflag), opposite to upstream's -G0 for
 *     this file: it is what makes the 4-byte table base $gp-relative as in
 *     the ROM (NOTE #13281 code G);
 *   - `#include "common.h"` dropped: its u8/u16/s8/s16/s32 typedefs are
 *     written here and only the declarations this function uses are kept.
 */
void func_L09_00404510(char *a0) {
    s16 *p;
    s32 v;
    char *a1;
    char *a2;
    if (*(u8 *)(a0 + 0x21) == 0xFF) {
        return;
    }
    v = (s32)D_L09_001FB4E0[*(u8 *)(a0 + 0x21)];
    if (v == 0) {
        return;
    }
    p = (s16 *)v;
    a2 = D_L09_001B27DC;
    do {
        a1 = a2 + ((*(u16 *)p & 0x7FFF) << 8);
        if (*(s8 *)(a1 + 0x20) >= 0) {
            if (*(s16 *)(a1 + 0xAA) == 0xD57) {
                *(u16 *)(*(char **)(a1 + 0x68) + 0x270) = 1;
            }
        }
    } while (*(s16 *)p++ >= 0);
}
