/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x003F3178, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 23 of 43
 * .text words differ from the disc, relocated fields masked, and .text is 0xAC
 * bytes, not the place's 0xB0.
 */

typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

extern void func_L11_0032FF88(u8 *arg0);
extern f32 func_L11_003013B8(u8 *arg0, u8 *arg1);
extern void func_L11_003F3228();
extern void func_L11_003F32C0(u8 *arg0);
extern void func_L11_003F3458(u8 *arg0);
extern void func_L11_003F3F40(u8 *arg0);
extern void func_L11_003F4138(u8 *arg0);
extern s32 func_L11_003F4310(u8 *arg0);
extern u8 D_L11_001BA940[];

/**
 * func_L11_003F3178 - per-frame dispatcher of a Joba object: set it up on its
 * first frame, otherwise run its four updates.
 *
 * @arg0: the object (moby); +0x20 its stage, +0x31 visible, +0x6F set to 0x2D.
 * Return: none.
 *
 * Non-obvious: Stage 0 calls the setup func_L11_003F3228 through upstream's
 * unprototyped declaration, with no argument, as upstream writes it. Later
 * stages return early when func_L11_003F4310 reports the object handled.
 * Otherwise, if it is visible and within 48.0 of the point D_L11_001BA940
 * (func_L11_003013B8 is the level's distance copy), func_L11_0032FF88 runs and
 * +0x6F becomes 0x2D. Then the four updates func_L11_003F32C0,
 * func_L11_003F3458, func_L11_003F3F40 and func_L11_003F4138 run in that
 * order.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_003D6270.c (blob f335195d), function
 * func_L11_003F1C68 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED: built at -G8 where upstream builds the file at -G0 (its
 * stem has no -g8; a tsv gflag setting, not a text edit; measured at -G0, task
 * #2578: .text 0xAC bytes, not 0xB0 (cobj refuses)); the text is verbatim but
 * for names. Respellings, each to the target of the v2.00 disc's own
 * relocation at this place (relverify by name on this object, task #2578):
 *   - func_L11_003F1C68 -> func_L11_003F3178 (our split name at the v2.00
 *     place);
 *   - D_L11_001BA8C0 -> D_L11_001BA940 (address-named, inside D_L11_001B39B0;
 *     RULING #13291 section 2);
 *   - func_00282E48 -> func_L11_003013B8 (the level's own copy at its L11
 *     place);
 *   - func_002A93B0 -> func_L11_0032FF88 (the level's own copy at its L11
 *     place);
 *   - func_L11_003F1D18 -> func_L11_003F3228 (its v2.00 split name);
 *   - func_L11_003F1DB0 -> func_L11_003F32C0 (its v2.00 split name);
 *   - func_L11_003F1F48 -> func_L11_003F3458 (its v2.00 split name);
 *   - func_L11_003F2A30 -> func_L11_003F3F40 (its v2.00 split name);
 *   - func_L11_003F2C28 -> func_L11_003F4138 (its v2.00 split name);
 *   - func_L11_003F2E00 -> func_L11_003F4310 (its v2.00 split name);
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_003F3178(u8 *arg0) {
    u8 *s0 = arg0;
    if (*(u8 *)(s0 + 0x20) == 0) {
        func_L11_003F3228();
    } else {
        if (func_L11_003F4310(s0) != 0)
            return;
        if (*(u8 *)(s0 + 0x31) != 0
            && func_L11_003013B8(s0 + 0x10, D_L11_001BA940) < 48.0f) {
            func_L11_0032FF88(s0);
            *(u8 *)(s0 + 0x6F) = 0x2D;
        }
        func_L11_003F32C0(s0);
        func_L11_003F3458(s0);
        func_L11_003F3F40(s0);
        func_L11_003F4138(s0);
    }
}
