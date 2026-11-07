/* Level 10 (Hrugis Cloud), USA v2.00: a level-only function built from C
 * (task #2657, RULING #13291 row L10-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x004191E0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L10/config/L10_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef unsigned int u32;
typedef signed int s32;

/* The level's copies of boot code and the level's own helpers, named by
   their v2.00 places. */
extern u32 *func_L10_0037CCF8(u8 *arg0);
extern void func_L10_0032DE28(u8 *arg0, s32 *a1, s32 *a2, s32 *a3);
extern void func_L10_0032DDF8(u8 *arg0, s32 a1, s32 a2, s32 a3);
extern void func_L10_004194D0(u8 *arg0);
extern void func_L10_00419628(u8 *arg0);

/**
 * func_L10_004191E0 - per-frame update of an object that follows its
 * parent's position.
 *
 * States 0 and 1 only (0 first becomes 1): copies the three words
 * func_L10_0032DE28 reads from the parent (the state's (+0x68) pointer at
 * +0x10) onto the object through func_L10_0032DDF8, then runs
 * func_L10_004194D0, and func_L10_00419628 too when the parent's record from
 * func_L10_0037CCF8 has a non-zero word at +0x120. Any other state returns
 * at once.
 *
 * @arg0: the object.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/10_hrugis_cloud/L10_003170D0.c, function
 * func_L10_00417C40 (v1.01 address), author commit 431ac503 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names (no device); the
 * respellings, each to the target of the v2.00 disc's own relocation at this
 * place (task #2657 relverify):
 *   - func_L10_00417C40 -> func_L10_004191E0 (our split name at the v2.00 place);
 *   - func_0032CBD0 -> func_L10_0032DE28, func_0032CBA0 -> func_L10_0032DDF8,
 *     func_0037BA88 -> func_L10_0037CCF8, func_L10_00417F30 ->
 *     func_L10_004194D0, func_L10_00418088 -> func_L10_00419628 (our split
 *     names at the v2.00 targets);
 *   - `#include "common.h"` dropped: the typedefs used are written here and
 *     only this function's declarations kept.
 */
void func_L10_004191E0(u8 *arg0) {
    u8 *s1 = *(u8 **)(arg0 + 0x68);
    u32 st = arg0[0x20];
    s32 a, b, c;
    u8 *v;

    switch (st) {
    case 0:
        arg0[0x20] = 1;
        /* fall through */
    case 1:
        func_L10_0032DE28(*(u8 **)(s1 + 0x10), &a, &b, &c);
        func_L10_0032DDF8(arg0, a, b, c);
        break;
    default:
        return;
    }
    func_L10_004194D0(arg0);
    v = (u8 *)func_L10_0037CCF8(*(u8 **)(s1 + 0x10));
    if (*(s32 *)(v + 0x120) != 0)
        func_L10_00419628(arg0);
}
