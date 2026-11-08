/* Level 17 (Smolg), USA v2.00: a level-only function built from C (task
 * #2667, RULING #13291 row L17-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003E5DF0, in place of its split asm; the row is
 * going-decompiled/levels/usa/L17/config/L17_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

typedef unsigned short u16;
typedef signed int s32;
typedef float f32;

extern void func_L17_002EF8D8(s32);
extern void func_L17_002EF908(s32);
extern void func_L17_00321E48(s32, s32);
extern void func_L17_00329158(s32, s32, s32, s32, f32);

/**
 * func_L17_003E5DF0 - per-frame update of an object's sub-systems.
 *
 * @a0: the object; its state block is at +0x68.
 * Return: nothing.
 *
 * Calls func_L17_00321E48(a0, state + 0x50), func_L17_002EF8D8(state + 0x3FC)
 * and func_L17_002EF908(state + 0x402), counts the halfword +0x96 up,
 * stopping at 0xFFFF, then calls func_L17_00329158(a0, state + 0x350, 1, 1,
 * 2.5f).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/17_smolg/L17_003C7C70.c, function func_L17_003E47E8 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L17_003E47E8 -> func_L17_003E5DF0 (our split name at the v2.00 place);
 *   - func_2EEAF8 -> func_L17_002EF8D8 (the v2.00 relocation target, our name there);
 *   - func_2EEB28 -> func_L17_002EF908 (the v2.00 relocation target, our name there);
 *   - func_320B58 -> func_L17_00321E48 (the v2.00 relocation target, our name there);
 *   - func_327E68 -> func_L17_00329158 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them).
 */
void func_L17_003E5DF0(s32 a0) {
    s32 s1 = *(s32 *)(a0 + 0x68);

    func_L17_00321E48(a0, s1 + 0x50);
    func_L17_002EF8D8(s1 + 0x3FC);
    func_L17_002EF908(s1 + 0x402);
    if (*(u16 *)(a0 + 0x96) < 0xFFFF) {
        *(u16 *)(a0 + 0x96) = *(u16 *)(a0 + 0x96) + 1;
    }
    func_L17_00329158(a0, s1 + 0x350, 1, 1, 2.5f);
}
