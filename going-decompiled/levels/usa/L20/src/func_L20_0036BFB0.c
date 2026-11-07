/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x0036BFB0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef struct { unsigned short id, kind; void *callback; unsigned short value, pad; } InitEntry12;
extern int g_anAvailableLevelOrder[];
typedef struct { unsigned short id, pad, value; char rest[6]; } SourceEntry12;
extern SourceEntry12 D_L20_00296920[];
extern char D_L20_002923C0[];
extern InitEntry12 D_L20_0021E0A0[];
extern int * volatile D_L20_001AA810 __attribute__((sda));
extern struct { char pad[0x230]; int selected; } g_mapVertexData;

/**
 * func_L20_0036BFB0 - build the level-ordered menu rows and select the current owner's entry.
 *
 * For each level in g_anAvailableLevelOrder (at most 28, stopping at the
 * first 0), fill one 12-byte row of D_L20_0021E0A0: kind 3, +4 pointing at
 * the menu-data descriptor D_L20_002923C0, id and value from that level's
 * entry in D_L20_00296920. A row with id 0 ends the list. Then clear
 * obj+0x40, set bit 0x8000 in obj+0x30, and walk the zero-terminated list
 * that D_L20_001AA810 points to; the first index whose entry equals
 * g_mapVertexData.selected is stored to obj+0x40.
 *
 * @obj: the menu object; only +0x30 (flags) and +0x40 (selected row) are
 * touched.
 * Return: always 0.
 *
 * Non-obvious: level 20's copy of L24's func_L24_0034D598, with this level's
 * tables. The search re-reads D_L20_001AA810 ($gp-relative at -G8) on every
 * step through upstream's `int * volatile` (kept; its `sda` text is ignored
 * by this cc1).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_0036AE68-g8.c, function func_L20_0036AE68 (v1.01
 * address), author commit 7c3ea917 (Fromnowhere). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for
 * names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_0036AE68 -> func_L20_0036BFB0; D_00139948 ->
 *     g_anAvailableLevelOrder; D_L20_001AA7F0 -> D_L20_001AA810;
 *     D_L20_001CA5A0 -> g_mapVertexData; D_L20_0021E0F0 -> D_L20_0021E0A0;
 *     D_L20_002923E0 -> D_L20_002923C0
 *   - D_L20_001CA520 -> g_mapVertexData (task #2646): the name
 *     symbol_addrs/L20_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2 class,
 *     NOTE #13409).
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 *   - upstream's file comment folded into this one (it calls this the level-20
 *     copy of func_L00_00354DF8)
 *   - `#include "include_asm.h"` dropped
 */
int func_L20_0036BFB0(char *obj) {
    int i, entry;
    int *list;
    for (i = 0; i < 28 && g_anAvailableLevelOrder[i]; i++) {
        D_L20_0021E0A0[i].kind = 3;
        D_L20_0021E0A0[i].callback = D_L20_002923C0;
        D_L20_0021E0A0[i].id = D_L20_00296920[g_anAvailableLevelOrder[i]].id;
        D_L20_0021E0A0[i].value = D_L20_00296920[g_anAvailableLevelOrder[i]].value;
    }
    list = D_L20_001AA810;
    D_L20_0021E0A0[i].id = 0;
    *(int *)(obj + 0x40) = 0;
    *(unsigned int *)(obj + 0x30) |= 0x8000;
    for (i = 0, entry = list[0]; entry; entry = D_L20_001AA810[++i]) {
        if (g_mapVertexData.selected == entry) {
            *(int *)(obj + 0x40) = i;
            break;
        }
    }
    return 0;
}
