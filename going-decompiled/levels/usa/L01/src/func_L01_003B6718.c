/* Level 01 (Oozla), USA v2.00: a level-only function built from C (row L01-G1,
 * task #2626, RULING #13291). Compiled ALONE as its own TU and linked at its
 * one place, 0x003B6718, in place of its split asm; the row is
 * going-decompiled/levels/usa/L01/config/L01_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef int s32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef float f32;

extern int func_L01_00311748(int, int);

extern void func_L01_002E3DE8();

extern void func_L01_0031CFA0(void *, void *, int, int, float);

/**
 * func_L01_003B6718 - the Oozla plant's per-frame timer upkeep: clear the
 * measured word, redraw the short duration, age the frame counter and tick
 * the animation.
 *
 * A set word at +0x350 of the state is cleared and, while the duration at
 * +0x330 is still under 200, redrawn as a fresh 300..360 random
 * (func_L01_00311748). The +0x330, +0x32C and +0x334 timers are released
 * through func_L01_002E3DE8 (the +0x330 one only when the state's +0x244 word
 * is not 1); the u16 frame counter at the object's +0x96 saturates at 65533
 * (it stops one short of 0xFFFE's branch bound); finally
 * func_L01_0031CFA0 steps the object's animation at the state's +0x250 with
 * 6 frames, speed 1, scale 4.0f.
 *
 * @a0: the object record (its state is at *(u8 **)(a0 + 0x68)).
 * Return: none.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` 0, so not Lombyte-origin),
 * file src/levels/01_oozla/L01_003835D8.c, function func_L01_003B5310 (v1.01
 * address), author commit cd8c62bc (Louis-Philippe Le Sieur), whose text keeps
 * 13 of 18 lines of the same author's near-miss draft b18262d1 (RULING #13427
 * rev 2, validator #2638). Adopted with the authors' permission (RULINGs
 * #10405, #12108, #13082, #13291). ADAPTED (byte-identical): names respelled,
 * each to the target of the v2.00 disc's
 * own relocation at this place (task #2626 relverify by name):
 *   - func_L01_003B5310 -> func_L01_003B6718 (our split name);
 *   - func_002A7820 -> func_L01_00311748, func_00282948 ->
 *     func_L01_002E3DE8 (upstream's prototype-less declaration kept) and
 *     func_002B0990 -> func_L01_0031CFA0 (L01's own copies at the v2.00
 *     places);
 *   - func_L01_0031CFA0's prototype written (void *, void *, int, int, float)
 *     where upstream's own file declares (u8 *, u8 *, s32, s32, f32);
 *     byte-neutral (validator #2638);
 *   - `#include "include_asm.h"`/`#include "common.h"` and the file's other
 *     declarations dropped, common.h's typedefs this body uses written out
 *     in the TU instead (validator #2638): this function references no other symbol.
 */
void func_L01_003B6718(u8 *a0) {
    u8 *p = *(u8 **)(a0 + 0x68);
    u16 flags;

    if (*(u32 *)(p + 0x350) != 0) {
        *(u32 *)(p + 0x350) = 0;
        if (*(s32 *)(p + 0x330) < 200) {
            *(u32 *)(p + 0x330) = func_L01_00311748(300, 360);
        }
    }
    if (*(u32 *)(p + 0x244) != 1)
        func_L01_002E3DE8(p + 0x330);
    flags = *(u16 *)(a0 + 0x96);
    if (flags < 65534)
        *(u16 *)(a0 + 0x96) = flags + 1;
    func_L01_002E3DE8(p + 0x32C);
    func_L01_002E3DE8(p + 0x334);
    func_L01_0031CFA0(a0, p + 0x250, 6, 1, 4.0f);
}
