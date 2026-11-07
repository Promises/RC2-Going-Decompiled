/* Level 08 (Tabora), USA v2.00: a level-only function built from C (task
 * #2627, RULING #13291 row L08-G1). Compiled ALONE as its own TU and linked at
 * its one place, 0x003D1498, in place of its split asm; the row is
 * going-decompiled/levels/usa/L08/config/L08_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* L08's copy of the shared joint-fetch helper, at its v2.00 place. */
extern void func_L08_0031F7C8(void *, int, void *);

/**
 * func_L08_003D1498 - fetch an object's first two part joints.
 *
 * @object: the object; its part-handle list pointer is at +0x68, each
 *   part's joint sits at part + 0x10.
 *
 * For parts 0 and 1 of the list: when the part handle is set, fetches the
 * part's joint with func_L08_0031F7C8 (index 0, then 1). Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/08_tabora/L08_002D90C8.c, function
 * func_L08_003D0040 (v1.01 address), author commit b31a91e7 (MantraAU).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; edits:
 *   - name func_L08_003D0040 -> func_L08_003D1498 (our split name at the
 *     v2.00 place);
 *   - func_0029FCC0 -> func_L08_0031F7C8 (L08's copy, named by the relverify
 *     disc-implied target 0x0031F7C8);
 *   - the file's `#include "include_asm.h"` line, the other functions and
 *     their externs not carried (nothing else here uses them).
 */
void func_L08_003D1498(void *object) {
    void **parts = *(void ***)((char *)object + 0x68);
    if (parts[0]) func_L08_0031F7C8(object, 0, (char *)parts[0] + 0x10);
    if (parts[1]) func_L08_0031F7C8(object, 1, (char *)parts[1] + 0x10);
}
