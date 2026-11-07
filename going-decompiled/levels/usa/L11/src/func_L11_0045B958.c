/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2556,
 * RULING #13291 row L11-D). Compiled ALONE as its own TU and linked at its one
 * place, 0x0045B958, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the boot’s ee-gcc 2.9-ee-991111 cc1
 * (tools/ee/ee_cc1.sh sdk29) at -O2 -G0.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

/**
 * func_L11_0045B958 - reset a 144-byte quad/transform block.
 *
 * Writes the four 16-byte corner slots of a square with extents -2..+2 in x
 * and y (each an int 0 followed by two floats and a third float of 1.0), then
 * the secondary header: the words at +80/+84/+92/+96 zeroed, the floats at
 * +88/+100/+104/+108 set to 1.0, the 64-bit word at +112 cleared and the
 * packed 0x0000FF9000000260 stored at +128.
 *
 * @p: the 144-byte block.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own
 * and no Lombyte mention), file
 * src/levels/11_joba/L11_00453358.c, function func_L11_00459EA0 (v1.01
 * address), author commit 7fccf735 (Flavius; upstream's root
 * commit, which squashed the earlier RAC2 history, so the text's first
 * author is not recoverable from upstream's log). Adopted with
 * the authors’ permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device):
 *   - func_L11_00459EA0 -> func_L11_0045B958 (our split name at the v2.00
 *     place);
 *   - `#include "common.h"` (none other) dropped: the u8/u16/u32/s16/s32/f32
 *     typedefs common.h supplied are written here;
 *   - the opening brace moved to the signature line (whitespace only);
 *   - the body’s `long long` stores are kept verbatim: this is the sdk29 arm,
 *     where ee-gcc 2.9 compiles them as-is (the s136 arm’s `long long`
 *     respelling of RULING #13197 does not arise).
 *
 */
void func_L11_0045B958(char *p) {
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -2.0f;
    *(float *)(p + 24) = 2.0f;
    *(float *)(p + 40) = -2.0f;
    *(float *)(p + 56) = 2.0f;
    *(float *)(p + 4) = -2.0f;
    *(float *)(p + 36) = 2.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 0x0000FF9000000260LL;
    *(float *)(p + 20) = -2.0f;
    *(float *)(p + 52) = 2.0f;
    *(float *)(p + 60) = 1.0f;
}
