/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x0043BC38, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 86 of 117
 * .text words differ from the disc, relocated fields masked, and .text is
 * 0x1D4 bytes, not the place's 0x228, and it emits a .lit4 pool of 40 bytes,
 * which level_elf.py cobj refuses (only .text is allowed).
 */

extern float func_L11_003013B8(void *, void *);
extern char D_L11_001BA940[];
extern int func_L11_00302400(float);
extern void func_L11_00340930(char *, float, float, float, float);
extern void func_L11_0032D918(void *, int, int, int);
extern void func_L11_0032DBF8(void *, int, int);
extern void func_L11_0032D918(void *, int, int, int);
extern void func_L11_0032DBF8(void *, int, int);
extern int func_L11_00302400(float);
extern void func_L11_0043C368(char *);
extern void func_L11_0033A1D8(char *, char *, int);
extern int func_L11_0033DAC8(char *, int, int, int, float, float);
extern float func_L11_003013B8(void *, void *);
extern void func_L11_0032FF88(char *);
extern char D_L11_001BA940[];

/**
 * func_L11_0043BC38 - per-frame update of a Joba object: set it up on its
 * first frame, then keep its collision, its draw state and its proximity ping.
 *
 * @object: the object (moby); +0x20 its stage, +0x31 visible, +0x43 its
 *          animation, +0x68 its state block.
 * Return: none.
 *
 * Non-obvious: On stage 0 it seeds its state block: sight ranges 15 and 12
 * (func_L11_00302400), its motion constants, func_L11_00340930's turn limits,
 * and 10 hit points when none are set. Then it sets animation 1 and starts
 * func_L11_0043C368. From stage 4 it runs func_L11_0033A1D8. From stage 3 it
 * runs func_L11_0043C368 again while its state byte +0x2E is 2. +0x2D8 is
 * refreshed by func_L11_0033DAC8. A visible object within 48.0 of
 * D_L11_001BA940 runs func_L11_0032FF88 and sets +0x6F to 45, as
 * func_L11_003F3178 does.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_00432628.c (blob 4ed37676), function
 * func_L11_0043A3F8 (v1.01 address), author commit 5acf9869 (Flavius). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * ADAPTED: built at -G8 where upstream builds the file at -G0 (its stem has no
 * -g8; a tsv gflag setting, not a text edit; measured at -G0, task #2578:
 * .text 0x224 bytes, not 0x228 (cobj refuses)); the text is verbatim but for
 * names. Respellings, each to the target of the v2.00 disc's own relocation at
 * this place (relverify by name on this object, task #2578):
 *   - func_L11_0043A3F8 -> func_L11_0043BC38 (our split name at the v2.00
 *     place);
 *   - D_L11_001BA8C0 -> D_L11_001BA940 (address-named, inside D_L11_001B39B0;
 *     RULING #13291 section 2);
 *   - func_00282E48 -> func_L11_003013B8 (the level's own copy at its L11
 *     place);
 *   - func_00283CF0 -> func_L11_00302400 (the level's own copy at its L11
 *     place);
 *   - func_002A7470 -> func_L11_0032D918 (the level's own copy at its L11
 *     place);
 *   - func_002A7750 -> func_L11_0032DBF8 (the level's own copy at its L11
 *     place);
 *   - func_002A93B0 -> func_L11_0032FF88 (the level's own copy at its L11
 *     place);
 *   - func_002B0ED8 -> func_L11_0033A1D8 (the level's own copy at its L11
 *     place);
 *   - func_002B3BA8 -> func_L11_0033DAC8 (the level's own copy at its L11
 *     place);
 *   - func_002B60B0 -> func_L11_00340930 (the level's own copy at its L11
 *     place);
 *   - func_L11_0043AB28 -> func_L11_0043C368 (its v2.00 split name);
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_0043BC38(char *object) {
    char *state = *(char **)(object + 0x68);

    if (*(unsigned char *)(object + 0x20) == 0) {
        *(unsigned char *)(state + 0x48) = func_L11_00302400(15.0f);
        *(unsigned char *)(state + 0x4A) = func_L11_00302400(12.0f);
        *(float *)(state + 0x1A0) = 1.0f;
        *(unsigned char *)(state + 0x180) = 6;
        *(int *)(state + 0x1B8) = 2;
        *(float *)(state + 0x1AC) = 0.15f;
        *(float *)(state + 0x80) = 1.0f;
        *(float *)(state + 0x84) = 0.4f;
        *(float *)(state + 0x88) = 0.3f;
        *(float *)(state + 0x8C) = 0.3f;
        *(float *)(state + 0x90) = 1.5707964897155762f;
        *(float *)(state + 0xB0) = 0.523598849773407f;
        *(float *)(state + 0xB4) = 2.0f;
        *(float *)(state + 0xAC) = 0.0027777778450399637f;
        *(int *)(state + 0x1A4) = 0;
        func_L11_00340930(object, 0.2777777910232544f, 0.3333333432674408f,
                      0.0017453294713050127f, 0.05235988646745682f);
        if (*(short *)(state + 0x24) == 0) {
            *(short *)(state + 0x24) = 10;
            *(float *)(state + 0x20) = 10;
        }
        if (*(unsigned char *)(object + 0x43) != 1)
            func_L11_0032D918(object, 1, 0, 0);
        func_L11_0032DBF8(object, 1, -1);
        func_L11_0043C368(object);
    }
    if (*(unsigned char *)(object + 0x20) >= 4)
        func_L11_0033A1D8(object, state + 0x170, 0);
    if (*(unsigned char *)(object + 0x20) >= 3 &&
        *(unsigned char *)(state + 0x2E) == 2)
        func_L11_0043C368(object);
    *(int *)(state + 0x2D8) = func_L11_0033DAC8(object, *(int *)(state + 0x2E8),
                                         0, 0, 255.0f, 255.0f);
    if (*(unsigned char *)(object + 0x31) != 0) {
        if (func_L11_003013B8(object + 0x10, D_L11_001BA940) < 48.0f) {
            func_L11_0032FF88(object);
            *(unsigned char *)(object + 0x6F) = 45;
        }
    }
}
