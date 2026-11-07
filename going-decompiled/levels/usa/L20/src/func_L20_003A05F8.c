/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x003A05F8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern void func_L20_003A0630(char *);
extern void func_L20_003A0928(char *);
extern void func_L20_003A0D30(char *);

/**
 * func_L20_003A05F8 - run a moby's three per-frame steps.
 *
 * Calls func_L20_003A0630, func_L20_003A0928 and func_L20_003A0D30 on the
 * moby, in that order.
 *
 * @arg0: the moby.
 * Return: nothing.
 *
 * Non-obvious: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_0039C448.c, function func_L20_0039F230 (v1.01
 * address), author commit fa8d6672 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for
 * names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_0039F230 -> func_L20_003A05F8; func_L20_0039F268 ->
 *     func_L20_003A0630; func_L20_0039F560 -> func_L20_003A0928;
 *     func_L20_0039F968 -> func_L20_003A0D30
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 */
void func_L20_003A05F8(char *arg0) {
    func_L20_003A0630(arg0);
    func_L20_003A0928(arg0);
    func_L20_003A0D30(arg0);
}
