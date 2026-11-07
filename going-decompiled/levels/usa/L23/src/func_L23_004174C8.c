/* Level 23 (Damosel Orbit), USA v2.00: a level-only function built from C (task
 * #2567, RULING #13291 row L23-A, as-is route). Compiled ALONE as its own TU
 * and linked at its one place, 0x004174C8, in place of its split asm; the row
 * is going-decompiled/levels/usa/L23/config/L23_c.tsv: the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

extern float D_L23_001B1378;
extern void func_L23_00325548(int, int, int);
extern void func_L23_004175B0(char *);

/**
 * func_L23_004174C8 - sets the object up by its byte +0x20. Kind 0: flags 0x100
 * and 0x4000 at +0x34, 0xFF at +0x32 and +0x30, 2000 as a float at state +0x20
 * and as a short at state +0x24, 0x3FF at +0x7C, func_L23_00325548(object, 1,
 * 0). Kind 1: when the level float D_L23_001B1378 is nonzero, it replaces the
 * 2000 in both fields; then func_L23_00325548(object, 2, 0). Always ends in
 * func_L23_004175B0(object).
 *
 * @arg0: the object (moby); +0x68 points at its state block.
 * Return: nothing.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on the file is 0, so not
 * Lombyte-origin), file src/levels/23_damosel_orbit/L23_0035CE90-g8.c,
 * function func_L23_00415FC8 (v1.01 address), author commit 91e04732
 * (llesieur99). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED (the audit's verdict was BYTE-IDENTICAL): names
 * respelled and upstream's inert `sda` attribute dropped, as listed below;
 * the body's statements unchanged (RULING #13291 section 7). Edits:
 *   - func_L23_00415FC8 -> func_L23_004174C8 (our split name at the v2.00
 *     place)
 *   - D_L23_001B13C8 -> D_L23_001B1378 (the target of the v2.00 disc's gp-
 *     relative load)
 *   - func_3242D0 -> func_L23_00325548, func_L23_004160B0 -> func_L23_004175B0
 *   - upstream's `__attribute__((sda))` on D_L23_001B13C8 dropped: inert at
 *     -G8 for a 4-byte object, which is gp-relative by its size alone
 *   - `#include "include_asm.h"` dropped: the TU declares what it uses, and
 *     upstream's comments are folded into this one
 */
void func_L23_004174C8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    unsigned char kind = *(unsigned char *)(arg0 + 0x20);

    if (kind == 0) {
        *(unsigned short *)(arg0 + 0x34) |= 0x100;
        *(unsigned short *)(arg0 + 0x32) = 0xFF;
        *(unsigned char *)(arg0 + 0x30) = 0xFF;
        *(float *)(state + 0x20) = 2000.0f;
        *(short *)(state + 0x24) = 2000;
        *(short *)(arg0 + 0x7C) = 0x3FF;
        func_L23_00325548(arg0, 1, 0);
        *(unsigned short *)(arg0 + 0x34) |= 0x4000;
    } else if (kind == 1) {
        float f = D_L23_001B1378;

        if (f != 0.0f) {
            *(float *)(state + 0x20) = f;
            *(short *)(state + 0x24) = f;
        }
        func_L23_00325548(arg0, 2, 0);
    }
    func_L23_004175B0(arg0);
}
