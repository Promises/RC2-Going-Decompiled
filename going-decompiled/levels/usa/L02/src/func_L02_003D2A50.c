/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003D2A50, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G0.
 */

extern void func_L02_0031F998(void *, int, int);
extern void func_L02_0031F6B8(void *, int, int, int);

/**
 * func_L02_003D2A50 - pick the actor's mode by whether its state holds a
 * sub-object.
 * @self: the actor; its state block is the pointer at +0x68.
 *
 * With a sub-object at state +0x2E4: func_L02_0031F998(@self, 4, 2), then,
 * unless the byte at +0x43 is already 0, func_L02_0031F6B8(@self, 0, 0, 10).
 * Without one: mode 5 and target 1 the same way.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_002ED7B0.c, function func_L02_003D14F8
 * (v1.01 address), author commit 91e04732 (llesieur99). Lombyte mentions in
 * the file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). BYTE-IDENTICAL: verbatim but for names, at upstream's -G0;
 * respellings:
 *   - name func_L02_003D14F8 -> func_L02_003D2A50 (our split name at the
 *     v2.00 place);
 *   - func_002A7750 -> func_L02_0031F998 and func_002A7470 ->
 *     func_L02_0031F6B8: our split names at the v2.00 addresses the ROM's
 *     own relocations imply (relverify, task #2557);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped (nothing
 *     here used them) and every other declaration of the file dropped.
 */
void func_L02_003D2A50(char *self) {
    char *p = *(char **)(self + 0x68);
    if (*(int *)(p + 0x2E4) != 0) {
        func_L02_0031F998(self, 4, 2);
        if (*(unsigned char *)(self + 0x43) != 0)
            func_L02_0031F6B8(self, 0, 0, 0xA);
    } else {
        func_L02_0031F998(self, 5, 2);
        if (*(unsigned char *)(self + 0x43) != 1)
            func_L02_0031F6B8(self, 1, 0, 0xA);
    }
}
