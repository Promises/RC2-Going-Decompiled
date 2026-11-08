/* Level 17 (Smolg), USA v2.00: a level-only function built from C (task
 * #2667, RULING #13291 row L17-A). Compiled ALONE as its own TU and linked at
 * its one place, 0x003C7080, in place of its split asm; the row is
 * going-decompiled/levels/usa/L17/config/L17_c.tsv, the arm SN 2.95.3 v1.36
 * cc1 (tools/ee/ee_cc1.sh s136, FACT #8810) at -O2 -G0 -fopt-stack, plain as.
 */

extern void func_L17_0031BFC8();
extern void func_L17_003C8738();

/**
 * func_L17_003C7080 - start an object of class 0x920, or re-run the 0x929
 * handler while the object is unmarked.
 *
 * @a0: the object (class halfword +0xAA, mark byte +0x20).
 * Return: nothing.
 *
 * Class 0x920 with mark 0: sets the mark to 1 and calls
 * func_L17_0031BFC8(a0, 1, 0). Class 0x929 with mark 0: calls
 * func_L17_003C8738(a0).
 *
 * Source: OpenRAC rac2-decomp <https://github.com/OpenRAC/rac2-decomp> at
 * fe6adf9baddcd31323c35adf102d078aa250852d, GPL-3.0 (root LICENSE; the file carries no
 * notice of its own; `grep -ic lombyte` on it = 0, so not Lombyte-origin),
 * file src/levels/17_smolg/L17_0032C070.c, function func_L17_003C5AF8 (v1.01 address), author commit 91e04732 (llesieur99).
 * Adopted with the authors' permission (RULINGs #10405, #12108, #13082, #13291).
 * VERBATIM but for names (no device); edits:
 *   - name func_L17_003C5AF8 -> func_L17_003C7080 (our split name at the v2.00 place);
 *   - func_002A7398 -> func_L17_0031BFC8 (the v2.00 relocation target, our name there);
 *   - func_L17_003C71B0 -> func_L17_003C8738 (the v2.00 relocation target, our name there);
 *   - `#include` lines dropped; only the declarations this function uses are
 *   kept, in upstream's order and text (the common.h typedefs it uses written
 *   out as common.h defines them);
 *   - upstream's comment above the function folded into this one, in our words.
 */
void func_L17_003C7080(int a0)
{
    if (*(short *)(a0 + 0xAA) == 0x920) {
        if (*(unsigned char *)(a0 + 0x20) == 0) {
            *(unsigned char *)(a0 + 0x20) = 1;
            func_L17_0031BFC8(a0, 1, 0);
            return;
        }
    }
    if (*(short *)(a0 + 0xAA) == 0x929) {
        if (*(unsigned char *)(a0 + 0x20) == 0)
            func_L17_003C8738(a0);
    }
}
