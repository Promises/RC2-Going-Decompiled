/* Level 11 (Joba), USA v2.00: a level-only function built from C through the
 * boot's assembler model (task #2578, RULING #13291 row L11-ASM). Compiled
 * ALONE as its own TU and linked at its one place, 0x0043AE08, in place of its
 * split asm; the row is going-decompiled/levels/usa/L11/config/L11_c.tsv, the
 * arm SN 2.95.3 v1.36 cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8
 * -fopt-stack, assembled by tools/ee/asm_unit.sh (the row's `asm_unit`
 * column). Measured with plain GNU as instead (task #2578, same TU): 24 of 27
 * .text words differ from the disc, relocated fields masked, and .text is 0x6C
 * bytes, not the place's 0x74, and it emits a .lit4 pool of 4 bytes, which
 * level_elf.py cobj refuses (only .text is allowed).
 */

typedef unsigned char u8;
typedef float f32;

extern float D_L11_001B12A8 __attribute__((sda));

/**
 * func_L11_0043AE08 - count down the ten rotation timers kept in the object's
 * state.
 *
 * @obj: the object (moby); +0x68 its state block, with the armed bytes at +0x380
 *       and the timers at +0x2F0.
 * Return: none.
 *
 * Non-obvious: Each armed timer (byte == 1) loses D_L11_001B12A8 / 60. One
 * that goes below zero is set to 0.0 and its byte to 2 (expired).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention, `grep -ic lombyte` =
 * 0), file src/levels/11_joba/L11_00432628-g8.c (blob debf25a2), function
 * func_L11_004395C8 (v1.01 address), author commit 4541c4ce (ReaperOfVeriod).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). VERBATIM but for names (no device). Respellings, each to the target
 * of the v2.00 disc's own relocation at this place (relverify by name on this
 * object, task #2578):
 *   - func_L11_004395C8 -> func_L11_0043AE08 (our split name at the v2.00
 *     place);
 *   - D_L11_001B12C8 -> D_L11_001B12A8 (our label at that address);
 *   - `__attribute__((sda))` kept as upstream's text: this cc1 ignores it (a
 *     warning), and the member closes with it as written;
 *   - `#include "common.h"` dropped: the typedefs this body uses are written
 *     here; upstream file-scope declarations it does not use are not carried,
 *     the rest keep upstream's order and text.
 */
void func_L11_0043AE08(char *obj)
{
    char *state = *(char **)(obj + 0x68);
    int i;

    for (i = 0; i < 10; i++) {
        u8 *armed = (u8 *)(state + 0x380 + i);
        f32 *timer = (f32 *)(state + 0x2F0 + 4 * i);
        if (*armed == 1) {
            f32 left = *timer - D_L11_001B12A8 * 0.016666667f;
            *timer = left;
            if (left < 0.0f) {
                *timer = 0.0f;
                *armed = 2;
            }
        }
    }
}
