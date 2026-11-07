/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * through the boot's assembler model (task #2579, RULING #13291 row L02-ASM).
 * Compiled ALONE as its own TU and linked at its one place, 0x003D8E68, in
 * place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by tools/ee/asm_unit.sh (the row's `asm_unit` column; the plain-`as`
 * ablation is in the row's FACT, task #2579).
 */

extern void func_L02_002F3238(void *, void *, void *);

extern void func_L02_002F2D58(void *, void *, void *);

extern float func_L02_002F3430(float, float);

extern int func_L02_002F3F40(float);

extern void func_L02_003D37A0(char *, int);

extern void func_L02_003F2D20(char *, int);

extern void func_L02_0040CFB0(char *, int);

extern void func_L02_0040E7C8(char *, int);

typedef struct {
    char pad0[0x80];
    float x;
    float y;
} L02_003D7900_Hero;

extern L02_003D7900_Hero g_soundBankHandlesBlk;

/**
 * func_L02_003D8E68 - place the actor's three spawned actors and start each
 * one's behaviour.
 *
 * @arg0: the actor (moby); +0x68 its state block, whose +0x10/+0x14/+0x18 are
 *        the three spawned actors (any may be 0).
 * Return: none.
 *
 * Non-obvious: the point (0, -9, -3.3) is transformed by the actor's matrix at
 * +0xC0 (func_L02_002F3238) and moved to its position (func_L02_002F2D58).
 * Actor 1 goes there with x - 2, actor 2 with y + 1, actor 3 with y - 1. Each
 * present actor gets +0xF0/+0xF4/+0xFC cleared and +0xF8 set to the bearing to
 * the hero (func_L02_002F3430). The hero is read through upstream's struct view
 * L02_003D7900_Hero over g_soundBankHandlesBlk, whose x/y at +0x80/+0x84 are
 * g_heroPos. Then it starts the behaviour its class (+0xAA) selects: 0x9CA,
 * 0xB22, 0xC3B, or 0xC3E, which is also raised by 0.5 in z. `count` is never
 * incremented (upstream's text), so 0xC3E always gets
 * func_L02_002F3F40(0.0f). The three pointers are cleared at the end.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/02_maktar_nebula/L02_003D26B0.c, function func_L02_003D7900
 * (v1.01 address), author commit 9980854e (Flavius). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the body's
 * statements unchanged. Respellings, each to the target of the v2.00 disc's
 * own relocation at this place (relverify by name, task #2579):
 *   - func_L02_003D7900 -> func_L02_003D8E68 (our split name at the v2.00
 *     place);
 *   - D_00189E20 -> g_soundBankHandlesBlk (the boot's name, same address);
 *   - func_00283098 -> func_L02_002F3238 (the level's own copy at its L02
 *     place);
 *   - func_00282CC0 -> func_L02_002F2D58 (the level's own copy at its L02
 *     place);
 *   - func_00283248 -> func_L02_002F3430 (the level's own copy at its L02
 *     place);
 *   - func_00283CF0 -> func_L02_002F3F40 (the level's own copy at its L02
 *     place);
 *   - func_L02_003D2248 -> func_L02_003D37A0 (its v2.00 split name);
 *   - func_L02_003F17B0 -> func_L02_003F2D20 (its v2.00 split name);
 *   - func_L02_0040B868 -> func_L02_0040CFB0 (its v2.00 split name);
 *   - func_L02_0040D080 -> func_L02_0040E7C8 (its v2.00 split name);
 *   - built at -G8 against upstream's -G0 (the audit's code G, NOTE #13281; a
 *     tsv setting);
 *   - `#include "common.h"` dropped (this body uses nothing from it); upstream
 *     file-scope declarations it does not use are not carried; the rest keep
 *     upstream's order and text.
 */
void func_L02_003D8E68(char *arg0) {
    float v[4];
    char *state = *(char **)(arg0 + 0x68);
    char *obj;
    int i;
    int count;

    v[0] = 0.0f;
    v[1] = -9.0f;
    v[2] = -3.3f;
    v[3] = 0.0f;
    func_L02_002F3238(v, v, arg0 + 0xC0);
    func_L02_002F2D58(v, v, arg0 + 0x10);
    if (*(char **)(state + 0x10) != 0) {
        *(float *)(*(char **)(state + 0x10) + 0x10) = v[0] - 2.0f;
        *(float *)(*(char **)(state + 0x10) + 0x14) = v[1];
        *(float *)(*(char **)(state + 0x10) + 0x18) = v[2];
        *(float *)(*(char **)(state + 0x10) + 0x1C) = v[3];
    }
    if (*(char **)(state + 0x14) != 0) {
        *(float *)(*(char **)(state + 0x14) + 0x10) = v[0];
        *(float *)(*(char **)(state + 0x14) + 0x14) = v[1] + 1.0f;
        *(float *)(*(char **)(state + 0x14) + 0x18) = v[2];
        *(float *)(*(char **)(state + 0x14) + 0x1C) = v[3];
    }
    if (*(char **)(state + 0x18) != 0) {
        *(float *)(*(char **)(state + 0x18) + 0x10) = v[0];
        *(float *)(*(char **)(state + 0x18) + 0x14) = v[1] - 1.0f;
        *(float *)(*(char **)(state + 0x18) + 0x18) = v[2];
        *(float *)(*(char **)(state + 0x18) + 0x1C) = v[3];
    }
    obj = *(char **)(state + 0x10);
    for (count = 0, i = 0; i < 3; i++) {
        L02_003D7900_Hero *hero = &g_soundBankHandlesBlk;

        if (i == 1) {
            obj = *(char **)(state + 0x14);
        }
        if (i == 2) {
            obj = *(char **)(state + 0x18);
        }
        if (obj == 0) {
            continue;
        }
        *(int *)(obj + 0xF0) = 0;
        *(int *)(obj + 0xF4) = 0;
        *(float *)(obj + 0xF8) =
            func_L02_002F3430(hero->x - *(float *)(obj + 0x10),
                          hero->y - *(float *)(obj + 0x14));
        *(int *)(obj + 0xFC) = 0;
        switch (*(short *)(obj + 0xAA)) {
        case 0x9CA:
            func_L02_003D37A0(obj, 0);
            break;
        case 0xB22:
            func_L02_003F2D20(obj, 0);
            break;
        case 0xC3B:
            func_L02_0040CFB0(obj, 0);
            break;
        case 0xC3E:
            *(float *)(obj + 0x18) = *(float *)(obj + 0x18) + 0.5f;
            func_L02_0040E7C8(obj, func_L02_002F3F40((float)count * 0.16666667f));
            break;
        }
    }
    *(int *)(state + 0x10) = 0;
    *(int *)(state + 0x14) = 0;
    *(int *)(state + 0x18) = 0;
}
