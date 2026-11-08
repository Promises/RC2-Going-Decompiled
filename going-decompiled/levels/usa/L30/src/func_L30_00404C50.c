/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00404C50, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, plain as.
 */

/* L30's .sdata words, at their v2.00 labels. At -G8 cc1 reaches these 4-byte
 * objects $gp-relative by size; upstream's `sda` attribute is a marker for its
 * own patched compiler and our cc1 ignores it (kept verbatim). */
extern int D_L30_001B1484 __attribute__((sda));
extern int D_L30_001B1488 __attribute__((sda));
extern float D_L30_001B148C __attribute__((sda));
extern float D_L30_001B1490 __attribute__((sda));
extern int D_L30_001B144C __attribute__((sda));
extern int D_L30_001B1450 __attribute__((sda));
/* L30's copies of boot helpers and of a shared emitter, at their v2.00
 * places. */
extern int func_L30_003221C8(int, int);
extern float func_L30_003222E8(void);
extern void func_L30_00404DC0(void *, int, float, int, float, float, int, int, void *);

/**
 * func_L30_00404C50 - emit the object's two layered particle bursts.
 * @object: the emitting object (position at +0x10, scale at +0x2C).
 *
 * Two calls of the emitter func_L30_00404DC0 at the object's position: the
 * first with effect D_L30_001B144C, size D_L30_001B148C * scale, a random
 * colour index in [95, 111] (func_L30_003221C8), a random phase
 * (func_L30_003222E8) and parameter D_L30_001B1484; the second with effect
 * D_L30_001B1450, the size further scaled by D_L30_001B1490, an index in
 * [111, 127] and parameter D_L30_001B1488. Both pass 1.0, 2 and the object.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/30_insomniac_museum/L30_0032AAF0-g8.c, function
 * func_L30_00404608 (v1.01 address), author commit 93d2ec41 (bigbeno37;
 * `git blame -M -C` gives all 18 definition lines to it, `git log -S`
 * agrees). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names, each name to the target of the
 * v2.00 disc's own relocation at this place (task #2673 relverify by name):
 *   - func_L30_00404608 -> func_L30_00404C50 (our split name);
 *   - D_L30_001B156C -> D_L30_001B144C, D_L30_001B1570 -> D_L30_001B1450,
 *     D_L30_001B15A4 -> D_L30_001B1484, D_L30_001B15A8 -> D_L30_001B1488,
 *     D_L30_001B15AC -> D_L30_001B148C, D_L30_001B15B0 -> D_L30_001B1490
 *     (our split .sdata labels at the v2.00 targets);
 *   - func_002A7820 -> func_L30_003221C8, func_002A7940 -> func_L30_003222E8,
 *     func_L16_00405818 -> func_L30_00404DC0 (L30's copies at their v2.00
 *     places);
 *   - the file's `#include` lines and other declarations dropped.
 */
void func_L30_00404C50(char *object) {
    int value;
    int value2;
    float scale;
    float scale2;
    float random;
    scale = D_L30_001B148C;
    scale *= *(float *)(object + 0x2C);
    value = func_L30_003221C8(95, 111);
    random = func_L30_003222E8();
    func_L30_00404DC0(object + 0x10, D_L30_001B144C, scale, value, random, 1.0f, D_L30_001B1484, 2, object);
    scale2 = D_L30_001B148C;
    scale2 *= *(float *)(object + 0x2C);
    scale2 *= D_L30_001B1490;
    value2 = func_L30_003221C8(111, 127);
    random = func_L30_003222E8();
    func_L30_00404DC0(object + 0x10, D_L30_001B1450, scale2, value2, random, 1.0f, D_L30_001B1488, 2, object);
}
