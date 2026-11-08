/* Level 06 (Notak), USA v2.00: a level-only function built from C (row L06-A,
 * task #2653, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x00421080, in place of its split asm; the row is
 * going-decompiled/levels/usa/L06/config/L06_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern void func_L06_0035B8C8(char *, int, int);
extern int func_L06_0032F590(float);
extern int func_L06_0032E0A0(char *);
extern int func_L06_0035B998(int);
extern void func_L06_0035B5E8(char *, int, int, int);

/**
 * func_L06_00421080 - choose a moby's next reaction once its timer at
 * pvar +0x25C expires, and restart the animation that goes with it.
 * @a0: the moby; its +0x68 points at its pvar block o, its +0x43 holds the
 *      current animation.
 *
 * In mode o+0x5A0 == 1: with no target (o+0x23C == -1) it waits for the timer
 * (func_L06_0032E0A0) and then plays reaction 7 / animation 0x13; with a
 * target it plays reaction 8 / animation 0x12 at once. Otherwise, when the
 * halfword o+0x492 is set and the timer has expired, it clears o+0x492, plays
 * reaction 7 and animation 0x11, or 0x13 when func_L06_0035B998(2) is
 * nonzero; when o+0x492 is clear and the timer has expired, reaction 9 and
 * animation 0x12. A reaction is func_L06_0035B8C8(@a0, n, 2); an animation is
 * started by func_L06_0035B5E8(@a0, anim, 0, 20) only if not already playing.
 * Each path then stores func_L06_0032F590(20.0f) and func_L06_0032F590(24.0f)
 * in the bytes o+0x48 and o+0x4A.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-
 * origin), file src/levels/06_notak/L06_0041DED8.c, function
 * func_L06_0041FAF8 (v1.01 address), author commit d5ffe9fb (bigbeno37;
 * `git blame -M -C` gives it 44 of the 47 definition lines and `git log -S`
 * agrees), which replaced Gianni's nonmatching draft
 * nonmatching/levels/func_L06_0041FAF8.c (2235d994, 2026-10-21) and kept
 * its lines: the signature, `*(short *)(o + 0x492) = 0;` and
 * `func_35A550(a0, 7, 2);` survive (`blame -C -C -C10`; RULING #13427 rev
 * 2). Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (BYTE-IDENTICAL), each name to the target
 * of the v2.00 disc's own relocation at this place (task #2653 relverify by
 * name); edits:
 *   - func_L06_0041FAF8 -> func_L06_00421080 (our split name at the v2.00
 *     place)
 *   - func_32D238 -> func_L06_0032E0A0, func_32E728 -> func_L06_0032F590,
 *     func_35A270 -> func_L06_0035B5E8, func_35A550 -> func_L06_0035B8C8,
 *     func_35A620 -> func_L06_0035B998 (callees respelled to L06's own
 *     copies at their v2.00 places; level-copy callee names stay
 *     func_L06_<addr>)
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped
 */
void func_L06_00421080(char *a0)
{
    char *o = *(char **)(a0 + 0x68);

    if (*(int *)(o + 0x5A0) == 1) {
        if (*(int *)(o + 0x23C) == -1) {
            if (func_L06_0032E0A0(o + 0x25C)) {
                func_L06_0035B8C8(a0, 7, 2);
                if (*(unsigned char *)(a0 + 0x43) != 0x13) {
                    func_L06_0035B5E8(a0, 0x13, 0, 20);
                }
                o[0x48] = func_L06_0032F590(20.0f);
                o[0x4A] = func_L06_0032F590(24.0f);
            }
        } else {
            func_L06_0035B8C8(a0, 8, 2);
            if (*(unsigned char *)(a0 + 0x43) != 0x12) {
                func_L06_0035B5E8(a0, 0x12, 0, 20);
            }
            o[0x48] = func_L06_0032F590(20.0f);
            o[0x4A] = func_L06_0032F590(24.0f);
        }
    } else if (*(short *)(o + 0x492) != 0 && func_L06_0032E0A0(o + 0x25C)) {
        *(short *)(o + 0x492) = 0;
        func_L06_0035B8C8(a0, 7, 2);
        if (!func_L06_0035B998(2)) {
            if (*(unsigned char *)(a0 + 0x43) != 0x11) {
                func_L06_0035B5E8(a0, 0x11, 0, 20);
            }
            o[0x48] = func_L06_0032F590(20.0f);
            o[0x4A] = func_L06_0032F590(24.0f);
        } else {
            if (*(unsigned char *)(a0 + 0x43) != 0x13) {
                func_L06_0035B5E8(a0, 0x13, 0, 20);
            }
            o[0x48] = func_L06_0032F590(20.0f);
            o[0x4A] = func_L06_0032F590(24.0f);
        }
    } else if (*(short *)(o + 0x492) == 0 && func_L06_0032E0A0(o + 0x25C)) {
        func_L06_0035B8C8(a0, 9, 2);
        if (*(unsigned char *)(a0 + 0x43) != 0x12) {
            func_L06_0035B5E8(a0, 0x12, 0, 20);
        }
        o[0x48] = func_L06_0032F590(20.0f);
        o[0x4A] = func_L06_0032F590(24.0f);
    }
}
