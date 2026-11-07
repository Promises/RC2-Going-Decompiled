/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x00443C48, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 34 of 111
 * .text words differ from the disc, relocated fields masked, and .text is
 * 0x1BC bytes, not the place's 0x1D0, and it emits a .lit4 pool of 8 bytes,
 * which level_elf.py cobj refuses (only .text is allowed).
 */

typedef int s32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef float f32;
extern u8 *func_L11_00443038(u8 *);
extern const f32 D_L11_001B159C __attribute__((sda));
extern const f32 D_L11_001B15A0 __attribute__((sda));
extern const f32 D_L11_001B15A4 __attribute__((sda));
extern const f32 D_L11_001B15A8 __attribute__((sda));
extern const f32 D_L11_001B15AC __attribute__((sda));
extern const f32 D_L11_001B15B0 __attribute__((sda));
extern const f32 D_L11_001B15B4 __attribute__((sda));
extern const f32 D_L11_001B15B8 __attribute__((sda));
extern const f32 D_L11_001B15BC __attribute__((sda));
extern const f32 D_L11_001B15C0 __attribute__((sda));
extern const f32 D_L11_001B15C4 __attribute__((sda));
extern const f32 D_L11_001B15C8 __attribute__((sda));
extern const f32 D_L11_001B15CC __attribute__((sda));
extern const f32 D_L11_001B15D0 __attribute__((sda));
extern const f32 D_L11_001B15D4 __attribute__((sda));
extern u8 g_soundBankHandlesBlk[];
extern void *func_L11_002E91F8(s32);
extern void func_L11_002E9298(void *);
extern void func_L11_00487920(void *, f32, f32);
#define GUIDE (*(f32 **)(s + 0x170))
#define SET_GUIDE(x, y, z) (GUIDE[0] = (x), GUIDE[1] = (y), GUIDE[2] = (z))

/**
 * func_L11_00443C48 - keep the object's guide point (stages 8 and 9) at the
 * preset for its current target, or free it outside those stages.
 *
 * @obj: the object (moby); +0x20 its stage, +0x96 a frame count, +0x68 its
 *       state block (+0x170 the guide, +0x180/+0x184 the target).
 * Return: none.
 *
 * Non-obvious: The guide is allocated on first use (func_L11_002E91F8(5)).
 * With a target record, it is set to one of the level's preset triples by the
 * target's class (+0xAA): 0xD4A, then 0xD4B or 0xB3A. The 0xD4A test is not
 * chained to the other two (upstream's text; retail makes all three compares).
 * With no target in stage 9 it takes the D_L11_001B15A8 triple, and when the
 * hero (g_soundBankHandlesBlk + 0x88) is more than 16.0 above the anchor
 * func_L11_00443038(obj), the anchor is turned (func_L11_00487920) by 10
 * degrees, or by half a degree after 500 frames. Outside stages 8/9 the guide
 * is freed (func_L11_002E9298) and cleared.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_0043E1A0-g8.c (blob 53f934ac), function
 * func_L11_004423F8 (v1.01 address), author commit c7d9e30a (bigbeno37).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2578):
 *   - func_L11_004423F8 -> func_L11_00443C48 (our split name at the v2.00
 *     place);
 *   - D_00189E20 -> g_soundBankHandlesBlk (the boot's name, same address);
 *   - D_L11_001B15BC -> D_L11_001B159C (our label at that address);
 *   - D_L11_001B15C0 -> D_L11_001B15A0 (our label at that address);
 *   - D_L11_001B15C4 -> D_L11_001B15A4 (our label at that address);
 *   - D_L11_001B15C8 -> D_L11_001B15A8 (our label at that address);
 *   - D_L11_001B15CC -> D_L11_001B15AC (our label at that address);
 *   - D_L11_001B15D0 -> D_L11_001B15B0 (our label at that address);
 *   - D_L11_001B15D4 -> D_L11_001B15B4 (our label at that address);
 *   - D_L11_001B15D8 -> D_L11_001B15B8 (our label at that address);
 *   - D_L11_001B15DC -> D_L11_001B15BC (our label at that address);
 *   - D_L11_001B15E0 -> D_L11_001B15C0 (our label at that address);
 *   - D_L11_001B15E4 -> D_L11_001B15C4 (our label at that address);
 *   - D_L11_001B15E8 -> D_L11_001B15C8 (our label at that address);
 *   - D_L11_001B15EC -> D_L11_001B15CC (our label at that address);
 *   - D_L11_001B15F0 -> D_L11_001B15D0 (our label at that address);
 *   - D_L11_001B15F4 -> D_L11_001B15D4 (our label at that address);
 *   - func_L00_002D70A0 -> func_L11_002E91F8 (the level's own copy at its L11
 *     place);
 *   - func_L00_002D7140 -> func_L11_002E9298 (the level's own copy at its L11
 *     place);
 *   - func_L00_00424F58 -> func_L11_00487920 (the level's own copy at its L11
 *     place);
 *   - func_L11_004417E8 -> func_L11_00443038 (its v2.00 split name);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning), and the member closes with it as written;
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_00443C48(u8 *obj) {
    u8 *s = *(u8 **)(obj + 0x68);

    if (obj[0x20] == 8 || obj[0x20] == 9) {
        if (GUIDE == 0) {
            GUIDE = func_L11_002E91F8(5);
        } else if (*(void **)(s + 0x180) != 0) {
            if (*(u8 **)(s + 0x184) == 0) {
                SET_GUIDE(D_L11_001B159C, D_L11_001B15A0, D_L11_001B15A4);
                return;
            }
            if (*(s16 *)(*(u8 **)(s + 0x184) + 0xAA) == 0xD4A)
                SET_GUIDE(D_L11_001B15B4, D_L11_001B15B8, D_L11_001B15BC);
            if (*(s16 *)(*(u8 **)(s + 0x184) + 0xAA) == 0xD4B)
                SET_GUIDE(D_L11_001B15CC, D_L11_001B15D0, D_L11_001B15D4);
            else if (*(s16 *)(*(u8 **)(s + 0x184) + 0xAA) == 0xB3A)
                SET_GUIDE(D_L11_001B15C0, D_L11_001B15C4, D_L11_001B15C8);
        } else if (obj[0x20] == 9) {
            u8 *player = g_soundBankHandlesBlk;
            SET_GUIDE(D_L11_001B15A8, D_L11_001B15AC, D_L11_001B15B0);
            if (*(f32 *)(player + 0x88) > ((f32 *)func_L11_00443038(obj))[2] + 16.0f) {
                if (*(u16 *)(obj + 0x96) < 500)
                    func_L11_00487920(func_L11_00443038(obj), 0.17453295f, 0.0f);
                else
                    func_L11_00487920(func_L11_00443038(obj), 0.008726647f, 0.0f);
            }
        } else {
            SET_GUIDE(D_L11_001B159C, D_L11_001B15A0, D_L11_001B15A4);
        }
    } else {
        func_L11_002E9298(GUIDE);
        GUIDE = 0;
    }
}
