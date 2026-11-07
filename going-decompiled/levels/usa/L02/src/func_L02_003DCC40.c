/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003DCC40, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G0.
 */

extern void func_L02_0031F998(char *, int, int);
extern void func_L02_00367AF8(int);
extern void func_L02_0031F6B8(char *, int, int, int);
extern void func_L02_003174E8(char *);
extern void func_L02_003157A0(char *);
extern void func_L02_00316610(char *, char *);

/* The boot's listener-position history (core, below 0x1A7C80); read here as
 * a table of 0x70-byte effect records. */
extern char g_listenerPosHistory[];

/* The entity table base (0x100-byte records): L02's boot-name map name at
 * 0x001B359C (symbol_addrs/L02_boot_names.txt). ADDRESSING-MODEL DEVICE
 * (RULING #8620, EE arm only): `.sdata` on the extern declaration makes cc1
 * print the one-instruction `lw` macro, which GNU as expands absolute at
 * -G0, as retail has it. It moves no data and emits nothing. */
extern char *g_mobyTableBase __attribute__((section(".sdata")));

/* An entity record (0x100 bytes, in the table at g_mobyTableBase). Both flag
 * updates below go through this struct: a struct member store neither orders
 * the next load of the table's base after it nor makes the compiler reload
 * that base, which is how retail's loads are placed. */
typedef struct {
    char pad0[0x34];
    unsigned short flags;
} Entity003DB6D8;

/* The record of the table that the actor's state names at +0x244. */
#define CHILD003DB6D8 (g_mobyTableBase + *(int *)(*(char **)(arg0 + 0x68) + 0x244) * 0x100)

/**
 * func_L02_003DCC40 - switch an actor off.
 * @arg0: the actor; its state block is the pointer at +0x68.
 *
 * Calls func_L02_0031F998(@arg0, 1, 0). When the state's slot at +0x26C is
 * not -1 and the 0x70-byte record it indexes in g_listenerPosHistory is
 * owned by this actor (+0x88) and live (byte +0x74), stops it
 * (func_L02_00367AF8); the slot becomes -1. Then func_L02_0031F6B8(@arg0, 0,
 * 0, 3) and func_L02_003174E8(@arg0), clears the actor's byte +0x31 and word
 * +0x98 and sets its flags to (flags & 0xEFFF) | 3. The child entity the
 * state names at +0x244 gets flag bit 0 set and its +0x31 and +0x98 cleared.
 * A state object at +0x260 goes to func_L02_003157A0, and each of the
 * state's sub-objects at +0x170, +0x1B0, +0x1F0 whose byte at +1 is set is
 * handed to func_L02_00316610 with the child.
 *
 * Non-obvious: CHILD003DB6D8 re-reads the table base and the state's +0x244
 * on every use, as retail does.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_003DB310.c, function func_L02_003DB6D8
 * (v1.01 address), author commit 0a1d634f (bigmak94). Lombyte mentions in
 * the file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L02_003DB6D8 -> func_L02_003DCC40 (our split name at the
 *     v2.00 place);
 *   - D_00188660 -> g_listenerPosHistory (the boot's name, same address);
 *   - func_002A7750 -> func_L02_0031F998, func_002E59F8 -> func_L02_00367AF8,
 *     func_002A7470 -> func_L02_0031F6B8, func_002A05A8 -> func_L02_003174E8,
 *     func_0029F338 -> func_L02_003157A0, func_0029F9F0 -> func_L02_00316610:
 *     our split names at the v2.00 addresses the ROM's own relocations imply
 *     (relverify, task #2557);
 *   - D_L02_001B359C -> g_mobyTableBase (task #2646): upstream's spelling,
 *     which states the v2.00 address, respelled to the name
 *     symbol_addrs/L02_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2
 *     class, NOTE #13409);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     g_mobyTableBase declaration (RULING #8620; upstream's `sda` is a marker
 *     for its own patched compiler, which our cc1 ignores);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped (nothing
 *     here used them), every other declaration of the file dropped, the
 *     closing `#undef CHILD003DB6D8` dropped (nothing follows it in this TU)
 *     and the upstream function comment folded into this one.
 */
void func_L02_003DCC40(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    int slot;
    char *rec;

    func_L02_0031F998(arg0, 1, 0);
    slot = *(int *)(state + 0x26C);
    if (slot != -1) {
        rec = g_listenerPosHistory + slot * 0x70;
        if (*(char **)(rec + 0x88) == arg0 && *(unsigned char *)(rec + 0x74) != 0)
            func_L02_00367AF8(slot);
    }
    *(int *)(state + 0x26C) = -1;
    func_L02_0031F6B8(arg0, 0, 0, 3);
    func_L02_003174E8(arg0);
    *(unsigned char *)(arg0 + 0x31) = 0;
    *(int *)(arg0 + 0x98) = 0;
    ((Entity003DB6D8 *)arg0)->flags = (((Entity003DB6D8 *)arg0)->flags & 0xEFFF) | 3;
    ((Entity003DB6D8 *)CHILD003DB6D8)->flags |= 1;
    *(unsigned char *)(CHILD003DB6D8 + 0x31) = 0;
    *(int *)(CHILD003DB6D8 + 0x98) = 0;
    if (*(char **)(*(char **)(arg0 + 0x68) + 0x260) != 0)
        func_L02_003157A0(*(char **)(*(char **)(arg0 + 0x68) + 0x260));
    if (*(unsigned char *)(state + 0x171) != 0)
        func_L02_00316610(CHILD003DB6D8, state + 0x170);
    if (*(unsigned char *)(state + 0x1B1) != 0)
        func_L02_00316610(CHILD003DB6D8, state + 0x1B0);
    if (*(unsigned char *)(state + 0x1F1) != 0)
        func_L02_00316610(CHILD003DB6D8, state + 0x1F0);
}
