/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x003983E0, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 58 of 71
 * .text words differ from the disc, relocated fields masked, and .text is
 * 0x11C bytes, not the place's 0x130, and it emits a .lit4 pool of 8 bytes,
 * which level_elf.py cobj refuses (only .text is allowed).
 */

typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

extern void func_L11_003011E0(f32 *a0, f32 *a1, f32 *a2);
extern f32 func_L11_003022A8(f32 a0, f32 a1);
extern f32 func_L11_00301150(f32 a0);
extern s32 func_L11_00300E50(void *a0);
extern s32 func_L11_00300EB0(void *a0);
extern void func_L11_00323A00(void *a0);
extern f32 D_L11_001AEBF8 __attribute__((sda));

/**
 * func_L11_003983E0 - per-frame update of a drifting debris piece: move it,
 * slow it, spin it, and free it when it expires.
 *
 * @arg0: the piece (moby); +0x10 position, +0xF0/+0xF8 its two angles, +0x23 and
 *         +0x31 its visibility bytes, +0x68 its state block.
 * Return: none.
 *
 * Non-obvious: The state's velocity (+0x00) is added to the position
 * (func_L11_003011E0, the level's copy of the boot's vector add), then its x/y
 * halves are scaled by the level factor D_L11_001AEBF8. The timer at +0x08
 * drops by 1/240 a frame and is clamped at the floor in +0x20. +0xF8 turns by
 * +0x28, and +0xF0 by +0x2C times the direction in +0x24 (func_L11_003022A8
 * adds and wraps an angle); the direction flips once func_L11_00301150(+0xF0)
 * exceeds pi/4. The piece is freed (func_L11_00323A00) when the countdown at
 * +0x1C expires (func_L11_00300E50), when it is no longer visible (+0x31 ==
 * 0), or, in its last 30 frames, when func_L11_00300EB0 on +0x23 says so.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_002DE9E0-g8.c (blob aa355d83), function
 * func_L11_00397068 (v1.01 address), author commit daf0e4a1 (Flavius). Adopted
 * with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device). Respellings, each to the target of the
 * v2.00 disc's own relocation at this place (relverify by name on this object,
 * task #2578):
 *   - func_L11_00397068 -> func_L11_003983E0 (our split name at the v2.00
 *     place);
 *   - D_L11_001AEC40 -> D_L11_001AEBF8 (our label at that address);
 *   - func_00282948 -> func_L11_00300E50 (the level's own copy at its L11
 *     place);
 *   - func_002829A8 -> func_L11_00300EB0 (the level's own copy at its L11
 *     place);
 *   - func_00282C48 -> func_L11_00301150 (the level's own copy at its L11
 *     place);
 *   - func_00282CC0 -> func_L11_003011E0 (the level's own copy at its L11
 *     place);
 *   - func_00283B98 -> func_L11_003022A8 (the level's own copy at its L11
 *     place);
 *   - func_0029F338 -> func_L11_00323A00 (the level's own copy at its L11
 *     place);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning), and the member closes with it as written;
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_003983E0(u8 *arg0) {
    u8 *s1 = arg0;
    u8 *s0 = *(u8 **)(s1 + 0x68);
    f32 f0, f1, f2, f4;

    func_L11_003011E0((f32 *)(s1 + 0x10), (f32 *)(s1 + 0x10), (f32 *)s0);
    f2 = *(f32 *)(s0 + 0x8) - 0.0041666667f;
    f0 = *(f32 *)(s0 + 0x0) * D_L11_001AEBF8;
    f1 = *(f32 *)(s0 + 0x4) * D_L11_001AEBF8;
    f4 = *(f32 *)(s0 + 0x20);
    *(f32 *)(s0 + 0x8) = f2;
    *(f32 *)(s0 + 0x0) = f0;
    *(f32 *)(s0 + 0x4) = f1;
    if (f2 < f4)
        *(f32 *)(s0 + 0x8) = f4;
    *(f32 *)(s1 + 0xF8) = func_L11_003022A8(*(f32 *)(s1 + 0xF8), *(f32 *)(s0 + 0x28));
    *(f32 *)(s1 + 0xF0) = func_L11_003022A8(*(f32 *)(s1 + 0xF0), *(f32 *)(s0 + 0x2C) * (f32)*(s32 *)(s0 + 0x24));
    f0 = func_L11_00301150(*(f32 *)(s1 + 0xF0));
    if (f0 > 0.78539824f)
        *(s32 *)(s0 + 0x24) = -*(s32 *)(s0 + 0x24);
    if (func_L11_00300E50(s0 + 0x1C) != 0) {
        func_L11_00323A00(s1);
    } else if (s1[0x31] == 0) {
        func_L11_00323A00(s1);
    } else if (*(s32 *)(s0 + 0x1C) < 0x1E) {
        if (func_L11_00300EB0(s1 + 0x23) != 0)
            func_L11_00323A00(s1);
    }
}
