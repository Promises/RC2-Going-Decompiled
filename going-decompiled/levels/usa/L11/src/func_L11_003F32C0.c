/* Level 11 (Joba), USA v2.00, task #2554 row L11-B: one function of the level
 * program built from adopted C. Compiled ALONE as its own TU by
 * tools/levels/level_build.sh and linked at its one place, 0x003F32C0, in place
 * of its split asm; the row is config/L11_c.tsv, the arm SN 2.95.3 v1.36 cc1
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack.
 */

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;
typedef int TI128 __attribute__((mode(TI)));

extern void func_L11_0032DBF8(u8 *arg0, s32 arg1, s32 arg2);
extern u8 *func_L11_00330218(s32 arg0, s32 arg1, s32 arg2);
extern f32 func_L11_00330278(u8 *arg0, u8 *arg1);
extern void func_L11_003326F8(u8 *arg0, u8 *arg1);
extern s32 func_L11_0033C680(u8 *arg0, u8 *arg1, u8 *arg2);
extern s32 func_L11_0033DAC8(u8 *arg0, s32 arg1, s32 arg2, s32 arg3, f32 arg4, f32 arg5);
extern void func_L11_003F45A8(u8 *arg0);
extern f32 D_L11_001B0520 __attribute__((sda));
extern s32 D_L11_001FFAF8[];

/**
 * func_L11_003F32C0 - run the object's timed-effect timer down, step the
 * shared table entry in step with it, and retire the object when the effect
 * is over.
 *
 * @arg0: the moby object; reads +0x20 (mode), +0x68 (state), writes +0xA8.
 *
 * Non-obvious: the table index is kept in `off` so the constant stays out of
 * the symbol's address, as the retail has it; the threshold D_L11_001B0520 is
 * reached through $gp; +0xA8 takes 0xFF every frame before the sound call.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d (the audit's pin), GPL-3.0 (root
 * LICENSE; the file carries no notice of its own; 0 Lombyte mentions), file
 * src/levels/11_joba/L11_003D6270-g8.c, function func_L11_003F1DB0 (v1.01 address), author commit bcafb24e (Gianni).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082; task
 * #2554, row L11-B). VERBATIM but for names:
 * 
 *   - `#include "include_asm.h"` dropped (nothing in the function used it);
 *   - `#include "common.h"` dropped. Every L11-B TU spells the same eight
 *     typedefs, whether or not its body reads each: s8, u8, s16, u16, s32,
 *     u32 and f32 as upstream include/common.h defines them, and TI128 as
 *     upstream L11 files define it locally (e.g. L11_00432628.c:118; it is
 *     not in common.h). A typedef emits no code;
 *   - `__attribute__((sda))` kept verbatim (this toolchain ignores it, warning
 *     only; measured inert at this arm and -G flag);
 *   - func_L11_003F1DB0 -> func_L11_003F32C0 (this function's v2.00 split name);
 *   - D_L11_001B0570 -> D_L11_001B0520 (exact v2.00 split dlabel);
 *   - D_L11_001FFA78 -> D_L11_001FFAF8 (address-named extern: the target lies inside a larger split data label, RULING #13291 s2);
 *   - func_002A7750 -> func_L11_0032DBF8, func_002A9640 -> func_L11_00330218,
 *   - func_002A96A0 -> func_L11_00330278, func_002AB800 -> func_L11_003326F8,
 *   - func_002B3380 -> func_L11_0033C680, func_002B3BA8 -> func_L11_0033DAC8,
 *   - func_L11_003F3098 -> func_L11_003F45A8 (v1.01-address callees respelled to the v2.00 level split names at their disc-implied places).
 * No codegen or addressing device is used; the statements are upstream's.
 */
void func_L11_003F32C0(u8 *arg0) {
    u8 *s0 = arg0;
    u8 *s1;
    u8 *s2;
    u8 *s3;
    u32 off;
    f32 f2;

    if (*(u8 *)(s0 + 0x20) == 0) {
        return;
    }
    s1 = *(u8 **)(s0 + 0x68);
    s2 = func_L11_00330218(s0, 0x10000, 0);
    f2 = func_L11_00330278(s0, s2);
    if (s2 != 0) {
        if (*(u8 *)(s0 + 0x20) >= 3) {
            *(f32 *)(s1 + 0x20) = *(f32 *)(s1 + 0x20) - f2;
            /* The index is a byte offset into the table: keeping it in a variable leaves
               the constant out of the symbol's address, as the retail has it. */
            off = *(u8 *)(s1 + 0x37) * 4 + 0xE0;
            *(s32 *)((u8 *)D_L11_001FFAF8 + off) += (s32)(f2 * 32.0f);
            if (*(f32 *)(s2 + 0x2C) != 0.0f) {
                s3 = s1 + 0x70;
                func_L11_003326F8(s0, s3);
                if (*(f32 *)(s2 + 0x2C) >= D_L11_001B0520 || *(f32 *)(s1 + 0x20) <= 0.0f) {
                    func_L11_003326F8(s0, s3);
                    func_L11_0032DBF8(s0, 8, -1);
                }
            }
        }
    }
    *(u8 *)(s0 + 0xA8) = 0xFF;
    func_L11_0033C680(s0, s1 + 0x50, s1 + 0x20);
    if (*(u8 *)(s0 + 0x20) >= 2 && *(u8 *)(s1 + 0x2E) == 2) {
        *(s32 *)(s1 + 0x20) = 0;
        func_L11_003F45A8(s0);
    }
    *(s32 *)(s1 + 0x258) = func_L11_0033DAC8(s0, *(s32 *)(s1 + 0x25C), 0, 0, 200.0f, 200.0f);
}
