/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2556,
 * RULING #13291 row L11-D). Compiled ALONE as its own TU and linked at its one
 * place, 0x0048A068, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the boot’s ee-gcc 2.9-ee-991111 cc1
 * (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;
extern char *D_L11_001BA990;

/**
 * func_L11_0048A068 - flag the object a blob-shadow record points at.
 *
 * Sets byte +0x88 of whatever the pointer inside the blob-shadow record at
 * D_L11_001BA990 (g_blobShadowQueue+0x3D0) references. Upstream carries this
 * as a vendor callback from rac1-decomp's func_L08_00315068.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own
 * and no Lombyte mention), file
 * src/levels/11_joba/L11_004618C0.c, function func_L11_004885A0 (v1.01
 * address), author commit 76651091 (Flavius). Adopted with
 * the authors’ permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device):
 *   - func_L11_004885A0 -> func_L11_0048A068 (our split name at the v2.00
 *     place);
 *   - r1_D_L08_00167680 -> D_L11_001BA990;
 *   - `#include "include_asm.h"` (none other) dropped: the
 *     u8/u16/u32/s16/s32/f32 typedefs common.h supplied are written here;
 *   - the datum target D_L11_001BA990 lies inside the boot’s g_blobShadowQueue
 *     table (a 32 x 0x20 record array at 0x001BA5C0; the reloc is base+off),
 *     so it keeps the address-named extern spelling of RULING #13291 section
 *     2: the level linker values the name from its address.
 *
 */
void func_L11_0048A068(void) {
    D_L11_001BA990[0x88] = 1;
}
