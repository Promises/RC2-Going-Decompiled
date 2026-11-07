/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x00453EA8, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 184 of
 * 214 .text words differ from the disc, relocated fields masked, and .text is
 * 0x358 bytes, not the place's 0x3BC, and it emits a .lit4 pool of 48 bytes,
 * which level_elf.py cobj refuses (only .text is allowed).
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

extern float func_L11_0032DD60(float, float);
extern s32 func_L11_0032DCC8(s32);
extern void func_L11_003011E0(void *, void *, void *);
extern s32 func_L11_00302400(float);
extern float func_L11_003013B8(void *, void *);
extern void func_L11_0032FF88(void *);
extern void *func_L11_00330218(void *, s32, s32);
extern s32 func_L11_0033A1D8(void *, void *, s32);
extern void func_L11_00301D20(void *, void *);
extern void func_L11_00338628(void *, void *, s32);
extern s32 func_L11_00447BE8(s32, void *);
extern void func_L11_0032DBF8(void *, s32, s32);
extern char g_cameraPos[];
extern s32 D_L11_001A8FF4;
extern void func_L11_00340930(char *, float, float, float, float);
extern void func_L11_00454850(void *);
extern void func_L11_00324B40(void *, int, void *);
extern void func_L11_00324AA0(void *, int, void *);
extern void func_L11_003011A8(void *);
extern void func_L11_00301700(void *, void *, void *);
extern s32 func_L11_0033DAC8(void *, s32, s32, s32, float, float);

