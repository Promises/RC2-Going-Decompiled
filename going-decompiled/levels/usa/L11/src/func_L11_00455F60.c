/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x00455F60, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 289 of
 * 340 .text words differ from the disc, relocated fields masked, and .text is
 * 0x550 bytes, not the place's 0x5D8, and it emits a .lit4 pool of 64 bytes,
 * which level_elf.py cobj refuses (only .text is allowed).
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed int s32;

extern float func_L11_0032DD60(float a, float b);
extern void func_L11_003014F8(char *a0, void *a1, float f12);
extern void func_L11_003011E0(void *a0, void *a1, void *a2);
extern s32 D_L11_001A8FF4 __attribute__((sda));
extern char g_listenerPosHistory[];
extern char g_cameraPos[];
extern int func_L11_00302400(float);
extern void func_L11_0032DBF8(void *, int, int);
extern void func_L11_00340930(void *, float, float, float, float);
extern void func_L11_003256A8(void *, void *, void *, void *);
extern void func_L11_0032FF88(void *);
extern float func_L11_00330278(void *, void *);
extern void *func_L11_00330218(void *, unsigned int, int);
extern float func_L11_003013B8(void *, void *);
extern int func_L11_00374B68(void *, int);
extern void func_L11_00374BB0(int);
extern int func_L11_00374EB0(int, int, char *);
extern void func_L11_00301D20(void *, void *);
extern void func_L11_00338628(void *, void *, int);
extern void func_L11_003326F8(void *, void *);
extern void func_L11_0032DFB0(void *, float, float);
extern void func_L11_00301210(void *, void *, void *);
extern float func_L11_00301300(void *, void *);
extern float func_L11_00301340(void *);
extern float func_L11_003022A8(float, float);
extern float func_L11_00301398(void *);
extern void func_L11_004565B8(char *);
extern void func_L11_00456830(char *);
extern u8 *func_L11_00443038(u8 *);
extern int func_L11_00447BE8(u8 *, u8 *);

