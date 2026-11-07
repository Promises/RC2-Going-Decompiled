/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2564, RULING #13291 row L14-B). Compiled ALONE as its own TU and
 * linked at its one place, 0x003606E8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack (upstream
 * builds its file at -G8; the audit's arm for this member is s136).
 */

typedef struct { unsigned short id, kind; void *callback; unsigned short value, pad; } InitEntry12;
typedef struct { unsigned short id, pad, value; char rest[6]; } SourceEntry12;

extern int g_anAvailableLevelOrder[];
extern SourceEntry12 D_L14_00291720[];
extern char D_L14_0028D1B0[];
extern InitEntry12 D_L14_0021D5E0[];
/* Reloaded at each search step in retail. */
extern int * volatile D_L14_001AA790 __attribute__((sda));
extern struct { char pad[0x230]; int selected; } D_L14_001C9A60;

/**
 * func_L14_003606E8 - fill the object's init-entry list from the level-order
 * table.
 *
 * @obj: the object; its selected-slot word is +0x40 and its flags word
 *       +0x30.
 * Return: always 0.
 *
 * Non-obvious: for each non-zero id in g_anAvailableLevelOrder (up to 28),
 * entry i of the callback table is filled as kind 3 with the callback and the
 * id/value read from the source table at that id; entry i then gets the
 * terminator. The list pointer is re-read through D_L14_001AA790 each step of
 * the second loop (the volatile), which searches the selected slot of the
 * scratch record D_L14_001C9A60 and stores its index at obj +0x40; the flags
 * word gains 0x8000 either way.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_0035F510-g8.c,
 * function func_L14_0035F510 (v1.01 address), author commit 7c3ea917
 * (Fromnowhere), which copied the text, token-identical but for level
 * names, from func_L00_00354DF8, written by bigbeno37 in 6001495a (RULING
 * #13427 rev 4, validator #2631). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L14_0035F510 -> func_L14_003606E8 (our split name at the
 *     v2.00 place);
 *   - D_00139948 -> g_anAvailableLevelOrder (our boot name at that address;
 *     L14 core_externs.txt line 200);
 *   - D_L14_001AA770 -> D_L14_001AA790 (our label at the v2.00 address the
 *     relocation implies);
 *   - D_L14_0028D1D0 -> D_L14_0028D1B0 (our label at that address);
 *   - D_L14_00291720 kept (the v2.00 label at that address has the same
 *     name);
 *   - D_L14_0021D630 -> D_L14_0021D5E0 and D_L14_001C9AE0 -> D_L14_001C9A60:
 *     address-named externs (RULING #13291 section 2, the L6 route) - the
 *     v2.00 targets lie inside larger split data labels (relverify:
 *     D_L14_0020FFF0+0xD5F0 and D_L14_001B28B0+0x171B0), and no label of
 *     either name exists at the exact address (grep of the boot splits,
 *     going-decompiled/symbol_addrs/usa/symbol_addrs.txt and the level's own
 *     split labels, with firing controls);
 *   - `#include "include_asm.h"` dropped (defines nothing the body uses);
 *     the two file-local typedefs InitEntry12/SourceEntry12 and upstream's
 *     declaration forms kept verbatim; the `extern int D_00139948[]` line
 *     (now g_anAvailableLevelOrder) sits after the second typedef rather
 *     than before it (order only, byte-neutral; validator #2631).
 */
int func_L14_003606E8(char *obj) {
    int i, entry;
    int *list;
    for (i = 0; i < 28 && g_anAvailableLevelOrder[i]; i++) {
        D_L14_0021D5E0[i].kind = 3;
        D_L14_0021D5E0[i].callback = D_L14_0028D1B0;
        D_L14_0021D5E0[i].id = D_L14_00291720[g_anAvailableLevelOrder[i]].id;
        D_L14_0021D5E0[i].value = D_L14_00291720[g_anAvailableLevelOrder[i]].value;
    }
    list = D_L14_001AA790;
    D_L14_0021D5E0[i].id = 0;
    *(int *)(obj + 0x40) = 0;
    *(unsigned int *)(obj + 0x30) |= 0x8000;
    for (i = 0, entry = list[0]; entry; entry = D_L14_001AA790[++i]) {
        if (D_L14_001C9A60.selected == entry) {
            *(int *)(obj + 0x40) = i;
            break;
        }
    }
    return 0;
}
