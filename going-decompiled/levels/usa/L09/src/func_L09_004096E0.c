/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (row L09-B,
 * task #2562, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x004096E0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef signed short s16;
typedef unsigned short u16;
typedef int s32;

/* The level's table of group id lists: upstream's D_L09_001FFFB560, whose
 * v2.00 target 0x001FB4E0 lies 0x8C08 inside the larger boot label
 * g_pTextTableLoadBuf, so it stays an address-named extern (RULING #13291
 * section 2; the level ELF values it from its name). */
extern char *D_L09_001FB4E0[];

extern void func_L09_00409680(char *a0, s32 a1, char *a2, s32 a3);

/**
 * func_L09_004096E0 - hand the record's colour key to every entry the index
 * list names, negating the three offsets when the caller asks for it.
 *
 * Walks the zero-sign-terminated s16 id list at D_L09_001FB4E0[@a2] and calls
 * func_L09_00409680(@a0, id & 0x7FFF, @a1, @a3) for every entry, stopping
 * after an entry whose sign bit is set (the list's terminator).
 *
 * @a0: the asking object.
 * @a1: the record view whose +0xBC/+0xBD/+0xBF offsets feed the recolour.
 * @a2: the group index selecting the id list.
 * @a3: nonzero to negate the three offsets.
 * Return: none.
 *
 * Non-obvious: the terminator entry itself IS processed before the walk ends
 * (do/while tests after the call). No codegen or addressing device is used.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/09_dobbo/L09_003FEA48.c, function func_L09_00408250 (v1.01
 * address), author commit 264cb2be (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (BYTE-IDENTICAL), each name to the target of the v2.00 disc's own
 * relocation at this place (task #2562 relverify by name):
 *   - func_L09_00408250 -> func_L09_004096E0 (our split name);
 *   - D_L09_001FFFB560 -> D_L09_001FB4E0 (address-named extern,
 *     g_pTextTableLoadBuf+0x8C08: RULING #13291 section 2);
 *   - func_L09_004081F0 -> func_L09_00409680 (our split name at the v2.00
 *     place);
 *   - `#include "common.h"` dropped: the typedefs this function uses are
 *     written here and only its declarations kept.
 */
void func_L09_004096E0(char *a0, char *a1, s32 a2, s32 a3) {
    s16 *p;
    p = (s16 *)D_L09_001FB4E0[a2];
    if (p == 0) {
        return;
    }
    do {
        func_L09_00409680(a0, *(u16 *)p & 0x7FFF, a1, a3);
    } while (*(s16 *)p++ >= 0);
}
