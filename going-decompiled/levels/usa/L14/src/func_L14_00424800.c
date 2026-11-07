/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2581, RULING #13291 row L14-ASM). Compiled ALONE as its own TU and
 * linked at its one place, 0x00424800, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled by
 * tools/ee/asm_unit.sh (the row's `asm_unit` column). Measured with plain GNU
 * as instead (task #2581, same TU): .text is 0x114 bytes, not the place's
 * 0x118; level_elf.py cobj refuses it.
 */

extern void *func_L14_00325680(void *a0, int a1, int a2);
extern void func_L14_00319058(void *a0);
extern float D_L14_001B1650 __attribute__((sda));
extern void func_L14_0032FCF0(char *a0, char *a1, int a2);
extern void func_L14_00328AE0(char *a0, char *a1, float f12, float f13, int a2);
extern void func_L14_00322740(char *a0, int a1, int a2);

/**
 * func_L14_00424800 - reset the scale of the moby this object carries when its
 * model says so, then let the object fall or die.
 *
 * @a0: the object (moby); +0x68 its state block (+0x20 its health), +0x18 its
 *      height, +0x31 its visibility.
 * Return: nothing.
 *
 * Non-obvious: Upstream's comment: "Resets the moby's scale when its model says
 * so, then lets it fall or die." func_L14_00325680(a0, 0x10000, 1) finds the
 * linked moby; when its model (+0x20) has class 0xB69 (+0xAA), its +0x2C
 * becomes 1.0 if this object is visible, else 0.0. func_L14_0032FCF0 then runs
 * on state +0x60. With health <= 0 it spawns an explosion (func_L14_00328AE0,
 * 1.0, 13.0), calls func_L14_00322740(a0, 0, 3) and deletes the object
 * (func_L14_00319058); below the height D_L14_001B1650 it does the same without
 * the explosion.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own, `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_003F4F98-g8.c (blob
 * b0bd52b1), function func_L14_00423240 (v1.01 address), author commit daf0e4a1
 * (Flavius). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). BYTE-IDENTICAL: VERBATIM but for names (no device). Edits,
 * each name to the target of the v2.00 disc's own relocation at this place
 * (relverify by name on this object, task #2581):
 *   - func_L14_00423240 -> func_L14_00424800 (our split name at the v2.00
 *     place);
 *   - D_L14_001B16B0 -> D_L14_001B1650 (our label at that address);
 *   - func_0029F338 -> func_L14_00319058 (the level's own copy at its L14
 *     place);
 *   - func_002A6C58 -> func_L14_00322740 (the level's own copy at its L14
 *     place);
 *   - func_002A9640 -> func_L14_00325680 (the level's own copy at its L14
 *     place);
 *   - func_002AC728 -> func_L14_00328AE0 (the level's own copy at its L14
 *     place);
 *   - func_002B0ED8 -> func_L14_0032FCF0 (the level's own copy at its L14
 *     place);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning) and the member closes with it as written;
 *   - `#include "common.h"` dropped (the body uses none of its typedefs);
 *     upstream file-scope declarations the body does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L14_00424800(char *a0)
{
    char *s0 = a0;
    char *s1;
    char *m;
    char *v;

    s1 = *(char **)(s0 + 0x68);
    v = (char *)func_L14_00325680(a0, 0x10000, 1);
    if (v != 0) {
        m = *(char **)(v + 0x20);
        if (m != 0 && *(short *)(m + 0xAA) == 0xB69) {
            if (*(unsigned char *)(s0 + 0x31) == 0)
                *(float *)(v + 0x2C) = 0.0f;
            else
                *(float *)(v + 0x2C) = 1.0f;
        }
    }
    func_L14_0032FCF0(s0, s1 + 0x60, 0);
    if (*(float *)(s1 + 0x20) <= 0.0f) {
        func_L14_00328AE0(s0, s0 + 0x10, 1.0f, 13.0f, -1);
        func_L14_00322740(s0, 0, 3);
        func_L14_00319058(s0);
    } else if (*(float *)(s0 + 0x18) < D_L14_001B1650) {
        func_L14_00322740(s0, 0, 3);
        func_L14_00319058(s0);
    }
}
