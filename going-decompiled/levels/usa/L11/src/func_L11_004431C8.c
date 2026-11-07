/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x004431C8, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the boot's
 * ee-gcc 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

typedef unsigned char u8;
typedef signed int s32;

/* The state's six per-class counters at +0xE0. */
typedef struct {
    u8 prefix[0xe0];
    s32 fielde0, fielde4, fielde8, fieldec, fieldf0, fieldf4;
} JobaClassCounterView;

/**
 * func_L11_004431C8 - add one to the state's counter for a class.
 *
 * Classes 0xD98, 0xDCF, 0xDF4, 0xDF7, 0xDF9 and 0xD37 own the counters at
 * state+0xE0..+0xF4 in that order; any other class is ignored. The inverse
 * of func_L11_004430B8.
 *
 * @object: the object; its state is at +0x68.
 * @class_id: the class whose counter to increment.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_0043E1A0.c, function func_L11_00441978 (v1.01
 * address), author commit 7fccf735 (Flavius; upstream's root
 * commit, which squashed the earlier RAC2 history, so the text's first
 * author is not recoverable from upstream's log). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device):
 *   - func_L11_00441978 -> func_L11_004431C8 (our split name at the v2.00 place);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the u8/s32
 *     typedefs common.h supplied are written here; upstream's two one-line
 *     comments above the function are folded into this one.
 */
void func_L11_004431C8(u8 *object, s32 class_id) {
    JobaClassCounterView *state = *(JobaClassCounterView **)(object + 0x68);
    switch (class_id) {
        case 0xd98: state->fielde0++; break;
        case 0xdcf: state->fielde4++; break;
        case 0xdf4: state->fielde8++; break;
        case 0xdf7: state->fieldec++; break;
        case 0xdf9: state->fieldf0++; break;
        case 0xd37: state->fieldf4++; break;
    }
}
