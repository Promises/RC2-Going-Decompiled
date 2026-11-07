/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x00418AB0, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 96 of 188
 * .text words differ from the disc, relocated fields masked, and .text is
 * 0x2F0 bytes, not the place's 0x2F8, and it emits a .lit4 pool of 4 bytes,
 * which level_elf.py cobj refuses (only .text is allowed).
 */

typedef unsigned char u8;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

extern void func_L11_003016D8(void *out, void *in, void *matrix);
extern void func_L11_003011E0(void *out, void *in, void *matrix);
extern f32 func_L11_003018D0(f32 a0, f32 a1);
extern void func_L11_00301A78(void *a0, void *a1);
extern s32 func_L11_00302400(f32 a0);
extern void func_L11_00438150(u8 *a0, s32 a1, s32 a2);
extern void func_L11_0044B0C8(u8 *a0, s32 a1, s32 a2);
extern void func_L11_0044DD60(u8 *a0, s32 a1, s32 a2);
extern void func_L11_004546B0(u8 *a0, s32 a1, s32 a2);
extern void func_L11_004566D0(u8 *a0, s32 a1, s32 a2);
extern void func_L11_00457E90(u8 *a0, s32 a1);
extern u32 D_L11_001B0D7C __attribute__((sda));
extern u8 g_soundBankHandlesBlk[];

