/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (task #2561,
 * RULING #13291 row L09-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x003DEF10, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* The level's copies of boot helpers, named by their v2.00 places in L09. */
extern void func_L09_00317818(unsigned char *, int, int, int);
extern void func_L09_00317AF8(unsigned char *, int, int);

/**
 * func_L09_003DEF10 - first step of a scripted record: start its track at a
 * phase drawn from the record's own address.
 *
 * Only while the record's state byte (+0x20) is clear: reads the class
 * descriptor's byte +0x10 (through the record's +0x24 class and its +0x48
 * block), scales it by ((int)a0 % 3) * 0.333, and starts track 0 at that
 * frame through func_L09_00317818(a0, 0, frame, 1); then func_L09_00317AF8
 * (a0, 1, -1) marks the record active.
 *
 * @a0: the record.
 *
 * Non-obvious: the record's ADDRESS modulo 3 is the phase source, so records
 * of one class start out of step with each other. func_L09_003DEFB0 is the
 * same body at its own place.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/09_dobbo/L09_002F8BA0.c, function func_L09_003DDAA0 (v1.01
 * address), author commit 264cb2be (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2561 relverify):
 *   - func_L09_003DDAA0 -> func_L09_003DEF10 (our split name at the v2.00 place);
 *   - func_002A7470 -> func_L09_00317818, func_002A7750_3 -> func_L09_00317AF8;
 *   - `#include "common.h"` dropped: only the declarations this function uses
 *     are kept, and upstream's function comment is folded into this one.
 * The audit (NOTE #13281) closed this member through its `long long`->`long`
 * TU variant only because the whole upstream file fails on this arm; this
 * function's own text has no `long long`, so alone in its TU it needs no
 * respelling.
 */
void func_L09_003DEF10(unsigned char *a0) {
    int v1;
    unsigned char *a2;
    if (a0[0x20] != 0)
        return;
    v1 = *(int *)(a0 + 0x24);
    a2 = *(unsigned char **)(v1 + 0x48);
    func_L09_00317818(a0, 0,
                  (int)((float)a2[0x10] * ((float)((int)a0 % 3) * 0.333f)), 1);
    func_L09_00317AF8(a0, 1, -1);
}
