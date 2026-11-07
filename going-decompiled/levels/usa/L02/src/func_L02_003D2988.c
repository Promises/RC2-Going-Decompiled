/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003D2988, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G0.
 */

extern int func_L02_002F29C8(short *);
extern void func_L02_003297F8(char *, char *, int, int);
extern void func_L02_0032C1D0(char *, char *, float, int, int);
extern int func_L02_003214D8(char *, void *, int, float, float, float, int, int, int);

/**
 * func_L02_003D2988 - update an actor and the sub-object its state holds.
 * @self: the actor; its state block is the pointer at +0x68.
 *
 * Passes the state's half-word at +0x2F4 to func_L02_002F29C8, counts the
 * actor's half-word at +0x96 up, saturating at 0xFFFF, and, when the state
 * holds a sub-object at +0x2E4, calls func_L02_003297F8(@self, sub, 2, 0)
 * and clears bit 1 of the sub-object's half-word at +0x34. Then runs
 * func_L02_0032C1D0 on the state's block at +0x210 (3.5, 4, 1) and
 * func_L02_003214D8 on @self + 0x10 with mask 0x4000000 and (1.5, 1, 1).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_002ED7B0.c, function func_L02_003D1430
 * (v1.01 address), author commit 91e04732 (llesieur99). Lombyte mentions in
 * the file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). BYTE-IDENTICAL: verbatim but for names, at upstream's -G0;
 * respellings:
 *   - name func_L02_003D1430 -> func_L02_003D2988 (our split name at the
 *     v2.00 place);
 *   - func_00282978 -> func_L02_002F29C8, func_002AF1D0 -> func_L02_003297F8,
 *     func_002B0990 -> func_L02_0032C1D0, func_002A8C28 -> func_L02_003214D8:
 *     our split names at the v2.00 addresses the ROM's own relocations imply
 *     (relverify, task #2557);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped (nothing
 *     here used them) and every other declaration of the file dropped.
 */
void func_L02_003D2988(char *self) {
    char *p = *(char **)(self + 0x68);
    func_L02_002F29C8((short *)(p + 0x2F4));
    if (*(unsigned short *)(self + 0x96) < 0xFFFF)
        (*(unsigned short *)(self + 0x96))++;
    if (*(int *)(p + 0x2E4) != 0) {
        func_L02_003297F8(self, *(char **)(p + 0x2E4), 2, 0);
        *(unsigned short *)(*(char **)(p + 0x2E4) + 0x34) &= 0xFFFD;
    }
    func_L02_0032C1D0(self, p + 0x210, 3.5f, 4, 1);
    func_L02_003214D8(self, self + 0x10, 0x4000000, 1.5f, 1.0f, 1.0f, 0, 1, 0);
}
