/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x00442FF8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the boot's
 * ee-gcc 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

typedef unsigned char u8;
typedef signed int s32;

/* A view of the level's one-shot flag block. 0x001BA800 lies inside the split
   data label D_L11_001B39B0, so it is an address-named extern (RULING #13291
   §2): the level ELF values it from its name. */
typedef struct { u8 pad[0x284]; s32 field284; s32 field288; } JobaFlagBlockView;

extern JobaFlagBlockView D_L11_001BA800;

/**
 * func_L11_00442FF8 - hand the level's one-shot flag to the object's new
 * value when the flag is idle.
 *
 * If the flag block's owner word (+0x284) still holds the object's previous
 * value (state+0x184) it is released to 0. Then, if @value is non-zero and the
 * block is idle, @value takes it and its second word (+0x288) is cleared.
 * Finally state+0x184 = @value.
 *
 * @object: the object; its state is at +0x68.
 * @value: the new flag value (0 releases).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_0043E1A0.c, function func_L11_004417A8 (v1.01
 * address), author commit 264cb2be (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2555 relverify):
 *   - func_L11_004417A8 -> func_L11_00442FF8 (our split name at the v2.00 place);
 *   - D_L11_001BA780 -> D_L11_001BA800 (address-named, inside D_L11_001B39B0);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the u8/s32
 *     typedefs common.h supplied are written here.
 */
void func_L11_00442FF8(u8 *object, s32 value) {
    u8 *state = *(u8 **)(object + 0x68);
    if (D_L11_001BA800.field284 == *(s32 *)(state + 0x184)) {
        D_L11_001BA800.field284 = 0;
    }
    if (value != 0 && D_L11_001BA800.field284 == 0) {
        D_L11_001BA800.field284 = value;
        D_L11_001BA800.field288 = 0;
    }
    *(s32 *)(state + 0x184) = value;
}
