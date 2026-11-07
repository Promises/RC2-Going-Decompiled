/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003DC2F0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G0.
 */

extern void func_L02_003DC3C8(char *, int);
extern void func_L02_003DC4F8(char *);
extern void func_L02_003DC598(char *);
extern void func_L02_003DC6C0(char *, int);

/* The entity table base (0x100-byte records): L02's boot-name map name at
 * 0x001B359C (symbol_addrs/L02_boot_names.txt). ADDRESSING-MODEL DEVICE
 * (RULING #8620, EE arm only): `.sdata` on the extern declaration makes cc1
 * print the one-instruction `lw` macro, which GNU as expands absolute at
 * -G0, as retail has it. It moves no data and emits nothing. */
extern int g_mobyTableBase __attribute__((section(".sdata")));

/**
 * func_L02_003DC2F0 - run the six set-up steps on the entity the actor's
 * state names.
 * @arg0: the actor; the int at its state block's (+0x68) +0x244 is the
 *        entity's index in the table at g_mobyTableBase.
 *
 * Calls func_L02_003DC3C8 with 0 and 1, func_L02_003DC6C0 with 5 and 6, then
 * func_L02_003DC4F8 and func_L02_003DC598, each on that entity.
 *
 * Non-obvious: the entity address is recomputed, table base included, for
 * every call, so each call sees the table as it stands then.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_003D26B0.c, function func_L02_003DAD88
 * (v1.01 address), author commit 431ac503 (llesieur99). Lombyte mentions in
 * the file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L02_003DAD88 -> func_L02_003DC2F0 (our split name at the
 *     v2.00 place);
 *   - func_L02_003DAE60 -> func_L02_003DC3C8, func_L02_003DB158 ->
 *     func_L02_003DC6C0, func_L02_003DAF90 -> func_L02_003DC4F8,
 *     func_L02_003DB030 -> func_L02_003DC598: our split names at the v2.00
 *     addresses the ROM's own relocations imply (relverify, task #2557);
 *   - D_L02_001B359C -> g_mobyTableBase (task #2646): upstream's spelling,
 *     which states the v2.00 address, respelled to the name
 *     symbol_addrs/L02_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2
 *     class, NOTE #13409);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     g_mobyTableBase declaration (RULING #8620; upstream's `sda` is a marker
 *     for its own patched compiler, which our cc1 ignores);
 *   - `#include "include_asm.h"` dropped (nothing here used it) and every
 *     other declaration of the file dropped.
 */
void func_L02_003DC2F0(char *arg0) {
    func_L02_003DC3C8(
        (char *)(g_mobyTableBase +
                 (*(int *)(*(char **)(arg0 + 0x68) + 0x244) << 8)), 0);
    func_L02_003DC3C8(
        (char *)(g_mobyTableBase +
                 (*(int *)(*(char **)(arg0 + 0x68) + 0x244) << 8)), 1);
    func_L02_003DC6C0(
        (char *)(g_mobyTableBase +
                 (*(int *)(*(char **)(arg0 + 0x68) + 0x244) << 8)), 5);
    func_L02_003DC6C0(
        (char *)(g_mobyTableBase +
                 (*(int *)(*(char **)(arg0 + 0x68) + 0x244) << 8)), 6);
    func_L02_003DC4F8(
        (char *)(g_mobyTableBase +
                 (*(int *)(*(char **)(arg0 + 0x68) + 0x244) << 8)));
    func_L02_003DC598(
        (char *)(g_mobyTableBase +
                 (*(int *)(*(char **)(arg0 + 0x68) + 0x244) << 8)));
}
