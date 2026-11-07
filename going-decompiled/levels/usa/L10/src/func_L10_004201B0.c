/* Level 10 (Hrugis Cloud), USA v2.00: a level-only function built from C
 * (task #2657, RULING #13291 row L10-A). Compiled ALONE as its own TU and
 * linked at its one place, 0x004201B0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L10/config/L10_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

/* The level's copies of boot code and the level's own functions, named by
   their v2.00 places. */
extern u8 *func_L10_0037C8E0();
extern u32 *func_L10_0037CCF8(u8 *arg0);
extern void func_L10_0037CEB8(u8 *arg0, s32 a1, s32 a2, s32 a3);
extern s32 func_L10_0037E600(u8 *arg0, s32 a1, s32 a2, s32 a3);
extern f32 func_L10_003365A8(void);
extern f32 func_L10_00309E88(f32 f12);
extern f32 func_L10_0030A8A0(f32 f12, f32 f13);
extern s32 func_L10_0030AA40(s32 a0, s32 a1, f32 f12);
extern void func_L10_004202F0(u8 *arg0);
extern void func_L10_004203A0(u8 *arg0);
extern void func_L10_00420398(void);
extern u8 func_L10_004252A8(void);

/* The save block's first word, the current level number: g_playerProgress
   at 0x001A79F8, the boot's name there and L10's boot-name map's.
   ADDRESSING-MODEL DEVICE (RULING #8620) on this declaration: upstream's
   `sda` (ignored by this cc1) is respelled `section(".sdata")`, so cc1
   prints the load as a one-instruction macro that the assembler expands at
   -G0 to the absolute lui/lw pair the ROM has. Ablation (task #2657,
   upstream's own `sda`, same arm and flags): 4 of 79 words differ. It moves
   no data and emits nothing. */
extern s32 g_playerProgress[] __attribute__((section(".sdata")));

/**
 * func_L10_004201B0 - per-frame update of a pulsing glow object.
 *
 * State 0 sets the alpha byte (+0x30) to 0xFF and, once func_L10_0037C8E0
 * returns non-zero, sets the object up (func_L10_004202F0), registers the
 * callback pair func_L10_00420398 / func_L10_004203A0 through
 * func_L10_0037CEB8 when the state's (+0x68) word at +0x264 is set, and
 * seeds the phase at +0x268 with func_L10_003365A8. State 1 clears +0x14C
 * of the record func_L10_0037CCF8 returns, sets flag 0x1000 of the halfword
 * at +0x34 only in level 10 while func_L10_004252A8 returns 2 (else clears
 * it), runs func_L10_0037E600, advances the phase by pi/30 through
 * func_L10_0030A8A0 and stores at +0x78 the colour func_L10_0030AA40 makes
 * from it ((func_L10_00309E88(phase) + 1) / 2). Other states do nothing.
 *
 * @arg0: the object.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/10_hrugis_cloud/L10_003170D0.c, function
 * func_L10_0041EC10 (v1.01 address), author commit 27b929ce (Lynder063).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). ADAPTED; the body's statements unchanged, the edits, each name to
 * the target of the v2.00 disc's own relocation at this place (task #2657
 * relverify):
 *   - func_L10_0041EC10 -> func_L10_004201B0 (our split name at the v2.00 place);
 *   - func_0037B670 -> func_L10_0037C8E0, func_L10_0041ED50 ->
 *     func_L10_004202F0, func_0037BC48 -> func_L10_0037CEB8,
 *     func_L00_00414018 -> func_L10_00420398, func_L10_0041EE00 ->
 *     func_L10_004203A0, func_00335320 -> func_L10_003365A8, func_0037BA88
 *     -> func_L10_0037CCF8, func_L10_00423780 -> func_L10_004252A8,
 *     func_0037D390 -> func_L10_0037E600, func_00309B38 ->
 *     func_L10_0030A8A0, func_00283D38 -> func_L10_0030AA40, func_00309120
 *     -> func_L10_00309E88 (our split names at the v2.00 targets);
 *   - D_001A79F0 -> g_playerProgress (the boot's and L10's boot-name map's
 *     name at the v2.00 target 0x001A79F8);
 *   - device: `__attribute__((sda))` -> `__attribute__((section(".sdata")))`
 *     on g_playerProgress's extern declaration (RULING #8620; the audit's
 *     `resp` variant, NOTE #13281);
 *   - `#include "common.h"` dropped: the typedefs used are written here, only
 *     this function's declarations kept (each once; upstream's file repeats
 *     several), and upstream's comment folded into this one.
 */
void func_L10_004201B0(u8 *arg0) {
    u8 *s1 = *(u8 **)(arg0 + 0x68);
    f32 f;
    u8 *p;
    s32 mode;

    switch (arg0[0x20]) {
    case 0:
        arg0[0x30] = 0xFF;
        if (func_L10_0037C8E0() != 0) {
            func_L10_004202F0(arg0);
            if (*(s32 *)(s1 + 0x264) != 0) {
                func_L10_0037CEB8(arg0, (s32)func_L10_00420398, (s32)func_L10_004203A0, 0);
            }
            *(f32 *)(s1 + 0x268) = func_L10_003365A8();
        }
        break;
    case 1:
        p = func_L10_0037CCF8(arg0);
        mode = g_playerProgress[0];
        *(s32 *)((u8 *)p + 0x14C) = 0;
        if (mode == 10 && func_L10_004252A8() == 2) {
            *(u16 *)(arg0 + 0x34) |= 0x1000;
        } else {
            *(u16 *)(arg0 + 0x34) &= 0xEFFF;
        }
        func_L10_0037E600(arg0, 0, 0, 0);
        f = func_L10_0030A8A0(*(f32 *)(s1 + 0x268), 0.10471977f);
        *(f32 *)(s1 + 0x268) = f;
        *(s32 *)(arg0 + 0x78) = func_L10_0030AA40(0x20202020, -1, (func_L10_00309E88(f) + 1.0f) * 0.5f);
        break;
    }
}
