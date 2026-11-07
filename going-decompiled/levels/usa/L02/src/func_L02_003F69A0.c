/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003F69A0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G0.
 */

typedef unsigned int u32;
typedef short s16;
typedef int s32;

/* A table of 32-byte records, inside the larger split label D_L02_001B3930:
 * address-named (RULING #13291 §2). ADDRESSING-MODEL DEVICE (RULING #8620,
 * EE arm only): `.sdata` on the extern declaration makes cc1 print the
 * one-instruction `lw` macro, which GNU as expands absolute at -G0, as retail
 * has it. It moves no data and emits nothing. */
extern char *D_L02_001B3AF8 __attribute__((section(".sdata")));

extern s32 func_L02_002F4008(char *, float);

/**
 * func_L02_003F69A0 - rebuild the actor's bit mask of answering records.
 * @a0: the actor; its state block is the pointer at +0x68.
 *
 * Clears the state's mask at +0x84, then for each of the state's +0x80
 * entries takes the 16-bit record index at +0x40 + 2*i, finds that 32-byte
 * record in D_L02_001B3AF8 and calls func_L02_002F4008 with it and its float
 * at +0x10. Every record for which that call does not return -1 sets bit i
 * of the mask.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_003F5430.c, function func_L02_003F5430
 * (v1.01 address), author commit adcbd51e (llesieur99). Lombyte mentions in
 * the file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L02_003F5430 -> func_L02_003F69A0 (our split name at the
 *     v2.00 place);
 *   - func_002F3220 -> func_L02_002F4008: our split name at the v2.00
 *     address the ROM's own relocation implies (relverify, task #2557);
 *   - D_L02_001B3AF8 keeps its upstream name, which already states its v2.00
 *     address (address-named, RULING #13291 §2);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     D_L02_001B3AF8 declaration (RULING #8620; upstream's `sda` is a marker
 *     for its own patched compiler, which our cc1 ignores);
 *   - `#include "include_asm.h"` dropped (nothing here used it); the file's
 *     own u32, s16 and s32 typedefs kept, every other declaration dropped.
 */
void func_L02_003F69A0(char *a0) {
    char *s0 = *(char **)(a0 + 0x68);
    s32 i;
    s16 *s2;
    *(u32 *)(s0 + 0x84) = 0;
    for (i = 0; i < *(s32 *)(s0 + 0x80); i++) {
        char *q;
        s2 = (s16 *)(s0 + 0x40 + i * 2);
        q = D_L02_001B3AF8 + *s2 * 32;
        if (func_L02_002F4008(q, *(float *)(q + 0x10)) != -1)
            *(u32 *)(s0 + 0x84) |= 1 << i;
    }
}
