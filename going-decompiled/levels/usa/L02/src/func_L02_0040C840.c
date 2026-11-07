/* Level 02 (Maktar Nebula), USA v2.00: a level-only function built from C
 * through the boot's assembler model (task #2579, RULING #13291 row L02-ASM).
 * Compiled ALONE as its own TU and linked at its one place, 0x0040C840, in
 * place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by tools/ee/asm_unit.sh (the row's `asm_unit` column; the plain-`as`
 * ablation is in the row's FACT, task #2579).
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
extern float func_L02_0031FB00(float, float);
extern void func_L02_002F2D58(void *, void *, void *);
extern int func_L02_003309B0(void *, u32, float, float, int, int);
extern int func_L02_002F3F40(float);
extern int func_L02_0031FA68(int);
extern float func_L02_002F2F18(void *, void *);
extern void func_L02_00321DB0(void *);
extern void func_L02_003168E0(void *, int, void *);
extern void func_L02_00316840(void *, int, void *);
extern void func_L02_002F2D20(void *);
extern void func_L02_002F3260(void *, void *, void *);
extern void *func_L02_00322040(void *, int, int);
extern int func_L02_0032CF98(void *, void *, int);
extern char D_L02_001BA500[];
extern void func_L02_0040D0C8(u8 *);
extern void func_L02_00333AC0(void *, float, float, float, float);

/**
 * func_L02_0040C840 - per-frame update of an L02 enemy: set-up, trigger, joint
 * points and damage.
 *
 * @a0: the enemy (moby); +0x20 its state, +0x68 its state block (s1).
 * Return: none.
 *
 * Non-obvious: in state 0 it sets the block's parameters: a turn rate of
 * func_L02_0031FB00(15, 40) degrees in radians, negated when
 * func_L02_0031FA68(2) is nonzero, and flag 0x8000 at +0x34 when another
 * func_L02_0031FA68(2) returns 1. It then enters its next state
 * (func_L02_0040D0C8). While +0x31 is set and it is within 24 units of
 * D_L02_001BA500, it calls func_L02_00321DB0 and sets +0x6F to 0x16. Every
 * frame it stores joints 1 and 2 at +0x290/+0x2A0 and the point (0.25, 0, *)
 * through joint 3's matrix at +0x2B0. When +0x31 is set and the state is at
 * least 3, it handles hits. The check that zeroes a hit from classes
 * 0xA03/0xB89 also requires +0x31 == 0, so it can never fire there
 * (upstream's text, kept). A reaction of 5 (func_L02_0032CF98) clears +0xE0
 * and sets +0xE4. It re-enters func_L02_0040D0C8 when the state is at least 2
 * and s1[0x2E] == 2, then stores func_L02_003309B0's result at +0x2E0 and
 * copies +0x200 to +0x2D4.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/02_maktar_nebula/L02_0040A4A8.c, function func_L02_0040B0F8
 * (v1.01 address), author commit 11c21414 (bigmak94). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the
 * body's statements unchanged. Respellings, each to the target of the v2.00
 * disc's own relocation at this place (relverify by name, task #2579):
 *   - func_L02_0040B0F8 -> func_L02_0040C840 (our split name at the v2.00
 *     place);
 *   - func_002A7878 -> func_L02_0031FB00 (the level's own copy at its L02
 *     place);
 *   - func_00282CC0 -> func_L02_002F2D58 (the level's own copy at its L02
 *     place);
 *   - func_002B3BA8 -> func_L02_003309B0 (the level's own copy at its L02
 *     place);
 *   - func_00283CF0 -> func_L02_002F3F40 (the level's own copy at its L02
 *     place);
 *   - func_002A77E0 -> func_L02_0031FA68 (the level's own copy at its L02
 *     place);
 *   - func_00282E48 -> func_L02_002F2F18 (the level's own copy at its L02
 *     place);
 *   - func_002A93B0 -> func_L02_00321DB0 (the level's own copy at its L02
 *     place);
 *   - func_0029FCC0 -> func_L02_003168E0 (the level's own copy at its L02
 *     place);
 *   - func_0029FC20 -> func_L02_00316840 (the level's own copy at its L02
 *     place);
 *   - func_00282C88 -> func_L02_002F2D20 (the level's own copy at its L02
 *     place);
 *   - func_002830C0 -> func_L02_002F3260 (the level's own copy at its L02
 *     place);
 *   - func_002A9640 -> func_L02_00322040 (the level's own copy at its L02
 *     place);
 *   - func_002B0ED8 -> func_L02_0032CF98 (the level's own copy at its L02
 *     place);
 *   - func_L02_0040B980 -> func_L02_0040D0C8 (its v2.00 split name);
 *   - func_002B60B0_1 -> func_L02_00333AC0 (the level's own copy at its L02
 *     place);
 *   - D_L02_001BA500 kept: an address-named extern inside our split label
 *     D_L02_001B3930 (RULING #13291 section 2), valued from its name by
 *     level_elf;
 *   - built at -G8 against upstream's -G0 (the audit's code G, NOTE #13281; a
 *     tsv setting);
 *   - `#include "common.h"` dropped (the file defines the typedefs it uses
 *     itself); upstream file-scope declarations it does not use are not
 *     carried; the rest keep upstream's order and text.
 */
