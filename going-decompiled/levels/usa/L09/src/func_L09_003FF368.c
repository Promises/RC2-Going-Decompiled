/* Level 09 (Dobbo), USA v2.00: a level-only function built from C (task #2561,
 * RULING #13291 row L09-A). Compiled ALONE as its own TU and linked at its one
 * place, 0x003FF368, in place of its split asm; the row is
 * going-decompiled/levels/usa/L09/config/L09_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

/* The level's copies of boot helpers, named by their v2.00 places in L09. */
extern int func_L09_002EAD98(unsigned short *);
extern float func_L09_002EC100(float, float);
extern int func_L09_00317BC8(int);
extern float func_L09_0031B048(unsigned char *, unsigned char *, float, float, float, float);
extern void func_L09_003208C8(unsigned char *, unsigned char *, unsigned char *, float, float, float);
extern void func_L09_00320980(unsigned char *, unsigned char *, unsigned char *, float, float, float);

/**
 * func_L09_003FF368 - per-frame wobble of an object: two channel ramps, then
 * in state 1 an occasional random 120-degree turn.
 *
 * Feeds the state's channel pairs +0x3C0/+0x3C4 and +0x3C8/+0x3CC through
 * func_L09_003208C8 and func_L09_00320980. Only in state 1 (+0x20): when the
 * state's countdown halfword +0x3D4 fires (func_L09_002EAD98 != 0) it is
 * re-armed to 0x104 and a 2.0943954 rad turn, its sign drawn from
 * func_L09_00317BC8(2), is folded into the object's +0xF8 angle as the target
 * +0x3D0; then the angle is ramped toward that target (velocity +0x3D8) by
 * func_L09_0031B048.
 *
 * @a0: the object; its state is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/09_dobbo/L09_002F8BA0.c, function func_L09_003FDED8 (v1.01
 * address), author commit ee3fee45 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2561 relverify):
 *   - func_L09_003FDED8 -> func_L09_003FF368 (our split name at the v2.00 place);
 *   - func_00282978 -> func_L09_002EAD98, func_00283B98 -> func_L09_002EC100,
 *     func_002A77E0 -> func_L09_00317BC8, func_002AA3A8 -> func_L09_0031B048,
 *     func_002AEE70 -> func_L09_003208C8, func_002AEF28 -> func_L09_00320980;
 *   - `#include "common.h"` dropped: only the declarations this function uses
 *     are kept, and upstream's function comment is folded into this one.
 * The audit (NOTE #13281) closed this member through its `long long`->`long`
 * TU variant only because the whole upstream file fails on this arm; this
 * function's own text has no `long long`, so alone in its TU it needs no
 * respelling.
 */
void func_L09_003FF368(unsigned char *a0) {
    unsigned char *s1 = a0;
    unsigned char *s0 = *(unsigned char **)(s1 + 0x68);
    func_L09_003208C8(a0, s0 + 0x3C0, s0 + 0x3C4, 0.25f, 0.06f, -1.0f);
    func_L09_00320980(s1, s0 + 0x3C8, s0 + 0x3CC, 0.15f, 0.05f, 0.025f);
    if (s1[0x20] != 1)
        return;
    if (func_L09_002EAD98((unsigned short *)(s0 + 0x3D4)) != 0) {
        float t = 2.0943954f;
        *(short *)(s0 + 0x3D4) = 0x104;
        if (func_L09_00317BC8(2) != 0)
            t = -t;
        *(float *)(s0 + 0x3D0) = func_L09_002EC100(*(float *)(s1 + 0xF8), t);
    }
    func_L09_0031B048(s1 + 0xF8, s0 + 0x3D8, *(float *)(s0 + 0x3D0), 0.003f, 0.2f,
                  0.02f);
}
