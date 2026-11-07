/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x0044D7A0, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 173 of
 * 204 .text words differ from the disc, relocated fields masked, and .text is
 * 0x330 bytes, not the place's 0x384, and it emits a .lit4 pool of 40 bytes,
 * which level_elf.py cobj refuses (only .text is allowed).
 */

/* RULING #8620 addressing device, EE arm only (level code has no native arm): a
 * section attribute on an extern DECLARATION moves no data and emits nothing.
 * Ablation (task #2578): with upstream's `sda`/`nosda` text instead, same
 * length, level .text 19 differing bytes, first at 0x0044D9C8. */
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef signed int s32;

extern float func_L11_0032DD60(float, float);
extern s32 func_L11_0032DCC8(s32);
extern s32 func_L11_00302400(float);
extern float func_L11_003013B8(char *, void *);
extern void func_L11_0032FF88(char *);
extern float func_L11_003022A8(float, float);
extern void func_L11_004513B0(char *, char *, s32, s32);
extern void func_L11_00451490(char *, char *, s32, void *, float);
extern void func_L11_004515E0(char *, char *);
extern char *func_L11_00330218(char *, s32, s32);
extern s32 func_L11_0033A1D8(char *, char *, s32);
extern void func_L11_00301D20(char *, char *);
extern void func_L11_00338628(char *, char *, s32);
extern s32 func_L11_00447BE8(s32, char *);
extern void func_L11_0032DBF8(char *, s32, s32);
extern char g_cameraPos[];
extern char D_L11_001A8F20[] __attribute__((section(".sdata")));
extern s32 D_L11_001A8FF4;
extern void func_L11_0044DF28(char *);
extern void func_L11_0044DCA0(char *);

