/* Level 10 (Hrugis Cloud), USA v2.00: a level-only function built from C
 * (task #2657, RULING #13291 row L10-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x0041A728, in place of its split asm; the row is
 * going-decompiled/levels/usa/L10/config/L10_c.tsv, the arm ee-gcc
 * 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0, plain as.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;

/* Id lists, one per group: L10's boot-name map name at 0x001FB160. */
extern u32 g_mobyGroupLists[];

/* The base of the level's 256-byte object records, L10's boot-name map name
   at 0x001B249C, read through an incomplete array. ADDRESSING-MODEL DEVICE
   (RULING #8620) on this declaration: upstream's `sda` (ignored by this cc1)
   is respelled `section(".sdata")`, so cc1 prints a one-instruction macro
   load that the assembler expands at -G0 to the absolute lui/lw pair the ROM
   has. Ablation (task #2657, upstream's own `sda`, same arm and flags): 21
   words where the disc has 22, 13 differing. It moves no data and emits
   nothing. */
extern u8 *g_mobyTableBase[] __attribute__((section(".sdata")));

/**
 * func_L10_0041A728 - set every object of this object's group to state 2.
 *
 * The group's id list is g_mobyGroupLists[@arg0's byte at +0x21]; a null
 * list does nothing. Each halfword's low 15 bits index a 256-byte record from
 * g_mobyTableBase, whose state byte (+0x20) becomes 2; the list ends after
 * the first id with its sign bit set.
 *
 * @arg0: the object.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/10_hrugis_cloud/L10_003170D0.c, function
 * func_L10_00419188 (v1.01 address), author commit 6c919b91 (Lynder063),
 * carrying Louis-Philippe Le Sieur's nonmatching draft
 * nonmatching/levels/func_L10_00419188.c (b9973721; its 10b13e2c lines that
 * survive are trivial) and writing the loop body (`git blame -M -C` at the
 * pin). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED; the body's statements unchanged, the edits:
 *   - func_L10_00419188 -> func_L10_0041A728 (our split name at the v2.00 place);
 *   - D_L10_001FB1E0 -> g_mobyGroupLists: the name
 *     symbol_addrs/L10_boot_names.txt gives at the v2.00 target 0x001FB160
 *     (task #2657 relverify);
 *   - upstream reads the record base through an asm-label view,
 *     `extern u8 *D_001B251C_base[] __asm__("D_001B251C")
 *     __attribute__((sda))`. This one-function TU has only the one
 *     declaration, so it is written directly as g_mobyTableBase, the name
 *     L10's boot-name map gives at the v2.00 target 0x001B249C, with the same
 *     type and no asm label (no device; as func_L11_00442A18 did). The use is
 *     respelled D_001B251C_base[0] -> g_mobyTableBase[0];
 *   - device: `__attribute__((sda))` -> `__attribute__((section(".sdata")))`
 *     on that declaration (RULING #8620; the audit's `resp` variant, NOTE
 *     #13281);
 *   - the task's row also listed address-named externs (code `o`); none is
 *     needed: both targets now carry boot-name map names (task #2646);
 *   - `#include "common.h"` dropped: the typedefs used are written here and
 *     only this function's declarations kept.
 */
void func_L10_0041A728(u8 *arg0)
{
    u32 s0 = g_mobyGroupLists[arg0[0x21]];
    s16 *p;
    u8 *q;

    if (s0 == 0) {
        return;
    }
    p = (s16 *)s0;
    do {
        q = g_mobyTableBase[0] + (((*(u16 *)p & 0x7FFF) << 8));
        *(q + 0x20) = 2;
    } while (*p++ >= 0);
}
