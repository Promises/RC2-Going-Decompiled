/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003CEB08, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm sdk29 (ee-gcc
 * 2.9-ee-991111) at -O2 -G0.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;

/* Per-group lists of 16-bit entity ids; a negative id ends a list. */
extern u16 *D_L02_001FC060[];

/* Both words sit inside the larger split label D_L02_001B2D80, so they are
 * address-named externs valued from their names (RULING #13291 §2).
 * ADDRESSING-MODEL DEVICE (RULING #8620, EE arm only): `.sdata` on an extern
 * array of unknown size makes cc1 print the one-instruction load, which GNU as
 * expands absolute at -G0 (`lui $r` / `lw $r,%lo($r)`), as retail has it. It
 * moves no data and emits nothing.
 */
extern u8 *D_L02_001B359C[] __attribute__((section(".sdata")));
extern int D_L02_001B35B8[] __attribute__((section(".sdata")));

/**
 * func_L02_003CEB08 - find the first entity of a group whose state byte is
 * 0x17.
 * @group: the group index; 0xFF in its low byte means "no group".
 *
 * Walks the group's id list from D_L02_001FC060[@group]. Each id, masked to
 * 15 bits, selects a 0x100-byte record of the entity table D_L02_001B359C[0].
 *
 * Return: the first record whose byte +0x20 is 0x17, or 0 when the group is
 * 0xFF or above D_L02_001B35B8[0], its list is empty, the list ends (an id
 * with bit 15 set ends it after that id is examined), or a record address
 * is 0.
 *
 * Non-obvious: the end test reads the PREVIOUS id (p[-1]); the first id is
 * let through by the `p == D_L02_001FC060[group]` arm, so the whole loop
 * condition is re-evaluated on every step, as in retail.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_002ED7B0.c, function func_L02_003CD5B0
 * (v1.01 address), author commit ac1ee013 (bigmak94). Lombyte mentions in
 * the file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L02_003CD5B0 -> func_L02_003CEB08 (our split name at the
 *     v2.00 place);
 *   - D_L02_001FC060, D_L02_001B359C, D_L02_001B35B8 keep their upstream
 *     names, which already state their v2.00 addresses; the last two are
 *     address-named (RULING #13291 §2);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     D_L02_001B359C and D_L02_001B35B8 declarations (RULING #8620;
 *     upstream's `sda` is a marker for its own patched compiler, which our
 *     cc1 ignores);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped; the u8,
 *     u16 and s16 typedefs it supplied are spelled here; every other
 *     declaration of the file dropped and the upstream comments folded into
 *     this one.
 */
u8 *func_L02_003CEB08(int group) {
    u16 *p;
    u8 *e;

    for (p = D_L02_001FC060[group];
         (u8)group != 0xFF && group <= D_L02_001B35B8[0] && p != 0 &&
         ((s16)p[-1] >= 0 || p == D_L02_001FC060[group]);
         p++) {
        e = D_L02_001B359C[0] + ((*p & 0x7FFF) << 8);
        if (e == 0)
            break;
        if (e[0x20] == 0x17)
            return e;
    }
    return 0;
}
