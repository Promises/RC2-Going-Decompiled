/* Level 20 (Yeedil), USA v2.00: a level-only function built from C (task #2565,
 * row L20-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x00436710, in place of its split asm; the row is
 * going-decompiled/levels/usa/L20/config/L20_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef signed short s16;
typedef signed int s32;
typedef float f32;

extern f32 func_L20_003031A0(void *, void *);
extern s32 func_L20_00342EA0(char *, s32);
extern char *D_L20_001FEC00[];

/**
 * func_L20_00436710 - test whether a moby is close enough to its target point.
 *
 * Returns func_L20_00342EA0(moby, target) when that is non-zero. Otherwise,
 * for a target other than -1, returns 1 when the distance from the moby to
 * the target's point (D_L20_001FEC00[target] + 0x10 + 16 * state +0x1CA) is
 * below the state's radius +0x134, else 0.
 *
 * @arg0: the moby. @arg1: target index, or -1.
 * Return: non-zero when reached.
 *
 * Non-obvious: the empty then-branch `if (!(r > d)) {} else` is upstream's
 * spelling; it keeps the unordered compare the ROM has.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/20_yeedil/L20_0039C448.c, function func_L20_00435178 (v1.01
 * address), author commit 7f907424 (ReaperOfVeriod). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2565): func_L20_00435178 -> func_L20_00436710; D_L20_001FEC80 ->
 *     D_L20_001FEC00; func_00282E80 -> func_L20_003031A0; func_L09_00329028 ->
 *     func_L20_00342EA0
 *   - `#include "common.h"` dropped; the common.h typedefs it uses written in
 *     the TU; only the declarations it uses kept
 */
s32 func_L20_00436710(char *arg0, s32 arg1) {
    char *s1 = arg0;
    s32 s0 = arg1;
    char *s3;
    s32 s2;
    f32 f0;

    s3 = *(char **)(s1 + 0x68);
    s2 = func_L20_00342EA0(s1, s0);
    if (s2 == 0) {
        if (s0 != -1) {
            f0 = func_L20_003031A0(s1 + 0x10,
                               D_L20_001FEC00[s0] + ((*(s16 *)(s3 + 0x1CA) << 4) + 0x10));
            if (!(*(f32 *)(s3 + 0x134) > f0)) {
            } else {
                s2 = 1;
            }
        }
    }
    return s2;
}
