/* Level 24 (Ship Shack), USA v2.00: the first function of a level program built
 * from C (task #2495, NOTE #13098 row L6). Compiled ALONE as its own TU and
 * linked at its one place, 0x0034D598, in place of its split asm; the row is
 * going-decompiled/levels/usa/L24/config/L24_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack.
 */

typedef struct {
    unsigned short id, kind;
    void *payload;
    unsigned short value, pad;
} InitEntry12;

typedef struct {
    unsigned short id, pad, value;
    char rest[6];
} SourceEntry12;

/* The boot's resident order of available levels (core, below 0x1A7C80). */
extern int g_anAvailableLevelOrder[];
extern SourceEntry12 D_L24_0028AE20[];
extern char D_L24_002868C0[];
extern InitEntry12 D_L24_0021C7A0[];
/* 4 bytes, so $gp-relative at -G8 by its size alone: no section device. */
extern int *D_L24_001AA790;
extern struct {
    char pad[0x230];
    int selected;
} D_L24_001C8C20;

/**
 * func_L24_0034D598 - build the level-ordered menu rows and select the
 * current owner's entry.
 *
 * For each level in g_anAvailableLevelOrder (at most 28, stopping at the first
 * 0), fill one 12-byte row of D_L24_0021C7A0. The row's kind is 3, its +4 field
 * points at the menu-data descriptor D_L24_002868C0, and its id and value come
 * from that level's entry in D_L24_0028AE20. A row with id 0 ends the list.
 * Then clear obj+0x40, set bit 0x8000 in obj+0x30, and walk the
 * zero-terminated list that D_L24_001AA790 points to. The first index whose
 * entry equals D_L24_001C8C20.selected is stored to obj+0x40.
 *
 * @obj: the menu object; only +0x30 (flags) and +0x40 (selected row) are touched.
 * Return: always 0.
 *
 * Non-obvious: the search loop re-reads D_L24_001AA790 ($gp-relative) on every
 * step, as the ROM does. The cc1 reproduces that without upstream's
 * `volatile` on the pointer; removing the qualifier was measured codegen-inert.
 * No codegen or addressing device is used.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * 5833be5b716983046be737d85080b96bc9f1435a (HEAD on 2026-11-06; the file's
 * blob 69442f63 is identical at b2a35115e0cf580a1fdab8dd1071b157799e590e),
 * GPL-3.0 (root LICENSE; the file carries no notice of its own), file
 * src/levels/24_ship_shack/L24_0034C450-init-g8.c, function func_L24_0034C450
 * (v1.01 address), author commit a96d3126 (Louis-Philippe Le Sieur). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082). ADAPTED, the
 * body's statements unchanged; respellings:
 *   - name func_L24_0034C450 -> func_L24_0034D598 (our split name at the v2.00
 *     place; the level's .data holds a `.word` to it);
 *   - D_00139948 -> g_anAvailableLevelOrder (the boot's name, same address);
 *   - D_L24_0021C7F0 -> D_L24_0021C7A0, D_L24_002868E0 -> D_L24_002868C0,
 *     D_L24_001AA770 -> D_L24_001AA790, D_L24_001C8CA0 -> D_L24_001C8C20:
 *     upstream names state v1.01 addresses, ours are the targets of the v2.00
 *     ROM's own relocations (task #2495 per-function compare);
 *   - type: `int * volatile` -> `int *` on D_L24_001AA790 (qualifier dropped,
 *     measured inert), and its `__attribute__((sda))` dropped (inert at -G8);
 *   - `#include "include_asm.h"` dropped (nothing in the file used it);
 *   - the typedefs and externs reformatted, one field per line, and the
 *     upstream file comment folded into this one.
 */
int func_L24_0034D598(char *obj) {
    int i, entry;
    int *list;
    for (i = 0; i < 28 && g_anAvailableLevelOrder[i]; i++) {
        D_L24_0021C7A0[i].kind = 3;
        D_L24_0021C7A0[i].payload = D_L24_002868C0;
        D_L24_0021C7A0[i].id = D_L24_0028AE20[g_anAvailableLevelOrder[i]].id;
        D_L24_0021C7A0[i].value = D_L24_0028AE20[g_anAvailableLevelOrder[i]].value;
    }
    list = D_L24_001AA790;
    D_L24_0021C7A0[i].id = 0;
    *(int *)(obj + 0x40) = 0;
    *(unsigned int *)(obj + 0x30) |= 0x8000;
    for (i = 0, entry = list[0]; entry; entry = D_L24_001AA790[++i]) {
        if (D_L24_001C8C20.selected == entry) {
            *(int *)(obj + 0x40) = i;
            break;
        }
    }
    return 0;
}
