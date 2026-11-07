/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * through the boot's assembler model (task #2579, RULING #13291 row L02-ASM).
 * Compiled ALONE as its own TU and linked at its one place, 0x003D3190, in
 * place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by tools/ee/asm_unit.sh (the row's `asm_unit` column; the plain-`as`
 * ablation is in the row's FACT, task #2579).
 */

extern int func_L02_003214D8(char *, void *, int, float, float, float, int, int, int);
extern float func_L02_00315998(char *);
extern void func_L02_003168E0(char *, int, void *);
extern void func_L02_00321390(void *, void *, void *, void *);
extern void func_L02_002F2D88(void *, void *, void *);
extern float func_L02_002F2EA0(void *);
extern int func_L02_00367DF8(int, int, char *);

/**
 * func_L02_003D3190 - the hit window of the actor's swing between two joints.
 *
 * @object: the actor (moby); +0x68 its state block, whose +0x2E0 is the target,
 *          +0x34 a byte passed on as a float and +0x2F8 a once-per-swing latch.
 * Return: none.
 *
 * Non-obvious: only while the animation time (func_L02_00315998) is in
 * [19, 31]. It takes joints 0 and 1 (func_L02_003168E0) as a segment, the point
 * on it closest to the target's position (func_L02_00321390), and the fraction
 * |closest - end| / |start - end| clamped to [0, 1]. It tests a hit at the
 * closest point with size fraction * 0.35 + 0.15 (func_L02_003214D8); the first
 * hit while the latch is clear plays sound 5 (func_L02_00367DF8) and sets the
 * latch. A second test at the end joint with size 0.5 always follows.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/02_maktar_nebula/L02_002ED7B0.c, function func_L02_003D1C38
 * (v1.01 address), author commit 455cde53 (Flavius). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the body's
 * statements unchanged. Respellings, each to the target of the v2.00 disc's
 * own relocation at this place (relverify by name, task #2579):
 *   - func_L02_003D1C38 -> func_L02_003D3190 (our split name at the v2.00
 *     place);
 *   - func_002A8C28 -> func_L02_003214D8 (the level's own copy at its L02
 *     place);
 *   - func_0029F530 -> func_L02_00315998 (the level's own copy at its L02
 *     place);
 *   - func_0029FCC0 -> func_L02_003168E0 (the level's own copy at its L02
 *     place);
 *   - func_L00_0031BDC0 -> func_L02_00321390 (the level's own copy at its L02
 *     place);
 *   - func_00282CF0 -> func_L02_002F2D88 (the level's own copy at its L02
 *     place);
 *   - func_00282DF0 -> func_L02_002F2EA0 (the level's own copy at its L02
 *     place);
 *   - func_002E5CF8 -> func_L02_00367DF8 (the level's own copy at its L02
 *     place);
 *   - built at -G8 against upstream's -G0 (the audit's code G, NOTE #13281; a
 *     tsv setting);
 *   - `#include "common.h"` dropped (this body uses nothing from it); upstream
 *     file-scope declarations it does not use are not carried; the rest keep
 *     upstream's order and text.
 */
void func_L02_003D3190(char *object) {
    float start[4], end[4], closest[4], remaining[4], span[4];
    char *state;
    float time, fraction;

    time = func_L02_00315998(object);
    state = *(char **)(object + 0x68);
    if (19.0f <= time && time <= 31.0f) {
        func_L02_003168E0(object, 0, start);
        func_L02_003168E0(object, 1, end);
        func_L02_00321390(closest, start, end, *(char **)(state + 0x2E0) + 0x10);
        func_L02_002F2D88(remaining, closest, end);
        func_L02_002F2D88(span, start, end);
        fraction = func_L02_002F2EA0(remaining);
        fraction /= func_L02_002F2EA0(span);
        if (fraction < 0.0f)
            fraction = 0.0f;
        if (fraction > 1.0f)
            fraction = 1.0f;
        if (func_L02_003214D8(object, closest, 0x10001, fraction * 0.35f + 0.15f,
                          (float)*(unsigned char *)(state + 0x34), 1.0f, 0, 1, 0) &&
            *(short *)(state + 0x2F8) == 0) {
            func_L02_00367DF8(5, 0, object);
            *(short *)(state + 0x2F8) = 1;
        }
        func_L02_003214D8(object, end, 0x10001, 0.5f,
                      (float)*(unsigned char *)(state + 0x34), 1.0f, 0, 1, 0);
    }
}
