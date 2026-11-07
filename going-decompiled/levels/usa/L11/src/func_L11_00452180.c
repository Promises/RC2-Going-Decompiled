/* Level 11 (Joba), USA v2.00: a level-only function built from C (task #2556,
 * RULING #13291 row L11-D). Compiled ALONE as its own TU and linked at its one
 * place, 0x00452180, in place of its split asm; the row is
 * going-decompiled/levels/usa/L11/config/L11_c.tsv, the arm the SN 2.95.3 v1.36 cc1 with -fopt-stack
 * (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed short s16;
typedef signed int s32;
typedef float f32;
extern void func_L11_00301280(char *dst, char *src, float scale);

/**
 * func_L11_00452180 - scale four consecutive 16-byte XYZ vectors in place.
 *
 * Each of the four vectors at @p, @p+16, @p+32 and @p+48 is scaled by @scale
 * through the shared vector helper func_L11_00301280.
 *
 * @p: the four vectors.
 * @scale: the scalar.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file
 * carries no notice of its own
 * and no Lombyte mention), file
 * src/levels/11_joba/L11_004465F8.c, function func_L11_00450748 (v1.01
 * address), author commit 7fccf735 (Flavius; upstream's root
 * commit, which squashed the earlier RAC2 history, so the text's first
 * author is not recoverable from upstream's log). Adopted with
 * the authors’ permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device):
 *   - func_L11_00450748 -> func_L11_00452180 (our split name at the v2.00
 *     place);
 *   - func_00282D30_2 -> func_L11_00301280;
 *   - `#include "common.h"` and `#include "include_asm.h"` dropped: the
 *     u8/u16/u32/s16/s32/f32 typedefs common.h supplied are written here;
 *   - the opening brace moved to the signature line (whitespace only);
 *   - upstream’s comment above the function is folded into this one.
 *
 */
void func_L11_00452180(char *p, float scale) {
    func_L11_00301280(p, p, scale);
    func_L11_00301280(p + 16, p + 16, scale);
    func_L11_00301280(p + 32, p + 32, scale);
    func_L11_00301280(p + 48, p + 48, scale);
}
