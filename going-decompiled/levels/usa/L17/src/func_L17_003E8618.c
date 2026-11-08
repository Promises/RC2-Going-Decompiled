/* Level 17 (Smolg), USA v2.00: a level-only function built from C (task
 * #2667, RULING #13291 row L17-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003E8618, in place of its split asm; the row is
 * going-decompiled/levels/usa/L17/config/L17_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef signed int s32;

extern s32 D_L17_001C0018;
extern s32 func_L17_002DF9B0(s32, s32, s32, s32, s32);

/**
 * func_L17_003E8618 - is the object's target lost or current?
 *
 * @a0: the object (position +0x10); its state block is at +0x68.
 * Return: 1 when func_L17_002DF9B0(position, state + 0x240, 0, a0, 0)
 * returns 0; otherwise whether D_L17_001C0018 equals the state's word +0x250.
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/17_smolg/L17_003C7C70.c, function func_L17_003E7010 (v1.01 address), author commit 8d0b9987 (lvl6-12).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L17_003E7010 -> func_L17_003E8618 (our split name at the v2.00 place);
 *   - func_2DF570 -> func_L17_002DF9B0 (the v2.00 relocation target, our name there);
 *   - D_L17_001C0018 kept: upstream's name already states the v2.00 address
 *   (0x001C0018, inside D_L17_001B2FB0 (+0xD068)); no name exists at that exact address
 *   (same grep); address-named (RULING #13291 section 2);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
s32 func_L17_003E8618(s32 a0) {
    s32 s0 = *(s32 *)(a0 + 0x68);

    if (func_L17_002DF9B0(a0 + 0x10, s0 + 0x240, 0, a0, 0) == 0)
        return 1;
    return D_L17_001C0018 == *(s32 *)(s0 + 0x250);
}
