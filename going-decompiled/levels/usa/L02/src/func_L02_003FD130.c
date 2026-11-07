/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003FD130, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G0.
 */

typedef unsigned short u16;
typedef unsigned int u32;
typedef short s16;

/* The entity table base (0x100-byte records): L02's boot-name map name at
 * 0x001B359C (symbol_addrs/L02_boot_names.txt). ADDRESSING-MODEL DEVICE
 * (RULING #8620, EE arm only): `.sdata` on the extern declaration makes cc1
 * print the one-instruction `lw` macro, which GNU as expands absolute at
 * -G0, as retail has it. It moves no data and emits nothing. */
extern char *g_mobyTableBase __attribute__((section(".sdata")));

/* Per-group list addresses: L02's boot-name map name at 0x001FC060. */
extern u32 g_mobyGroupLists[];

extern void func_L02_003D38E8(char *);
extern void func_L02_003D5E68(char *);
extern void func_L02_003DCC40(char *);
extern void func_L02_003F2E58(char *);
extern void func_L02_0040D0C8(char *);
extern void func_L02_0040E8C8(char *);

/**
 * func_L02_003FD130 - run the per-kind handler of every entity in the
 * actor's group.
 * @a0: the actor; the u32 at its state block's (+0x68) +0x40 is its group
 *      index (0xFF = none).
 *
 * Walks the 16-bit id stream at g_mobyGroupLists[group]; each id, masked to
 * 15 bits, selects a 0x100-byte record of g_mobyTableBase, and a negative id
 * ends the stream after it is handled. By the record's kind (+0xAA): 0x9CA
 * func_L02_003D38E8, 0xB22 func_L02_003F2E58, 0xC3B func_L02_0040D0C8,
 * 0xC3E func_L02_0040E8C8, 0x9F8 func_L02_003D5E68, 0xA44 func_L02_003DCC40;
 * any other kind is skipped.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_003F5430.c, function func_L02_003FBB90
 * (v1.01 address), author commit 455cde53 (Flavius). Lombyte mentions in the
 * file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L02_003FBB90 -> func_L02_003FD130 (our split name at the
 *     v2.00 place);
 *   - func_L02_003D2390 -> func_L02_003D38E8, func_L02_003F18E8 ->
 *     func_L02_003F2E58, func_L02_0040B980 -> func_L02_0040D0C8,
 *     func_L02_0040D180 -> func_L02_0040E8C8, func_L02_003D4900 ->
 *     func_L02_003D5E68, func_L02_003DB6D8 -> func_L02_003DCC40: our split
 *     names at the v2.00 addresses the ROM's own relocations imply
 *     (relverify, task #2557);
 *   - D_L02_001B359C -> g_mobyTableBase, D_L02_001FC060 -> g_mobyGroupLists
 *     (task #2646): upstream's spellings, which state the v2.00 addresses,
 *     respelled to the names symbol_addrs/L02_boot_names.txt gives at
 *     exactly those addresses; the address-named spelling was a second name
 *     for the same address (A2 class, NOTE #13409);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     g_mobyTableBase declaration (RULING #8620; upstream's `sda` is a marker
 *     for its own patched compiler, which our cc1 ignores);
 *   - `#include "include_asm.h"` dropped (nothing here used it); the file's
 *     own typedefs kept, every other declaration dropped, and the upstream
 *     comment folded into this one.
 */
void func_L02_003FD130(char *a0) {
    char *state = *(char **)(a0 + 0x68);
    u32 index = *(u32 *)(state + 0x40);
    u32 entry;
    short *p;
    char *rec;
    s16 type;

    if (index == 0xFF)
        return;
    entry = g_mobyGroupLists[index];
    if (entry == 0)
        return;
    p = (short *)entry;
    do {
        rec = g_mobyTableBase + ((*(u16 *)p & 0x7FFF) << 8);
        type = *(s16 *)(rec + 0xAA);
        switch (type) {
        case 0x9CA:
            func_L02_003D38E8(rec);
            break;
        case 0xB22:
            func_L02_003F2E58(rec);
            break;
        case 0xC3B:
            func_L02_0040D0C8(rec);
            break;
        case 0xC3E:
            func_L02_0040E8C8(rec);
            break;
        case 0x9F8:
            func_L02_003D5E68(rec);
            break;
        case 0xA44:
            func_L02_003DCC40(rec);
            break;
        }
    } while (*p++ >= 0);
}
