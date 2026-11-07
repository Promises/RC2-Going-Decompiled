/* Level 14 (Aranos Prison), USA v2.00: a level-only function built from C
 * (task #2581, RULING #13291 row L14-ASM). Compiled ALONE as its own TU and
 * linked at its one place, 0x003E0D10, in place of its split asm; the row is
 * going-decompiled/levels/usa/L14/config/L14_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled by
 * tools/ee/asm_unit.sh (the row's `asm_unit` column). Measured with plain GNU
 * as instead (task #2581, same TU): .text is 0x1F4 bytes, not the place's
 * 0x218, and it emits a .lit4 pool of 12 bytes; level_elf.py cobj refuses it.
 */

extern void func_L14_00323250();
extern void func_L14_003E0F28();
extern int func_L14_003336B8();
extern int func_L14_00333648();
extern void func_L14_0036ADA0();
extern void func_L14_002F69F8();
extern void func_L14_002F6A20(void *dst, void *src, void *src2);
extern void func_L14_002F6B50(void *dst, void *src, void *mtx);
extern void func_L14_002F6D00(void *dst, void *src, float s);
extern float func_L14_002F7A68(float a, float b);
extern float func_L14_002F6960(float a);
extern void func_L14_00319058();
extern void func_L14_002EF098();
extern void func_L14_003E0F78();
extern float D_L14_001B0338 __attribute__((sda));
extern float D_L14_001B033C __attribute__((sda));
extern float D_L14_001B0340 __attribute__((sda));

/**
 * func_L14_003E0D10 - run a sliding platform's three-stage state machine:
 * start, wait, then slide and accelerate until it has travelled 4 units.
 *
 * @arg0: the object (moby); its stage byte is +0x20, its state block +0x68, its
 *        position +0x10, its axis +0xC0 and its angle +0xF0.
 * Return: nothing.
 *
 * Non-obvious: Stage 0 sets stage 1 (func_L14_00323250) and, when state +0x00
 * is not -1, calls func_L14_003E0F28(arg0, 0). Stage 1 waits for
 * func_L14_003336B8, then calls func_L14_003E0F28(arg0, 1) under the same
 * condition, sets stage 2 and starts the speed (state +0x0C) at D_L14_001B0338
 * / 60, negated when state +0x08 is set; the distance (state +0x10) is reset.
 * Stage 2 plays a sound when func_L14_00333648 says so, moves the position
 * along the axis by the speed, adds the speed to the distance, changes the
 * speed by D_L14_001B033C / 3600 (direction by state +0x08), turns the angle by
 * speed * D_L14_001B0340 (func_L14_002F7A68 adds and wraps) and deletes the
 * object (func_L14_00319058) and returns once |distance| exceeds 4.0. A visible
 * object (+0x31) with state +0x04 not -1 then queues func_L14_003E0F78 for it
 * (func_L14_002EF098).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own, `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/14_aranos_prison/L14_00304190-g8.c (blob
 * efa1b3a1), function func_L14_003DF7F8 (v1.01 address), author commit ee3fee45
 * (llesieur99); daf0e4a1 (Flavius) later only renamed its call func_332310 to
 * func_002B38B8. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). BYTE-IDENTICAL: VERBATIM but for names (no device). Edits,
 * each name to the target of the v2.00 disc's own relocation at this place
 * (relverify by name on this object, task #2581):
 *   - func_L14_003DF7F8 -> func_L14_003E0D10 (our split name at the v2.00
 *     place);
 *   - D_L14_001B0388 -> D_L14_001B0338 (our label at that address);
 *   - D_L14_001B038C -> D_L14_001B033C (our label at that address);
 *   - D_L14_001B0390 -> D_L14_001B0340 (our label at that address);
 *   - func_002B38B8 -> func_L14_003336B8 (the level's own copy at its L14
 *     place);
 *   - func_2EE5F0 -> func_L14_002EF098 (the level's own copy at its L14 place);
 *   - func_2F5B78 -> func_L14_002F6960 (the level's own copy at its L14 place);
 *   - func_2F5C10 -> func_L14_002F69F8 (the level's own copy at its L14 place);
 *   - func_2F5C38 -> func_L14_002F6A20 (the level's own copy at its L14 place);
 *   - func_2F5D68 -> func_L14_002F6B50 (the level's own copy at its L14 place);
 *   - func_2F5F18 -> func_L14_002F6D00 (the level's own copy at its L14 place);
 *   - func_2F6C80 -> func_L14_002F7A68 (the level's own copy at its L14 place);
 *   - func_317D90 -> func_L14_00319058 (the level's own copy at its L14 place);
 *   - func_321F58 -> func_L14_00323250 (the level's own copy at its L14 place);
 *   - func_3322A0 -> func_L14_00333648 (the level's own copy at its L14 place);
 *   - func_369AD0 -> func_L14_0036ADA0 (the level's own copy at its L14 place);
 *   - func_3DFA10 -> func_L14_003E0F28 (the level's own copy at its L14 place);
 *   - func_L14_003DFA60 -> func_L14_003E0F78 (its v2.00 split name);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning) and the member closes with it as written;
 *   - a repeated file-scope declaration of func_L14_002F6A20 (upstream
 *     func_2F5C38) carried once;
 *   - `#include "common.h"` dropped (the body uses none of its typedefs);
 *     upstream file-scope declarations the body does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L14_003E0D10(char *arg0) {
    char *v = *(char **)(arg0 + 0x68);

    switch (*(unsigned char *)(arg0 + 0x20)) {
    case 0:
        func_L14_00323250(arg0, 1, -1);
        if (*(int *)(v + 0x0) != -1) {
            func_L14_003E0F28(arg0, 0);
        }
        break;
    case 1:
        if (func_L14_003336B8(arg0) != 0) {
            if (*(int *)(v + 0x0) != -1) {
                func_L14_003E0F28(arg0, 1);
            }
            func_L14_00323250(arg0, 2, -1);
            {
                float s = D_L14_001B0338 * 0.016666668f;

                *(float *)(v + 0xC) = s;
                if (*(int *)(v + 0x8) != 0) {
                    *(float *)(v + 0xC) = -s;
                }
            }
            *(int *)(v + 0x10) = 0;
        }
        break;
    case 2:
        {
            float a[4];
            float b[4];

            if (func_L14_00333648(arg0) != 0) {
                func_L14_0036ADA0(0, 0, arg0);
            }
            func_L14_002F69F8(b);
            func_L14_002F6B50(a, arg0 + 0xC0, b);
            func_L14_002F6D00(a, a, *(float *)(v + 0xC));
            func_L14_002F6A20(arg0 + 0x10, arg0 + 0x10, a);
            {
                float old = *(float *)(v + 0xC);
                float m;

                *(float *)(v + 0x10) = *(float *)(v + 0x10) + old;
                if (*(int *)(v + 0x8) != 0) {
                    m = old - D_L14_001B033C * 0.00027777778f;
                } else {
                    m = old + D_L14_001B033C * 0.00027777778f;
                }
                {
                    float x = m * D_L14_001B0340;

                    *(float *)(v + 0xC) = m;
                    *(float *)(arg0 + 0xF0) = func_L14_002F7A68(*(float *)(arg0 + 0xF0), x);
                }
            }
            if (func_L14_002F6960(*(float *)(v + 0x10)) > 4.0f) {
                func_L14_00319058(arg0);
                return;
            }
        }
        break;
    }
    if (*(unsigned char *)(arg0 + 0x31) != 0 && *(int *)(v + 0x4) != -1) {
        func_L14_002EF098(func_L14_003E0F78, arg0);
    }
}
