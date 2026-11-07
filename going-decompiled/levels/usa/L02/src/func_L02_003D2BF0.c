/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * through the boot's assembler model (task #2579, RULING #13291 row L02-ASM).
 * Compiled ALONE as its own TU and linked at its one place, 0x003D2BF0, in
 * place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by tools/ee/asm_unit.sh (the row's `asm_unit` column; the plain-`as`
 * ablation is in the row's FACT, task #2579).
 */

typedef unsigned char      u8;
extern void func_L02_0031F998(void *, int, int);
extern void func_L02_0031F6B8(void *, int, int, int);
extern int func_L02_003D2E30(char *);
extern float func_L02_002F2F50(char *, char *);
extern float func_L02_002F3430(float, float);
extern float func_L02_002F3ED0(float, float);
extern int func_L02_0031FA68(int);

/**
 * func_L02_003D2BF0 - start a close-range attack on the actor's target if it
 * is near and in front.
 *
 * @object: the actor (moby); +0x68 its state block, whose +0x2E0 is the target
 *          and +0x2E4 a flag selecting the second attack.
 * Return: 1 if an attack was started, 0 otherwise.
 *
 * Non-obvious: it returns 0 at once when func_L02_003D2E30(object) is nonzero.
 * With no target the distance stays 100, so nothing starts. Within 4 units,
 * and with the bearing to the target (func_L02_002F3430) within 0.2618 rad
 * (15 degrees) of the facing at +0xF8 (func_L02_002F3ED0), it picks one of
 * three attacks: func_L02_0031FA68(5) == 0 gives (0xC, 5) and animation 7;
 * else a set +0x2E4 gives (0x12, 6) and animation 0xD; else (0xF, 6) and
 * animation 0xA. func_L02_0031F998 takes the pair; the animation is switched
 * (func_L02_0031F6B8) only when +0x43 is not already it.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/02_maktar_nebula/L02_002ED7B0.c, function func_L02_003D1698
 * (v1.01 address), author commit 24d977dd (bigmak94). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the
 * body's statements unchanged. Respellings, each to the target of the v2.00
 * disc's own relocation at this place (relverify by name, task #2579):
 *   - func_L02_003D1698 -> func_L02_003D2BF0 (our split name at the v2.00
 *     place);
 *   - func_L02_003D18D8 -> func_L02_003D2E30 (its v2.00 split name);
 *   - func_00282E80 -> func_L02_002F2F50 (the level's own copy at its L02
 *     place);
 *   - func_00283248 -> func_L02_002F3430 (the level's own copy at its L02
 *     place);
 *   - func_00283C80 -> func_L02_002F3ED0 (the level's own copy at its L02
 *     place);
 *   - func_002A7470 -> func_L02_0031F6B8 (the level's own copy at its L02
 *     place);
 *   - func_002A7750 -> func_L02_0031F998 (the level's own copy at its L02
 *     place);
 *   - func_002A77E0 -> func_L02_0031FA68 (the level's own copy at its L02
 *     place);
 *   - built at -G8 against upstream's -G0 (the audit's code G, NOTE #13281; a
 *     tsv setting);
 *   - `#include "common.h"` dropped; the typedefs this body uses are copied
 *     from upstream's include/common.h, and upstream file-scope declarations
 *     it does not use are not carried; the rest keep upstream's order and
 *     text.
 */
int func_L02_003D2BF0(char *object) {
    char *state = *(char **)(object + 0x68);
    float dist;
    float ang;

    if (func_L02_003D2E30(object) != 0)
        return 0;
    dist = 100.0f;
    ang = 1.0f;
    if (*(char **)(state + 0x2E0) != 0) {
        dist = func_L02_002F2F50(object + 0x10, *(char **)(state + 0x2E0) + 0x10);
        ang = func_L02_002F3430(*(float *)(*(char **)(state + 0x2E0) + 0x10) - *(float *)(object + 0x10),
                            *(float *)(*(char **)(state + 0x2E0) + 0x14) - *(float *)(object + 0x14));
    }
    if (dist < 4.0f) {
        if (func_L02_002F3ED0(ang, *(float *)(object + 0xF8)) < 0.26179942f) {
            if (func_L02_0031FA68(5) == 0) {
                func_L02_0031F998(object, 0xC, 5);
                if (*(u8 *)(object + 0x43) != 7)
                    func_L02_0031F6B8(object, 7, 0, 0xA);
            } else if (*(int *)(state + 0x2E4) != 0) {
                func_L02_0031F998(object, 0x12, 6);
                if (*(u8 *)(object + 0x43) != 0xD)
                    func_L02_0031F6B8(object, 0xD, 8, 0xF);
            } else {
                func_L02_0031F998(object, 0xF, 6);
                if (*(u8 *)(object + 0x43) != 0xA)
                    func_L02_0031F6B8(object, 0xA, 8, 0xF);
            }
            return 1;
        }
    }
    return 0;
}
