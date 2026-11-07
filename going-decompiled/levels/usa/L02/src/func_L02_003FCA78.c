/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003FCA78, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G8 -- the opposite of upstream's -G0 for this
 * file (a table setting, not a device): retail reads the 4-byte entity-table
 * base $gp-relative here.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short s16;
typedef int s32;

/* The entity table base (0x100-byte records), inside the larger split label
 * D_L02_001B2D80: address-named (RULING #13291 §2). `sda` is upstream's
 * marker for its own patched compiler; our cc1 ignores it (with a warning).
 * At this row's -G8 the 4-byte pointer is $gp-relative by its size alone.
 * Kept verbatim. */
extern char *D_L02_001B359C __attribute__((sda));

/* Per-group list addresses, inside the larger split label D_L02_001F0000:
 * address-named. Unknown size, so never $gp-relative. */
extern u32 D_L02_001FC060[];

extern s32 func_L02_003FCC60(char *, char *);

/**
 * func_L02_003FCA78 - claim the first idle entity of a kind in the actor's
 * group and run func_L02_003FCC60 on it.
 * @a0: the actor; the u32 at its state block's (+0x68) +0x40 is its group
 *      index (0xFF = none).
 * @a1: the entity kind wanted (half-word +0xAA).
 * @a2: not read.
 *
 * Walks the group's id list in D_L02_001FC060; each id, masked to 15 bits,
 * selects a 0x100-byte record of D_L02_001B359C, and an id with bit 15 set
 * ends the walk after it is examined. The first non-null record that is not
 * being freed (state byte +0x20 neither 0xFE nor 0xFD), is of kind @a1 and
 * has state byte 1 gets @a0 as its owner (+0xB8).
 *
 * Return: func_L02_003FCC60(@a0, record) for that record, else 0.
 *
 * Non-obvious: the walk is the `do { ... } while ((s16)*p++ >= 0);` of
 * func_L02_003FCB40. A `for (;;)` with the test and the increment in the body
 * makes this cc1 annul the loop-bottom slot (`bgezl`), which retail does not.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_003F5430.c, function func_L02_003FB4D8
 * (v1.01 address), author commit 9e5c365f (bigmak94). Lombyte mentions in
 * the file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L02_003FB4D8 -> func_L02_003FCA78 (our split name at the
 *     v2.00 place);
 *   - func_L02_003FB6C0 -> func_L02_003FCC60: our split name at the v2.00
 *     address the ROM's own relocation implies (relverify, task #2557);
 *   - D_L02_001B359C and D_L02_001FC060 keep their upstream names, which
 *     already state their v2.00 addresses (address-named, RULING #13291 §2);
 *   - built at -G8 where upstream's file is -G0 (config/L02_c.tsv gflag; at
 *     -G0 this body differs from retail in 5 words);
 *   - `#include "include_asm.h"` dropped (nothing here used it); the file's
 *     own typedefs kept, every other declaration dropped, and the upstream
 *     comment folded into this one.
 */
s32 func_L02_003FCA78(char *a0, s32 a1, char *a2) {
    char *state = *(char **)(a0 + 0x68);
    u32 index = *(u32 *)(state + 0x40);
    u32 entry;
    u16 *p;
    char *rec;

    if (index == 0xFF)
        return 0;
    entry = D_L02_001FC060[index];
    if (entry == 0)
        return 0;
    p = (u16 *)entry;
    do {
        rec = D_L02_001B359C + ((*p & 0x7FFF) << 8);
        if (rec != 0 && *(u8 *)(rec + 0x20) != 0xFE && *(u8 *)(rec + 0x20) != 0xFD
            && *(s16 *)(rec + 0xAA) == a1 && *(u8 *)(rec + 0x20) == 1) {
            *(char **)(rec + 0xB8) = a0;
            return func_L02_003FCC60(a0, rec);
        }
    } while ((s16)*p++ >= 0);
    return 0;
}
