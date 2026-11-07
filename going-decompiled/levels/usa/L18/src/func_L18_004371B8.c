/* Level 18 (Damosel), USA v2.00: a level-only function built from C (task
 * #2662, RULING #13291 row L18-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x004371B8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L18/config/L18_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef signed int s32;

extern void func_L18_0039BD28(char *, s32);
extern char D_L18_002A9C00[];

/**
 * func_L18_004371B8 - start the level's table D_L18_002A9C00 with code 0x12.
 *
 * Calls func_L18_0039BD28(D_L18_002A9C00, 0x12).
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/18_damosel/L18_00433460.c,
 * function func_L18_00435CC8 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L18_00435CC8 -> func_L18_004371B8 (our split name at the v2.00 place);
 *   - D_L18_002A9C40 -> D_L18_002A9C00 (our split label at the v2.00 address);
 *   - func_L04_003A21F8 -> func_L18_0039BD28 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept (the common.h typedefs it uses written out as common.h defines them).
 * The audit (NOTE #13281) closed this member through its `long long`->`long`
 * TU variant only because the upstream file as a whole fails on this arm;
 * this function's own text has no `long long` (nor s64/u64), so alone in its
 * TU it needs no respelling, and none is made.
 */
void func_L18_004371B8(void) {
    func_L18_0039BD28(D_L18_002A9C00, 0x12);
}
