/* Level 03 (Endako), USA v2.00: a level-only function built from C
 * (task #2655, RULING #13291 row L03-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003F7D18, in place of its split asm; the row is
 * going-decompiled/levels/usa/L03/config/L03_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* The upstream file's own typedefs (it includes include_asm.h, not
 * common.h). */
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef int s32;
/* L03's copy of a level helper, at its v2.00 place. */
extern void func_L03_003A4478(void *entry);
/* The level's entity table base, L03's boot-name map name at 0x001B2A1C.
 * Upstream: `lui $v0,%hi ; lw $v0,%lo($v0)` in the retail, one register, no
 * HIGH/LO_SUM split, which is what the sda attribute gives in -G0. Here it is
 * the ADDRESSING-MODEL DEVICE of RULING #8620: a section(".sdata") attribute
 * on the extern DECLARATION (no data moved, nothing emitted) that only
 * changes how cc1 addresses it. Ablation (this TU, s136 -G0, plain as,
 * masked compare at the place): without it the function is 0x78 bytes and 28
 * of 30 words differ from the disc, with it 0x70 bytes and 0 of 28. */
extern u8 *g_mobyTableBase __attribute__((section(".sdata")));
/* The level's per-group record streams, L03's boot-name map name at
 * 0x001FB5A0. */
extern u8 *g_mobyGroupLists[];
/* Upstream: one word of a stream: bits 0..14 name a record, bit 15 ends the
 * walk. */
typedef struct {
    u16 raw;
} StreamEntry;

/**
 * func_L03_003F7D18 - hand every record of the entity's stream to
 * func_L03_003A4478.
 *
 * @entity: the entity; its state block pointer is at +0x68, and the block's
 *          word +0xAC is the stream index (-1 = none).
 * Return: nothing.
 *
 * The stream is g_mobyGroupLists[index] (none when null). Each 16-bit entry
 * names a 256-byte record of g_mobyTableBase by its low 15 bits; the walk
 * stops after the entry whose bit 15 is set.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/03_endako/L03_003F09C0.c, function
 * func_L03_003F66D8 (v1.01 address), author commit 431ac503 (llesieur99;
 * `git blame -M -C` names it for all 14 definition lines at fe6adf9b, and
 * `git log --reverse -S'func_L03_003F66D8('` agrees; the file changed
 * upstream after b2a35115, this body did not). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; edits:
 *   - name func_L03_003F66D8 -> func_L03_003F7D18 (our split name at the
 *     v2.00 place);
 *   - `__attribute__((sda))` written `__attribute__((section(".sdata")))`
 *     (RULING #8620 addressing-model device on an extern declaration,
 *     ablation above);
 *   - D_L03_001B2A1C -> g_mobyTableBase and D_L03_001FB5A0 ->
 *     g_mobyGroupLists, the names L03's boot-name map gives at those
 *     addresses (the v2.00 ROM's relocation targets equal the v1.01 names'
 *     addresses; the NAMES rule: no second spelling);
 *   - callee func_3A2F48 -> func_L03_003A4478 (L03's copy, the v2.00 ROM's
 *     call target);
 *   - `#include "include_asm.h"`, the file's other functions and their
 *     externs not carried; upstream's comments folded into these.
 */
void func_L03_003F7D18(u8 *entity) {
    u8 *state = *(u8 **)(entity + 0x68);
    s32 index = *(s32 *)(state + 0xAC);
    StreamEntry *entry;

    if (index == -1) return;
    entry = (StreamEntry *)g_mobyGroupLists[index];
    if (entry == 0) return;
    do {
        func_L03_003A4478(g_mobyTableBase + ((entry->raw & 0x7FFF) << 8));
        if ((s16)entry->raw < 0) break;
        entry++;
    } while (1);
}
