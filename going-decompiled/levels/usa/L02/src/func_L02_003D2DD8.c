/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003D2DD8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm sdk29 (ee-gcc
 * 2.9-ee-991111) at -O2 -G0.
 */

/**
 * func_L02_003D2DD8 - predicate: is the actor in one of its "engaged" states
 * while its state holds a sub-object?
 * @arg0:      the actor; its state block is the pointer at +0x68.
 * @candidate: supplied by callers, not read.
 *
 * Return: 1 when the state's sub-object pointer (+0x2E4) is set and either
 * the actor's state byte +0x20 is 0x13, 4, 0x10, 0x11 or 0x12 or its byte
 * +0x95 is 4; else 0.
 *
 * The function has no relocations: it calls nothing and reads no global.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_002ED7B0.c, function func_L02_003D1880
 * (v1.01 address), author commit 91e04732 (llesieur99), last changed in
 * 863719df (Louis-Philippe Le Sieur). Lombyte mentions in the file: 0.
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: verbatim but for names, at upstream's -G0;
 * respellings:
 *   - name func_L02_003D1880 -> func_L02_003D2DD8 (our split name at the
 *     v2.00 place);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped (nothing
 *     here used them), every declaration of the file dropped (none is used)
 *     and the upstream comment folded into this one.
 */
int func_L02_003D2DD8(char *arg0, void *candidate) {
    int v = 0;
    if (*(int *)(*(char **)(arg0 + 0x68) + 0x2E4) != 0) {
        unsigned char c = *(unsigned char *)(arg0 + 0x20);
        if (c == 0x13 || c == 4 || *(unsigned char *)(arg0 + 0x95) == 4 ||
            c == 0x10 || c == 0x11 || c == 0x12)
            v = 1;
    }
    return v;
}
