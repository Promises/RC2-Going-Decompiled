/* Level 30 (Insomniac Museum), USA v2.00: a level-only function built from C
 * (row L30-A, task #2673, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x0040ECE0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L30/config/L30_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, plain as.
 */

/* L30's copies of boot helpers and of a shared emitter, at their v2.00
 * places. */
extern int func_L30_003221C8(int, int);
extern float func_L30_003222E8(void);
extern void func_L30_0040EE50(void *, float, int, int, int, float, float, int, void *);
/* L30's .sdata words, at their v2.00 labels. At -G8 cc1 reaches these 4-byte
 * objects $gp-relative by size; upstream's `sda` attribute is a marker for its
 * own patched compiler and our cc1 ignores it (kept verbatim). */
extern float D_L30_001B1864 __attribute__((sda));
extern float D_L30_001B1868 __attribute__((sda));
extern int D_L30_001B1824 __attribute__((sda));
extern int D_L30_001B1828 __attribute__((sda));
extern int D_L30_001B185C __attribute__((sda));
extern int D_L30_001B1860 __attribute__((sda));

/**
 * func_L30_0040ECE0 - emit the object's two layered particle bursts (a
 * second emitter's variant of func_L30_00404C50).
 * @a0: the emitting object (position at +0x10, scale at +0x2C).
 *
 * Two calls of the emitter func_L30_0040EE50 at the object's position: size
 * D_L30_001B1864 * scale with effect D_L30_001B1824, a random index in
 * [0x5F, 0x6F] (func_L30_003221C8), parameter D_L30_001B185C and a random
 * phase (func_L30_003222E8); then the size further scaled by D_L30_001B1868,
 * effect D_L30_001B1828, an index in [0x6F, 0x7F] and D_L30_001B1860. Both
 * pass 1.0, 2 and the object.
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/30_insomniac_museum/L30_0040E660-g8.c, function
 * func_L30_0040E660 (v1.01 address), author commit 27b929ce (Lynder063;
 * `git blame -M -C` gives all 4 definition lines to it, `git log -S`
 * agrees). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). VERBATIM but for names, each name to the target of the
 * v2.00 disc's own relocation at this place (task #2673 relverify by name):
 *   - func_L30_0040E660 -> func_L30_0040ECE0 (our split name);
 *   - D_001B193C -> D_L30_001B1824, D_001B1940 -> D_L30_001B1828,
 *     D_001B1974 -> D_L30_001B185C, D_001B1978 -> D_L30_001B1860,
 *     D_001B197C -> D_L30_001B1864, D_001B1980 -> D_L30_001B1868 (our split
 *     .sdata labels at the v2.00 targets);
 *   - func_002A7820 -> func_L30_003221C8, func_002A7940 -> func_L30_003222E8,
 *     func_L16_00414038 -> func_L30_0040EE50 (L30's copies at their v2.00
 *     places).
 */
void func_L30_0040ECE0(char *a0) {
    func_L30_0040EE50(a0 + 0x10, D_L30_001B1864 * *(float *)(a0 + 0x2C), D_L30_001B1824, func_L30_003221C8(0x5F, 0x6F), D_L30_001B185C, func_L30_003222E8(), 1.0f, 2, a0);
    func_L30_0040EE50(a0 + 0x10, D_L30_001B1864 * *(float *)(a0 + 0x2C) * D_L30_001B1868, D_L30_001B1828, func_L30_003221C8(0x6F, 0x7F), D_L30_001B1860, func_L30_003222E8(), 1.0f, 2, a0);
}
