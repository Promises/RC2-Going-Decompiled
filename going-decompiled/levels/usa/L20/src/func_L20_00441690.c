/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task
 * #2566, RULING #13291 row L20-B). Compiled ALONE as its own TU and linked
 * at its one place, 0x00441690, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern float D_L20_001B1FD4 __attribute__((sda));
extern void func_L20_0033C3D8(void *, void *, int, int, float);

/**
 * func_L20_00441690 - advance an object's saturating 16-bit counter and pass
 * its state's +0x2B0 block to func_L20_0033C3D8.
 *
 * @arg0: the object; +0x68 points to its state, +0x96 is a u16 counter.
 * Return: none.
 *
 * Non-obvious: The counter stops at 0xFFFF: it is only incremented while it is
 * <= 0xFFFE. The call is func_L20_0033C3D8(arg0, state + 0x2B0, 2, 1,
 * D_L20_001B1FD4). D_L20_001B1FD4 is a 4-byte float, so at -G8 cc1 reaches it
 * $gp-relative by its size. Upstream's `__attribute__((sda))` on it is kept as
 * written; this cc1 ignores it (a warning), and the member closes with it as
 * written.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/20_yeedil/L20_0043ECB0-g8.c (blob 3c384040), function
 * func_L20_00440150 (v1.01 address), author commit ee3fee45 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Upstream builds the file with
 * -mno-split-addresses; the row does not pass it. Measured inert (task #2566):
 * this TU at s136 -O2 -G8 with and without the flag gives an identical .s and
 * .o. Respellings, each to the target of the v2.00 disc's own relocation at
 * this place (relverify by name on this object, task #2566):
 *   - func_L20_00440150 -> func_L20_00441690 (our split name at the v2.00
 *     place);
 *   - D_L20_001B2024 -> D_L20_001B1FD4 (our label at that address);
 *   - func_002B0990 -> func_L20_0033C3D8 (the level's own copy at its L20
 *     place);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning), and the member closes with it as written;
 *   - `#include "include_asm.h"` dropped; the typedefs this body uses are
 *     written here (or are the file's own); upstream file-scope declarations
 *     it does not use are not carried, the rest keep upstream's order and
 *     text, duplicates included.
 */
void func_L20_00441690(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    unsigned short n = *(unsigned short *)(arg0 + 0x96);

    if (n <= 0xFFFE) {
        *(unsigned short *)(arg0 + 0x96) = n + 1;
    }
    func_L20_0033C3D8(arg0, state + 0x2B0, 2, 1, D_L20_001B1FD4);
}