void func_L02_0040C840(u8 *a0) {
    u8 *s1 = *(u8 **)(a0 + 0x68);
    float m[16];
    float f;
    u8 *p;
    u8 *rec;
    u8 *other;
    short cls;

    if (a0[0x20] == 0) {
        *(u8 **)(s1 + 0x18) = s1 + 0x100;
        s1[0x48] = func_L02_002F3F40(15.0f);
        s1[0x4A] = func_L02_002F3F40(12.0f);
        f = func_L02_0031FB00(15.0f, 40.0f) * 0.01745329424738884f;
        if (func_L02_0031FA68(2) != 0)
            f = -f;
        *(float *)(s1 + 0x2D8) = f;
        if (func_L02_0031FA68(2) == 1)
            *(u16 *)(a0 + 0x34) = *(u16 *)(a0 + 0x34) | 0x8000;
        s1[0x60] = 8;
        *(float *)(s1 + 0x80) = 0.5f;
        *(float *)(s1 + 0x84) = 0.5f;
        *(float *)(s1 + 0x8C) = 0.4f;
        *(int *)(s1 + 0x98) = 4;
        *(float *)(s1 + 0x100) = 0.8f;
        *(float *)(s1 + 0x104) = 0.5f;
        *(float *)(s1 + 0x108) = 0.4f;
        *(float *)(s1 + 0x10C) = 0.4f;
        *(float *)(s1 + 0x110) = 1.5707965f;
        *(float *)(s1 + 0x130) = 0.52359885f;
        *(float *)(s1 + 0x12C) = 0.0055555557f;
        func_L02_00333AC0(a0, 0.004444444552063942f, 0.13333334028720856f,
                        0.0011635529808700085f, 0.10471977293491364f);
        s1[0x28] = 3;
        func_L02_0040D0C8(a0);
    }
    if (a0[0x31] != 0) {
        if (func_L02_002F2F18(a0 + 0x10, D_L02_001BA500) < 24.0f) {
            func_L02_00321DB0(a0);
            a0[0x6F] = 0x16;
        }
    }
    func_L02_003168E0(a0, 1, s1 + 0x290);
    p = s1 + 0x2B0;
    func_L02_003168E0(a0, 2, s1 + 0x2A0);
    func_L02_00316840(a0, 3, m);
    func_L02_002F2D20(p);
    *(float *)(s1 + 0x2B0) = 0.25f;
    *(int *)(s1 + 0x2B8) = 0;
    func_L02_002F3260(p, p, m);
    func_L02_002F2D58(p, p, m + 12);
    if (a0[0x31] != 0 && a0[0x20] >= 3) {
        rec = func_L02_00322040(a0, 0x10000, 1);
        if (rec != 0) {
            other = *(u8 **)(rec + 0x20);
            if (other != 0 && a0[0x31] == 0) {
                cls = *(short *)(other + 0xAA);
                if (cls == 0xA03 || cls == 0xB89)
                    *(int *)(rec + 0x2C) = 0;
            }
        }
        if (func_L02_0032CF98(a0, s1 + 0x50, 0) == 5) {
            *(int *)(s1 + 0xE0) = 0;
            *(float *)(s1 + 0xE4) = 0.8377582f;
        }
        if (a0[0x20] >= 2 && s1[0x2E] == 2)
            func_L02_0040D0C8(a0);
        *(s16 *)(s1 + 0x2E0) = func_L02_003309B0(a0, *(u32 *)(s1 + 0x2DC), 100.0f, 100.0f, 0, 0);
        *(int *)(s1 + 0x2D4) = *(int *)(s1 + 0x200);
    }
}
