/* Level 08 (Tabora), USA v2.00: a level-only function built from C (task
 * #2627, RULING #13291 row L08-G1). Compiled ALONE as its own TU and linked at
 * its one place, 0x003D1440, in place of its split asm; the row is
 * going-decompiled/levels/usa/L08/config/L08_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* The level's kind-7 spawn table. */
extern char D_L08_002A0D80[];
/* L08's copies of the shared spawn routines, at their v2.00 places. */
extern void func_L08_0038E260(void *, int);

/**
 * func_L08_003D1440 - release the level's kind-7 spawn table.
 *
 * Passes D_L08_002A0D80 and kind 7 to func_L08_0038E260. Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/08_tabora/L08_002D90C8.c, function
 * func_L08_003CFFE8 (v1.01 address), author commit b31a91e7 (MantraAU).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L08_003CFFE8 -> func_L08_003D1440 (our split name at the
 *     v2.00 place);
 *   - D_L08_002A0D00 -> D_L08_002A0D80 (the split label at the relverify
 *     disc-implied target 0x002A0D80);
 *   - func_L04_003A21F8 -> func_L08_0038E260 (L08's copy, named by the
 *     relverify disc-implied target 0x0038E260);
 *   - the file's `#include "include_asm.h"` line, the INCLUDE_ASM block,
 *     the other functions and their externs not carried (nothing else here
 *     uses them).
 */
void func_L08_003D1440(void) {
    func_L08_0038E260(D_L08_002A0D80, 7);
}
