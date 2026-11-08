/* Level 17 (Smolg), USA v2.00: a level-only function built from C (task
 * #2667, RULING #13291 row L17-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003595C8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L17/config/L17_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack (upstream builds its file at -G8), plain as.
 */

typedef struct { unsigned short id, kind; void *payload; unsigned short value, pad; } InitEntry12;
extern int g_anAvailableLevelOrder[];
typedef struct { unsigned short id, pad, value; char rest[6]; } SourceEntry12;
extern SourceEntry12 D_L17_0028C4A0[];
extern char D_L17_00287F40[];
extern InitEntry12 D_L17_0021DD20[];
extern int * volatile D_L17_001AA790 __attribute__((sda));
extern struct { char pad[0x230]; int selected; } g_mapVertexData;

/**
 * func_L17_003595C8 - build the level-menu rows in level order and select
 * the current owner's entry.
 *
 * @obj: the menu object.
 * Return: 0.
 *
 * For each nonzero entry of g_anAvailableLevelOrder (at most 28) it fills a
 * 12-byte row of D_L17_0021DD20: kind 3, payload D_L17_00287F40, and the id
 * and value of that level's entry in D_L17_0028C4A0. The row after the last
 * gets id 0. It clears obj+0x40, sets bit 0x8000 of obj+0x30, then walks the
 * list that D_L17_001AA790 points to; at the first entry equal to the
 * selected word (+0x230) of g_mapVertexData it stores that index in obj+0x40.
 *
 * Non-obvious: D_L17_001AA790 is declared `int * volatile`, so the list
 * pointer is re-read at every search step, as retail does. Upstream's
 * `__attribute__((sda))` on it is not an attribute our cc1 knows (it warns
 * "directive ignored"); the 4-byte extern is $gp-addressed at -G8 anyway.
 * g_mapVertexData is the name this level's boot-name map gives 0x001CA1A0;
 * upstream's struct view of it is kept.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/17_smolg/L17_003583F8-init-g8.c, function func_L17_003583F8 (v1.01 address), author commit a96d3126 (Louis-Philippe Le
 * Sieur), a rename-copy of bigbeno37's func_L00_00354DF8 in
 * src/levels/00_aranos_tutorial/L00_002FC7A8-g8.c (author commit 6001495a),
 * token-identical but for symbol names and the struct field callback ->
 * payload (RULING #13427 rev 4).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L17_003583F8 -> func_L17_003595C8 (our split name at the v2.00 place);
 *   - D_00139948 -> g_anAvailableLevelOrder (our boot name at that address);
 *   - D_L17_001AA770 -> D_L17_001AA790 (our split label at the v2.00 address);
 *   - D_L17_001CA1A0 -> g_mapVertexData: the name symbol_addrs/L17_boot_names.txt gives at
 *   exactly 0x001CA1A0, where the split has no label of its own (NAMES rule,
 *   task #2646 class A2);
 *   - D_L17_0021DCF0 -> D_L17_0021DD20: an address-named extern (RULING #13291 section 2) at
 *   the v2.00 relocation target 0x0021DD20, inside the split label
 *   D_L17_0020FFF0 (+0xDD30); no name exists at that exact address in
 *   the boot splits (labels or operands), symbol_addrs.txt, L17_boot_names.txt
 *   or the level's split labels (grep with firing controls, task #2667);
 *   - D_L17_00287EE0 -> D_L17_00287F40 (our split label at the v2.00 address);
 *   - D_L17_0028C420 -> D_L17_0028C4A0 (our split label at the v2.00 address);
 *   - upstream's `__attribute__((sda))` kept verbatim (our cc1 ignores it with a
 *   warning; the measured bytes are with it in place);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
int func_L17_003595C8(char *obj) {
    int i, entry;
    int *list;
    for (i = 0; i < 28 && g_anAvailableLevelOrder[i]; i++) {
        D_L17_0021DD20[i].kind = 3;
        D_L17_0021DD20[i].payload = D_L17_00287F40;
        D_L17_0021DD20[i].id = D_L17_0028C4A0[g_anAvailableLevelOrder[i]].id;
        D_L17_0021DD20[i].value = D_L17_0028C4A0[g_anAvailableLevelOrder[i]].value;
    }
    list = D_L17_001AA790;
    D_L17_0021DD20[i].id = 0;
    *(int *)(obj + 0x40) = 0;
    *(unsigned int *)(obj + 0x30) |= 0x8000;
    for (i = 0, entry = list[0]; entry; entry = D_L17_001AA790[++i]) {
        if (g_mapVertexData.selected == entry) {
            *(int *)(obj + 0x40) = i;
            break;
        }
    }
    return 0;
}
