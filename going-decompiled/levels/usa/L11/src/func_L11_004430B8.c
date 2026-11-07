/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x004430B8, in place of its split asm; the row is
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
 * func_L11_004430B8 - take one from the state's counter for a class.
 *
 * Classes 0xD98, 0xDCF, 0xDF4, 0xDF7, 0xDF9 and 0xD37 own the counters at
 * state+0xE0..+0xF4 in that order. A positive counter is decremented.
 *
 * @object: the object; its state is at +0x68.
 * @class_id: the class whose counter to take from.
 * Return: 1 when a counter was decremented; 0 when it was not positive or the
 * class has no counter.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_0043E1A0.c, function func_L11_00441868 (v1.01
 * address), author commit 7fccf735 (Flavius). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device):
 *   - func_L11_00441868 -> func_L11_004430B8 (our split name at the v2.00 place);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the u8/s32
 *     typedefs common.h supplied are written here.
 */
s32 func_L11_004430B8(u8 *object, s32 class_id) {
    JobaClassCounterView *state = *(JobaClassCounterView **)(object + 0x68);
    switch (class_id) {
        case 0xd98:
            if (state->fielde0 > 0) { state->fielde0--; return 1; }
            break;
        case 0xdcf:
            if (state->fielde4 > 0) { state->fielde4--; return 1; }
            break;
        case 0xdf4:
            if (state->fielde8 > 0) { state->fielde8--; return 1; }
            break;
        case 0xdf7:
            if (state->fieldec > 0) { state->fieldec--; return 1; }
            break;
        case 0xdf9:
            if (state->fieldf0 > 0) { state->fieldf0--; return 1; }
            break;
        case 0xd37:
            if (state->fieldf4 > 0) { state->fieldf4--; return 1; }
            break;
    }
    return 0;
}
