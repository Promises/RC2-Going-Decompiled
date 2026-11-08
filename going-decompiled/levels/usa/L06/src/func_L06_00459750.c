/* Level 06 (Notak), USA v2.00: a level-only function built from C (row L06-A,
 * task #2653, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x00459750, in place of its split asm; the row is
 * going-decompiled/levels/usa/L06/config/L06_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern void func_L06_0032E430(void *, void *, void *);
extern float func_L06_0032E3A0(float);
extern float func_L06_0035BA30(float, float);
extern float D_L06_001B234C __attribute__((section(".sdata")));
extern float D_001B2350 __attribute__((section(".sdata")));
extern int func_L06_003A1C98(int, int, char *);
extern void func_L06_003516D0(char *);
extern void func_L06_0032E8F0(void *, void *, void *);
extern double func_001234F0(float);
extern void func_L06_002E3FF0(char *, ...);
extern char D_L06_001B1890[];
extern void func_L06_00367818(char *, void *, int, float, int, void *, float, unsigned int, float, float, int);

/**
 * func_L06_00459750 - emit a burst of falling particles from a moby,
 * then retire the moby.
 * @a0: the emitting moby (position at +0x10, orientation matrix at +0xC0).
 *
 * Emits 13 particles, or 40 when both level floats D_L06_001B234C and
 * D_001B2350 are below 0.9. Each takes a random x in [-8.045, 8.045]
 * (func_L06_0035BA30), y = func_L06_0032E3A0(x * 0.1243008) * 1.745 and a
 * random z in [0, 3.99], a velocity of (x * 0.0041433605, -0.13333334,
 * random(3, 7) / 60), both rotated by the matrix (func_L06_0032E8F0) and the
 * position offset by the moby's (func_L06_0032E430). The six components go,
 * widened to double by the boot's func_001234F0, to func_L06_002E3FF0 with
 * the level string D_L06_001B1890, then func_L06_00367818 spawns the particle
 * and func_L06_003A1C98(0, 0, @a0) runs once per particle. Finally
 * func_L06_003516D0(@a0).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-
 * origin), file src/levels/06_notak/L06_00458138.c, function
 * func_L06_00458138 (v1.01 address), author commit 9b1a6e8d (bigbeno37;
 * `git blame -M -C` gives all 20 definition lines to it, `git log -S`
 * agrees). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED, the body's statements unchanged; each name to
 * the target of the v2.00 disc's own relocation at this place (task #2653
 * relverify by name); edits:
 *   - func_L06_00458138 -> func_L06_00459750 (our split name at the v2.00
 *     place)
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     D_L06_001B234C and D_001B2350 declarations (RULING #8620 ADDRESSING-
 *     MODEL DEVICE, EE arm only: cc1 then prints the one-instruction
 *     `lwc1`, which GNU as expands absolute at -G0 as retail has it; it
 *     moves no data and emits nothing; upstream's `sda` is a marker for its
 *     own patched compiler, which our cc1 ignores)
 *   - D_L06_001B234C and D_001B2350 kept address-named, in upstream's two
 *     spellings (RULING #13291 section 2; L06's split, its
 *     L06_boot_names.txt and the boot asm name neither address. The boot
 *     symbol_addrs.txt names 0x001B234C g_pFmvArenaBase and 0x001B2350
 *     g_fmvThreadId, but those are BOOT-image addresses in level space (>=
 *     0x1A7C80), outside the scope L06 links against, and this body reads
 *     them as level floats)
 *   - D_L06_001B18E0 -> D_L06_001B1890 (our split .lit label at the v2.00
 *     target)
 *   - func_0026FEB8 -> func_L06_002E3FF0, func_00282C48 ->
 *     func_L06_0032E3A0, func_00282CC0 -> func_L06_0032E430, func_00283098
 *     -> func_L06_0032E8F0, func_002A7878 -> func_L06_0035BA30,
 *     func_00350388 -> func_L06_003516D0, func_003A0948 ->
 *     func_L06_003A1C98, func_L00_003263D0 -> func_L06_00367818 (callees
 *     respelled to L06's own copies at their v2.00 places; level-copy
 *     callee names stay func_L06_<addr>); func_001234F0 kept (the boot's
 *     own operand spelling of 0x001234F0)
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped
 */
void func_L06_00459750(char *a0) {
    float pos[4], vel[4];
    int count = 13, i;
    if (D_L06_001B234C < 0.9f && D_001B2350 < 0.9f) count = 40;
    for (i = 0; i < count; i++) {
        pos[0] = func_L06_0035BA30(-8.045f, 8.045f);
        pos[1] = func_L06_0032E3A0(pos[0] * 0.1243008077f) * 1.745f;
        pos[2] = func_L06_0035BA30(0.0f, 3.99f);
        vel[0] = pos[0] * 0.0041433605f;
        vel[1] = -0.13333334f;
        vel[2] = func_L06_0035BA30(3.0f, 7.0f) * 0.0166666675f;
        func_L06_0032E8F0(pos, pos, a0 + 0xC0);
        func_L06_0032E430(pos, pos, a0 + 0x10);
        func_L06_0032E8F0(vel, vel, a0 + 0xC0);
        func_L06_002E3FF0(D_L06_001B1890, func_001234F0(pos[0]), func_001234F0(pos[1]), func_001234F0(pos[2]), func_001234F0(vel[0]), func_001234F0(vel[1]), func_001234F0(vel[2]));
        func_L06_00367818(a0, pos, 1, 0.0f, 1, vel, 3.0f, 0x50808040, 1.0f, 5.5f, 0);
        func_L06_003A1C98(0, 0, a0);
    }
    func_L06_003516D0(a0);
}