/**
 * func_L11_0044D7A0 - per-frame update of a Joba enemy with a child rig: set
 * both up on its first frame, then update aim, damage and activation.
 *
 * @object: the enemy (moby); +0x20 its stage, +0x68 its state block.
 * Return: none.
 *
 * Non-obvious: On stage 0 it points +0x18 at its parameter block (+0x100). It
 * draws a random-sign turn rate of 15 to 40 degrees and a random mirror flag
 * in +0x34, and seeds sight ranges 15 and 12 and its motion constants. The -1
 * bytes at +0xBC..+0xC3 mark empty rig slots.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_0044BD68-g8.c (blob 9a00ac7a), function
 * func_L11_0044BD68 (v1.01 address), author commit 644c5b4d (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED: the section respelling below; otherwise verbatim but for
 * names. Respellings, each to the target of the v2.00 disc's own relocation at
 * this place (relverify by name on this object, task #2578):
 *   - func_L11_0044BD68 -> func_L11_0044D7A0 (our split name at the v2.00
 *     place);
 *   - D_001A8FF4 -> D_L11_001A8FF4 (our label at that address);
 *   - D_L11_001BA8C0 -> g_cameraPos (L11's boot-map name at
 *     the v2.00 target 0x001BA940);
 *   - func_00282E48 -> func_L11_003013B8 (the level's own copy at its L11
 *     place);
 *   - func_00283698 -> func_L11_00301D20 (the level's own copy at its L11
 *     place);
 *   - func_00283B98 -> func_L11_003022A8 (the level's own copy at its L11
 *     place);
 *   - func_00283CF0 -> func_L11_00302400 (the level's own copy at its L11
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
 *   - func_00325B70 -> func_L11_004513B0 (the level's own copy at its L11
 *     place);
 *   - func_00325C50 -> func_L11_00451490 (the level's own copy at its L11
 *     place);
 *   - func_00325DA0 -> func_L11_004515E0 (the level's own copy at its L11
 *     place);
 *   - func_L11_004461C0 -> func_L11_00447BE8 (its v2.00 split name);
 *   - func_L11_0044C268 -> func_L11_0044DCA0 (its v2.00 split name);
 *   - func_L11_0044C4F0 -> func_L11_0044DF28 (its v2.00 split name);
 *   - D_L11_001BA940 -> g_cameraPos (task #2646): the name
 *     symbol_addrs/L11_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2 class,
 *     NOTE #13409).
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on 1
 *     extern declaration(s) (RULING #8620: upstream's patched cc1 reads sda,
 *     ours does not);
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_0044D7A0(char *object)
{
    char *state = *(char **)(object + 0x68);
    char *hit, *other;
    float speed, angle;
    if (*(u8 *)(object + 0x20) == 0) {
        *(char **)(state + 0x18) = state + 0x100;
        speed = func_L11_0032DD60(15.0f, 40.0f) * 0.01745329424738884f;
        if (func_L11_0032DCC8(2)) speed = -speed;
        *(float *)(state + 0x494) = speed;
        if (func_L11_0032DCC8(2) == 1) *(u16 *)(object + 0x34) |= 0x8000;
        *(u8 *)(state + 0x48) = func_L11_00302400(15.0f);
        *(u8 *)(state + 0x4A) = func_L11_00302400(12.0f);
        *(float *)(state + 0x80) = 0.8f;
        *(float *)(state + 0x8C) = 0.4f;
        *(s8 *)(state + 0xBC) = -1;
        *(s8 *)(state + 0xBD) = -1;
        *(s8 *)(state + 0xBE) = -1;
        *(s8 *)(state + 0xBF) = -1;
        *(s8 *)(state + 0xC2) = -1;
        *(float *)(state + 0x108) = 0.4f;
        *(float *)(state + 0x4AC) = 8.0f;
        *(u8 *)(state + 0x60) = 10;
        *(int *)(state + 0x98) = 5;
        *(float *)(state + 0x84) = 0.8f;
        *(s8 *)(state + 0xC3) = -1;
        *(float *)(state + 0x100) = 0.6f;
        *(float *)(state + 0x104) = 0.2f;
        *(float *)(state + 0x10C) = 0.4f;
        *(float *)(state + 0x110) = 1.5707964897155762f;
        *(float *)(state + 0x130) = 0.523598849773407f;
        *(float *)(state + 0x12C) = 0.0055555556900799274f;
        *(u8 *)(state + 0x28) = 3;
        *(int *)(state + 0x49C) = 0;
        func_L11_004513B0(object, state + 0x4A0, 0, 1);
        *(short *)(state + 0x4CA) = 0;
        func_L11_0044DF28(object);
    }
    if (*(short *)(state + 0x4D2) == 0 && *(u8 *)(object + 0x20) != 11 && !D_L11_001A8FF4) {
        *(u16 *)(object + 0x34) |= 0x400;
        if (*(u8 *)(object + 0x31) && func_L11_003013B8(object + 0x10, g_cameraPos) < 24.0f) {
            func_L11_0032FF88(object);
            *(u8 *)(object + 0x6F) = 22;
        }
    } else *(u16 *)(object + 0x34) &= 0xFBFF;
    angle = func_L11_003022A8(*(float *)(state + 0x49C), *(float *)(state + 0x4AC) * 0.01745329424738884f);
    *(float *)(state + 0x49C) = angle;
    func_L11_00451490(object, state + 0x4A0, 1, D_L11_001A8F20, angle);
    func_L11_004515E0(object, state + 0x4BC);
    if (*(u8 *)(object + 0x20) >= 3) {
        hit = func_L11_00330218(object, 0x10000, 1);
        if (hit && (other = *(char **)(hit + 0x20)) && !*(u8 *)(object + 0x31)) {
            if (*(short *)(other + 0xAA) == 0xA03 || *(short *)(other + 0xAA) == 0xB89)
                *(int *)(hit + 0x2C) = 0;
        }
        if (func_L11_0033A1D8(object, state + 0x50, 0) == 5) {
            *(int *)(state + 0xE0) = 0;
            *(float *)(state + 0xE4) = 0.8377581834793091f;
        }
        func_L11_0044DCA0(object);
        if (*(short *)(state + 0x4D2)) {
            func_L11_00301D20(state + 0x230, object + 0xC0);
            func_L11_00338628(object + 0x10, state + 0x260, 0);
            if (!func_L11_00447BE8(*(int *)(object + 0xB8), object + 0x10)) {
                *(int *)(state + 0x20) = 0;
                func_L11_0032DBF8(object, 11, 5);
            }
        }
    }
    if (*(u8 *)(object + 0x20) >= 2 && *(u8 *)(state + 0x2E) == 2)
        func_L11_0044DF28(object);
}