/**
 * func_L11_00418AB0 - place the object's three spawned children around a point
 * below and behind it, face each toward the hero, and start them by class.
 *
 * @arg0: the object (moby); +0x68 its state block, whose +0x10/+0x14/+0x18 hold
 *         the three child pointers (any may be 0).
 * Return: none.
 *
 * Non-obvious: The local point (0, -9, -3) is taken through the object's
 * matrix (+0xC0) and added to its position. The children are put there with x
 * - 2.0, with y + 1.0 and with y - 1.0. Each child's yaw (+0xF8) is the angle
 * to the hero position (g_soundBankHandlesBlk + 0x80/+0x84) by
 * func_L11_003018D0, and its matrix is rebuilt. Then its class id (+0xAA)
 * selects its start call: 0xD98, 0xDCF, 0xDF4, 0xDF7 (raised 0.5 first), 0xDF9
 * or 0xD37. The delay argument grows by D_L11_001B0D7C per child. The three
 * pointers are cleared afterwards.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_003D6270-g8.c (blob ae8c1be2), function
 * func_L11_00417340 (v1.01 address), author commit aaa28c3b (Flavius). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device). Respellings, each to the target of the
 * v2.00 disc's own relocation at this place (relverify by name on this object,
 * task #2578):
 *   - func_L11_00417340 -> func_L11_00418AB0 (our split name at the v2.00
 *     place);
 *   - D_00189E20 -> g_soundBankHandlesBlk (the boot's name, same address);
 *   - D_L11_001B0D9C -> D_L11_001B0D7C (our label at that address);
 *   - func_00282CC0 -> func_L11_003011E0 (the level's own copy at its L11
 *     place);
 *   - func_00283098 -> func_L11_003016D8 (the level's own copy at its L11
 *     place);
 *   - func_00283248 -> func_L11_003018D0 (the level's own copy at its L11
 *     place);
 *   - func_002833F0 -> func_L11_00301A78 (the level's own copy at its L11
 *     place);
 *   - func_00283CF0 -> func_L11_00302400 (the level's own copy at its L11
 *     place);
 *   - func_L11_00436910 -> func_L11_00438150 (its v2.00 split name);
 *   - func_L11_004496A0 -> func_L11_0044B0C8 (its v2.00 split name);
 *   - func_L11_0044C328 -> func_L11_0044DD60 (its v2.00 split name);
 *   - func_L11_00452C58 -> func_L11_004546B0 (its v2.00 split name);
 *   - func_L11_00454C78 -> func_L11_004566D0 (its v2.00 split name);
 *   - func_L11_00456438 -> func_L11_00457E90 (its v2.00 split name);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning), and the member closes with it as written;
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_00418AB0(u8 *arg0) {
    u8 *s0 = arg0;
    u8 *s1 = *(u8 **)(s0 + 0x68);
    u8 *p;
    u8 *base;
    f32 loc[4];
    s32 total;
    s32 i;

    loc[0] = 0.0f;
    loc[1] = -9.0f;
    loc[2] = -3.0f;
    loc[3] = 0.0f;
    func_L11_003016D8(loc, loc, s0 + 0xC0);
    func_L11_003011E0(loc, loc, s0 + 0x10);

    if (*(u8 **)(s1 + 0x10) != 0) {
        *(f32 *)(*(u8 **)(s1 + 0x10) + 0x10) = loc[0] - 2.0f;
        *(f32 *)(*(u8 **)(s1 + 0x10) + 0x14) = loc[1];
        *(f32 *)(*(u8 **)(s1 + 0x10) + 0x18) = loc[2];
        *(f32 *)(*(u8 **)(s1 + 0x10) + 0x1C) = loc[3];
    }
    if (*(u8 **)(s1 + 0x14) != 0) {
        *(f32 *)(*(u8 **)(s1 + 0x14) + 0x10) = loc[0];
        *(f32 *)(*(u8 **)(s1 + 0x14) + 0x14) = loc[1] + 1.0f;
        *(f32 *)(*(u8 **)(s1 + 0x14) + 0x18) = loc[2];
        *(f32 *)(*(u8 **)(s1 + 0x14) + 0x1C) = loc[3];
    }
    if (*(u8 **)(s1 + 0x18) != 0) {
        *(f32 *)(*(u8 **)(s1 + 0x18) + 0x10) = loc[0];
        *(f32 *)(*(u8 **)(s1 + 0x18) + 0x14) = loc[1] - 1.0f;
        *(f32 *)(*(u8 **)(s1 + 0x18) + 0x18) = loc[2];
        *(f32 *)(*(u8 **)(s1 + 0x18) + 0x1C) = loc[3];
    }

    base = g_soundBankHandlesBlk;
    p = *(u8 **)(s1 + 0x10);
    total = 0;
    for (i = 0; i < 3; i++) {
        if (i == 1) {
            p = *(u8 **)(s1 + 0x14);
        }
        if (i == 2) {
            p = *(u8 **)(s1 + 0x18);
        }
        if (p == 0) {
            continue;
        }
        *(s32 *)(p + 0xF0) = 0;
        *(f32 *)(p + 0xF4) = 0.0f;
        *(f32 *)(p + 0xF8) = func_L11_003018D0(*(f32 *)(base + 0x80) - *(f32 *)(p + 0x10),
                                          *(f32 *)(base + 0x84) - *(f32 *)(p + 0x14));
        *(s32 *)(p + 0xFC) = 0;
        func_L11_00301A78(p + 0xC0, p + 0xF0);
        switch (*(s16 *)(p + 0xAA)) {
        case 0xD98:
            func_L11_0044B0C8(p, total, 0);
            break;
        case 0xDCF:
            func_L11_0044DD60(p, total, 0);
            break;
        case 0xDF4:
            func_L11_004546B0(p, total, 0);
            break;
        case 0xDF7:
            *(f32 *)(p + 0x18) += 0.5f;
            func_L11_004566D0(p, func_L11_00302400((f32)total * 0.16666667f), 0);
            break;
        case 0xDF9:
            func_L11_00457E90(p, total);
            break;
        case 0xD37:
            func_L11_00438150(p, total, 0);
            break;
        }
        total += D_L11_001B0D7C;
    }

    *(u8 **)(s1 + 0x10) = 0;
    *(u8 **)(s1 + 0x14) = 0;
    *(u8 **)(s1 + 0x18) = 0;
}
