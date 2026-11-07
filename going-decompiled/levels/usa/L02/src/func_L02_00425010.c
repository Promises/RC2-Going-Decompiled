/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x00425010, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G0.
 */

typedef unsigned char u8;

extern float func_L02_0031FB00(float, float);
extern void func_L02_002F2D58(void *, void *, void *);
extern int func_L02_002F3F40(float);
extern void func_L02_002F2D20(void *);
extern void func_L02_002F2DE0(void *, void *, float);
extern void func_L02_002F3058(void *, void *, float);
extern int func_L02_002F3F88(int, int, float);
extern void *func_L02_00339300(void *, void *, void *, int, int, int, int, int, int);
/* The level's two colour pairs (split labels in .lit). ADDRESSING-MODEL
 * DEVICE (RULING #8620, EE arm only): `.sdata` on the extern declarations
 * makes cc1 print the one-instruction load, which GNU as expands absolute at
 * -G0, as retail has it. It moves no data and emits nothing. */
extern int D_L02_001B2090[2] __attribute__((section(".sdata")));
extern int D_L02_001B2098[2] __attribute__((section(".sdata")));

/**
 * func_L02_00425010 - throw twenty sparks from the actor.
 * @a0: the actor: a transform at +0x10 and three vectors at +0xC0, +0xD0,
 *      +0xE0.
 * @lo: lowest launch speed.
 * @hi: highest launch speed.
 *
 * The origin is +0x10 + 1.75 * (+0xE0) - 0.5 * (+0xC0). Each spark flies
 * along +0xC0 plus random multiples of +0xD0 (-1..1) and +0xE0 (-0.15..1),
 * scaled by a speed drawn in [@lo, @hi]; a random size r in [0.5, 1.5] gives
 * three integer lifetimes (r * 10, r * 30, r * 7), the fourth float of the
 * velocity is drawn in [0.2, 0.6], and the pull vector's z is -k times the
 * three lifetimes' sum. Its two colours are blended at random between the
 * level's pairs D_L02_001B2090 and D_L02_001B2098 (func_L02_002F3F88), and
 * func_L02_00339300 spawns it with flags 0x1003C.
 *
 * Non-obvious: retail negates k at run time (neg.s), so k stays a variable.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_0040A4A8.c, function func_L02_00423838
 * (v1.01 address), author commit 11c21414 (bigmak94). Lombyte mentions in
 * the file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L02_00423838 -> func_L02_00425010 (our split name at the
 *     v2.00 place);
 *   - D_L02_001B2050 -> D_L02_001B2090 and D_L02_001B2058 ->
 *     D_L02_001B2098 (our split labels), func_00282D30 -> func_L02_002F2DE0,
 *     func_00282CC0 -> func_L02_002F2D58, func_00282F20 ->
 *     func_L02_002F3058, func_002A7878 -> func_L02_0031FB00, func_00283CF0 ->
 *     func_L02_002F3F40, func_00282C88 -> func_L02_002F2D20, func_00283D38 ->
 *     func_L02_002F3F88, func_002BAA28 -> func_L02_00339300: our split names
 *     at the v2.00 addresses the ROM's own relocations imply (relverify,
 *     task #2557);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     two colour-pair declarations (RULING #8620; upstream's `sda` is a
 *     marker for its own patched compiler, which our cc1 ignores);
 *   - `#include "include_asm.h"` dropped (nothing here used it); the file's
 *     own u8 typedef kept, every other declaration dropped, and the upstream
 *     comment folded into this one.
 */
void func_L02_00425010(u8 *a0, float lo, float hi) {
    float v10[4];
    float v20[4];
    float v30[4];
    float v40[4];
    float v50[4];
    float v60[4];
    int i;
    float r;
    int a, b, c;
    int c0, c1;
    float k = 0.0013888889f;

    func_L02_002F2DE0(v10, a0 + 0xE0, 1.75f);
    func_L02_002F2D58(v20, v10, a0 + 0x10);
    func_L02_002F2DE0(v10, a0 + 0xC0, -0.5f);
    func_L02_002F2D58(v20, v10, v20);
    i = 19;
    do {
        func_L02_002F3058(v40, a0 + 0xD0, func_L02_0031FB00(-1.0f, 1.0f));
        func_L02_002F3058(v30, a0 + 0xE0, func_L02_0031FB00(-0.15f, 1.0f));
        func_L02_002F2D58(v50, a0 + 0xC0, v40);
        func_L02_002F2D58(v50, v50, v30);
        func_L02_002F3058(v50, v50, func_L02_0031FB00(lo, hi));
        r = func_L02_0031FB00(0.5f, 1.5f);
        a = func_L02_002F3F40(r * 10.0f);
        b = func_L02_002F3F40(r * 30.0f);
        c = func_L02_002F3F40(r * 7.0f);
        v50[3] = func_L02_0031FB00(0.2f, 0.6f);
        func_L02_002F2D20(v60);
        v60[2] = -k * (float)(a + b + c);
        func_L02_002F2D58(v60, v60, v50);
        v60[3] = 0.01f;
        c0 = func_L02_002F3F88(D_L02_001B2090[0], D_L02_001B2090[1], func_L02_0031FB00(0.0f, 1.0f));
        c1 = func_L02_002F3F88(D_L02_001B2098[0], D_L02_001B2098[1], func_L02_0031FB00(0.0f, 1.0f));
        func_L02_00339300(v20, v50, v60, c0, c1, a, b, c, 0x1003C);
    } while (--i >= 0);
}
