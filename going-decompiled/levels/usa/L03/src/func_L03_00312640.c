/* Level 03 (Endako), USA v2.00: a level-only function built from C
 * (task #2655, RULING #13291 row L03-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x00312640, in place of its split asm; the row is
 * going-decompiled/levels/usa/L03/config/L03_c.tsv, the arm Sony 2.9-ee-991111
 * cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

/* Upstream include/common.h's own definitions of the typedefs the body uses
 * (their `#include "common.h"` is not carried; nothing else of it is used). */
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;

/* A level pointer to a list of 0x28-byte records. No name exists at 0x001B2918
 * (it is g_pRainHeightmap+0x38 in L03's boot-name map), so it stays
 * address-named (RULING #13291 section 2); the level ELF values it from its
 * name. */
extern u8 *D_L03_001B2918;

/**
 * func_L03_00312640 - count the live records of kind 1 in the level's
 * 0x28-byte record list.
 *
 * Walks the list D_L03_001B2918 points to, record by record, until one whose
 * leading halfword is 0. A record counts when its halfword at +0x24 is 1 and
 * bit 1 of its halfword at +0x10 is clear.
 * Return: the count, or 1 when the list pointer is null.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/03_endako/L03_002CCB70.c, function
 * func_L03_003112F8 (v1.01 address), author commit 86097b24 (Louis-Philippe
 * Le Sieur; `git blame -M -C` names it for all 18 definition lines at
 * fe6adf9b, and `git log --reverse -S'func_L03_003112F8('` agrees).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L03_003112F8 -> func_L03_00312640 (our split name at the
 *     v2.00 place);
 *   - D_L03_001B2918 kept: the v2.00 ROM's relocation targets that same
 *     address;
 *   - `#include "common.h"` dropped, its four typedefs written out; the
 *     file's other functions and externs not carried.
 */
s32 func_L03_00312640(void)
{
    u8 *p = D_L03_001B2918;
    u8 *q;
    s32 n = 0;

    if (p == 0)
        return 1;
    q = p;
    while (*(s16 *)q != 0) {
        if (*(s16 *)(q + 0x24) == 1) {
            if ((*(u16 *)(q + 0x10) & 2) == 0)
                n++;
        }
        q += 0x28;
    }
    return n;
}
