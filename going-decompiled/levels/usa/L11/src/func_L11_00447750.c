/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x00447750, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned char u8;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;

/* A view of the boot's voice-call slot (g_fileLoadVoiceState, 0x001A63A8). */
typedef struct {
    u8 pad0[0x24];
    s32 field24;
    s32 field28;
    u8 pad2[0x3C];
    s32 field68;
} JobaCallSlotView;

extern u8 g_fileLoadVoiceState[];

/**
 * func_L11_00447750 - claim the shared voice-call slot for one of the
 * object's lines.
 *
 * @idx (0..0x26) selects the line's id: state word 0x84 + idx for idx below
 * 0x14, else state word 0x98 + (idx - 0x14). When the slot is idle (its +0x68
 * word 0 and its +0x24 word -1) the slot's +0x24 becomes (id & 0x3FFF) + 0x7D0,
 * its +0x28 is cleared and the line's played flag (state halfword 0x2A0 +
 * idx * 2) is set to 1.
 *
 * @obj: the object; its state is at +0x68.
 * @idx: the line index.
 * Return: 1 when the slot was claimed; 0 when @idx is out of range or the
 * slot is busy or waiting.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_0043E1A0.c, function func_L11_00445D28 (v1.01
 * address), author commit 4541c4ce (ReaperOfVeriod). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2555 relverify):
 *   - func_L11_00445D28 -> func_L11_00447750 (our split name at the v2.00 place);
 *   - D_001A63A8 -> g_fileLoadVoiceState (the boot's name, same address);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the
 *     u8/u32/s16/s32 typedefs common.h supplied are written here; upstream's
 *     comments above the function are folded into this one.
 */
s32 func_L11_00447750(u8 *obj, s32 idx) {
    u8 *state = *(u8 **)(obj + 0x68);
    s32 v;

    if ((u32)idx >= 0x27) {
        return 0;
    }
    if (idx < 0x14) {
        s32 *p = (s32 *)((u8 *)state + idx * 4);
        v = p[0x84];
    } else {
        s32 *p = (s32 *)((u8 *)state + (idx - 0x14) * 4);
        v = p[0x98];
    }
    {
        JobaCallSlotView *slot = (JobaCallSlotView *)g_fileLoadVoiceState;

        if (slot->field68 != 0) {
            goto fail;
        }
        if (slot->field24 != -1) {
            goto fail;
        }
        slot->field24 = (v & 0x3FFF) + 0x7D0;
        slot->field28 = 0;
        {
            s16 *p = (s16 *)((u8 *)state + idx * 2 + 0x2A0);
            *p = 1;
        }
    }
    return 1;
fail:
    return 0;
}
