/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2556,
 * RULING #13291 row L11-D). Compiled ALONE as its own TU and linked at its one
 * place, 0x0048A080, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the SN 2.95.3 v1.36 cc1 with -fopt-stack
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;
extern void func_L11_003011A8(void *);
extern void func_L11_00301A98(void *, void *);
extern void func_L11_00301D00(void *, void *);

/**
 * func_L11_0048A080 - push colour ramps into the two sub-views and refresh.
 *
 * The +0xC0 word of the +0x70 view becomes 4; the +0x30 sub-view takes the
 * ramp (0.05, 0.3, 1.0, 1) and the +0x10 sub-view (0.3, 0.3, 1.0, 1), both
 * applied by func_L11_003011A8. A local 0x40-byte buffer is built from the
 * object's +0x40 block (func_L11_00301A98) and run back over the object
 * (func_L11_00301D00); the +0x7E halfword is cleared.
 *
 * @a0: the object.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own
 * and no Lombyte mention), file
 * src/levels/11_joba/L11_004618C0.c, function func_L11_004885B8 (v1.01
 * address), author commit ee3fee45 (llesieur99). Adopted with
 * the authors’ permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device):
 *   - func_L11_004885B8 -> func_L11_0048A080 (our split name at the v2.00
 *     place);
 *   - func_803004A8 -> func_L11_003011A8;
 *   - func_80300D98 -> func_L11_00301A98;
 *   - func_80301000 -> func_L11_00301D00;
 *   - `#include "include_asm.h"` (none other) dropped: the
 *     u8/u16/u32/s16/s32/f32 typedefs common.h supplied are written here;
 *   - the inner block’s `unsigned char *q` declaration is upstream’s own.
 *
 */
void func_L11_0048A080(unsigned char *a0) {
    unsigned char buf[0x40];
    unsigned char *p;

    *(int *)(*(unsigned char **)(a0 + 0x70) + 0xC0) = 4;
    p = *(unsigned char **)(a0 + 0x70) + 0x30;
    *(float *)(p + 0x20) = 0.05f;
    *(float *)(p + 0x24) = 0.3f;
    *(float *)(p + 0x28) = 1.0f;
    *(unsigned short *)(p + 0x2C) = 1;
    func_L11_003011A8(p);
    {
        unsigned char *q = *(unsigned char **)(a0 + 0x70);

        *(float *)(q + 0x10) = 0.3f;
        *(float *)(q + 0x14) = 0.3f;
        *(float *)(q + 0x18) = 1.0f;
        *(unsigned short *)(q + 0x1C) = 1;
        func_L11_003011A8(q);
    }
    func_L11_00301A98(buf, a0 + 0x40);
    func_L11_00301D00(a0, buf);
    *(unsigned short *)(a0 + 0x7E) = 0;
}
