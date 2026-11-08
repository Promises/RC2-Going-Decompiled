/* Level 07 (Siberius), USA v2.00: a level-only function built from C (row
 * L07-A, task #2656, RULING #13291). Compiled ALONE as its own TU and linked
 * at its one place, 0x004096D8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L07/config/L07_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef signed int s32;
typedef unsigned char u8;
typedef float f32;

extern void func_L07_00311E98(void *a, f32 b, f32 c);

extern f32 func_L07_002E4E88(f32 a);

extern f32 func_L07_00311C48(f32 a, f32 b);

extern void func_L07_0031C0F0(void *a, void *b, f32 c, f32 d, f32 e);

extern void func_L07_00336A18(void *a, void *b, s32 c);

/* The two shared level floats. ADDRESSING-MODEL DEVICE (RULING #8620, EE arm
 * only): `.sdata` on the extern declarations makes cc1 print the
 * one-instruction macros, which GNU as expands absolute at -G0, giving the
 * retail's `lui %hi ; swc1 %lo($at)` form. It moves no data and emits
 * nothing. D_L07_001AA6B8 is L07's own split label; D_L07_001B3118 lies
 * inside the larger split label D_001B2354 (+0x624) and stays address-named
 * (RULING #13291 section 2). */
extern f32 D_L07_001AA6B8 __attribute__((section(".sdata")));

extern f32 D_L07_001B3118 __attribute__((section(".sdata")));

/**
 * func_L07_004096D8 - save a shared level float, pin it to 0.99, and run 200
 * randomised spawn steps around the object.
 *
 * D_L07_001AA6B8 is first saved into D_L07_001B3118, then set to 0.99f. Each
 * of the 200 steps fills a direction with func_L07_00311E98(buf, 1/60,
 * 0.23333335), lifts its third float to func_L07_002E4E88(buf[2]) + 1/12,
 * draws three ranged randoms with func_L07_00311C48 (two in [-5, 5], one in
 * [10, 25]), turns them into a position next to @a0 with func_L07_0031C0F0,
 * and hands position and direction to func_L07_00336A18 with 0x1E0.
 *
 * @a0: the object the steps are placed around.
 * Return: none.
 *
 * Non-obvious: the loop counts down from 0xC7 to 0, so it runs 200 times;
 * the three random draws are evaluated in source order (f21, f20, f14).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/07_siberius/L07_003F4578.c, function func_L07_00408338
 * (v1.01 address), author commit 913bd1bc (Gianni), by blame -M -C of the
 * definition (all 21 lines; no rename-copy found).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED, the body's statements unchanged; edits, each name to the
 * target of the v2.00 disc's own relocation at this place (task #2656
 * relverify by name):
 *   - func_L07_00408338 -> func_L07_004096D8 (our split name);
 *   - func_310D80 -> func_L07_00311E98, func_2E4280 -> func_L07_002E4E88,
 *     func_310B30 -> func_L07_00311C48, func_31AFD8 -> func_L07_0031C0F0,
 *     func_335848 -> func_L07_00336A18 (L07's own copies at the v2.00
 *     places);
 *   - D_L07_001AA698 -> D_L07_001AA6B8 (our committed split label at the
 *     v2.00 target);
 *   - D_L07_001B3198 -> D_L07_001B3118 (address-named, RULING #13291
 *     section 2: no boot or level name at that address, task #2656 grep);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on both
 *     float declarations (RULING #8620; upstream's `sda` is a marker for its
 *     own patched compiler, which our cc1 ignores), and upstream's comment
 *     on them folded into the device comment;
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped, common.h's three typedefs this body uses written
 *     out in the TU instead.
 */
void func_L07_004096D8(u8 *a0)
{
    f32 v[4];
    f32 buf[4];
    s32 i;
    f32 f14;
    f32 f20;
    f32 f21;

    D_L07_001B3118 = D_L07_001AA6B8;
    D_L07_001AA6B8 = 0.99f;
    for (i = 0xC7; i >= 0; i--) {
        func_L07_00311E98(buf, 1.0f / 60.0f, 0.23333335f);
        buf[2] = func_L07_002E4E88(buf[2]) + 1.0f / 12.0f;
        f21 = func_L07_00311C48(-5.0f, 5.0f);
        f20 = func_L07_00311C48(-5.0f, 5.0f);
        f14 = func_L07_00311C48(10.0f, 25.0f);
        func_L07_0031C0F0(a0, v, f21, f20, f14);
        func_L07_00336A18(v, buf, 0x1E0);
    }
}
