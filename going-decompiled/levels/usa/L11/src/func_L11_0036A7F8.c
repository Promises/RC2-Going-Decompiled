/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2553,
 * RULING #13291 row L11-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x0036A7F8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack.
 * L24's func_L24_0034D598 is the same body at another level's addresses.
 */

typedef struct { unsigned short id, kind; void *payload; unsigned short value, pad; } InitEntry12;
extern int g_anAvailableLevelOrder[];
typedef struct { unsigned short id, pad, value; char rest[6]; } SourceEntry12;
extern SourceEntry12 D_L11_002A2C60[];
extern char D_L11_0029E6F0[];
/* An element inside L11's larger .bss label D_L11_0020FFF0, so address-named
 * (RULING #13291 §2). */
extern InitEntry12 D_L11_0021EAA0[];
/* Reloaded at each search step in retail. 4 bytes, so $gp-relative at -G8 by
 * its size alone; upstream's `sda` attribute is kept verbatim (cc1 ignores
 * it). */
extern int * volatile D_L11_001AA810 __attribute__((sda));
/* An element inside L11's .bss block D_L11_001B39B0, so address-named
 * (RULING #13291 §2). */
extern struct { char pad[0x230]; int selected; } D_L11_001CAF20;

/**
 * func_L11_0036A7F8 - build the level-ordered menu rows and select the
 * current owner's entry.
 *
 * @obj: the menu object; only +0x30 (flags) and +0x40 (selected row) are
 *       touched.
 *
 * For each level in g_anAvailableLevelOrder (at most 28, stopping at the
 * first 0), fill one 12-byte row of D_L11_0021EAA0: kind 3, the +4 field
 * pointing at the menu-data descriptor D_L11_0029E6F0, and id and value from
 * that level's entry in D_L11_002A2C60. A row with id 0 ends the list. Then
 * clear obj+0x40, set bit 0x8000 in obj+0x30, and walk the zero-terminated
 * list D_L11_001AA810 points to; the first index whose entry equals
 * D_L11_001CAF20.selected is stored to obj+0x40.
 * Return: always 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/11_joba/L11_00369700-init-g8.c, function
 * func_L11_00369700 (v1.01 address), author commit a96d3126 (Louis-Philippe
 * Le Sieur). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L11_00369700 -> func_L11_0036A7F8 (our split name at the
 *     v2.00 place);
 *   - D_00139948 -> g_anAvailableLevelOrder (the boot's name, same address);
 *   - D_L11_002A2B60 -> D_L11_002A2C60, D_L11_0029E610 -> D_L11_0029E6F0,
 *     D_L11_0021E9F0 -> D_L11_0021EAA0, D_L11_001AA7F0 -> D_L11_001AA810,
 *     D_L11_001CAEA0 -> D_L11_001CAF20: upstream names state v1.01
 *     addresses, ours are the targets of the v2.00 ROM's own relocations
 *     (relverify, task #2553);
 *   - `#include "include_asm.h"` dropped (nothing used it); the upstream file
 *     comment folded into this one, and a comment added on the three
 *     address-named externs.
 */
int func_L11_0036A7F8(char *obj) {
    int i, entry;
    int *list;
    for (i = 0; i < 28 && g_anAvailableLevelOrder[i]; i++) {
        D_L11_0021EAA0[i].kind = 3;
        D_L11_0021EAA0[i].payload = D_L11_0029E6F0;
        D_L11_0021EAA0[i].id = D_L11_002A2C60[g_anAvailableLevelOrder[i]].id;
        D_L11_0021EAA0[i].value = D_L11_002A2C60[g_anAvailableLevelOrder[i]].value;
    }
    list = D_L11_001AA810;
    D_L11_0021EAA0[i].id = 0;
    *(int *)(obj + 0x40) = 0;
    *(unsigned int *)(obj + 0x30) |= 0x8000;
    for (i = 0, entry = list[0]; entry; entry = D_L11_001AA810[++i]) {
        if (D_L11_001CAF20.selected == entry) {
            *(int *)(obj + 0x40) = i;
            break;
        }
    }
    return 0;
}
