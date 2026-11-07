/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x00426EE0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G8 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef int s32;
extern s32 D_L23_001B1974 __attribute__((sda));
extern s32 D_L23_001B1978 __attribute__((sda));
extern float D_L23_001B197C __attribute__((sda));
extern float D_L23_001B1980 __attribute__((sda));
extern float D_L23_001B1984 __attribute__((sda));
extern s32 D_L23_001B1988 __attribute__((sda));
extern void func_L23_00426FF8(void *obj, s32 a, s32 b, s32 c, float x, float y, float z, s32 d, s32 e);

/**
 * func_L23_00426EE0 - draw a moby's record twice, plain and scaled.
 *
 * Draws the record at state +0x30 with func_L23_00426FF8 from the state's
 * bytes +0x28/+0x29, the position at state +0x10..+0x18 and the two words at
 * state +0 and +4. When state byte +0x2A is 0 that is all. Otherwise the
 * first draw uses D_L23_001B1988 for both words when byte +0x2B is set, and a
 * second draw follows with the bytes and the position scaled by
 * D_L23_001B1974..D_L23_001B1984.
 *
 * @a0: the moby; its +0x68 points at the state.
 * Return: nothing.
 *
 * Non-obvious: the six scale words are separate 4-byte objects, $gp-relative
 * by size at -G8. Upstream's `__attribute__((sda))` text on the extern
 * declarations is kept as written; this cc1 ignores the attribute.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/23_damosel_orbit/L23_0041E418-g8.c, function func_L23_004259E0
 * (v1.01 address), author commit 00641a1b (bigmak94). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_L23_004259E0 -> func_L23_00426EE0; D_L23_001B19C4 ->
 *     D_L23_001B1974; D_L23_001B19C8 -> D_L23_001B1978; D_L23_001B19CC ->
 *     D_L23_001B197C; D_L23_001B19D0 -> D_L23_001B1980; D_L23_001B19D4 ->
 *     D_L23_001B1984; D_L23_001B19D8 -> D_L23_001B1988; func_L23_00425AF8 ->
 *     func_L23_00426FF8
 *   - `#include "include_asm.h"` dropped; the typedefs it uses written in the
 *     TU (the file's own where it has them, else common.h's); only the
 *     file-level declarations it uses kept, without their comments
 *   - upstream's one-line comment folded into this one
 */
void func_L23_00426EE0(u8 *a0) {
    u8 *obj = *(u8 **)(a0 + 0x68);

    if (obj[0x2A] == 0) {
        func_L23_00426FF8(obj + 0x30, obj[0x28], obj[0x29], 5,
                          *(float *)(obj + 0x10), *(float *)(obj + 0x14), *(float *)(obj + 0x18),
                          *(s32 *)obj, *(s32 *)(obj + 4));
        return;
    }
    if (obj[0x2B] != 0) {
        func_L23_00426FF8(obj + 0x30, obj[0x28], obj[0x29], 5,
                          *(float *)(obj + 0x10), *(float *)(obj + 0x14), *(float *)(obj + 0x18),
                          D_L23_001B1988, D_L23_001B1988);
    } else {
        func_L23_00426FF8(obj + 0x30, obj[0x28], obj[0x29], 5,
                          *(float *)(obj + 0x10), *(float *)(obj + 0x14), *(float *)(obj + 0x18),
                          *(s32 *)obj, *(s32 *)(obj + 4));
    }
    func_L23_00426FF8(obj + 0x30, obj[0x28] * D_L23_001B1974, obj[0x29] * D_L23_001B1978, 5,
                      *(float *)(obj + 0x10) * D_L23_001B197C, *(float *)(obj + 0x14) * D_L23_001B1980,
                      *(float *)(obj + 0x18) * D_L23_001B1984, *(s32 *)obj, *(s32 *)(obj + 4));
}
