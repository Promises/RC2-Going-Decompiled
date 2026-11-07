/* Level 2 (Maktar Nebula), USA v2.00, adopted from OpenRAC (row L02-D, task
 * #2560). Compiled ALONE as its own TU and linked at its one place, 0x0043F998,
 * in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm tools/ee/ee_cc1.sh
 * s136 at -O2 -G0 -fopt-stack.
 *
 * func_L02_0043F998 - runs the timed effect state machine and emits the
 * initial and debris bursts: kind 0 (from a0+0x20) starts the burst and the
 * +0x98 record copy, kind 1 refreshes the debris timer, kind 2 rebuilds the
 * debris velocity pair twice while the timer runs or has run out.
 *
 * @a0: the actor object; its state is *(a0 + 0x68).
 * Return: nothing.
 *
 * Non-obvious: the debris helper's float parameters interleave with the
 * pointers: this caller's argument setup is the evidence (upstream's file
 * comment). The two tuning pairs and the 128-bit pair are const data read
 * into locals first; d and e are 16-byte aligned so cc1 can lq/sq them.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file src/levels/02_maktar_nebula/
 * L02_0043E150.c, function func_L02_0043E150 (v1.01 address), author commit
 * b8fd9b4c (Flavius), which moved bigmak94's nonmatching draft from 20ad739c
 * (validator #2622, RULING #13427). Adopted with the authors' permission
 * (RULINGs #10405, #12108, #13082). BYTE-IDENTICAL: verbatim but for names, plus:
 *   - `#include "common.h"` dropped; the u8/u16/u32/s32 typedef lines are written out
 *     at the top instead (the file's own TI128 typedef and struct
 *     Pair43E150 are kept verbatim);
 *   - name respellings, upstream v1.01-address names -> our v2.00 split names
 *     (relverify by address, NOTE #13279's relverify.py, BAD 0):
 *     func_L02_0043E150 -> func_L02_0043F998,
 *     func_002A7750 -> func_L02_0031F998,
 *     func_0032C700 -> func_L02_00420A08,
 *     D_L02_001B29B0 -> D_L02_001B29F0,
 *     D_L02_001B29B8 -> D_L02_001B29F8,
 *     D_L02_001B29C0 -> D_L02_001B2A00,
 *     func_L00_0032A258 -> func_L02_003311F8,
 *     func_L00_00324F18 -> func_L02_0032A5E8,
 *     func_00282948 -> func_L02_002F2998,
 *     func_00283098 -> func_L02_002F3238,
 *     func_002A7820 -> func_L02_0031FAA8,
 *     func_L00_002EC248 -> func_L02_002F0B68.
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef int TI128 __attribute__((mode(TI)));

extern void func_L02_0031F998(u8 *,s32,s32);
extern u8 *func_L02_00420A08(void *, int, int, int, int, int, int, int, float, float,
                           int, int, int, void *);

struct Pair43E150 {
    float a;
    float b;
};

extern const struct Pair43E150 D_L02_001B29F0;
extern const struct Pair43E150 D_L02_001B29F8;
extern const TI128 D_L02_001B2A00[2];
extern void func_L02_003311F8(void *, void *, void *, void *, s32, s32, s32, s32, s32, unsigned char, s32);
extern void func_L02_0032A5E8(char *, char *, float, float, float);
extern int func_L02_002F2998(void *);
extern void func_L02_002F3238(void *, void *, void *);
extern int func_L02_0031FAA8(int, int);
extern void func_L02_002F0B68(void *,void *,float,void *,float,int,unsigned char,float,float,float);

void func_L02_0043F998(u8 *a0) {
    u8 *state = *(u8 **)(a0 + 0x68);
    int kind = a0[0x20];
    struct Pair43E150 a = D_L02_001B29F0;
    struct Pair43E150 b = D_L02_001B29F8;
    TI128 c[2];
    float d[4] __attribute__((aligned(16)));
    float e[4] __attribute__((aligned(16)));

    c[0] = D_L02_001B2A00[0];
    c[1] = D_L02_001B2A00[1];
    *(TI128 *)d = 0;
    switch (kind) {
    case 0:
        func_L02_003311F8(a0, c, &b, &a, 0xFFFF, 0, -1, 0x10, 0xFFFF, 0xFF, 0);
        *(u32 *)(a0 + 0x98) = *(u32 *)(*(u8 **)(a0 + 0x24) + 0x10);
        *(u16 *)(a0 + 0x7C) = 0;
        func_L02_0032A5E8((char *)a0, (char *)e, d[0], d[1], d[2]);
        func_L02_00420A08(e, 0, 0x19, 0x14, 0x14, 0x14, 0x2D, 0, *(float *)(a0 + 0x2C) * 7.0f,
                      20.0f, 1, 1, 1, 0);
        *(s32 *)state = 0x14;
        func_L02_0031F998(a0, 1, -1);
        break;
    case 1:
        if (func_L02_002F2998(state) != 0) {
            func_L02_0032A5E8((char *)a0, (char *)e, d[0], d[1], d[2]);
            func_L02_00420A08(e, 0, 0x19, 0x14, 0x14, 0x14, 0x2D, 0,
                          *(float *)(a0 + 0x2C) * 3.5f, 20.0f, 0, 0, 0, 0);
            *(s32 *)(state + 4) = 0x78;
            func_L02_0031F998(a0, 2, -1);
        }
        break;
    case 2:
        if (func_L02_002F2998(state + 4) != 0 || *(s32 *)(state + 4) >= 0x5B) {
            *(TI128 *)e = 0;
            e[1] = -0.016666668f;
            e[2] = 0.050000004f;
            func_L02_002F3238(e, e, a0 + 0xC0);
            func_L02_002F0B68(a0,d,0.0f,e,0.033333335f,func_L02_0031FAA8(3,6),0,0.5f,15.0f,2.0f);
            func_L02_002F0B68(a0,d,0.0f,e,0.033333335f,func_L02_0031FAA8(3,6),1,0.5f,15.0f,2.0f);
            *(s32 *)(state + 4) = func_L02_0031FAA8(0x2D, 0x5A);
        }
        break;
    }
}
