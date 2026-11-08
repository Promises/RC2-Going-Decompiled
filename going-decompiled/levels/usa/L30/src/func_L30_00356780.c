/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00356780, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, plain as.
 */

typedef struct { unsigned short id, kind; void *payload; unsigned short value, pad; } InitEntry12;
typedef struct { unsigned short id, pad, value; char rest[6]; } SourceEntry12;

/* The boot's ordered available-level list (core, symbol_addrs). */
extern int g_anAvailableLevelOrder[];
/* L30's level table and menu-data descriptor, at their v2.00 split labels. */
extern SourceEntry12 D_L30_002922A0[];
extern char D_L30_0028DD40[];
/* The menu rows, inside a larger L30 label (g_pLevelSelectListEntries+0xA8
 * in L30's boot-name map), so address-named (RULING #13291 section 2). */
extern InitEntry12 D_L30_0021DDA0[];
/* The owner-list pointer, an .sdata word. Upstream's `volatile` makes it
 * reloaded at each search step, as in retail; at -G8 cc1 reaches it
 * $gp-relative by size, and upstream's `sda` marker is ignored by our cc1
 * (kept verbatim). */
extern int * volatile D_L30_001AA790 __attribute__((sda));
/* The name L30's boot-name map gives exactly 0x001CA220, seen through
 * upstream's view of the selected-owner word at +0x230. */
extern struct { char pad[0x230]; int selected; } g_mapVertexData;

/**
 * func_L30_00356780 - build the level menu's rows in level order and select
 * the current owner's entry.
 * @obj: the menu object.
 *
 * For each level of g_anAvailableLevelOrder (up to 28, stopping at 0) a row
 * of D_L30_0021DDA0 gets kind 3, the descriptor D_L30_0028DD40 and the id and
 * value of that level's D_L30_002922A0 entry; the next row's id is set to 0
 * as a terminator. Then @obj's +0x40 is cleared, bit 0x8000 of its +0x30
 * set, and the zero-terminated list at D_L30_001AA790 is searched (the
 * pointer reloaded at each step) for the selected owner (+0x230 of
 * g_mapVertexData); its index goes to @obj's +0x40.
 * Return: 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/30_insomniac_museum/L30_003557F0-init-g8.c, function
 * func_L30_003557F0 (v1.01 address), author commit a96d3126 (Louis-Philippe
 * Le Sieur; `git blame -M -C` gives all 21 definition lines to it, `git log
 * -S` agrees), a rename-copy of bigbeno37's func_L00_00354DF8 in
 * src/levels/00_aranos_tutorial/L00_002FC7A8-g8.c (author commit 6001495a,
 * earlier): token-identical but for symbol names (RULING #13427 rev 4,
 * validator #2680). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names, each name to the target of the
 * v2.00 disc's own relocation at this place (task #2673 relverify by name):
 *   - func_L30_003557F0 -> func_L30_00356780 (our split name);
 *   - D_00139948 -> g_anAvailableLevelOrder (the boot's symbol_addrs name at
 *     exactly 0x00139948);
 *   - D_L30_001CA3A0 -> g_mapVertexData (the name L30's boot-name map gives
 *     exactly 0x001CA220, the v2.00 target; upstream's struct view kept);
 *   - D_L30_001AA770 -> D_L30_001AA790, D_L30_0028DE60 -> D_L30_0028DD40,
 *     D_L30_002923A0 -> D_L30_002922A0 (our split labels at the v2.00
 *     targets); D_L30_0021DEF0 -> D_L30_0021DDA0 (the v2.00 target, inside
 *     a larger label, address-named);
 *   - `#include "include_asm.h"` dropped; upstream's file comment folded
 *     into this one.
 */
int func_L30_00356780(char *obj) {
    int i, entry;
    int *list;
    for (i = 0; i < 28 && g_anAvailableLevelOrder[i]; i++) {
        D_L30_0021DDA0[i].kind = 3;
        D_L30_0021DDA0[i].payload = D_L30_0028DD40;
        D_L30_0021DDA0[i].id = D_L30_002922A0[g_anAvailableLevelOrder[i]].id;
        D_L30_0021DDA0[i].value = D_L30_002922A0[g_anAvailableLevelOrder[i]].value;
    }
    list = D_L30_001AA790;
    D_L30_0021DDA0[i].id = 0;
    *(int *)(obj + 0x40) = 0;
    *(unsigned int *)(obj + 0x30) |= 0x8000;
    for (i = 0, entry = list[0]; entry; entry = D_L30_001AA790[++i]) {
        if (g_mapVertexData.selected == entry) {
            *(int *)(obj + 0x40) = i;
            break;
        }
    }
    return 0;
}
