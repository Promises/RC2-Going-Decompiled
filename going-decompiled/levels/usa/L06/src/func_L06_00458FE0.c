/* Level 06 (Notak), USA v2.00: a level-only function built from C (row L06-A,
 * task #2653, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x00458FE0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L06/config/L06_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern void func_L06_0032E430(char *, char *, char *);
extern int func_L06_0032F590(float);
extern void func_L06_00353348(char *, int, int, int);
extern void func_L06_0032E710(char *, char *, float);
extern int D_L06_001B375C __attribute__((section(".sdata")));
extern int D_L06_001B3758 __attribute__((section(".sdata")));
extern void func_L06_003DF1E0();

/**
 * func_L06_00458FE0 - per-frame step of a moving moby: advance it by its
 * velocity and refresh its three byte parameters from the level integer
 * D_L06_001B375C.
 * @a0: the moby; its +0x68 points at its pvar block, whose first vec4 is the
 *      velocity.
 *
 * Adds the velocity to the position at +0x10 (func_L06_0032E430), then calls
 * func_L06_00353348(@a0, f(1.5 * n), f(1.25 * n), n) with n = D_L06_001B375C
 * and f = func_L06_0032F590. When the level flag D_L06_001B3758 is set it also
 * scales the velocity by 0.11666667 (func_L06_0032E710), adds 0.18333334 to its
 * z, installs func_L06_003DF1E0 as the moby's +0x64 update hook, scales the
 * moby scale at +0x2C by 0.75 and calls func_L06_00353348(@a0, 16, 16, 16).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-
 * origin), file src/levels/06_notak/L06_0041DED8.c, function
 * func_L06_004579C8 (v1.01 address), author commit ee3fee45 (llesieur99;
 * `git blame -M -C` gives all 16 definition lines to it; `git log -S` first
 * hits 431ac503, which only added the INCLUDE_ASM stub). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED,
 * the body's statements unchanged; each name to the target of the v2.00
 * disc's own relocation at this place (task #2653 relverify by name);
 * edits:
 *   - func_L06_004579C8 -> func_L06_00458FE0 (our split name at the v2.00
 *     place)
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     D_L06_001B375C and D_L06_001B3758 declarations (RULING #8620
 *     ADDRESSING-MODEL DEVICE, EE arm only: cc1 then prints the one-
 *     instruction `lw`, which GNU as expands absolute at -G0 as retail has
 *     it; it moves no data and emits nothing; upstream's `sda` is a marker
 *     for its own patched compiler, which our cc1 ignores)
 *   - D_L06_001B375C and D_L06_001B3758 kept address-named (RULING #13291
 *     section 2: both lie inside L06's split label D_L06_001B2BB0, +0xBAC
 *     and +0xBA8; no boot dlabel, boot operand, symbol_addrs entry or
 *     L06_boot_names.txt entry names either address)
 *   - func_32D5C8 -> func_L06_0032E430, func_32D8A8 -> func_L06_0032E710,
 *     func_32E728 -> func_L06_0032F590, func_352000 -> func_L06_00353348,
 *     func_L06_003DDCA8 -> func_L06_003DF1E0 (callees respelled to L06's
 *     own copies at their v2.00 places; level-copy callee names stay
 *     func_L06_<addr>)
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped
 */
void func_L06_00458FE0(char *a0)
{
    char *s2 = *(char **)(a0 + 0x68);
    int v;

    func_L06_0032E430(a0 + 0x10, a0 + 0x10, s2);
    v = func_L06_0032F590((float)D_L06_001B375C * 1.5f);
    func_L06_00353348(a0, v, func_L06_0032F590((float)D_L06_001B375C * 1.25f), D_L06_001B375C);
    if (D_L06_001B3758 != 0) {
        func_L06_0032E710(s2, s2, 0.11666667461395264f);
        *(float *)(s2 + 0x8) = *(float *)(s2 + 0x8) + 0.18333333730697632f;
        *(unsigned int *)(a0 + 0x64) = (unsigned int)func_L06_003DF1E0;
        *(float *)(a0 + 0x2C) = *(float *)(a0 + 0x2C) * 0.75f;
        func_L06_00353348(a0, 0x10, 0x10, 0x10);
    }
}
