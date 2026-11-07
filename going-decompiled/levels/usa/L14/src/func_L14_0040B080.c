/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2581, RULING #13291 row L14-ASM). Compiled ALONE as its own TU and
 * linked at its one place, 0x0040B080, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled by
 * tools/ee/asm_unit.sh (the row's `asm_unit` column). Measured with plain GNU
 * as instead (task #2581, same TU): .text is 0x120 bytes, not the place's
 * 0x130, and it emits a .lit4 pool of 8 bytes; level_elf.py cobj refuses it.
 */

extern void func_L14_00323250(void *self, int state, int mode);
extern int func_L14_0036ADA0(int a, int b, void *self);
extern float func_L14_002F7028(float);
extern int g_mobyTableBase __attribute__((sda));
extern int func_L14_0032CEA8(char *self);
extern void func_L14_00333698(int table, int mode);
extern int func_L14_00333648(char *self);
extern float func_L14_002F7A68(float a, float b);
extern float func_L14_002F7028(float a);
extern int func_L14_002F7C08(int a, int b, float c);

/**
 * func_L14_0040B080 - step a light object's two-stage state and blend its
 * colour between two pulsing colours.
 *
 * @self: the object (moby); its stage byte is +0x20, its state block +0x68
 *        (+0x00 a moby index or -1, +0x04 a phase), its colour +0x78.
 * Return: nothing.
 *
 * Non-obvious: Stage 0 waits for func_L14_0032CEA8; then, when the state's
 * index is not -1, it calls func_L14_00333698 on that 0x100-byte moby record of
 * the table g_mobyTableBase, and sets stage 1. Stage 1 plays a sound when
 * func_L14_00333648 says so and uses the pair 0x80468CDC / 0x801E78AA instead
 * of the pair 0x808C8C8C / 0x80282828. The phase advances by 0.10762865
 * (func_L14_002F7A68 adds and wraps), the pair is blended (func_L14_002F7C08)
 * at func_L14_002F7028(phase) * 0.5 + 0.5, and +0x78 moves 15% of the way to
 * that colour.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own, `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_003F4F98-g8.c (blob
 * b0bd52b1), function func_L14_00409B20 (v1.01 address), author commit daf0e4a1
 * (Flavius). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). BYTE-IDENTICAL: VERBATIM but for names (no device). Edits,
 * each name to the target of the v2.00 disc's own relocation at this place
 * (relverify by name on this object, task #2581):
 *   - func_L14_00409B20 -> func_L14_0040B080 (our split name at the v2.00
 *     place);
 *   - D_L14_001B259C -> g_mobyTableBase, the name symbol_addrs/
 *     L14_boot_names.txt gives at the v2.00 relocation target 0x001B251C;
 *   - D_L14_001B251C -> g_mobyTableBase (task #2646): the address-named
 *     spelling was a second name for the same address (A2 class, NOTE
 *     #13409);
 *   - func_00283198 -> func_L14_002F7028 (the level's own copy at its L14
 *     place);
 *   - func_00283B98 -> func_L14_002F7A68 (the level's own copy at its L14
 *     place);
 *   - func_00283D38 -> func_L14_002F7C08 (the level's own copy at its L14
 *     place);
 *   - func_002A7750 -> func_L14_00323250 (the level's own copy at its L14
 *     place);
 *   - func_002B3868 -> func_L14_00333648 (the level's own copy at its L14
 *     place);
 *   - func_002E5CF8 -> func_L14_0036ADA0 (the level's own copy at its L14
 *     place);
 *   - func_L00_00324120 -> func_L14_0032CEA8 (the level's own copy at its L14
 *     place);
 *   - func_L00_003294F0 -> func_L14_00333698 (the level's own copy at its L14
 *     place);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning) and the member closes with it as written;
 *   - a repeated file-scope declaration of func_L14_002F7C08 (upstream
 *     func_00283D38) carried once;
 *   - `#include "common.h"` dropped (the body uses none of its typedefs);
 *     upstream file-scope declarations the body does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L14_0040B080(char *self)
{
    char *v = *(char **)(self + 0x68);
    int c1 = 0x808C8C8C;
    int c2 = 0x80282828;
    int idx;
    float f;
    int r;

    switch (*(unsigned char *)(self + 0x20)) {
    case 0:
        if (func_L14_0032CEA8(self) != 0) {
            idx = *(int *)(v + 0);
            if (idx != -1) {
                func_L14_00333698(g_mobyTableBase + (idx << 8), 1);
            }
            func_L14_00323250(self, 1, -1);
        }
        break;
    case 1:
        if (func_L14_00333648(self) != 0) {
            func_L14_0036ADA0(0, 0, self);
        }
        c1 = 0x80468CDC;
        c2 = 0x801E78AA;
        break;
    }
    f = func_L14_002F7A68(*(float *)(v + 4), 0.10762865f);
    *(float *)(v + 4) = f;
    r = func_L14_002F7C08(c1, c2, func_L14_002F7028(f) * 0.5f + 0.5f);
    *(int *)(self + 0x78) = func_L14_002F7C08(*(int *)(self + 0x78), r, 0.15f);
}
