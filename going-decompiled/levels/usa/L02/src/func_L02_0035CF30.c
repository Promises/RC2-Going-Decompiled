/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x0035CF30, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G8. Level 24's func_L24_0034D598 is the same
 * function in that level's program.
 */

typedef struct { unsigned short id, kind; void *payload; unsigned short value, pad; } InitEntry12;
/* The boot's resident order of available levels (core, below 0x1A7C80). */
extern int g_anAvailableLevelOrder[];
typedef struct { unsigned short id, pad, value; char rest[6]; } SourceEntry12;
extern SourceEntry12 D_L02_0028DAA0[];
extern char D_L02_00289540[];
/* Inside the larger split label D_L02_0020FFF0: address-named (RULING #13291 §2). */
extern InitEntry12 D_L02_0021E660[];
/* Reloaded at each search step in retail. `sda` is upstream's marker for its
 * own patched compiler; our cc1 ignores it (with a warning). At -G8 the
 * 4-byte pointer is $gp-relative by its size alone. Kept verbatim. */
extern int * volatile D_L02_001AA850 __attribute__((sda));
/* Inside the larger split label D_L02_001B3930: address-named (RULING #13291 §2). */
extern struct { char pad[0x230]; int selected; } D_L02_001CAAE0;

/**
 * func_L02_0035CF30 - build the level-ordered menu rows and select the
 * current owner's entry.
 * @obj: the menu object; only +0x30 (flags) and +0x40 (selected row) are
 *       touched.
 *
 * For each level in g_anAvailableLevelOrder (at most 28, stopping at the
 * first 0), fills one 12-byte row of D_L02_0021E660: kind 3, the +4 field
 * pointing at the menu-data descriptor D_L02_00289540, and the id and value
 * of that level's entry in D_L02_0028DAA0. A row with id 0 ends the list.
 * Then clears @obj+0x40, sets bit 0x8000 in @obj+0x30 and walks the
 * zero-terminated list D_L02_001AA850 points to; the first index whose entry
 * equals D_L02_001CAAE0.selected is stored to @obj+0x40.
 *
 * Return: always 0.
 *
 * Non-obvious: the search loop re-reads D_L02_001AA850 ($gp-relative) on
 * every step, as the ROM does; upstream keeps the pointer `volatile`.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_0035BD60-init-g8.c, function
 * func_L02_0035BD60 (v1.01 address), author commit a96d3126 (Louis-Philippe
 * Le Sieur), a rename-copy of bigbeno37's func_L00_00354DF8 in
 * src/levels/00_aranos_tutorial/L00_002FC7A8-g8.c (author commit 6001495a):
 * the body is token-identical to it but for symbol names and the struct
 * field `callback` -> `payload` (RULING #13427 rev 4, task #2645). Lombyte
 * mentions in the file: 0. Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). BYTE-IDENTICAL:
 * verbatim but for names, at upstream's -G8 (the `volatile` and the `sda`
 * attribute kept); respellings:
 *   - name func_L02_0035BD60 -> func_L02_0035CF30 (our split name at the
 *     v2.00 place);
 *   - D_00139948 -> g_anAvailableLevelOrder (the boot's name, same address);
 *   - D_L02_0028DA20 -> D_L02_0028DAA0, D_L02_002894E0 -> D_L02_00289540,
 *     D_L02_001AA830 -> D_L02_001AA850: our split labels at the v2.00
 *     addresses the ROM's own relocations imply (relverify, task #2557);
 *   - D_L02_0021E630 -> D_L02_0021E660 (address-named, inside
 *     D_L02_0020FFF0); D_L02_001CAAE0 keeps its name, which already states
 *     its v2.00 address (address-named, inside D_L02_001B3930);
 *   - `#include "include_asm.h"` dropped (nothing used it) and the upstream
 *     file comment folded into this one.
 */
int func_L02_0035CF30(char *obj) {
    int i, entry;
    int *list;
    for (i = 0; i < 28 && g_anAvailableLevelOrder[i]; i++) {
        D_L02_0021E660[i].kind = 3;
        D_L02_0021E660[i].payload = D_L02_00289540;
        D_L02_0021E660[i].id = D_L02_0028DAA0[g_anAvailableLevelOrder[i]].id;
        D_L02_0021E660[i].value = D_L02_0028DAA0[g_anAvailableLevelOrder[i]].value;
    }
    list = D_L02_001AA850;
    D_L02_0021E660[i].id = 0;
    *(int *)(obj + 0x40) = 0;
    *(unsigned int *)(obj + 0x30) |= 0x8000;
    for (i = 0, entry = list[0]; entry; entry = D_L02_001AA850[++i]) {
        if (D_L02_001CAAE0.selected == entry) {
            *(int *)(obj + 0x40) = i;
            break;
        }
    }
    return 0;
}
