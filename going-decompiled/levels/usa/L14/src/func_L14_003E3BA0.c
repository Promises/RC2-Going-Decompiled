/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2581, RULING #13291 row L14-ASM). Compiled ALONE as its own TU and
 * linked at its one place, 0x003E3BA0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled by
 * tools/ee/asm_unit.sh (the row's `asm_unit` column). Measured with plain GNU
 * as instead (task #2581, same TU): .text is 0x254 bytes, not the place's
 * 0x25C, and it emits a .lit4 pool of 4 bytes; level_elf.py cobj refuses it.
 */

extern void func_L14_002F6AA8(void *dst, void *src, float s);
extern float D_L14_001B0460 __attribute__((sda));
extern float D_L14_001B0464 __attribute__((sda));
extern float D_L14_001B0468 __attribute__((sda));
extern void func_L14_002F6A20(void *, void *, void *);
extern void func_L14_002EEA28(void *, float *, float *);
extern float func_L14_002F7028(float);
extern float func_L14_003E37C0(char *);
extern float func_L14_003E3E00(float);
typedef struct {
    float rows[4][4];
    unsigned int colors[4];
    float pairs[4][2];
} L14Swing;

/**
 * func_L14_003E3BA0 - place one corner of a swinging quad and give its screen
 * point a wobble, clamped to [0, 1].
 *
 * @object: the owning object, read by func_L14_003E37C0 (its swing progress).
 * @base:   the quad: four corners, four colours and four screen pairs.
 * @index:  the corner to set.
 * @a:      the corner's origin; @b and @c the two edge vectors.
 * @phase0: the step along @c, in eighths; @phase1 the step along @b.
 * Return: nothing.
 *
 * Non-obvious: The corner is a + b * phase1 / 8 + c * phase0 / 8, projected to
 * a screen pair by func_L14_002EEA28. Each pair component then gets (1 -
 * progress) * D_L14_001B0460 * func_L14_002F7028(func_L14_003E3E00(2 *
 * D_L14_001B0464 * progress * pi + phase * pi / D_L14_001B0468)), with phase0
 * for the first component and phase1 for the second; func_L14_003E37C0 is
 * called once per use, as upstream wrote it.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own, `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190-g8.c (blob
 * efa1b3a1), function func_L14_003E2688 (v1.01 address), author commit bde5ccd6
 * (DrLegitamate). Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). BYTE-IDENTICAL: VERBATIM but for names (no device). Edits,
 * each name to the target of the v2.00 disc's own relocation at this place
 * (relverify by name on this object, task #2581):
 *   - func_L14_003E2688 -> func_L14_003E3BA0 (our split name at the v2.00
 *     place);
 *   - D_L14_001B04B0 -> D_L14_001B0460 (our label at that address);
 *   - D_L14_001B04B4 -> D_L14_001B0464 (our label at that address);
 *   - D_L14_001B04B8 -> D_L14_001B0468 (our label at that address);
 *   - func_0027C768 -> func_L14_002EEA28 (the level's own copy at its L14
 *     place);
 *   - func_00282CC0 -> func_L14_002F6A20 (the level's own copy at its L14
 *     place);
 *   - func_00282D30 -> func_L14_002F6AA8 (the level's own copy at its L14
 *     place);
 *   - func_00283198 -> func_L14_002F7028 (the level's own copy at its L14
 *     place);
 *   - func_L14_003E22A8 -> func_L14_003E37C0 (its v2.00 split name);
 *   - func_L14_003E28E8 -> func_L14_003E3E00 (its v2.00 split name);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning) and the member closes with it as written;
 *   - `#include "common.h"` dropped (the body uses none of its typedefs);
 *     upstream file-scope declarations the body does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L14_003E3BA0(char *object, L14Swing *base, int index, float *a, float *b, float *c,
                       int phase0, int phase1)
{
    float v[4];

    func_L14_002F6AA8(v, b, (float)phase1 * 0.125f);
    func_L14_002F6A20(base->rows[index], a, v);
    func_L14_002F6AA8(v, c, (float)phase0 * 0.125f);
    func_L14_002F6A20(base->rows[index], base->rows[index], v);
    func_L14_002EEA28(base->rows[index], &base->pairs[index][0], &base->pairs[index][1]);
    base->pairs[index][0] += (1.0f - func_L14_003E37C0(object)) * D_L14_001B0460
        * func_L14_002F7028(func_L14_003E3E00((D_L14_001B0464 + D_L14_001B0464) * (func_L14_003E37C0(object) * 3.1415927f)
                                          + (float)phase0 * 3.1415927f / D_L14_001B0468));
    base->pairs[index][1] += (1.0f - func_L14_003E37C0(object)) * D_L14_001B0460
        * func_L14_002F7028(func_L14_003E3E00((D_L14_001B0464 + D_L14_001B0464) * (func_L14_003E37C0(object) * 3.1415927f)
                                          + (float)phase1 * 3.1415927f / D_L14_001B0468));
    if (1.0f < base->pairs[index][0])
        base->pairs[index][0] = 1.0f;
    else if (base->pairs[index][0] < 0.0f)
        base->pairs[index][0] = 0.0f;
    if (1.0f < base->pairs[index][1])
        base->pairs[index][1] = 1.0f;
    else if (base->pairs[index][1] < 0.0f)
        base->pairs[index][1] = 0.0f;
}
