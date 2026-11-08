/* Level 17 (Smolg), USA v2.00: a level-only function built from C (task
 * #2667, RULING #13291 row L17-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003E8560, in place of its split asm; the row is
 * going-decompiled/levels/usa/L17/config/L17_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef signed int s32;
typedef float f32;

extern s32 D_L17_001FE880[];
extern s32 func_L17_0031C450(s32);
extern f32 func_L17_002EFE90(s32, s32);

/**
 * func_L17_003E8560 - pick a point of the object's path record at least
 * 3.0 away from it.
 *
 * @a0: the object (position +0x10); its state block is at +0x68.
 * Return: the chosen point's index.
 *
 * The record is D_L17_001FE880[state word +0x684]; its first word is the
 * point count and the 16-byte points follow the 16-byte header. Up to five
 * times it draws an index with func_L17_0031C450(count) and returns it when
 * func_L17_002EFE90 of the object's position and that point exceeds 3.0;
 * after five misses it returns the last index drawn.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/17_smolg/L17_003C7C70.c, function func_L17_003E6F58 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L17_003E6F58 -> func_L17_003E8560 (our split name at the v2.00 place);
 *   - func_2EF0B0 -> func_L17_002EFE90 (the v2.00 relocation target, our name there);
 *   - func_31B160 -> func_L17_0031C450 (the v2.00 relocation target, our name there);
 *   - D_L17_001FE880 kept: upstream's name already states the v2.00 address
 *   (0x001FE880, inside D_L17_001F0000 (+0xE880)); no name exists at that exact address
 *   (same grep); address-named (RULING #13291 section 2);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
s32 func_L17_003E8560(s32 a0) {
    s32 s0;
    s32 s1 = D_L17_001FE880[*(s32 *)(*(s32 *)(a0 + 0x68) + 0x684)];
    s32 s4 = *(s32 *)s1;
    s32 s2 = 0;

    do {
        s32 off;

        s0 = func_L17_0031C450(s4);
        off = (s0 << 4) + 0x10;
        if (func_L17_002EFE90(a0 + 0x10, s1 + off) > 3.0f) {
            return s0;
        }
    } while (++s2 < 5);
    return s0;
}
