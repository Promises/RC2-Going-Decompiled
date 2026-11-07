/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003FCB40, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G0.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short s16;
typedef int s32;

/* The entity table base (0x100-byte records), inside the larger split label
 * D_L02_001B2D80: address-named (RULING #13291 §2). ADDRESSING-MODEL DEVICE
 * (RULING #8620, EE arm only): `.sdata` on the extern declaration makes cc1
 * print the one-instruction `lw` macro, which GNU as expands absolute at
 * -G0, as retail has it. It moves no data and emits nothing. */
extern char *D_L02_001B359C __attribute__((section(".sdata")));

/* Per-group list addresses, inside the larger split label D_L02_001F0000:
 * address-named. */
extern u32 D_L02_001FC060[];

extern void func_L02_003D5D18(char *, s32);
extern void func_L02_003DC878(char *, s32);

/**
 * func_L02_003FCB40 - claim and start the first idle entity of a kind in the
 * actor's group.
 * @a0: the actor; the u32 at its state block's (+0x68) +0x40 is its group
 *      index (0xFF = none).
 * @a1: the entity kind wanted (half-word +0xAA): 0x9F8 or 0xA44 start one.
 * @a2: passed to the 0xA44 start function.
 *
 * Walks the group's id list in D_L02_001FC060 (15-bit ids into the
 * 0x100-byte records of D_L02_001B359C; an id with bit 15 set ends the walk
 * after it is examined). The first non-null record that is not being freed
 * (+0x20 neither 0xFE nor 0xFD), is of kind @a1 and has state byte 1 gets
 * @a0 as its owner (+0xB8). Kind 0x9F8 is started by func_L02_003D5D18 (its
 * flag: the state's half-word +0x120 == 0xC) and counted at state +0xA0;
 * kind 0xA44 by func_L02_003DC878(record, @a2), counted at +0xA4.
 *
 * Return: the started record, else 0 (also for a matching record of any
 * other kind, which is claimed but not started, and the walk goes on).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_003F5430.c, function func_L02_003FB5A0
 * (v1.01 address), author commit 7a7816b9 (bigmak94). Lombyte mentions in
 * the file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L02_003FB5A0 -> func_L02_003FCB40 (our split name at the
 *     v2.00 place);
 *   - func_L02_003D47B0 -> func_L02_003D5D18 and func_L02_003DB310 ->
 *     func_L02_003DC878: our split names at the v2.00 addresses the ROM's
 *     own relocations imply (relverify, task #2557);
 *   - D_L02_001B359C and D_L02_001FC060 keep their upstream names, which
 *     already state their v2.00 addresses (address-named, RULING #13291 §2);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     D_L02_001B359C declaration (RULING #8620; upstream's `sda` is a marker
 *     for its own patched compiler, which our cc1 ignores);
 *   - `#include "include_asm.h"` dropped (nothing here used it); the file's
 *     own typedefs kept, every other declaration dropped, and the upstream
 *     comment folded into this one.
 */
char *func_L02_003FCB40(char *a0, s32 a1, s32 a2) {
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
            switch (a1) {
            case 0x9F8:
                func_L02_003D5D18(rec, *(s16 *)(state + 0x120) == 0xC);
                *(s32 *)(state + 0xA0) += 1;
                return rec;
            case 0xA44:
                func_L02_003DC878(rec, a2);
                *(s32 *)(state + 0xA4) += 1;
                return rec;
            }
        }
    } while ((s16)*p++ >= 0);
    return 0;
}
