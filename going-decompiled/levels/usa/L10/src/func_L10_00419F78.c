/* Level 10 (Hrugis Cloud), USA v2.00: a level-only function built from C
 * (task #2657, RULING #13291 row L10-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x00419F78, in place of its split asm; the row is
 * going-decompiled/levels/usa/L10/config/L10_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

/* The level's copies of boot code, named by their v2.00 places. */
extern u32 *func_L10_0037CCF8(u8 *arg0);
extern void func_L10_0037DF20(u8 *arg0, s32 a1);
extern f32 func_L10_003364E0(f32, f32);
extern f32 func_L10_003365A8(void);
extern void func_L10_0033E310(void *, f32, f32, f32);
extern void func_L10_00309898(void *, void *, void *);

/* The mission state at 0x00221C80, whose first word picks the table row, and
   the table's base pointer at 0x001B2594. Neither address starts a label in our split
   (each lies inside a larger data label), so both externs are ADDRESS-NAMED
   and the level ELF values them from their names (RULING #13291 §2).
   ADDRESSING-MODEL DEVICE (RULING #8620) on D_L10_001B2594's declaration:
   upstream's `sda` (ignored by this cc1) is respelled
   `section(".sdata")`. cc1 then prints the load as the one-instruction `lw`
   macro, which the assembler expands at -G0 to an absolute lui/lw pair, as
   the ROM has it. Ablation (task #2657, upstream's own `sda`, same arm and
   flags): 8 of 63 words differ from the disc. It moves no data and emits
   nothing. */
extern u8 D_L10_00221C80[];
extern u8 *D_L10_001B2594 __attribute__((section(".sdata")));

/**
 * func_L10_00419F78 - start an object drifting from a row of the level's
 * position table.
 *
 * Sets the state byte (+0x20) to 1 and flag 0x100 of the halfword at +0x34,
 * stores 10 at +0x148 of the record func_L10_0037CCF8 returns and calls
 * func_L10_0037DF20(@arg0, 1). The table row is the 128-byte entry of
 * D_L10_001B2594 that the mission state's first word indexes. The state
 * (+0x68) gets -1 at +0x26C, 10.0 at +0x180 and 0 at +0x268, and the scale
 * (+0x2C) 0.04. A random float in [0, row's first float) and two more from
 * func_L10_003365A8 are written to the position (+0x10) by
 * func_L10_0033E310, then the row's vector at +0x30 is added to it.
 *
 * @arg0: the object.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/10_hrugis_cloud/L10_003170D0.c, function
 * func_L10_004189D8 (v1.01 address), author commit d38f046c (Flavius; its
 * message calls it a port of a matched near-twin with its own offsets,
 * constants and callees, so no twin's text survives as such; the PAL copy
 * func_PAL_L10_0041D788 is later, e5e80a80). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). ADAPTED; the body's
 * statements unchanged, the edits, each name to the target of the v2.00
 * disc's own relocation at this place (task #2657 relverify):
 *   - func_L10_004189D8 -> func_L10_00419F78 (our split name at the v2.00 place);
 *   - func_0037BA88 -> func_L10_0037CCF8, func_0037CCB0 -> func_L10_0037DF20,
 *     func_002A7878 -> func_L10_003364E0, func_002A7940 -> func_L10_003365A8,
 *     func_002AF000 -> func_L10_0033E310, func_00282CC0 -> func_L10_00309898
 *     (our split names at the v2.00 targets);
 *   - D_001B2614 -> D_L10_001B2594 and D_L10_00221CD0 -> D_L10_00221C80,
 *     address-named externs for targets inside larger split data labels
 *     (RULING #13291 §2);
 *   - device: `__attribute__((sda))` -> `__attribute__((section(".sdata")))`
 *     on D_L10_001B2594's extern declaration (RULING #8620; the audit's
 *     `resp` variant, NOTE #13281);
 *   - `#include "common.h"` dropped: the typedefs used are written here and
 *     only this function's declarations kept.
 */
void func_L10_00419F78(u8 *arg0) {
    u8 *s2 = *(u8 **)(arg0 + 0x68);
    u8 *table;
    f32 a, b, c;

    arg0[0x20] = 1;
    *(u16 *)(arg0 + 0x34) |= 0x100;
    *(s32 *)((u8 *)func_L10_0037CCF8(arg0) + 0x148) = 10;
    func_L10_0037DF20(arg0, 1);
    table = D_L10_001B2594 + *(s32 *)D_L10_00221C80 * 128;
    *(s32 *)(s2 + 0x26C) = -1;
    *(f32 *)(s2 + 0x180) = 10.0f;
    *(s32 *)(s2 + 0x268) = 0;
    *(f32 *)(arg0 + 0x2C) = 0.04f;
    a = func_L10_003364E0(0.0f, *(f32 *)table);
    b = func_L10_003365A8();
    c = func_L10_003365A8();
    func_L10_0033E310(arg0 + 0x10, a, b, c);
    func_L10_00309898(arg0 + 0x10, table + 0x30, arg0 + 0x10);
}
