/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x00444BD0, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 4 of 22
 * .text words differ from the disc, relocated fields masked.
 */

typedef int s32;
extern s32 D_L11_001B1510 __attribute__((sda));
extern s32 D_L11_001B1520 __attribute__((sda));
extern s32 D_L11_001B1530 __attribute__((sda));
extern s32 D_L11_001B1540 __attribute__((sda));
extern void func_L11_003011A8(void *);
extern void func_L11_00489DB8(void *, void *, s32, s32, s32);
extern void func_L11_00489D30(s32, s32);
extern void func_L11_00489C70(void *);
extern void func_L11_00489CC8(void *);

/**
 * func_L11_00444BD0 - start the level's scripted bridge over its measured
 * buffers on the shared slot.
 *
 * Takes no arguments.
 * Return: none.
 *
 * Non-obvious: It fills a 16-byte local with func_L11_003011A8, then arms
 * func_L11_00489DB8 over the level's two descriptors D_L11_001B1530 and
 * D_L11_001B1540 (arguments 1, 0, 0). It calls func_L11_00489D30(2, 0x168) and
 * hands D_L11_001B1510 and D_L11_001B1520 to func_L11_00489C70 and
 * func_L11_00489CC8.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_0043E1A0-g8.c (blob 53f934ac), function
 * func_L11_00443380 (v1.01 address), author commit 264cb2be (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2578):
 *   - func_L11_00443380 -> func_L11_00444BD0 (our split name at the v2.00
 *     place);
 *   - D_L11_001B1530 -> D_L11_001B1510 (our label at that address);
 *   - D_L11_001B1540 -> D_L11_001B1520 (our label at that address);
 *   - D_L11_001B1550 -> D_L11_001B1530 (our label at that address);
 *   - D_L11_001B1560 -> D_L11_001B1540 (our label at that address);
 *   - func_3004A8 -> func_L11_003011A8 (the level's own copy at its L11
 *     place);
 *   - func_4881A8 -> func_L11_00489C70 (the level's own copy at its L11
 *     place);
 *   - func_488200 -> func_L11_00489CC8 (the level's own copy at its L11
 *     place);
 *   - func_488268 -> func_L11_00489D30 (the level's own copy at its L11
 *     place);
 *   - func_4882F0 -> func_L11_00489DB8 (the level's own copy at its L11
 *     place);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning), and the member closes with it as written;
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_00444BD0(void) {
    s32 buffer[4];
    func_L11_003011A8(buffer);
    func_L11_00489DB8(&D_L11_001B1530, &D_L11_001B1540, 1, 0, 0);
    func_L11_00489D30(2, 0x168);
    func_L11_00489C70(&D_L11_001B1510);
    func_L11_00489CC8(&D_L11_001B1520);
}
