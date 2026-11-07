/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2556,
 * RULING #13291 row L11-D). Compiled ALONE as its own TU and linked at its one
 * place, 0x0044A708, in place of its split asm; the row is
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
 * func_L11_0044A708 - is this a kind-4/0xE/0xF object currently holding its
 * sub-moby?
 *
 * Returns 1 when the object's state (+0x68 -> +0x354) owns a live sub-moby
 * AND the object's type byte (+0x20) is 0xF, 4 or 0xE, or its counter byte
 * (+0x95) is 4; otherwise 0.
 *
 * @a0: the object.
 * Return: 1 in the case above, else 0.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own
 * and no Lombyte mention), file
 * src/levels/11_joba/L11_004465F8.c, function func_L11_00448CE0 (v1.01
 * address), author commit fa8d6672 (llesieur99). Adopted with
 * the authors’ permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device):
 *   - func_L11_00448CE0 -> func_L11_0044A708 (our split name at the v2.00
 *     place);
 *   - `#include "common.h"` and `#include "include_asm.h"` dropped: the
 *     u8/u16/u32/s16/s32/f32 typedefs common.h supplied are written here;
 *   - the opening brace moved to the signature line (whitespace only);
 *   - no other symbol is referenced (0 relocations).
 *
 */
s32 func_L11_0044A708(char *a0) {
    s32 r = 0;
    u8 x;

    if (*(s32 *)(*(char **)(a0 + 0x68) + 0x354) != 0) {
        x = *(u8 *)(a0 + 0x20);
        if (x == 0xF || x == 4 || *(u8 *)(a0 + 0x95) == 4 || x == 0xE)
            r = 1;
    }
    return r;
}
