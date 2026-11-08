/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00418CF0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/* L30's functions at their v2.00 places. */
extern void func_L30_0042C6B8(int);
extern void func_L30_00317E40(char *);
/* The boot's active-gadget item (core, symbol_addrs). */
extern int g_activeGadgetItem;

/**
 * func_L30_00418CF0 - tear down the actor: restore the saved gadget and
 * retire its companion moby.
 * @a0: the actor; its state block is at +0x68.
 *
 * Calls func_L30_0042C6B8(3), writes the item saved at state +0x5C back to
 * g_activeGadgetItem and retires the companion moby at state +0x50 through
 * func_L30_00317E40.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/30_insomniac_museum/L30_0040E660.c, function
 * func_L30_00418670 (v1.01 address), author commit ee3fee45 (llesieur99;
 * `git blame -M -C` gives all 7 definition lines to it, `git log -S`
 * agrees). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names, each name to the target of the
 * v2.00 disc's own relocation at this place (task #2673 relverify by name):
 *   - func_L30_00418670 -> func_L30_00418CF0 (our split name);
 *   - D_0018C0E8 -> g_activeGadgetItem (the boot's symbol_addrs name at
 *     exactly 0x0018C0E8);
 *   - func_42C028 -> func_L30_0042C6B8, func_316DB8 -> func_L30_00317E40
 *     (L30's copies at their v2.00 places);
 *   - NOTE #13281 codes this entry `l` (`long long`->`long`), a FILE-level
 *     attribution: this body and its declarations hold no `long long`, so no
 *     respelling is made;
 *   - `#include "include_asm.h"` and the file's other declarations dropped.
 */
void func_L30_00418CF0(char *a0) {
    char *st = *(char **)(a0 + 0x68);

    func_L30_0042C6B8(3);
    g_activeGadgetItem = *(int *)(st + 0x5C);
    func_L30_00317E40(*(char **)(st + 0x50));
}
