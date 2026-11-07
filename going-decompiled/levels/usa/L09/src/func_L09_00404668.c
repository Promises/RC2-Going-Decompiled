/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (row L09-B,
 * task #2562, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x00404668, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, plain as.
 */

typedef unsigned short u16;
typedef signed short s16;
typedef int s32;
typedef unsigned char u8;

typedef union {
    u16 h;
    s32 w;
} U16S32;

/* The level's frame-counter word, read both as its low half and its whole
 * word: upstream's D_L09_001B13C0, our label at the v2.00 address 0x001B1370.
 * $gp-relative at -G8 by its size alone; the `sda` attribute text is kept
 * verbatim (this cc1 ignores it, a warning). */
extern U16S32 D_L09_001B1370 __attribute__((sda));

extern void func_L09_002EAD98(char *p);

/**
 * func_L09_00404668 - reset the state's drop counters on a new frame.
 *
 * The state is at *(char **)(a0+0x68). When its s16 at +0x270 is nonzero, or
 * its word +0x234 is 1 and the s16 at +0x26C is below half the frame counter
 * D_L09_001B1370.w, the s16 at +0x26C is reloaded from D_L09_001B1370.h and
 * the s16 at +0x270 cleared. Both RNG cells at state+0x26C and state+0x26E
 * are then ticked through func_L09_002EAD98, and the object's u16 counter at
 * +0x96 is incremented unless it already holds 0xFFFF.
 *
 * @a0: the object.
 * Return: none.
 *
 * Non-obvious: the two func_L09_002EAD98 calls tick neighbouring RNG cells
 * (0x26C and 0x26E) whether or not the reset above fired. No codegen or
 * addressing device is used.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_003FEA48-g8.c, function func_L09_004031D8
 * (v1.01 address), author commit 431ac503 (llesieur99). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2562 relverify by name):
 *   - func_L09_004031D8 -> func_L09_00404668 (our split name);
 *   - D_L09_001B13C0 -> D_L09_001B1370 (our label, both the .w and .h reads);
 *   - func_2EA110 -> func_L09_002EAD98 (L09's own copy);
 *   - `#include "common.h"` dropped: the typedefs this function uses are
 *     written here and only its declarations kept.
 */
void func_L09_00404668(char *a0) {
    char *s1 = a0;
    char *s0 = *(char **)(s1 + 0x68);
    u16 n;
    if (*(s16 *)(s0 + 0x270) != 0
        || (*(s32 *)(s0 + 0x234) == 1 && *(s16 *)(s0 + 0x26C) < D_L09_001B1370.w / 2)) {
        *(u16 *)(s0 + 0x26C) = D_L09_001B1370.h;
        *(s16 *)(s0 + 0x270) = 0;
    }
    func_L09_002EAD98(s0 + 0x26C);
    func_L09_002EAD98(s0 + 0x26E);
    n = *(u16 *)(s1 + 0x96);
    if (n < 0xFFFF) {
        *(u16 *)(s1 + 0x96) = n + 1;
    }
}
