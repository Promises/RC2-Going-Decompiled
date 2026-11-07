/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x0041EEB0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern char *D_L23_001B1434 __attribute__((sda));
extern int D_L23_001B13D4 __attribute__((sda));
extern unsigned char D_1395B8[];
extern void func_L23_003176F8(int, int);
extern void func_L23_00318E98(void);
extern void func_L23_0031B350(void *);

/**
 * func_L23_0041EEB0 - release the level object and its five slot objects, then raise three flags.
 *
 * Frees (func_L23_0031B350) and clears each non-null pointer of the five-slot
 * array at the level object's state +0x358. When the level object
 * D_L23_001B1434 is set it calls func_L23_003176F8(0, 0) and
 * func_L23_00318E98(), frees the object and clears the pointer. Finally it
 * sets D_L23_001B13D4 and the two boot flag bytes D_1395B8[0xA1] and
 * D_1395B8[0x88] to 1.
 *
 * No parameters.
 * Return: nothing.
 *
 * Non-obvious: the flags are written through a pointer loaded after the
 * calls, which gives retail's lui/addiu base with the 0x88 and 0xA1
 * displacements; written as D_1395B8[0xA1] directly, cc1 hoists the high
 * half into a saved register (upstream's note, 160 bytes). D_1395B8 is a
 * boot address, the spelling the boot and L23 split asm already use; it lies
 * in the boot data split label D_139599 [0x139599, 0x1395C1), at +0x1F, not
 * inside g_quickSelectMap. It stays an address-named extern, which level_elf
 * values from its name (RULING #13291 section 2). Upstream's `__attribute__((sda))` text on the
 * extern declarations is kept as written; this cc1 ignores the attribute.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/us2/levels/23_damosel_orbit/L23_0041EEB0-g8.c, function
 * func_US2_L23_0041EEB0 (v2.00 address), author commit cde7e4dd (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v2.00-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_US2_L23_0041EEB0 -> func_L23_0041EEB0; D_L23_001B1424 ->
 *     D_L23_001B13D4; D_L23_001B1484 -> D_L23_001B1434; func_0029B758 ->
 *     func_L23_003176F8; func_0029CE80 -> func_L23_00318E98; func_0029F338 ->
 *     func_L23_0031B350; D_001395B8 -> D_1395B8 (the boot's existing
 *     spelling of the same address, task #2606)
 *   - `#include "include_asm.h"` dropped; the typedefs it uses written in the
 *     TU (the file's own where it has them, else common.h's); only the
 *     file-level declarations it uses kept, without their comments
 *   - upstream's comment folded into this one
 */
void func_L23_0041EEB0(void) {
    char **p = (char **)(*(char **)(D_L23_001B1434 + 0x68) + 0x358);
    unsigned char *flags;
    int i;

    for (i = 4; i >= 0; i--) {
        if (*p != 0) {
            func_L23_0031B350(*p);
            *p = 0;
        }
        p++;
    }
    if (D_L23_001B1434 != 0) {
        func_L23_003176F8(0, 0);
        func_L23_00318E98();
        func_L23_0031B350(D_L23_001B1434);
        D_L23_001B1434 = 0;
    }
    flags = D_1395B8;
    D_L23_001B13D4 = 1;
    flags[0xA1] = 1;
    flags[0x88] = 1;
}
