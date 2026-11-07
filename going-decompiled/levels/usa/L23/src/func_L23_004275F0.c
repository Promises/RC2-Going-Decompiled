/* Level 23 (Damosel orbit), USA v2.00: a level-only function built from C
 * (task #2568, row L23-B, RULING #13291). Compiled ALONE as its own TU and
 * linked at its one place, 0x004275F0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L23/config/L23_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, assembled
 * by plain GNU as (the row's `as` column).
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;

extern void func_L23_002F91A8(float *, float *, float);
extern void func_L23_002F8FF8(float *, float *, float *);
typedef int TI16 __attribute__((mode(TI)));
extern void func_L23_002F8EF8(float *, float *, float *);
extern int func_L23_00325658(int, int);
extern float func_L23_002F9FA0(float);
extern float func_L23_002F94D0(float);
extern void func_L23_002F8F38(float *, float *, float);
extern void func_L23_002F8EC8(float *, float *, float *);
extern float D_L23_001BA5C0[];

/**
 * func_L23_004275F0 - build a randomly turned point between two vectors.
 *
 * Copies the two 128-bit source vectors, builds the vectors between them and
 * from the first to D_L23_001BA5C0 (func_L23_002F8EF8), combines them
 * (func_L23_002F8FF8) and normalises the result to length 1
 * (func_L23_002F91A8). An angle in degrees, ((+0x96 << state +0xE) +
 * func_L23_00325658(-x, x)) % 360 - 180 with x the state halfword +0xC, is
 * converted to radians and passed through func_L23_002F9FA0 and
 * func_L23_002F94D0; the result times @scale scales the direction
 * (func_L23_002F8F38), which is added to the first source into @dst
 * (func_L23_002F8EC8).
 *
 * @obj: the moby (+0x68 the state, +0x96 the frame counter). @dst: the output
 * vector. @src1, @src2: the two 128-bit source vectors. @scale: the length
 * factor.
 * Return: nothing.
 *
 * Non-obvious: the vectors are copied as 128-bit TI-mode values (upstream's
 * TI16 typedef, carried). D_L23_001BA5C0 lies inside the split's larger label
 * D_L23_001B2AB0 (+0x7B10); the address-named extern is valued from its name
 * by tools/levels/level_elf.py (RULING #13291 section 2).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own and no Lombyte mention), file
 * src/levels/23_damosel_orbit/L23_0041E418.c, function func_L23_004260F0
 * (v1.01 address), author commit ac1d508f (bigmak94). Adopted with the
 * authors' permission (RULINGs #10405, #12108, #13082, #13291). VERBATIM but
 * for names; the statements are upstream's. Edits:
 *   - names respelled from upstream's v1.01-address names to the targets of
 *     the v2.00 disc's own relocations (relverify on the upstream object, task
 *     #2568): func_L23_004260F0 -> func_L23_004275F0; D_L23_001BA640 ->
 *     D_L23_001BA5C0; func_00282CC0 -> func_L23_002F8EC8; func_00282CF0 ->
 *     func_L23_002F8EF8; func_00282D30 -> func_L23_002F8F38; func_00282DD8 ->
 *     func_L23_002F8FF8; func_00282F20 -> func_L23_002F91A8; func_00283198 ->
 *     func_L23_002F94D0; func_00283C28 -> func_L23_002F9FA0; func_002A7820 ->
 *     func_L23_00325658
 *   - `#include "include_asm.h"` and `#include "common.h"` dropped; the
 *     typedefs it uses written in the TU (the file's own where it has them,
 *     else common.h's); only the file-level declarations it uses kept, without
 *     their comments
 *   - upstream's duplicate declarations of func_00282DD8 and func_00282F20
 *     written once
 *   - upstream's one-line comment folded into this one
 */
void func_L23_004275F0(u8 *obj, float *dst, TI16 *src1, TI16 *src2, float scale) {
    TI16 loc[6];
    TI16 va;
    TI16 vb;
    u8 *o68;
    s16 x;
    s32 t;
    float f;
    float h;

    va = *src2;
    vb = *src1;
    loc[1] = va;
    loc[0] = vb;
    o68 = *(u8 **)(obj + 0x68);
    func_L23_002F8EF8((float *)&loc[2], (float *)&loc[0], (float *)&loc[1]);
    func_L23_002F8EF8((float *)&loc[3], (float *)&loc[0], D_L23_001BA5C0);
    func_L23_002F8FF8((float *)&loc[4], (float *)&loc[3], (float *)&loc[2]);
    func_L23_002F91A8((float *)&loc[4], (float *)&loc[4], 1.0f);
    x = *(s16 *)(o68 + 0xC);
    t = func_L23_00325658(-x, x);
    t = (((s32)*(u16 *)(obj + 0x96) << *(s16 *)(o68 + 0xE)) + t) % 360 - 180;
    f = func_L23_002F9FA0((float)t * 0.017453294f);
    h = func_L23_002F94D0(f) * scale;
    func_L23_002F8F38((float *)&loc[5], (float *)&loc[4], h);
    func_L23_002F8EC8(dst, (float *)&loc[0], (float *)&loc[5]);
}
