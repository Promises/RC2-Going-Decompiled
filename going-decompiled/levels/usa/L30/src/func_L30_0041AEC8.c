/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x0041AEC8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

/* L30's functions at their v2.00 places. */
extern int func_L30_002F56B0(char *);
extern void func_L30_002EE6A0(void *, char *);
extern void func_L30_00317E40(char *);
extern void func_L30_0041AF30(char *);

/**
 * func_L30_0041AEC8 - run the actor's two countdowns and keep its draw hook
 * queued until the first expires.
 * @a0: the actor; its state block is at +0x68.
 *
 * While func_L30_002F56B0 on the first counter (state +0) returns 0, the
 * second counter (state +4) is stepped the same way and the hook
 * func_L30_0041AF30 is queued for @a0 through func_L30_002EE6A0. Once it
 * returns non-zero the actor is retired (func_L30_00317E40).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/30_insomniac_museum/L30_0040E660.c, function
 * func_L30_0041A848 (v1.01 address), author commit ee3fee45 (llesieur99;
 * `git blame -M -C` gives all 10 definition lines to it, `git log -S`
 * agrees). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names, each name to the target of the
 * v2.00 disc's own relocation at this place (task #2673 relverify by name):
 *   - func_L30_0041A848 -> func_L30_0041AEC8 (our split name);
 *   - func_2F4B08 -> func_L30_002F56B0, func_2EDD78 -> func_L30_002EE6A0,
 *     func_316DB8 -> func_L30_00317E40, func_L00_00415AA0 ->
 *     func_L30_0041AF30 (L30's functions at their v2.00 places);
 *   - NOTE #13281 codes this entry `l` (`long long`->`long`), a FILE-level
 *     attribution: this body and its declarations hold no `long long`, so no
 *     respelling is made;
 *   - `#include "include_asm.h"` and the file's other declarations dropped.
 */
void func_L30_0041AEC8(char *a0) {
    char *st = *(char **)(a0 + 0x68);

    if (func_L30_002F56B0(st) == 0) {
        func_L30_002F56B0(st + 4);
        func_L30_002EE6A0((void *)func_L30_0041AF30, a0);
    } else {
        func_L30_00317E40(a0);
    }
}
