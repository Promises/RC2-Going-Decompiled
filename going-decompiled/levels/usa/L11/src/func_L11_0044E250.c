/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2556,
 * RULING #13291 row L11-D). Compiled ALONE as its own TU and linked at its one
 * place, 0x0044E250, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the SN 2.95.3 v1.36 cc1 with -fopt-stack
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;
extern void func_L11_0032DBF8();

/**
 * func_L11_0044E250 - stash the class bytes and switch group.
 *
 * Copies the object's type byte (+0x20) and counter byte (+0x95) into the
 * state's +0x4CC and +0x4CE halfwords, then switches it to group 6, state 4.
 *
 * @a0: the object.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own
 * and no Lombyte mention), file
 * src/levels/11_joba/L11_004465F8.c, function func_L11_0044C818 (v1.01
 * address), author commit 431ac503 (llesieur99). Adopted with
 * the authors’ permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device):
 *   - func_L11_0044C818 -> func_L11_0044E250 (our split name at the v2.00
 *     place);
 *   - func_32C9E0 -> func_L11_0032DBF8;
 *   - `#include "common.h"` and `#include "include_asm.h"` dropped: the
 *     u8/u16/u32/s16/s32/f32 typedefs common.h supplied are written here;
 *   - the opening brace moved to the signature line (whitespace only).
 *
 */
void func_L11_0044E250(char *a0) {
    char *p = *(char **)(a0 + 0x68);

    *(u16 *)(p + 0x4CC) = *(u8 *)(a0 + 0x20);
    *(u16 *)(p + 0x4CE) = *(u8 *)(a0 + 0x95);
    func_L11_0032DBF8(a0, 6, 4);
}
