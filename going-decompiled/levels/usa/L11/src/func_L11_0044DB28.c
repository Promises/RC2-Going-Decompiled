/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2556,
 * RULING #13291 row L11-D). Compiled ALONE as its own TU and linked at its one
 * place, 0x0044DB28, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the SN 2.95.3 v1.36 cc1 with -fopt-stack
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;
extern void func_L11_003395B0(char *a0, char *a1, float a2, int a3, int a4);
extern void func_L11_0032F698(char *a0, char *a1, float f12, float f13, float f14,
                        int a2, int a3, int a4, int a5);

/**
 * func_L11_0044DB28 - age the object one frame and emit its draw packets.
 *
 * Increments the object's frame age (+0x96, saturating at 0xFFFF), then emits
 * the billboard pass from the state's +0x4E0 table (func_L11_003395B0) and
 * the LOD packet around the object's own position (func_L11_0032F698, scale
 * 1.3, radius 1.0).
 *
 * @a0: the object; its state is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own
 * and no Lombyte mention), file
 * src/levels/11_joba/L11_004465F8.c, function func_L11_0044C0F0 (v1.01
 * address), author commit fa8d6672 (llesieur99). Adopted with
 * the authors’ permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device):
 *   - func_L11_0044C0F0 -> func_L11_0044DB28 (our split name at the v2.00
 *     place);
 *   - func_32E480 -> func_L11_0032F698;
 *   - func_338398 -> func_L11_003395B0;
 *   - `#include "common.h"` and `#include "include_asm.h"` dropped: the
 *     u8/u16/u32/s16/s32/f32 typedefs common.h supplied are written here;
 *   - the opening brace moved to the signature line (whitespace only).
 *
 */
void func_L11_0044DB28(char *a0) {
    char *p = *(char **)(a0 + 0x68);

    if (*(u16 *)(a0 + 0x96) < 0xFFFF)
        *(u16 *)(a0 + 0x96) = *(u16 *)(a0 + 0x96) + 1;
    func_L11_003395B0(a0, p + 0x4E0, 4.0f, 7, 1);
    func_L11_0032F698(a0, a0 + 0x10, 1.3f, 1.0f, 1.0f, 0x4000000, 0, 1, 0);
}
