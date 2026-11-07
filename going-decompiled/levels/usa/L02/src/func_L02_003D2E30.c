/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003D2E30, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G8.
 */

/* Per-group lists of 16-bit entity ids; a negative id ends a list. Inside
 * the larger split label D_L02_001F0000: address-named (RULING #13291 §2). */
extern unsigned short *D_L02_001FC060[];

/* The entity table base (0x100-byte records). Inside the larger split label
 * D_L02_001B2D80: address-named. A 4-byte object, so $gp-relative at -G8 by
 * its size alone. */
extern unsigned char *D_L02_001B359C;

/**
 * func_L02_003D2E30 - does the object's group hold an armed 0x9CA entity?
 * @a0: the object; its byte +0x21 is the group index (0xFF = none).
 *
 * Walks the group's id list in D_L02_001FC060. Each id, masked to 15 bits,
 * selects a 0x100-byte record of the table at D_L02_001B359C. An id with
 * bit 15 set is examined and then ends the walk.
 *
 * Return: 1 for the first record that is non-null, not being freed (state
 * byte +0x20 neither 0xFE nor 0xFD), of kind 0x9CA (+0xAA), class byte
 * +0x95 == 6, and whose state block (+0x68) has the half-word +0x2F6 == 1;
 * else 0, also when the group is 0xFF or its list is empty.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_002ED7B0-g8.c, function func_L02_003D18D8
 * (v1.01 address), author commit ac1ee013 (bigmak94). Lombyte mentions in
 * the file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). BYTE-IDENTICAL: verbatim but for names, at upstream's -G8;
 * respellings:
 *   - name func_L02_003D18D8 -> func_L02_003D2E30 (our split name at the
 *     v2.00 place);
 *   - D_L02_001FC060 and D_L02_001B359C keep their upstream names, which
 *     already state their v2.00 addresses (address-named, RULING #13291 §2);
 *   - `#include "include_asm.h"` dropped (nothing here used it), every other
 *     declaration of the file dropped and the upstream comment folded into
 *     this one.
 */
int func_L02_003D2E30(unsigned char *a0) {
    unsigned short *q;
    unsigned short *p;
    unsigned char *e;

    if (a0[0x21] == 0xFF)
        return 0;
    q = D_L02_001FC060[a0[0x21]];
    if (q == 0)
        return 0;
    p = q;
    do {
        e = D_L02_001B359C + ((*p & 0x7FFF) << 8);
        if (e != 0 && e[0x20] != 0xFE && e[0x20] != 0xFD && *(short *)(e + 0xAA) == 0x9CA &&
            e[0x95] == 6 && *(short *)(*(unsigned char **)(e + 0x68) + 0x2F6) == 1)
            return 1;
    } while (*(short *)p++ >= 0);
    return 0;
}
