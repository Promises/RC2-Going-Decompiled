/* Level 12 (Todano), USA v2.00: a level-only function built from C (task #2670,
 * row L12-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x0035B820, in place of its split asm; the row is
 * going-decompiled/levels/usa/L12/config/L12_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef struct { unsigned short id, kind; void *callback; unsigned short value, pad; } InitEntry12;
extern int g_anAvailableLevelOrder[];
typedef struct { unsigned short id, pad, value; char rest[6]; } SourceEntry12;
extern SourceEntry12 D_L12_0028C720[];
extern char D_L12_002881B0[];
/* The menu rows at 0x0021DEE0. No label starts there in our split (it lies
   inside a larger data label), so this extern is ADDRESS-NAMED and the level
   ELF values it from its name (RULING #13291 §2). */
extern InitEntry12 D_L12_0021DEE0[];
/* Reloaded at each search step in retail. */
extern int * volatile D_L12_001AA810 __attribute__((sda));
extern struct { char pad[0x230]; int selected; } g_mapVertexData;

/**
 * func_L12_0035B820 - build the level-ordered menu rows and select the current owner's entry.
 *
 * For each level in g_anAvailableLevelOrder (at most 28, stopping at the
 * first 0), fill one 12-byte row of D_L12_0021DEE0: kind 3, +4 pointing at
 * the menu-data descriptor D_L12_002881B0, id and value from that level's
 * entry in D_L12_0028C720. A row with id 0 ends the list. Then clear
 * obj+0x40, set bit 0x8000 in obj+0x30, and walk the zero-terminated list
 * that D_L12_001AA810 points to; the first index whose entry equals
 * g_mapVertexData.selected is stored to obj+0x40.
 *
 * @obj: the menu object; only +0x30 (flags) and +0x40 (selected row) are
 * touched.
 * Return: always 0.
 *
 * Non-obvious: level 12's copy of the body L20 landed as func_L20_0036BFB0
 * (upstream func_L20_0036AE68, a token-identical twin), with this level's
 * tables. The search re-reads
 * D_L12_001AA810 ($gp-relative at -G8) on every step through upstream's
 * `int * volatile` (kept; its `sda` text is ignored by this cc1).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/12_todano/L12_0035A838-g8.c, function
 * func_L12_0035A838 (v1.01 address), author commit 7c3ea917 (Fromnowhere), a
 * rename-copy of bigbeno37's func_L00_00354DF8 in
 * src/levels/00_aranos_tutorial/L00_002FC7A8-g8.c (author commit 6001495a,
 * the earliest of its 17 token-identical twins at the pin): the body is
 * token-identical to it but for symbol names (RULING #13427 rev 4). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * BYTE-IDENTICAL: VERBATIM but for names; the statements are upstream's.
 * Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2670): func_L12_0035A838 -> func_L12_0035B820; D_00139948 ->
 *     g_anAvailableLevelOrder; D_L12_001AA7F0 -> D_L12_001AA810 (our split
 *     label); D_L12_001CA3E0 -> g_mapVertexData (the name
 *     symbol_addrs/L12_boot_names.txt gives at exactly 0x001CA360);
 *     D_L12_002881D0 -> D_L12_002881B0 (our split label); D_L12_0028C720
 *     unchanged (our split label at the same address);
 *   - D_L12_0021DF30 -> D_L12_0021DEE0, an address-named extern for a target
 *     inside a larger split data label (RULING #13291 §2);
 *   - `#include "include_asm.h"` dropped; upstream's file comment folded into
 *     this one (it calls this the level-12 copy of func_L00_00354DF8).
 */
int func_L12_0035B820(char *obj) {
    int i, entry;
    int *list;
    for (i = 0; i < 28 && g_anAvailableLevelOrder[i]; i++) {
        D_L12_0021DEE0[i].kind = 3;
        D_L12_0021DEE0[i].callback = D_L12_002881B0;
        D_L12_0021DEE0[i].id = D_L12_0028C720[g_anAvailableLevelOrder[i]].id;
        D_L12_0021DEE0[i].value = D_L12_0028C720[g_anAvailableLevelOrder[i]].value;
    }
    list = D_L12_001AA810;
    D_L12_0021DEE0[i].id = 0;
    *(int *)(obj + 0x40) = 0;
    *(unsigned int *)(obj + 0x30) |= 0x8000;
    for (i = 0, entry = list[0]; entry; entry = D_L12_001AA810[++i]) {
        if (g_mapVertexData.selected == entry) {
            *(int *)(obj + 0x40) = i;
            break;
        }
    }
    return 0;
}
