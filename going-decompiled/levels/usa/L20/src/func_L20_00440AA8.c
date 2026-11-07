/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x00440AA8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef int TI128 __attribute__((mode(TI)));

extern int D_L20_001B1F88 __attribute__((sda));
extern void func_L20_00301140(char *, int);
extern void func_L20_003014A8(TI128 *, char *, int, int, float, float);

/**
 * func_L20_00440AA8 - draw a moby's beam from its position to its state's target.
 *
 * Calls func_L20_00301140(moby + 0x10, D_L20_001B1F88), then copies the
 * moby's position as one 128-bit value and passes it with state + 0x10 to
 * func_L20_003014A8(&copy, state + 0x10, D_L20_001B1F88, 3, state +0x74,
 * 0.25).
 *
 * @arg0: the moby; its +0x68 points at the state.
 * Return: nothing.
 *
 * Non-obvious: the copy is one 128-bit TI-mode load/store (upstream's TI128
 * typedef). Upstream builds this file with -mno-split-addresses; this TU is
 * built without it and closes (D_L20_001B1F88 is $gp-relative at -G8).
 * Upstream's `sda` text is kept (ignored by this cc1).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_0043ECB0-g8.c, function func_L20_0043F510 (v1.01
 * address), author commit adcbd51e (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for
 * names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_0043F510 -> func_L20_00440AA8; D_L20_001B1FD8 ->
 *     D_L20_001B1F88; func_L00_002EC608 -> func_L20_00301140;
 *     func_L00_002EC970 -> func_L20_003014A8
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 *   - the file-local TI128 typedef (upstream's) carried with it
 */
void func_L20_00440AA8(char *arg0) {
    TI128 tmp;
    char *state = *(char **)(arg0 + 0x68);

    func_L20_00301140(arg0 + 0x10, D_L20_001B1F88);
    tmp = *(TI128 *)(arg0 + 0x10);
    func_L20_003014A8(&tmp, state + 0x10, D_L20_001B1F88, 3,
                      *(float *)(state + 0x74), 0.25f);
}
