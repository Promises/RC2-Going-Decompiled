/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2556,
 * RULING #13291 row L11-D). Compiled ALONE as its own TU and linked at its one
 * place, 0x00471C58, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the SN 2.95.3 v1.36 cc1 with -fopt-stack
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;
extern void func_L11_00300198(void *a0, int a1, unsigned int a2, int a3, float f12, float f13,
                          float f14);

extern float func_L11_0032DE28(void);

extern int func_L11_00302400(float f12);

/**
 * func_L11_00471C58 - emit two coloured passes over the state's entry table.
 *
 * Builds two 0x31-packet colours: the base 0x106060 shaded by the state's
 * +0x18 value scaled to 63, and the base 0x808080 shaded by the same scaled
 * to 128 (the shade through func_L11_00302400, shifted into the alpha byte).
 * Then, for each of the +0x10-count 16-byte entries at state+0x20, emits two
 * func_L11_00300198 packets - sizes 0.2/0.08 and 0.08/0.08 - each with a
 * fresh random rotation (func_L11_0032DE28).
 *
 * @a0: the object; its state is at +0x68.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own
 * and no Lombyte mention), file
 * src/levels/11_joba/L11_004618C0.c, function func_L11_004701A0 (v1.01
 * address), author commit daf0e4a1 (Flavius). Adopted with
 * the authors’ permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device):
 *   - func_L11_004701A0 -> func_L11_00471C58 (our split name at the v2.00
 *     place);
 *   - func_00281EB8 -> func_L11_00300198;
 *   - func_00283CF0 -> func_L11_00302400;
 *   - func_002A7940 -> func_L11_0032DE28;
 *   - `#include "include_asm.h"` (none other) dropped: the
 *     u8/u16/u32/s16/s32/f32 typedefs common.h supplied are written here;
 *   - upstream’s comment above the function is folded into this one.
 *
 */
void func_L11_00471C58(char *a0) {
    char *s4 = *(char **)(a0 + 0x68);
    float w = 0.2f;
    float f20;
    unsigned int c1, c2;
    int i;
    float t;

    c1 = 0x106060;
    c2 = 0x808080;
    c1 |= func_L11_00302400(*(float *)(s4 + 0x18) * 63.0f) << 24;
    c2 |= func_L11_00302400(*(float *)(s4 + 0x18) * 128.0f) << 24;
    for (i = 0; i < *(int *)(s4 + 0x10); i++) {
        t = func_L11_0032DE28();
        func_L11_00300198(s4 + 0x20 + i * 0x10, 0x31, c1, 1, 0.2f, 0.08f, t);
        f20 = w * 0.4f;
        t = func_L11_0032DE28();
        func_L11_00300198(s4 + 0x20 + i * 0x10, 0x31, c2, 1, f20, 0.08f, t);
    }
}
