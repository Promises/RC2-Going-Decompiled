/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003CC348, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G0.
 */

extern int func_L02_0032FD40(char *, int, float *, char *, short, char *, char *, float, float);
extern int func_L02_003329B8(char *, int, float);

/**
 * func_L02_003CC348 - run an actor's move query against its state's path and
 * pass the resulting value on.
 * @self: the actor; its state block is the pointer at +0x68.
 * @arg1: passed through to both calls.
 *
 * Calls func_L02_0032FD40 on @self + 0x10 with the state's fields at +0x290,
 * +0x3B6 (a short), +0x3B8, +0x3BC, 0.0 and the float at +0x50; it writes one
 * float out. That float is then handed with @self and @arg1 to
 * func_L02_003329B8.
 *
 * Return: func_L02_0032FD40's result.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_002ED7B0.c, function func_L02_003CADF0
 * (v1.01 address), author commit 91e04732 (llesieur99). Lombyte mentions in
 * the file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). BYTE-IDENTICAL: verbatim but for names, at upstream's -G0;
 * respellings:
 *   - name func_L02_003CADF0 -> func_L02_003CC348 (our split name at the
 *     v2.00 place);
 *   - func_L02_0032E998 -> func_L02_0032FD40 and func_002B50E0 ->
 *     func_L02_003329B8: our split names at the v2.00 addresses the ROM's
 *     own relocations imply (relverify, task #2557);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped (nothing
 *     here used them) and every other declaration of the file dropped.
 */
int func_L02_003CC348(char *self, int arg1) {
    float out;
    char *p = *(char **)(self + 0x68);
    int r = func_L02_0032FD40(self + 0x10, arg1, &out, p + 0x290, *(short *)(p + 0x3B6),
                              p + 0x3B8, p + 0x3BC, 0.0f, *(float *)(p + 0x50));
    func_L02_003329B8(self, arg1, out);
    return r;
}
