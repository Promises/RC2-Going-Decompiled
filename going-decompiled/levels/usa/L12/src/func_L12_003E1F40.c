/* Level 12 (Todano), USA v2.00: a level-only function built from C (task #2670,
 * row L12-A, RULING #13291). Compiled ALONE as its own TU and linked at its one
 * place, 0x003E1F40, in place of its split asm; the row is
 * going-decompiled/levels/usa/L12/config/L12_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

/* The level's copies of boot code, named by their v2.00 places. Upstream
   declares the first without a prototype; kept. */
extern s32 func_L12_00316220();
extern void func_L12_002F1658(void *buf, s32 arg1, u32 arg2, s32 arg3, f32 x, f32 y, f32 z);

/**
 * func_L12_003E1F40 - register an object's two-slot primitive.
 *
 * Fills a 16-byte buffer from the object with func_L12_00316220(object, 2,
 * buffer), then registers two entries with func_L12_002F1658: id 0x2E with
 * colour 0x8080FFFF and id 0xC with colour 0x8020F020, each with flag 1 and
 * the floats 0.25, 0, 0.
 *
 * @object: the object.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own; `grep -ic lombyte` on it = 0, so not
 * Lombyte-origin), file src/levels/12_todano/L12_00398E50.c, function
 * func_L12_003E0B68 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082,
 * #13291). BYTE-IDENTICAL: VERBATIM but for names; the respellings, each to
 * the target of the v2.00 disc's own relocation at this place (task #2670
 * relverify):
 *   - func_L12_003E0B68 -> func_L12_003E1F40 (our split name at the v2.00 place);
 *   - func_315148 -> func_L12_00316220, func_2F0A60 -> func_L12_002F1658
 *     (our split names at the v2.00 targets);
 *   - `#include "common.h"` dropped: the typedefs used are written here and
 *     only this function's declarations kept; upstream's one-line comment
 *     folded into this one.
 */
void func_L12_003E1F40(u8 *object) {
    s32 buffer[4];

    func_L12_00316220(object, 2, buffer);
    func_L12_002F1658(buffer, 0x2E, 0x8080FFFF, 1, 0.25f, 0.0f, 0.0f);
    func_L12_002F1658(buffer, 0xC, 0x8020F020, 1, 0.25f, 0.0f, 0.0f);
}
