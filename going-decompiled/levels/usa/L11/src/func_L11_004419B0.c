/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x004419B0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack.
 */

typedef unsigned char u8;
typedef unsigned short u16;

/* The level function at 0x00448020, passed by address as a callback. */
extern u8 func_L11_00448020[];
/* The level's copies of boot helpers, named by their v2.00 places in L11. */
extern void func_L11_00300E50(void *);
extern void func_L11_002F9490(void *, void *);
extern void func_L11_003204A8(void);

/**
 * func_L11_004419B0 - bump the object's hit counter, start the state's timer
 * and report the object.
 *
 * Increments the 16-bit counter at +0x96 unless it is already 0xFFFF, starts
 * the timer block at state+0x190 (func_L11_00300E50), hands the object to
 * func_L11_002F9490 with the level function func_L11_00448020 as the
 * callback, then calls func_L11_003204A8.
 *
 * @obj: the object; its state is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_0043E1A0.c, function func_L11_00440160 (v1.01
 * address), author commit 264cb2be (llesieur99). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2555 relverify):
 *   - func_L11_00440160 -> func_L11_004419B0 (our split name at the v2.00 place);
 *   - D_L11_004465F8 -> func_L11_00448020 (the v2.00 target is our split's
 *     function label there; the declaration's type `u8 []` is kept);
 *   - func_300150 -> func_L11_00300E50, func_2F8AD0 -> func_L11_002F9490,
 *     func_31F338 -> func_L11_003204A8;
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the u8/u16
 *     typedefs common.h supplied are written here.
 */
void func_L11_004419B0(u8 *obj) {
    u8 *state = *(u8 **)(obj + 0x68);
    u16 *counter = (u16 *)(obj + 0x96);
    if (*counter <= 0xFFFE) {
        *counter = *counter + 1;
    }
    func_L11_00300E50(state + 0x190);
    func_L11_002F9490(func_L11_00448020, obj);
    func_L11_003204A8();
}
