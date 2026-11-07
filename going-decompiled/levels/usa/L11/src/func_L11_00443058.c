/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2555,
 * RULING #13291 row L11-C). Compiled ALONE as its own TU and linked at its one
 * place, 0x00443058, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the boot's
 * ee-gcc 2.9-ee-991111 cc1 (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

/* A polygon of the level's table: its vertex count, then the vertices. */
typedef struct { s32 field00; u8 remainder[12]; } JobaPolygonHeaderView;

/* The level's polygon table. 0x001FF600 lies inside the split data label
   D_L11_001F0000, so it is an address-named extern (RULING #13291 §2): the
   level ELF values it from its name. */
extern JobaPolygonHeaderView *D_L11_001FF600[];

/* The level's copy of the point-in-polygon (crossing parity) helper. */
extern s32 func_L11_0032F328(f32 *point, f32 *vertices, s32 count);

/**
 * func_L11_00443058 - is a point inside the object's polygon?
 *
 * The state's polygon index (+0x164) selects a polygon of D_L11_001FF600;
 * its vertices follow the 16-byte header and its count is the header's first
 * word. The test is func_L11_0032F328.
 *
 * @object: the object; its state is at +0x68.
 * @point: the point to test.
 * Return: the helper's result, or 0 when the index is -1.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/11_joba/L11_0043E1A0.c, function func_L11_00441808 (v1.01
 * address), author commit 7fccf735 (Flavius; upstream's root
 * commit, which squashed the earlier RAC2 history, so the text's first
 * author is not recoverable from upstream's log). Adopted with the authors'
 * permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but for names
 * (no device); the respellings, each to the target of the v2.00 disc's own
 * relocation at this place (task #2555 relverify):
 *   - func_L11_00441808 -> func_L11_00443058 (our split name at the v2.00 place);
 *   - D_L11_001FF580 -> D_L11_001FF600 (address-named, inside D_L11_001F0000);
 *   - func_002A8AF0 -> func_L11_0032F328 (the level's copy at that place);
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped: the
 *     u8/s32/f32 typedefs common.h supplied are written here.
 */
s32 func_L11_00443058(u8 *object, f32 *point) {
    u8 *state = *(u8 **)(object + 0x68);
    s32 index = *(s32 *)(state + 0x164);
    JobaPolygonHeaderView *header;
    s32 result;
    if (index == -1) {
        result = 0;
    } else {
        header = D_L11_001FF600[index];
        result = func_L11_0032F328(point, (f32 *)(header + 1), header->field00);
    }
    return result;
}
