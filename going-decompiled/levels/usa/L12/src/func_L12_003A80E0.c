/* Level 12 (Todano), USA v2.00: a level-only function built from C (task #2670,
 * row L12-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x003A80E0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L12/config/L12_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

/* The current level number, the first word of g_playerProgress (0x001A79F8).
   ADDRESSING-MODEL DEVICE (RULING #8620) on its extern declaration:
   upstream's `sda` (ignored by this cc1) is respelled `section(".sdata")`.
   cc1 then prints the load as the one-instruction `lw` macro, which the
   assembler expands at -G0 to the absolute lui/lw pair the ROM has. Ablation
   (task #2670, upstream's own `sda`, same arm and flags): 5 of 56 words
   differ from the disc. It moves no data and emits nothing. */
extern s32 g_playerProgress __attribute__((section(".sdata")));
/* The level's copies of boot code, named by their v2.00 places. */
extern s32 func_L12_00320478(u8 **out, s32 kind, s32 arg2, s32 arg3);
extern s32 func_L12_00320560(u8 **out, u32 value, s32 arg2, s32 arg3);
/* A level-program function (upstream's func_L00_00325AE8), by its v2.00 place. */
extern void func_L12_0032A400(void *arg0, s32 arg1, f32 arg2, f32 arg3);

/**
 * func_L12_003A80E0 - scale every moby of an object's class by the object's
 * own rate.
 *
 * Iterates the mobys of class object[0x21] (func_L12_00320478 starts the
 * walk, func_L12_00320560 steps it). For each one that is visible (+0x31)
 * and has the object's class id (the halfword at +0xAA), calls
 * func_L12_0032A400 on its position (+0x10) with the second word of its vars
 * block, the factor scale * ratio and 0.5. The ratio is the object's +0x2C
 * float over the +0x24 float of the block its +0x24 points to; scale is 1.2,
 * or 1.0 when the current level is 0 or 0x12.
 *
 * @object: the controlling object.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/12_todano/L12_00398E50.c, function
 * func_L12_003A6E18 (v1.01 address), author commit daf0e4a1 (Flavius).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED; the body's statements unchanged, the edits, each name to
 * the target of the v2.00 disc's own relocation at this place (task #2670
 * relverify):
 *   - func_L12_003A6E18 -> func_L12_003A80E0 (our split name at the v2.00 place);
 *   - func_002A8600 -> func_L12_00320478, func_002A86E8 -> func_L12_00320560,
 *     func_L00_00325AE8 -> func_L12_0032A400 (our split names at the v2.00
 *     targets);
 *   - D_001A79F0 -> g_playerProgress (0x001A79F8, the boot's name,
 *     symbol_addrs/usa);
 *   - device: `__attribute__((sda))` -> `__attribute__((section(".sdata")))`
 *     on g_playerProgress's extern declaration (RULING #8620; the audit's
 *     `resp` variant, NOTE #13281);
 *   - `#include "common.h"` dropped: the typedefs used are written here and
 *     only this function's declarations kept; upstream's one-line comment
 *     folded into this one.
 */
void func_L12_003A80E0(u8 *object) {
    s32 mode = g_playerProgress;
    f32 scale = 1.2f;
    u8 *out;

    if (mode == 0 || mode == 0x12) {
        scale = 1.0f;
    }
    func_L12_00320478(&out, object[0x21], 0, 0);
    while (out != 0) {
        u8 *cur = out;

        if (cur[0x31] != 0) {
            u32 *f68 = *(u32 **)(cur + 0x68);

            if (*(s16 *)(cur + 0xAA) == *(s16 *)(object + 0xAA)) {
                f32 ratio = *(f32 *)(object + 0x2C) / *(f32 *)(*(u32 *)(object + 0x24) + 0x24);

                func_L12_0032A400(cur + 0x10, f68[1], scale * ratio, 0.5f);
            }
        }
        func_L12_00320560(&out, (u32)out, 0, 0);
    }
}
