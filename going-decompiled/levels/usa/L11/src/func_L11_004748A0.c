/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2556,
 * RULING #13291 row L11-D). Compiled ALONE as its own TU and linked at its one
 * place, 0x004748A0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the boot’s ee-gcc 2.9-ee-991111 cc1
 * (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

/**
 * func_L11_004748A0 - first-touch latch: mark the object started.
 *
 * When the state byte at +0x20 is still 0 it becomes 1 and the +0x34 halfword
 * gains flags 0x41; nothing happens on later calls.
 *
 * @a0: the object.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own
 * and no Lombyte mention), file
 * src/levels/11_joba/L11_004618C0.c, function func_L11_00472DE8 (v1.01
 * address), author commit ee3fee45 (llesieur99). Adopted with
 * the authors’ permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device):
 *   - func_L11_00472DE8 -> func_L11_004748A0 (our split name at the v2.00
 *     place);
 *   - `#include "include_asm.h"` (none other) dropped: the
 *     u8/u16/u32/s16/s32/f32 typedefs common.h supplied are written here;
 *   - no other symbol is referenced (0 relocations).
 *
 */
void func_L11_004748A0(unsigned char *a0) {
    unsigned short v;

    if (a0[0x20] != 0)
        return;
    v = *(unsigned short *)(a0 + 0x34);
    a0[0x20] = 1;
    *(unsigned short *)(a0 + 0x34) = v | 0x41;
}
