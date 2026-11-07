/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x003FD638, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm sdk29 (ee-gcc
 * 2.9-ee-991111) at -O2 -G0.
 */

typedef unsigned int u32;

/* A table of 128-byte records, inside the larger split label
 * D_L02_001B2D80: address-named (RULING #13291 §2). ADDRESSING-MODEL DEVICE
 * (RULING #8620, EE arm only): `.sdata` on the extern declaration makes cc1
 * print the one-instruction `lw` macro, which GNU as expands absolute at
 * -G0, as retail has it. It moves no data and emits nothing. */
extern char *D_L02_001B368C __attribute__((section(".sdata")));

/**
 * func_L02_003FD638 - address of the vector in the actor's 128-byte record.
 * @a0: the actor; the u32 at its state block's (+0x68) +0x88 is the record
 *      index.
 *
 * Return: D_L02_001B368C + index * 128 + 0x30.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_003F5430.c, function func_L02_003FC098
 * (v1.01 address), author commit adcbd51e (llesieur99). Lombyte mentions in
 * the file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L02_003FC098 -> func_L02_003FD638 (our split name at the
 *     v2.00 place);
 *   - D_L02_001B368C keeps its upstream name, which already states its v2.00
 *     address (address-named, RULING #13291 §2);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     D_L02_001B368C declaration (RULING #8620; upstream's `sda` is a marker
 *     for its own patched compiler, which our cc1 ignores);
 *   - `#include "include_asm.h"` dropped (nothing here used it); the file's
 *     own u32 typedef kept and every other declaration dropped.
 */
char *func_L02_003FD638(char *a0) {
    char *a1 = *(char **)(a0 + 0x68);
    return D_L02_001B368C + *(u32 *)(a1 + 0x88) * 128 + 0x30;
}
