/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (task #2561,
 * RULING #13291 row L09-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x003E4630, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* The level's copies of boot helpers, named by their v2.00 places in L09. */
extern float func_L09_002EC100(float, float);
extern int func_L09_00317C08(int, int);
extern int func_L09_00317BC8(int);

/**
 * func_L09_003E4630 - two-state turn of a scripted record.
 *
 * State 0 (+0x20): draws a random duration in 150..200 into byte +0xBC
 * (func_L09_00317C08), picks a turn direction from a two-way draw
 * (func_L09_00317BC8(2) != 0 sets byte +0x95) and moves to state 1.
 * State 1: the stored duration times 2*pi/21600 (negated when +0x95 is set)
 * is folded into the record's angle (+0xF4) through func_L09_002EC100.
 * Any other state does nothing.
 *
 * @a0: the record.
 *
 * Non-obvious: the state byte is a switch, not an if/else chain: the ROM
 * compares it against 0 and 1 in turn and falls straight out otherwise.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/09_dobbo/L09_002F8BA0.c, function func_L09_003E31C0 (v1.01
 * address), author commit 264cb2be (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2561 relverify):
 *   - func_L09_003E31C0 -> func_L09_003E4630 (our split name at the v2.00 place);
 *   - func_00283B98 -> func_L09_002EC100, func_002A77E0 -> func_L09_00317BC8,
 *     func_002A7820 -> func_L09_00317C08;
 *   - `#include "common.h"` dropped: only the declarations this function uses
 *     are kept, and upstream's function comment is folded into this one.
 * The audit (NOTE #13281) closed this member through its `long long`->`long`
 * TU variant only because the whole upstream file fails on this arm; this
 * function's own text has no `long long`, so alone in its TU it needs no
 * respelling.
 */
void func_L09_003E4630(unsigned char *a0) {
    switch (a0[0x20]) {
    case 0:
        a0[0xBC] = func_L09_00317C08(0x96, 0xC8);
        if (func_L09_00317BC8(2) != 0) {
            a0[0x95] = 1;
        }
        a0[0x20] = 1;
        break;
    case 1:
        if (a0[0x95] != 0) {
            *(float *)(a0 + 0xF4) = func_L09_002EC100(
                *(float *)(a0 + 0xF4), -((float)a0[0xBC] * 0.000290888245f));
        } else {
            *(float *)(a0 + 0xF4) = func_L09_002EC100(
                *(float *)(a0 + 0xF4), (float)a0[0xBC] * 0.000290888245f);
        }
        break;
    }
}