/**
 * func_L11_00453EA8 - per-frame update of a Joba object: set it up on its
 * first frame, then run proximity, frame and, from stage 3, hit, death and
 * tracking checks.
 *
 * @object: the object (moby); +0x20 its stage, +0x68 its state block.
 * Return: none.
 *
 * Non-obvious: On stage 0 it seeds its state block: a random-sign turn rate of
 * 15 to 40 degrees, the 0.5/0.4 constant set, and func_L11_00340930's turn
 * limits. Then it runs its setup. Every frame it sets the proximity flag and
 * rebuilds the frame matrix at state + 0x290.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_004465F8-g8.c (blob c5476c37), function
 * func_L11_00452450 (v1.01 address), author commit 96f56db3 (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2578):
 *   - func_L11_00452450 -> func_L11_00453EA8 (our split name at the v2.00
 *     place);
 *   - D_001A8FF4 -> D_L11_001A8FF4 (our label at that address);
 *   - D_L11_001BA8C0 -> g_cameraPos (L11's boot-map name at
 *     the v2.00 target 0x001BA940);
 *   - func_00282C88 -> func_L11_003011A8 (the level's own copy at its L11
 *     place);
 *   - func_00282CC0 -> func_L11_003011E0 (the level's own copy at its L11
 *     place);
 *   - func_00282E48 -> func_L11_003013B8 (the level's own copy at its L11
 *     place);
 *   - func_002830C0 -> func_L11_00301700 (the level's own copy at its L11
 *     place);
 *   - func_00283698 -> func_L11_00301D20 (the level's own copy at its L11
 *     place);
 *   - func_00283CF0 -> func_L11_00302400 (the level's own copy at its L11
 *     place);
 *   - func_0029FC20 -> func_L11_00324AA0 (the level's own copy at its L11
 *     place);
 *   - func_0029FCC0 -> func_L11_00324B40 (the level's own copy at its L11
 *     place);
 *   - func_002A7750 -> func_L11_0032DBF8 (the level's own copy at its L11
 *     place);
 *   - func_002A77E0 -> func_L11_0032DCC8 (the level's own copy at its L11
 *     place);
 *   - func_002A7878 -> func_L11_0032DD60 (the level's own copy at its L11
 *     place);
 *   - func_002A93B0 -> func_L11_0032FF88 (the level's own copy at its L11
 *     place);
 *   - func_002A9640 -> func_L11_00330218 (the level's own copy at its L11
 *     place);
 *   - func_002AFFD8 -> func_L11_00338628 (the level's own copy at its L11
 *     place);
 *   - func_002B0ED8 -> func_L11_0033A1D8 (the level's own copy at its L11
 *     place);
 *   - func_002B3BA8 -> func_L11_0033DAC8 (the level's own copy at its L11
 *     place);
 *   - func_002B60B0 -> func_L11_00340930 (the level's own copy at its L11
 *     place);
 *   - func_L11_004461C0 -> func_L11_00447BE8 (its v2.00 split name);
 *   - func_L11_00452DF8 -> func_L11_00454850 (its v2.00 split name);
 *   - D_L11_001BA940 -> g_cameraPos (task #2646): the name
 *     symbol_addrs/L11_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2 class,
 *     NOTE #13409).
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_00453EA8(char *object)
{
    char *state = *(char **)(object + 0x68);
    char *aim;
    char *hit;
    char *source;
    float matrix[16];
    float turnRate;
    s16 kind;

    if (*(u8 *)(object + 0x20) == 0) {
        *(char **)(state + 0x18) = state + 0x100;
        turnRate = func_L11_0032DD60(15.0f, 40.0f) * 0.017453294f;
        if (func_L11_0032DCC8(2) != 0)
            turnRate = -turnRate;
        *(f32 *)(state + 0x2C8) = turnRate;
        if (func_L11_0032DCC8(2) == 1)
            *(u16 *)(object + 0x34) |= 0x8000;
        *(u8 *)(state + 0x48) = func_L11_00302400(15.0f);
        *(u8 *)(state + 0x4A) = func_L11_00302400(12.0f);
        *(u8 *)(state + 0x60) = 0xB;
        *(s32 *)(state + 0x98) = 4;
        *(f32 *)(state + 0x100) = 0.8f;
        *(f32 *)(state + 0x104) = *(f32 *)(state + 0x84) = *(f32 *)(state + 0x80) = 0.5f;
        *(f32 *)(state + 0x10C) = *(f32 *)(state + 0x108) = *(f32 *)(state + 0x8C) = 0.4f;
        *(f32 *)(state + 0x110) = 1.5707965f;
        *(f32 *)(state + 0x130) = 0.52359885f;
        *(f32 *)(state + 0x12C) = 0.0055555557f;
        *(f32 *)(state + 0x134) = 2.8f;
        func_L11_00340930(object, 0.004444444552063942f, 0.13333334028720856f,
                      0.0011635529808700085f, 0.10471977293491364f);
        *(u8 *)(state + 0x28) = 3;
        func_L11_00454850(object);
    }
    if (*(s16 *)(state + 0x2D8) == 0 && *(u8 *)(object + 0x20) != 0xC && D_L11_001A8FF4 == 0) {
        *(u16 *)(object + 0x34) |= 0x400;
        if (*(u8 *)(object + 0x31) != 0 && func_L11_003013B8(object + 0x10, g_cameraPos) < 24.0f) {
            func_L11_0032FF88(object);
            *(u8 *)(object + 0x6F) = 0x16;
        }
    } else {
        *(u16 *)(object + 0x34) &= 0xFBFF;
    }
    aim = state + 0x290;
    func_L11_00324B40(object, 1, state + 0x270);
    func_L11_00324B40(object, 2, state + 0x280);
    func_L11_00324AA0(object, 3, matrix);
    func_L11_003011A8(aim);
    *(s32 *)(state + 0x298) = 0;
    *(f32 *)(state + 0x290) = 0.25f;
    func_L11_00301700(aim, aim, matrix);
    func_L11_003011E0(aim, aim, matrix + 12);
    if (*(u8 *)(object + 0x20) >= 3) {
        hit = (char *)func_L11_00330218(object, 0x10000, 1);
        if (hit != 0) {
            source = *(char **)(hit + 0x20);
            if (source != 0 && *(u8 *)(object + 0x31) == 0) {
                kind = *(s16 *)(source + 0xAA);
                if (kind == 0xA03 || kind == 0xB89)
                    *(s32 *)(hit + 0x2C) = 0;
            }
        }
        if (func_L11_0033A1D8(object, state + 0x50, 0) == 5) {
            *(s32 *)(state + 0xE0) = 0;
            *(f32 *)(state + 0xE4) = 0.8377582f;
        }
        if (*(s16 *)(state + 0x2D8) != 0) {
            char *pos;
            func_L11_00301D20(state + 0x230, object + 0xC0);
            pos = object + 0x10;
            func_L11_00338628(pos, state + 0x260, 0);
            if (func_L11_00447BE8(*(s32 *)(object + 0xB8), pos) == 0) {
                *(s32 *)(state + 0x20) = 0;
                func_L11_0032DBF8(object, 0xC, 4);
            }
        }
        if (*(u8 *)(object + 0x20) >= 2 && *(u8 *)(state + 0x2E) == 2)
            func_L11_00454850(object);
        *(s16 *)(state + 0x2D0) = func_L11_0033DAC8(object, *(s32 *)(state + 0x2CC), 0, 0, 100.0f, 100.0f);
        *(s32 *)(state + 0x2C4) = *(s32 *)(state + 0x220);
    }
}
