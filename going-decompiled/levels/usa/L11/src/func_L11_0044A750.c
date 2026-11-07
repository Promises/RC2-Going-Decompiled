/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x0044A750, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;

/* The level's id lists and its 256-byte record bank. Both addresses lie inside
   larger split data labels (D_L11_001F0000, D_L11_001B2E00), so they are
   address-named externs (RULING #13291 §2): the level ELF values them from
   their names. The `sda` attribute is upstream's own text; this cc1 ignores
   it, and at -G8 the 4-byte pointer is $gp-relative by its size alone. */
extern u8 *D_L11_001FC4A0[];
extern u8 *D_L11_001B361C __attribute__((sda));

/**
 * func_L11_0044A750 - are three members of the object's group armed?
 *
 * The object's group byte (+0x21) names an id list in D_L11_001FC4A0 (0xFF or
 * a null list: return 0); each halfword's low 15 bits index a 256-byte record
 * of the bank at D_L11_001B361C, and the walk ends after the halfword whose
 * sign bit is set. It counts live records (+0x20 neither 0xFE nor 0xFD) of
 * class 0xD98 with byte +0x95 == 6 and their state's halfword +0x366 == 1.
 *
 * @a0: the object.
 * Return: 1 as soon as the third such record is seen, else 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_004465F8-g8.c, function func_L11_00448D28 (v1.01
 * address), author commit 352d5dad (Louis-Philippe Le Sieur). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names (no device); the respellings, each to the target of the v2.00
 * disc's own relocation at this place (task #2555 relverify):
 *   - func_L11_00448D28 -> func_L11_0044A750 (our split name at the v2.00 place);
 *   - D_L11_001FC420 -> D_L11_001FC4A0 (address-named, inside D_L11_001F0000);
 *   - D_L11_001B359C -> D_L11_001B361C (address-named, inside D_L11_001B2E00);
 *   - `#include "common.h"` dropped: the u8/u16/s16/s32 typedefs it supplied
 *     are written here.
 */
s32 func_L11_0044A750(u8 *a0)
{
    u8 *base;
    u8 *p;
    u16 *q;
    u8 *e;
    s32 n;

    if (a0[0x21] == 0xFF)
        return 0;
    p = D_L11_001FC4A0[a0[0x21]];
    if (p == 0)
        return 0;
    q = (u16 *)p;
    base = D_L11_001B361C;
    n = 0;
    for (;;) {
        e = base + (*q & 0x7FFF) * 256;
        if (e != 0 && e[0x20] != 0xFE && e[0x20] != 0xFD && *(s16 *)(e + 0xAA) == 0xD98
            && e[0x95] == 6 && *(s16 *)(*(char **)(e + 0x68) + 0x366) == 1) {
            n++;
            if (n >= 3)
                return 1;
        }
        if ((s16)*q < 0)
            break;
        q++;
    }
    return 0;
}
