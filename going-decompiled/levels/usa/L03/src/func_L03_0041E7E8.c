/* Level 03 (Endako), USA v2.00: a level-only function built from C
 * (task #2655, RULING #13291 row L03-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x0041E7E8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L03/config/L03_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* The upstream file's own typedefs (it includes include_asm.h, not
 * common.h). */
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
/* L03's copy of a boot helper (a random integer in a range), at its v2.00
 * place. */
extern int func_L03_0031CDA8(int a, int b);
/* The level's entity table base, L03's boot-name map name at 0x001B2A1C.
 * Upstream marks it `sda`; here it is the ADDRESSING-MODEL DEVICE of RULING
 * #8620: a section(".sdata") attribute on the extern DECLARATION (no data
 * moved, nothing emitted) that only changes how cc1 addresses it. Ablation
 * (this TU, s136 -G0, plain as, masked compare at the place): without it the
 * function is 0xB8 bytes and 42 of 46 words differ from the disc, with it
 * 0xB0 bytes and 0 of 44. */
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
 * func_L03_0041E7E8 - give every live 0xFF3 record of the entity's stream a
 * fresh random value.
 *
 * @entity: the entity; its byte +0x21 is the stream index (0xFF = none).
 * Return: nothing.
 *
 * Upstream's note: walks the entity's stream, asking func_31BA30 (here
 * func_L03_0031CDA8) for the value it stores at +0x2D8 of every record whose
 * halfword at +0xAA is 0xFF3. The stream is g_mobyGroupLists[index] (none
 * when null); each 16-bit entry names a 256-byte record of g_mobyTableBase
 * by its low 15 bits, and the walk stops after the entry whose bit 15 is set.
 * A record counts only while its byte +0x20 is non-negative; the value,
 * func_L03_0031CDA8(0x28, 0x32), goes to +0x2D8 of the record's own state
 * block (+0x68).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/03_endako/L03_003F09C0.c, function
 * func_L03_0041D128 (v1.01 address), author commit 431ac503 (llesieur99;
 * `git blame -M -C` names it for all 22 definition lines at fe6adf9b, and
 * `git log --reverse -S'func_L03_0041D128('` agrees; the file changed
 * upstream after b2a35115, this body did not). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; edits:
 *   - name func_L03_0041D128 -> func_L03_0041E7E8 (our split name at the
 *     v2.00 place);
 *   - `__attribute__((sda))` written `__attribute__((section(".sdata")))`
 *     (RULING #8620 addressing-model device on an extern declaration,
 *     ablation above);
 *   - D_L03_001B2A1C -> g_mobyTableBase and D_L03_001FB5A0 ->
 *     g_mobyGroupLists, the names L03's boot-name map gives at those
 *     addresses (the v2.00 ROM's relocation targets equal the v1.01 names'
 *     addresses; the NAMES rule: no second spelling);
 *   - callee func_31BA30 -> func_L03_0031CDA8 (L03's copy, the v2.00 ROM's
 *     call target);
 *   - `#include "include_asm.h"`, the file's other functions and their
 *     externs not carried; upstream's comments folded into these.
 */
void func_L03_0041E7E8(u8 *entity) {
    StreamEntry *entry;
    StreamEntry *p;
    u8 *record;

    if (entity[0x21] == 0xFF) return;
    entry = (StreamEntry *)g_mobyGroupLists[entity[0x21]];
    if (entry == 0) return;
    p = entry;
    do {
        record = g_mobyTableBase + ((p->raw & 0x7FFF) << 8);
        if (*(signed char *)(record + 0x20) >= 0) {
            if (*(short *)(record + 0xAA) == 0xFF3) {
                u8 *owner = *(u8 **)(record + 0x68);

                *(int *)(owner + 0x2D8) = func_L03_0031CDA8(0x28, 0x32);
            }
        }
        if ((s16)p->raw < 0) break;
        p++;
    } while (1);
}
