/* Level 02 (Maktar Nebula), USA v2.00: one level-only function built from C
 * (task #2557, RULING #13291 row L02-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x00402258, in place of its split asm; the row is
 * going-decompiled/levels/usa/L02/config/L02_c.tsv, arm s136 (SN 2.95.3 v1.36
 * cc1, -fopt-stack) at -O2 -G0.
 */

typedef short s16;

/* The head of a list of objects, inside the larger split label
 * D_L02_001B2D80: address-named (RULING #13291 §2). ADDRESSING-MODEL DEVICE
 * (RULING #8620, EE arm only): `.sdata` on the extern declaration makes cc1
 * print the one-instruction `lw` macro, which GNU as expands absolute at
 * -G0, as retail has it. It moves no data and emits nothing. */
extern char *D_L02_001B35A8 __attribute__((section(".sdata")));
extern void func_L02_0042F6C8(char *);
extern void func_L02_00435480(char *);
extern void func_L02_00416490(char *);
extern void func_L02_003F1B98(char *);
extern void func_L02_003940C0(char *);
extern void func_L02_00395D20(char *);
extern void func_L02_003ABAD0(char *);
extern void func_L02_003E0270(char *);
extern void func_L02_003157A0(char *);

/**
 * func_L02_00402258 - run the per-kind handler of every object in the list
 * at D_L02_001B35A8.
 *
 * The list is linked through +0x28. By each object's kind (half-word +0xAA):
 * 0xE7D func_L02_0042F6C8, 0xEB0 func_L02_00435480, 0xCB2 func_L02_00416490,
 * 0xB02 func_L02_003F1B98, 0x1366 func_L02_003940C0, 0x1367
 * func_L02_00395D20, 0x326 func_L02_003ABAD0, 0xA5C and 0x1192
 * func_L02_003E0270; the seventeen kinds 0xAE0-0xAE2, 0xAEC, 0xB06, 0xB09,
 * 0xCC0-0xCC9 and 0xCDB go to func_L02_003157A0; any other kind is skipped.
 *
 * Non-obvious: the next link is read after the handler returns.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own), file
 * src/levels/02_maktar_nebula/L02_003F5430.c, function func_L02_00400B10
 * (v1.01 address), author commit 6eae4d38 (bigmak94). Lombyte mentions in
 * the file: 0. Adopted with the authors' permission (RULINGs #10405, #12108,
 * #13082, #13291). ADAPTED, the body's statements unchanged; edits:
 *   - name func_L02_00400B10 -> func_L02_00402258 (our split name at the
 *     v2.00 place);
 *   - func_003320F0 -> func_L02_0042F6C8, func_00333048 -> func_L02_00435480,
 *     func_L02_00414D48 -> func_L02_00416490, func_L02_003F0628 ->
 *     func_L02_003F1B98, func_L02_00392C70 -> func_L02_003940C0,
 *     func_L02_003948D0 -> func_L02_00395D20, func_L02_003AA650 ->
 *     func_L02_003ABAD0, func_L02_003DED08 -> func_L02_003E0270,
 *     func_0029F338 -> func_L02_003157A0: our split names at the v2.00
 *     addresses the ROM's own relocations imply (relverify, task #2557);
 *   - D_L02_001B35A8 keeps its upstream name, which already states its v2.00
 *     address (address-named, RULING #13291 §2);
 *   - `__attribute__((sda))` -> `__attribute__((section(".sdata")))` on the
 *     D_L02_001B35A8 declaration (RULING #8620; upstream's `sda` is a marker
 *     for its own patched compiler, which our cc1 ignores);
 *   - `#include "include_asm.h"` dropped (nothing here used it); the file's
 *     own s16 typedef kept, every other declaration dropped, and the
 *     upstream comment folded into this one.
 */
void func_L02_00402258(void) {
    char *e;

    for (e = D_L02_001B35A8; e != 0; e = *(char **)(e + 0x28)) {
        switch (*(s16 *)(e + 0xAA)) {
        case 0xE7D:
            func_L02_0042F6C8(e);
            break;
        case 0xEB0:
            func_L02_00435480(e);
            break;
        case 0xCB2:
            func_L02_00416490(e);
            break;
        case 0xB02:
            func_L02_003F1B98(e);
            break;
        case 0x1366:
            func_L02_003940C0(e);
            break;
        case 0x1367:
            func_L02_00395D20(e);
            break;
        case 0x326:
            func_L02_003ABAD0(e);
            break;
        case 0xA5C:
        case 0x1192:
            func_L02_003E0270(e);
            break;
        case 0xAE0:
        case 0xAE1:
        case 0xAE2:
        case 0xAEC:
        case 0xB06:
        case 0xB09:
        case 0xCC0:
        case 0xCC1:
        case 0xCC2:
        case 0xCC3:
        case 0xCC4:
        case 0xCC5:
        case 0xCC6:
        case 0xCC7:
        case 0xCC8:
        case 0xCC9:
        case 0xCDB:
            func_L02_003157A0(e);
            break;
        }
    }
}