/**
 * func_L11_00455F60 - state handler of the level's launcher: re-arm the shot,
 * aim the head at the target and hand the launch to the level's helpers.
 *
 * @p: the launcher (moby); +0x20 its stage, +0x68 its state block.
 * Return: none.
 *
 * Non-obvious: On stage 0 it scales its size by 0.75. It draws a random spread
 * in +/-(+0x1D0) degrees, converts its rate (+0x1D8) to per-frame, and sets
 * func_L11_00340930's turn limits. g_listenerPosHistory is the boot's name for
 * the address upstream calls D_00188660. Upstream's file needs a `long long`
 * -> `long` respelling on its own (FACT #13321), but this body has no `long
 * long`, so none is made here.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_00453358-g8.c (blob 02c436b7), function
 * func_L11_00454508 (v1.01 address), author commit 4014e0b6 (Flavius). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device). The audit closed it only with its
 * file-level `long long` -> `long` respelling (RULING #13197), but this body's
 * own text has no `long long`, so alone in its TU it needs none (FACT #13321).
 * Respellings, each to the target of the v2.00 disc's own relocation at this
 * place (relverify by name on this object, task #2578):
 *   - func_L11_00454508 -> func_L11_00455F60 (our split name at the v2.00
 *     place);
 *   - D_00188660 -> g_listenerPosHistory (the boot's name, same address);
 *   - D_L11_001BA8C0 -> g_cameraPos (L11's boot-map name at
 *     the v2.00 target 0x001BA940);
 *   - func_00282CC0 -> func_L11_003011E0 (the level's own copy at its L11
 *     place);
 *   - func_00282CF0 -> func_L11_00301210 (the level's own copy at its L11
 *     place);
 *   - func_00282DB0 -> func_L11_00301300 (the level's own copy at its L11
 *     place);
 *   - func_00282DF0 -> func_L11_00301340 (the level's own copy at its L11
 *     place);
 *   - func_00282E48 -> func_L11_003013B8 (the level's own copy at its L11
 *     place);
 *   - func_00282F20 -> func_L11_003014F8 (the level's own copy at its L11
 *     place);
 *   - func_00283698 -> func_L11_00301D20 (the level's own copy at its L11
 *     place);
 *   - func_00283B98 -> func_L11_003022A8 (the level's own copy at its L11
 *     place);
 *   - func_00283CF0 -> func_L11_00302400 (the level's own copy at its L11
 *     place);
 *   - func_002A04B8 -> func_L11_003256A8 (the level's own copy at its L11
 *     place);
 *   - func_002A7750 -> func_L11_0032DBF8 (the level's own copy at its L11
 *     place);
 *   - func_002A7878 -> func_L11_0032DD60 (the level's own copy at its L11
 *     place);
 *   - func_002A7A00 -> func_L11_0032DFB0 (the level's own copy at its L11
 *     place);
 *   - func_002A93B0 -> func_L11_0032FF88 (the level's own copy at its L11
 *     place);
 *   - func_002A9640 -> func_L11_00330218 (the level's own copy at its L11
 *     place);
 *   - func_002A96A0 -> func_L11_00330278 (the level's own copy at its L11
 *     place);
 *   - func_002AB800 -> func_L11_003326F8 (the level's own copy at its L11
 *     place);
 *   - func_002AFFD8 -> func_L11_00338628 (the level's own copy at its L11
 *     place);
 *   - func_002B60B0 -> func_L11_00340930 (the level's own copy at its L11
 *     place);
 *   - func_002E59B0 -> func_L11_00374B68 (the level's own copy at its L11
 *     place);
 *   - func_002E59F8 -> func_L11_00374BB0 (the level's own copy at its L11
 *     place);
 *   - func_002E5CF8 -> func_L11_00374EB0 (the level's own copy at its L11
 *     place);
 *   - func_L00_002EE290 -> func_L11_00301398 (the level's own copy at its L11
 *     place);
 *   - func_L11_004417E8 -> func_L11_00443038 (its v2.00 split name);
 *   - func_L11_004461C0 -> func_L11_00447BE8 (its v2.00 split name);
 *   - func_L11_00454B60 -> func_L11_004565B8 (its v2.00 split name);
 *   - func_L11_00454DD8 -> func_L11_00456830 (its v2.00 split name);
 *   - D_L11_001BA940 -> g_cameraPos (task #2646): the name
 *     symbol_addrs/L11_boot_names.txt gives at exactly that address; the
 *     address-named spelling was a second name for the same address (A2 class,
 *     NOTE #13409).
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning), and the member closes with it as written;
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_00455F60(char *p)
{
    char *q = *(char **)(p + 0x68);
    float vec0[4];
    float vec1[4];
    float vec2[4];
    float d;
    float dot;
    float len;
    int idx;
    char *ptr;
    char *entry;
    u8 *r;

    if (*(u8 *)(p + 0x20) == 0) {
        *(int *)(q + 0x18) = (int)(q + 0x60);
        *(float *)(p + 0x2C) *= 0.75f;
        *(int *)(q + 0x1DC) = -1;
        *(u8 *)(q + 0x48) = func_L11_00302400(15.0f);
        *(u8 *)(q + 0x4A) = func_L11_00302400(12.0f);
        *(float *)(q + 0x1D0) =
            func_L11_0032DD60(-*(float *)(q + 0x1D0), *(float *)(q + 0x1D0)) *
            0.01745329424738884f;
        *(u8 *)(p + 0x30) = 0x80;
        *(int *)(q + 0x1D4) = 1;
        *(float *)(q + 0x1D8) *= 0.01666666753590107f;
        func_L11_00340930(p, 0.02777777798473835f, 1.6666667461395264f,
                      0.0011635529808700085f, 0.10471977293491364f);
        *(int *)(q + 0x98) |= 4;
        *(float *)(q + 0x60) = 0.75f;
        *(float *)(q + 0x68) = 0.75f;
        *(int *)(q + 0x64) = 0;
        *(float *)(q + 0x6C) = 0.5f;
        *(float *)(q + 0x70) = 1.134464144706726f;
        *(float *)(q + 0x90) = 0.523598849773407f;
        *(float *)(q + 0x8C) = 0.0027777778450399637f;
        func_L11_003256A8(p, q + 0x1F0, q + 0x1F4, q + 0x1F8);
        *(u8 *)(q + 0x28) = 3;
        *(int *)(q + 0x30) = 0;
        *(short *)(q + 0x1EC) = 0;
        func_L11_00456830(p);
    }
    if (*(short *)(q + 0x1FC) == 0 && *(u8 *)(p + 0x20) != 0xE &&
        D_L11_001A8FF4 == 0) {
        *(u16 *)(p + 0x34) |= 0x400;
        if (*(u8 *)(p + 0x31) != 0) {
            if (func_L11_003013B8(p + 0x10, g_cameraPos) < 24.0f) {
                func_L11_0032FF88(p);
                *(u8 *)(p + 0x6F) = 0x16;
            }
        }
    } else {
        *(u16 *)(p + 0x34) &= 0xFBFF;
    }
    if (*(u8 *)(p + 0x95) >= 2) {
        if (func_L11_00374B68(p, *(int *)(q + 0x1DC)) == 0)
            *(int *)(q + 0x1DC) = func_L11_00374EB0(1, 4, p);
    } else {
        if (func_L11_00374B68(p, *(int *)(q + 0x1DC))) {
            idx = *(int *)(q + 0x1DC);
            if (idx != -1) {
                entry = g_listenerPosHistory + idx * 0x70;
                if (*(char **)(entry + 0x88) == p && *(u8 *)(entry + 0x74) != 0)
                    func_L11_00374BB0(idx);
            }
            *(int *)(q + 0x1DC) = -1;
        }
    }
    if (*(short *)(q + 0x1FC) != 0)
        *(int *)(q + 0x98) &= ~0x10;
    else
        *(int *)(q + 0x98) |= 0x10;
    ptr = (char *)func_L11_00330218(p, 0x10000, 0);
    if (ptr != 0) {
        r = *(u8 **)(ptr + 0x20);
        if (r != 0) {
            if (*(short *)(r + 0xAA) == 0xA03)
                *(int *)(ptr + 0x2C) = 0;
            else if (*(u8 *)(p + 0x31) == 0 && *(short *)(r + 0xAA) == 0xB89)
                *(int *)(ptr + 0x2C) = 0;
        }
    }
    d = func_L11_00330278(p, ptr);
    *(u8 *)(p + 0xA8) = 0xFF;
    *(float *)(q + 0x20) -= d;
    if (d != 0.0f) {
        if (*(u8 *)(p + 0x20) != 3 && *(u8 *)(p + 0x20) != 0xE) {
            func_L11_003014F8((char *)vec0, ptr + 0x10, 0.8333333730697632f);
            func_L11_0032DFB0(vec1, -0.1388888955116272f, 0.1388888955116272f);
            func_L11_003011E0(vec0, vec0, vec1);
            if (*(short *)(q + 0x1FC) == 0) {
                vec0[2] = func_L11_0032DD60(0.0f, 0.0f) * 0.00027777778450399637f;
                vec0[3] = 5627.9248046875f;
            } else {
                func_L11_00301210(vec2, func_L11_00443038(*(u8 **)(p + 0xB8)),
                              p + 0x10);
                vec2[2] = 0.0f;
                func_L11_003014F8((char *)vec2, vec2, 1.0f);
                dot = func_L11_00301300(vec0, vec2);
                if (dot < 0.0f) {
                    func_L11_003014F8((char *)vec2, vec2, dot);
                    len = func_L11_00301340(vec0);
                    func_L11_00301210(vec0, vec0, vec2);
                    func_L11_003014F8((char *)vec0, vec0, len);
                }
            }
            func_L11_003011E0(q + 0x100, q + 0x100, vec0);
            func_L11_0032DFB0(vec1, 0.0017453294713050127f, 0.006981317885220051f);
            *(float *)(q + 0x1B0) = func_L11_003022A8(*(float *)(q + 0x1B0), vec1[0]);
            *(float *)(q + 0x1B4) = func_L11_003022A8(*(float *)(q + 0x1B4), vec1[1]);
            *(float *)(q + 0x1B8) = func_L11_003022A8(*(float *)(q + 0x1B8), vec1[2]);
            func_L11_003326F8(p, q + 0x50);
            func_L11_0032DBF8(p, 0xD, 4);
        }
    }
    if (*(short *)(q + 0x1FC) != 0) {
        func_L11_00301D20(q + 0x170, p + 0xC0);
        func_L11_00338628(p + 0x10, q + 0x1A0, 0);
        if (func_L11_00447BE8(*(u8 **)(p + 0xB8), (u8 *)(p + 0x10)) == 0) {
            *(int *)(q + 0x20) = 0;
            func_L11_0032DBF8(p, 0xE, 4);
        }
        if (*(u8 *)(p + 0x20) != 0xE) {
            len = *(float *)(p + 0x18);
            r = func_L11_00443038(*(u8 **)(p + 0xB8));
            func_L11_00301210(vec0, p + 0x10, r);
            vec0[2] = 0.0f;
            if (func_L11_00301398(vec0) > 225.0f) {
                func_L11_003014F8((char *)vec0, vec0, 15.0f);
                r = func_L11_00443038(*(u8 **)(p + 0xB8));
                func_L11_003011E0(p + 0x10, vec0, r);
                *(float *)(p + 0x18) = len;
            }
        }
    }
    if (*(u8 *)(p + 0x20) >= 2 && *(u8 *)(q + 0x2E) == 2)
        func_L11_00456830(p);
    func_L11_004565B8(p);
}
