/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x004428E0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned char u8;
typedef signed int s32;

/* The level's copies of the group helpers at these v2.00 places. */
extern s32 func_L11_004189A8(s32);

extern void func_L11_00418A58(s32);

/**
 * func_L11_004428E0 - renew every live group in the state's slot table.
 *
 * Walks the slots at state+0x120, index 0 through the word at state+0x18C,
 * and calls func_L11_00418A58 on each non-null slot for which
 * func_L11_004189A8 returns non-zero.
 *
 * @obj: the owner; its state is at +0x68.
 *
 * Non-obvious: the slot count is re-read from the state on every step.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_0043E1A0.c, function func_L11_00441090 (v1.01
 * address), author commit ee3fee45 (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2555 relverify):
 *   - func_L11_00441090 -> func_L11_004428E0 (our split name at the v2.00 place);
 *   - func_417238 -> func_L11_004189A8, func_4172E8 -> func_L11_00418A58;
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the u8/s32
 *     typedefs common.h supplied are written here.
 */
void func_L11_004428E0(u8 *obj) {
    u8 *state = *(u8 **)(obj + 0x68);
    s32 i;
    for (i = 0; i <= *(s32 *)(state + 0x18C); i++) {
        s32 *slot = (s32 *)(state + 0x120 + i * 4);
        if (slot[0] == 0) {
            continue;
        }
        if (func_L11_004189A8(slot[0]) == 0) {
            continue;
        }
        func_L11_00418A58(slot[0]);
    }
}
