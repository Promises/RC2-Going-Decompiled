/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x0044A4D0, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 62 of 114
 * .text words differ from the disc, relocated fields masked, and .text is
 * 0x1C8 bytes, not the place's 0x1D8, and it emits a .lit4 pool of 4 bytes,
 * which level_elf.py cobj refuses (only .text is allowed).
 */

typedef int Rac2TI128 __attribute__((mode(TI)));
extern int func_L11_0044A750(char *object);
extern void func_L11_00301210(void *, void *, void *);
extern float func_L11_00338E18(char *object, float *src, char *third);
extern float func_L11_003018D0(float, float);
extern float func_L11_00302390(float, float);
extern float func_L11_003013B8(void *, void *);
extern int func_L11_0032DCC8(int);
extern void func_L11_0032DBF8(char *, int, int);
extern void func_L11_0032D918(char *, int, int, int);
extern char g_heroPos[];

/**
 * func_L11_0044A4D0 - pick the attack the actor plays against its target, if
 * the target is close and well aimed.
 *
 * @object: the actor (moby); +0x10 position, +0xF8 yaw, +0x43 its animation,
 *          +0x68 its state block (+0x350 the target record, +0x354 a weapon
 *          record, +0x36E the aim mode).
 * Return: 1 when an attack was started; else 0 (also when func_L11_0044A750
 * holds it back).
 *
 * Non-obvious: The aim point is the target record's position, or g_heroPos
 * without one. The aim angle comes from func_L11_00338E18 in aim mode, else
 * from the yaw difference to the point (func_L11_003018D0, func_L11_00302390).
 * Within 4.0 and 15 degrees, a 1-in-4 roll (func_L11_0032DCC8(4) == 0) starts
 * state 0xC (animation 7). Otherwise it starts 0xE (animation 9) with a weapon
 * record, or 0xD (animation 8) without one.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_00448AA8.c (blob 1e5fcaf8), function
 * func_L11_00448AA8 (v1.01 address), author commit d282effa (Flavius). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED: built at -G8 where upstream builds the file at -G0 (its stem has no
 * -g8; a tsv gflag setting, not a text edit; measured at -G0, task #2578:
 * .text 0x1D0 bytes, not 0x1D8 (cobj refuses)); the text is verbatim but for
 * names. Respellings, each to the target of the v2.00 disc's own relocation at
 * this place (relverify by name on this object, task #2578):
 *   - func_L11_00448AA8 -> func_L11_0044A4D0 (our split name at the v2.00
 *     place);
 *   - D_00189EA0 -> g_heroPos (the boot's name, same address);
 *   - func_00282CF0 -> func_L11_00301210 (the level's own copy at its L11
 *     place);
 *   - func_00282E48 -> func_L11_003013B8 (the level's own copy at its L11
 *     place);
 *   - func_00283248 -> func_L11_003018D0 (the level's own copy at its L11
 *     place);
 *   - func_00283C80 -> func_L11_00302390 (the level's own copy at its L11
 *     place);
 *   - func_002A7470 -> func_L11_0032D918 (the level's own copy at its L11
 *     place);
 *   - func_002A7750 -> func_L11_0032DBF8 (the level's own copy at its L11
 *     place);
 *   - func_002A77E0 -> func_L11_0032DCC8 (the level's own copy at its L11
 *     place);
 *   - func_002B04E0 -> func_L11_00338E18 (the level's own copy at its L11
 *     place);
 *   - func_L11_00448D28 -> func_L11_0044A750 (its v2.00 split name);
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
int func_L11_0044A4D0(char *object)
{
    char *state = *(char **)(object + 0x68);
    float v[4];
    float tmp[4];
    float ang;
    char *p;

    if (func_L11_0044A750(object) != 0)
        return 0;
    p = *(char **)(state + 0x350);
    if (p != 0)
        *(Rac2TI128 *)v = *(Rac2TI128 *)(p + 0x10);
    else
        *(Rac2TI128 *)v = *(Rac2TI128 *)g_heroPos;
    if (*(short *)(state + 0x36E) != 0) {
        func_L11_00301210(tmp, v, object + 0x10);
        ang = func_L11_00338E18(object, tmp, state + 0x2F0);
    } else {
        ang = func_L11_00302390(func_L11_003018D0(v[0] - *(float *)(object + 0x10), v[1] - *(float *)(object + 0x14)),
                            *(float *)(object + 0xF8));
    }
    if (func_L11_003013B8(object + 0x10, v) < 4.0f) {
        if (ang < 0.26179942f) {
            if (func_L11_0032DCC8(4) == 0) {
                func_L11_0032DBF8(object, 0xC, 5);
                if (*(unsigned char *)(object + 0x43) != 7)
                    func_L11_0032D918(object, 7, 0, 0xA);
            } else if (*(int *)(state + 0x354) != 0) {
                func_L11_0032DBF8(object, 0xE, 6);
                if (*(unsigned char *)(object + 0x43) != 9)
                    func_L11_0032D918(object, 9, 8, 0xF);
            } else {
                func_L11_0032DBF8(object, 0xD, 6);
                if (*(unsigned char *)(object + 0x43) != 8)
                    func_L11_0032D918(object, 8, 8, 0xF);
            }
            return 1;
        }
    }
    return 0;
}
